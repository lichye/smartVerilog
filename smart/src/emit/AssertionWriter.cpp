#include "AssertionWriter.h"

#include "../Version.h"

#include <atomic>
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <regex>
#include <sstream>
#include <thread>

#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

namespace smart {
namespace emit {
namespace {

namespace fs = std::filesystem;

std::string quote(const std::string& text) { return "'" + text + "'"; }

std::string readFile(const std::string& path) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("cannot read " + path);
    std::ostringstream text;
    text << in.rdbuf();
    return text.str();
}

// Run a shell command with a deadline. Returns the exit code, or -1 if it was
// killed for running too long.
int runWithTimeout(const std::string& command, int timeoutSeconds) {
    const pid_t pid = fork();
    if (pid < 0) return -2;

    if (pid == 0) {
        setpgid(0, 0);
        execl("/bin/sh", "sh", "-c", command.c_str(), nullptr);
        _exit(127);
    }
    setpgid(pid, pid);

    const auto deadline =
        std::chrono::steady_clock::now() + std::chrono::seconds(timeoutSeconds);
    int status = 0;
    for (;;) {
        const pid_t waited = waitpid(pid, &status, WNOHANG);
        if (waited == pid) return WIFEXITED(status) ? WEXITSTATUS(status) : -2;
        if (waited < 0) return -2;
        if (std::chrono::steady_clock::now() >= deadline) {
            kill(-pid, SIGKILL);
            waitpid(pid, &status, 0);
            return -1;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}

}  // namespace

std::string ebmcCommand(const std::string& designFile,
                        const CheckOptions& options) {
    std::ostringstream command;
    command << options.ebmc << " " << quote(designFile);
    // The spaces around -D FORMAL are load-bearing: without them the define
    // and the next argument run together (fixed once in VerilogChecker.cpp).
    command << " -D FORMAL";
    for (const auto& file : options.extraFiles) command << " " << quote(file);

    if (options.unbounded)
        command << " --k-induction";
    else
        command << " --bound " << options.bound;

    if (!options.topModule.empty()) command << " --top " << options.topModule;
    return command.str();
}

std::string injectAssertions(const std::string& source, const std::string& top,
                             const std::vector<std::string>& assertions) {
    if (assertions.empty()) return source;

    // Find `module <top>` and then the first `endmodule` after it. Textual on
    // purpose: the user's file is re-emitted as written, so it has to be
    // edited as written (plan WP2).
    const std::regex moduleStart("\\bmodule\\s+" + top + "\\b");
    std::smatch match;
    if (!std::regex_search(source, match, moduleStart))
        throw std::runtime_error("module '" + top + "' not found in the design");

    const auto searchFrom = static_cast<std::size_t>(match.position(0));
    const auto endmodule = source.find("endmodule", searchFrom);
    if (endmodule == std::string::npos)
        throw std::runtime_error("no endmodule for module '" + top + "'");

    std::ostringstream injected;
    injected << "\n";
    for (const auto& assertion : assertions)
        injected << "    assert property (" << assertion << ");\n";

    return source.substr(0, endmodule) + injected.str() + source.substr(endmodule);
}

std::vector<CheckResult> checkAssertions(const std::string& designFile,
                                         const std::vector<std::string>& assertions,
                                         const CheckOptions& options) {
    std::vector<CheckResult> results(assertions.size());
    if (assertions.empty()) return results;

    fs::create_directories(options.scratchDir);
    const auto source = readFile(designFile);
    const auto stem = fs::path(designFile).stem().string();
    const auto extension = fs::path(designFile).extension().string();

    std::atomic<std::size_t> next{0};
    const int workers =
        std::max(1, std::min<int>(options.jobs, static_cast<int>(assertions.size())));

    std::vector<std::thread> pool;
    pool.reserve(workers);
    for (int worker = 0; worker < workers; ++worker) {
        pool.emplace_back([&] {
            for (;;) {
                const std::size_t index = next.fetch_add(1);
                if (index >= assertions.size()) return;

                CheckResult& result = results[index];
                result.assertion = assertions[index];

                const auto start = std::chrono::steady_clock::now();
                const auto candidate = options.scratchDir + "/" + stem + "_check" +
                                       std::to_string(index) + extension;
                try {
                    std::ofstream out(candidate);
                    out << injectAssertions(source,
                                            options.injectModule.empty()
                                                ? options.topModule
                                                : options.injectModule,
                                            {assertions[index]});
                    out.close();

                    const auto command =
                        ebmcCommand(candidate, options) + " > /dev/null 2>&1";
                    const int status = runWithTimeout(command, options.timeoutSeconds);

                    if (status == 0)
                        result.status = CheckStatus::Verified;
                    else if (status == -1)
                        result.status = CheckStatus::TimedOut;
                    else if (status > 0)
                        result.status = CheckStatus::Refuted;
                    else
                        result.status = CheckStatus::Error;
                } catch (const std::exception&) {
                    result.status = CheckStatus::Error;
                }

                result.seconds = std::chrono::duration<double>(
                                     std::chrono::steady_clock::now() - start)
                                     .count();

                if (result.status == CheckStatus::Verified) {
                    std::error_code error;
                    fs::remove(candidate, error);
                }
            }
        });
    }
    for (auto& thread : pool) thread.join();

    return results;
}

void writeAssertionFile(const std::string& designFile, const std::string& output,
                        const std::string& top,
                        const std::vector<std::string>& assertions,
                        const std::string& configJson) {
    const auto source = readFile(designFile);

    std::ostringstream header;
    header << "// Generated by SMART " << smart::version() << ": " << assertions.size()
           << " formally verified assertion" << (assertions.size() == 1 ? "" : "s")
           << " added to module " << top << ".\n"
           << "//\n"
           << "// Settings that produced this file:\n";
    std::istringstream configLines(configJson);
    std::string line;
    while (std::getline(configLines, line)) header << "//   " << line << "\n";
    header << "\n";

    std::ofstream out(output);
    if (!out) throw std::runtime_error("cannot write " + output);
    out << header.str() << injectAssertions(source, top, assertions);
}

}  // namespace emit
}  // namespace smart
