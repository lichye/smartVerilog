#include "BlockRunner.h"

#include <algorithm>
#include <atomic>
#include <cctype>
#include <cerrno>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <sstream>
#include <thread>
#include <vector>

#include <fcntl.h>
#include <signal.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <unistd.h>

namespace smart {
namespace pipeline {
namespace {

namespace fs = std::filesystem;

std::string trimmed(std::string text) {
    const auto notSpace = [](unsigned char c) { return !std::isspace(c); };
    text.erase(text.begin(), std::find_if(text.begin(), text.end(), notSpace));
    text.erase(std::find_if(text.rbegin(), text.rend(), notSpace).base(), text.end());
    return text;
}

std::string readFile(const std::string& path) {
    std::ifstream in(path);
    if (!in) return {};
    std::ostringstream text;
    text << in.rdbuf();
    return text.str();
}

// Run one block to completion or to its deadline.
//
// The child gets its own process group so that a timeout kills the whole tree
// — a block that hangs is usually hanging inside cvc5 or ebmc, not in our own
// code, and killing only the block would leave those running.
BlockResult runOne(const BlockJob& job, const BlockRunnerOptions& options,
                   std::chrono::steady_clock::time_point deadline) {
    BlockResult result;
    result.job = job;

    const auto start = std::chrono::steady_clock::now();
    const auto blockDeadline =
        std::min(deadline, start + std::chrono::seconds(options.coreTimeoutSeconds));

    if (start >= deadline) {
        result.status = BlockStatus::TimedOut;
        return result;
    }

    const std::string logPath = options.logsDir + "/core_" + job.coreId + ".log";

    const pid_t pid = fork();
    if (pid < 0) {
        result.status = BlockStatus::Failed;
        return result;
    }

    if (pid == 0) {
        // --- child ---
        setpgid(0, 0);

        // Die with the parent. Without this, killing `smart` (a `timeout` on
        // the whole run, a Ctrl-C, a crash) leaves its blocks running: they
        // are re-parented to init and keep burning CPU with nobody waiting on
        // them. That is not hypothetical — a block was found alive 4h45m after
        // its 60s deadline, orphaned when the run around it was killed at its
        // own 1800s cap.
        prctl(PR_SET_PDEATHSIG, SIGKILL);
        // The parent may already be gone; PDEATHSIG only fires on a future
        // death, so check once here to close the race.
        if (getppid() == 1) _exit(127);

        if (chdir(options.workDir.c_str()) != 0) _exit(127);

        const int logFd =
            open(logPath.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (logFd >= 0) {
            dup2(logFd, STDOUT_FILENO);
            dup2(logFd, STDERR_FILENO);
            if (logFd > STDERR_FILENO) close(logFd);
        }

        // Same argv contract as the standalone smart.out.
        std::vector<std::string> args = {options.executable,
                                         "--block",
                                         options.workDir,
                                         options.topModule,
                                         job.resultFile,
                                         job.variablesFile,
                                         job.coreId,
                                         std::to_string(job.latency),
                                         options.configPath};
        std::vector<char*> argv;
        argv.reserve(args.size() + 1);
        for (auto& arg : args) argv.push_back(arg.data());
        argv.push_back(nullptr);

        execv(options.executable.c_str(), argv.data());
        _exit(127);  // exec failed
    }

    // --- parent ---
    setpgid(pid, pid);  // racy with the child's own call; harmless either way

    int status = 0;
    bool timedOut = false;
    for (;;) {
        const pid_t waited = waitpid(pid, &status, WNOHANG);
        if (waited == pid) break;
        if (waited < 0 && errno != EINTR) {
            result.status = BlockStatus::Failed;
            return result;
        }
        if (std::chrono::steady_clock::now() >= blockDeadline) {
            kill(-pid, SIGKILL);
            waitpid(pid, &status, 0);
            timedOut = true;

            // Did the group actually die? Checking immediately cries wolf:
            // the block's own children (cvc5, ebmc) are re-parented to init
            // when it dies and linger as zombies until init reaps them, and a
            // zombie still answers kill(-pgid, 0). Give them a moment, then
            // ask — a group still present after that is a genuine escape, and
            // an escaped block silently burns a core for as long as the run
            // lasts.
            for (int attempt = 0; attempt < 20; ++attempt) {
                if (kill(-pid, 0) != 0) break;
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                result.survivedKill = (attempt == 19);
            }
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    result.seconds = std::chrono::duration<double>(
                         std::chrono::steady_clock::now() - start)
                         .count();
    result.exitCode = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    result.pid = static_cast<int>(pid);
    result.killed = timedOut;

    if (timedOut) {
        result.status = BlockStatus::TimedOut;
    } else if (result.exitCode == 0) {
        // Exit 0 means the block verified something; the assertion is in its
        // result file. An empty file is still "nothing found".
        // resultFile is absolute; the child wrote it from inside workDir.
        const auto assertion = trimmed(readFile(job.resultFile));
        if (assertion.empty()) {
            result.status = BlockStatus::NoAssertion;
        } else {
            result.status = BlockStatus::Verified;
            result.assertion = assertion;
        }
    } else {
        result.status = BlockStatus::NoAssertion;
    }

    if (result.status != BlockStatus::Verified) {
        std::error_code error;
        fs::remove(job.resultFile, error);
    }
    return result;
}

}  // namespace

std::vector<BlockResult> runBlocks(
    const std::vector<BlockJob>& jobs, const BlockRunnerOptions& options,
    std::chrono::steady_clock::time_point deadline,
    const std::function<void(const BlockResult&, std::size_t, std::size_t)>&
        progress) {
    std::vector<BlockResult> results(jobs.size());

    std::atomic<std::size_t> next{0};
    std::atomic<std::size_t> done{0};
    std::mutex progressMutex;

    const int workers =
        std::max(1, std::min<int>(options.jobs, static_cast<int>(jobs.size())));

    std::vector<std::thread> pool;
    pool.reserve(workers);
    for (int worker = 0; worker < workers; ++worker) {
        pool.emplace_back([&] {
            for (;;) {
                const std::size_t index = next.fetch_add(1);
                if (index >= jobs.size()) return;
                results[index] = runOne(jobs[index], options, deadline);
                const std::size_t completed = done.fetch_add(1) + 1;
                if (progress) {
                    std::lock_guard<std::mutex> lock(progressMutex);
                    progress(results[index], completed, jobs.size());
                }
            }
        });
    }
    for (auto& thread : pool) thread.join();

    return results;
}

}  // namespace pipeline
}  // namespace smart
