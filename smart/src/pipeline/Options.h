// Options: the single source of truth for every knob SMART has (plan §1.1).
//
// ONE table (`optionTable()`) defines each option's flat config key, its CLI
// flag, its type, its default, and where it lived in the legacy nested
// `Config/*.json` schema. Both parsers are driven from that table, so a new
// option cannot be reachable from the command line but not from a config file
// (or vice versa), which is the failure mode §1.1 exists to prevent.
//
// Precedence: CLI flag > --config file > built-in default.

#ifndef SMART_PIPELINE_OPTIONS_H
#define SMART_PIPELINE_OPTIONS_H

#include <map>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace smart {
namespace pipeline {

enum class OptionType { Bool, Int, Double, String, StringList };

using OptionValue =
    std::variant<bool, long long, double, std::string, std::vector<std::string>>;

struct OptionSpec {
    std::string key;          // flat config key, e.g. "core_timeout"
    std::string flag;         // long CLI flag without "--"; empty = config-only
    char shortFlag = 0;       // e.g. 'j'; 0 = none
    OptionType type = OptionType::String;
    OptionValue defaultValue;
    std::string legacyPath;   // "Workflow.Core_timeout"; empty = no legacy key
    std::string metavar;      // "N", "FILE", ...; empty for flags
    std::string help;
};

const std::vector<OptionSpec>& optionTable();
const OptionSpec* findByKey(const std::string& key);

// Resolved configuration: every key in the table always has a value.
class Options {
public:
    Options();  // built-in defaults

    // Merge a JSON file. Accepts both the new flat schema and the legacy
    // nested one (auto-detected); legacy misspellings `Threadhold` and
    // `Nagative_state_number` are understood. Warnings about ignored or
    // unusable keys are appended to `warnings()`.
    void mergeJsonFile(const std::string& path);

    // Parse a command line. Recognises every flag in the table plus the
    // handful of actions that are not configuration (--help, --check-env,
    // --dump-config) and the positional design files.
    // Throws std::runtime_error on a malformed command line.
    void parseCommandLine(int argc, char** argv);

    // Validate relationships and finite-choice string values after all
    // sources have been merged. Callers do this only after CLI precedence has
    // been applied, so a valid CLI value may override a stale config value.
    void validate() const;

    bool getBool(const std::string& key) const;
    long long getInt(const std::string& key) const;
    double getDouble(const std::string& key) const;
    const std::string& getString(const std::string& key) const;
    std::vector<std::string> getStringList(const std::string& key) const;

    void set(const std::string& key, OptionValue value);
    bool wasSetExplicitly(const std::string& key) const;

    // The fully resolved configuration as new-schema JSON — written to
    // workdir/effective-config.json, printed by --dump-config, and echoed
    // into the emitted assertion file so a result names its own settings.
    std::string toJson() const;

    const std::vector<std::string>& designFiles() const { return designFiles_; }
    const std::vector<std::string>& warnings() const { return warnings_; }

    bool helpRequested() const { return help_; }
    bool checkEnvRequested() const { return checkEnv_; }
    bool versionRequested() const { return version_; }
    bool dumpConfigRequested() const { return dumpConfig_; }
    bool dumpFrontendRequested() const { return dumpFrontend_; }

    static std::string usage();

private:
    void applyFlat(const void* json);    // const nlohmann::json*
    void applyLegacy(const void* json);  // const nlohmann::json*
    void assign(const std::string& key, const std::string& text,
                const std::string& origin);

    std::map<std::string, OptionValue> values_;
    std::map<std::string, bool> explicitlySet_;
    std::vector<std::string> designFiles_;
    std::vector<std::string> warnings_;
    bool help_ = false;
    bool checkEnv_ = false;
    bool version_ = false;
    bool dumpConfig_ = false;
    bool dumpFrontend_ = false;
};

}  // namespace pipeline
}  // namespace smart

#endif  // SMART_PIPELINE_OPTIONS_H
