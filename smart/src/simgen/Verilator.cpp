// The Verilator side of simgen: generate a C++ testbench, build it, run it.
//
// Verilator is the default simulator because it is cycle-based and 2-state.
// Both properties matter here beyond speed: no x/z leaks into the traces to
// disqualify constraints, and a design driven into a zero-delay feedback loop
// settles instead of spinning forever at one timestamp.
//
// The stimulus is the same as the Icarus path's — the semantics are the spec
// (gen_bench.render_sim_py), not the language it is written in.

#include "Harness.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>
#include <system_error>

namespace smart {
namespace simgen {
namespace {

namespace fs = std::filesystem;

int effectiveWidth(int width) { return width > 0 ? width : 1; }

// C++ type Verilator gives a signal of this width.
const char* cppType(int width) {
    const int w = effectiveWidth(width);
    if (w <= 8) return "CData";
    if (w <= 16) return "SData";
    if (w <= 32) return "IData";
    return "QData";
}

// A draw of `width` random bits, as a C++ expression.
std::string randomDraw(int width) {
    const int w = effectiveWidth(width);
    if (w >= 64) return "((static_cast<QData>(::random()) << 32) ^ ::random())";
    if (w >= 32) return "static_cast<IData>(::random())";
    // ::random() yields 31 usable bits; masking below that is exact.
    return "(::random() & ((1u << " + std::to_string(w) + ") - 1u))";
}

// Internal free registers are reached through Verilator's flattened public
// names: --public-flat-rw exposes them on the root as <top>__DOT__<name>.
std::string freeRegRef(const std::string& top, const std::string& name) {
    return "top->rootp->" + top + "__DOT__" + name;
}

// Wrap a drive in "use the scripted value if the stimulus file still has one".
// Falling back to the draw means a short file degrades to random rather than
// silently driving zeros for the rest of the trace.
std::string scripted(const std::string& target, const std::string& randomForm,
                     const std::string& indent) {
    std::ostringstream os;
    os << indent << "{\n"
       << indent << "  bool ok = false;\n"
       << indent << "  const long long v = nextStim(&ok);\n"
       << indent << "  if (ok) " << target << " = v;\n"
       << indent << "  else {\n"
       << randomForm
       << indent << "  }\n"
       << indent << "}\n";
    return os.str();
}

std::string driveLine(const std::string& target, int width,
                      const SignalSpec* spec, const std::string& indent) {
    if (spec != nullptr && !spec->values.empty()) {
        std::ostringstream os;
        os << indent << "{\n"
           << indent << "  static const long long choices[] = {";
        for (std::size_t i = 0; i < spec->values.size(); ++i)
            os << (i ? ", " : "") << spec->values[i];
        os << "};\n"
           << indent << "  " << target << " = choices[::random() % "
           << spec->values.size() << "];\n"
           << indent << "}\n";
        return os.str();
    }
    if (spec != nullptr && spec->constant.has_value())
        return indent + target + " = " + std::to_string(*spec->constant) + ";\n";
    if (spec != nullptr && spec->maxValue.has_value())
        return indent + target + " = ::random() % " +
               std::to_string(*spec->maxValue + 1) + ";\n";
    return indent + target + " = " + randomDraw(width) + ";\n";
}

const SignalSpec* specFor(const HarnessOptions& options, const std::string& name) {
    auto it = options.inputs.find(name);
    return it == options.inputs.end() ? nullptr : &it->second;
}

}  // namespace


// --- state signal enumeration ------------------------------------------------
//
// `verilator --xml-only` elaborates and dumps the AST without compiling, which
// is the cheapest way to learn what signals a design has. We want the top
// module's outputs and internals: the driven inputs are excluded on purpose,
// because a state vector containing them is dominated by the stimulus itself
// and tells a search nothing about where the design went.
namespace {

std::string quote(const std::string& s) { return "'" + s + "'"; }

// The generated XML is regular, so a scan beats pulling in an XML library.
std::string attribute(const std::string& tag, const std::string& name) {
    const auto key = name + "=\"";
    const auto at = tag.find(key);
    if (at == std::string::npos) return {};
    const auto from = at + key.size();
    const auto to = tag.find('"', from);
    if (to == std::string::npos) return {};
    return tag.substr(from, to - from);
}

}  // namespace

namespace {

// Run `verilator --xml-only` and return the XML, or empty on failure.
std::string elaborateXml(const std::vector<std::string>& designFiles,
                         const std::string& top, const std::string& verilator,
                         const std::string& scratchDir) {
    if (designFiles.empty()) return {};
    std::error_code error;
    fs::create_directories(scratchDir, error);

    // Two callers want this — the instance list and the state signals — and
    // elaborating twice is pure waste. The XML is written into the workdir, so
    // reuse it when it is already there.
    const auto cached = scratchDir + "/xml/V" + top + ".xml";
    if (fs::exists(cached, error)) {
        std::ifstream in(cached);
        if (in)
            return std::string((std::istreambuf_iterator<char>(in)),
                               std::istreambuf_iterator<char>());
    }

    std::ostringstream command;
    command << quote(verilator) << " --xml-only -Wno-fatal --top-module "
            << quote(top) << " --Mdir " << quote(scratchDir + "/xml");
    for (const auto& file : designFiles) command << " " << quote(file);
    command << " > /dev/null 2>&1";
    if (std::system(command.str().c_str()) != 0) return {};

    std::ifstream in(scratchDir + "/xml/V" + top + ".xml");
    if (!in) return {};
    return std::string((std::istreambuf_iterator<char>(in)),
                       std::istreambuf_iterator<char>());
}

}  // namespace

std::vector<Instance> enumerateInstances(
    const std::vector<std::string>& designFiles, const std::string& top,
    const std::string& verilator, const std::string& scratchDir) {
    std::vector<Instance> instances;
    const auto xml = elaborateXml(designFiles, top, verilator, scratchDir);
    if (xml.empty()) return instances;

    for (std::size_t at = xml.find("<cell "); at != std::string::npos;
         at = xml.find("<cell ", at + 1)) {
        const auto end = xml.find('>', at);
        if (end == std::string::npos) break;
        const auto tag = xml.substr(at, end - at);

        Instance instance;
        instance.hierarchy = attribute(tag, "hier");
        instance.module = attribute(tag, "submodname");
        if (instance.hierarchy.empty() || instance.module.empty()) continue;
        // The top names itself; it is not an instance of anything.
        if (instance.hierarchy == top) continue;
        const auto mangled = instance.module.find("__");
        if (mangled != std::string::npos)
            instance.module = instance.module.substr(0, mangled);
        instances.push_back(instance);
    }
    return instances;
}

std::string definingModule(const std::vector<std::string>& designFiles,
                           const std::string& top, const std::string& instance,
                           const std::string& verilator,
                           const std::string& scratchDir) {
    const auto xml = elaborateXml(designFiles, top, verilator, scratchDir);
    if (xml.empty()) return {};

    for (std::size_t at = xml.find("<cell "); at != std::string::npos;
         at = xml.find("<cell ", at + 1)) {
        const auto end = xml.find('>', at);
        if (end == std::string::npos) break;
        const auto tag = xml.substr(at, end - at);
        if (attribute(tag, "name") != instance) continue;
        auto defined = attribute(tag, "submodname");
        // The top cell names itself; that is not an instance of something
        // else, so treat it as "already a module name".
        if (defined == instance) return {};
        // Verilator specialises a parameterised module per parameter set and
        // names the result `axis_fifo__D20_DBc_K0_L0_U0`. No such module is
        // written anywhere, so the property has to go into the source name.
        // That constrains every specialisation, which is the honest reading:
        // an invariant that only holds for one parameterisation gets refuted.
        const auto mangled = defined.find("__");
        if (mangled != std::string::npos) defined = defined.substr(0, mangled);
        return defined;
    }
    return {};
}

std::vector<StateSignal> enumerateStateSignals(
    const std::vector<std::string>& designFiles, const std::string& top,
    const std::string& verilator, const std::string& scratchDir) {
    std::vector<StateSignal> signals;
    if (designFiles.empty()) return signals;

    std::error_code error;
    fs::create_directories(scratchDir, error);

    std::ostringstream command;
    command << quote(verilator) << " --xml-only -Wno-fatal --top-module "
            << quote(top) << " --Mdir " << quote(scratchDir + "/xml");
    for (const auto& file : designFiles) command << " " << quote(file);
    command << " > /dev/null 2>&1";
    if (std::system(command.str().c_str()) != 0) return signals;

    std::ifstream in(scratchDir + "/xml/V" + top + ".xml");
    if (!in) return signals;
    std::string xml((std::istreambuf_iterator<char>(in)),
                    std::istreambuf_iterator<char>());

    // dtype id -> width. A basicdtype with no left/right is one bit.
    std::map<std::string, int> widths;
    for (std::size_t at = xml.find("<basicdtype"); at != std::string::npos;
         at = xml.find("<basicdtype", at + 1)) {
        const auto end = xml.find('>', at);
        if (end == std::string::npos) break;
        const auto tag = xml.substr(at, end - at);
        const auto id = attribute(tag, "id");
        if (id.empty()) continue;
        const auto left = attribute(tag, "left"), right = attribute(tag, "right");
        int width = 1;
        if (!left.empty() && !right.empty()) {
            try {
                width = std::stoi(left) - std::stoi(right) + 1;
            } catch (const std::exception&) {
                width = 1;
            }
        }
        widths[id] = width;
    }

    // Only the module marked topModule="1"; a submodule's signals are not
    // reachable through the flat accessors we generate.
    const auto topAt = xml.find("topModule=\"1\"");
    if (topAt == std::string::npos) return signals;
    const auto scopeEnd = xml.find("</module>", topAt);

    for (std::size_t at = xml.find("<var ", topAt);
         at != std::string::npos && at < scopeEnd;
         at = xml.find("<var ", at + 1)) {
        const auto end = xml.find('>', at);
        if (end == std::string::npos) break;
        const auto tag = xml.substr(at, end - at);
        const auto name = attribute(tag, "name");
        const auto dir = attribute(tag, "dir");
        if (name.empty() || dir == "input") continue;
        // Verilator's own temporaries (__VdfgTmp_*, __Vtask*, ...) appear in
        // the AST but are not emitted as members of the root class, so naming
        // one produces code that does not compile. They are not design state
        // in any case.
        if (name.rfind("__", 0) == 0) continue;

        StateSignal signal;
        signal.name = name;
        signal.isPort = !dir.empty();
        const auto found = widths.find(attribute(tag, "dtype_id"));
        signal.width = found == widths.end() ? 1 : found->second;
        // Wider than a QData has no plain integral accessor; hashing it would
        // not compile. Rare for control state, and skipping is honest.
        if (signal.width > 64) continue;
        signals.push_back(signal);
    }
    return signals;
}

DrivenWidths drivenWidths(const frontend::ModuleInfo& info,
                          const HarnessOptions& options) {
    DrivenWidths widths;
    const auto clock = options.clock.empty()
                           ? frontend::guessClock(info)
                           : std::optional<std::string>(options.clock);
    const auto reset = clock ? (options.reset ? options.reset
                                              : frontend::guessReset(info, clock))
                             : std::optional<frontend::ResetInfo>();

    for (const auto& reg : info.freeRegs)
        if (reg.kind == "anyconst") widths.constant.push_back(effectiveWidth(reg.width));
    for (const auto& reg : options.hierFreeRegs)
        if (reg.kind == "anyconst") widths.constant.push_back(effectiveWidth(reg.width));

    for (const auto& port : info.inputs()) {
        if (clock && port.name == *clock) continue;
        if (reset && port.name == reset->signal) continue;
        widths.perCycle.push_back(effectiveWidth(port.width));
    }
    for (const auto& reg : info.freeRegs)
        if (reg.kind == "anyseq") widths.perCycle.push_back(effectiveWidth(reg.width));
    for (const auto& reg : options.hierFreeRegs)
        if (reg.kind == "anyseq") widths.perCycle.push_back(effectiveWidth(reg.width));

    return widths;
}

std::string renderVerilatorHarness(const frontend::ModuleInfo& info,
                                   const HarnessOptions& options) {
    const auto clock = options.clock.empty()
                           ? frontend::guessClock(info)
                           : std::optional<std::string>(options.clock);
    const auto reset = clock ? (options.reset ? options.reset
                                              : frontend::guessReset(info, clock))
                             : std::optional<frontend::ResetInfo>();

    const std::string model = "V" + info.top;
    // The root header is what makes <top>__DOT__<name> a complete type. Free
    // registers need it, and so does any internal signal in the state vector.
    const bool needsRoot =
        !info.freeRegs.empty() || !options.hierFreeRegs.empty() ||
        std::any_of(options.stateSignals.begin(), options.stateSignals.end(),
                    [](const StateSignal& s) { return !s.isPort; });
    const bool hasFreeRegs = needsRoot;

    std::ostringstream os;
    os << "// Auto-generated by SMART (simgen/Verilator.cpp) — do not edit.\n"
       << "// Design: " << info.top
       << "  clock: " << (clock ? *clock : "none (combinational)")
       << "  reset: " << (reset ? reset->signal : "none") << "\n"
       << "#include <verilated.h>\n"
       << "#include <verilated_vcd_c.h>\n"
       << "#include \"" << model << ".h\"\n";
    if (hasFreeRegs) os << "#include \"" << model << "___024root.h\"\n";
    os << "#include <cstdlib>\n"
       << "#include <fstream>\n"
       << "#include <set>\n"
       << "#include <sstream>\n"
       << "#include <string>\n"
       << "#include <vector>\n\n"
       << "int main(int argc, char** argv) {\n"
       << "    unsigned seed = " << options.seed << ";\n"
       << "    std::string vcdPath = \"dump.vcd\";\n"
       << "    std::string stimPath, statesPath;\n"
       << "    bool dump = true;\n"
       << "    for (int i = 1; i < argc; ++i) {\n"
       << "        const std::string arg = argv[i];\n"
       << "        if (arg.rfind(\"+seed=\", 0) == 0) seed = std::stoul(arg.substr(6));\n"
       << "        else if (arg.rfind(\"+vcd=\", 0) == 0) vcdPath = arg.substr(5);\n"
       << "        else if (arg.rfind(\"+stim=\", 0) == 0) stimPath = arg.substr(6);\n"
       << "        else if (arg.rfind(\"+states=\", 0) == 0) statesPath = arg.substr(8);\n"
       << "        else if (arg == \"+nodump\") dump = false;\n"
       << "    }\n\n"
       // A fuzzing round runs this thousands of times and throws the waveform
       // away; writing one costs more than the simulation. +nodump skips it.
       << "    std::vector<long long> stim;\n"
       << "    std::size_t stimAt = 0;\n"
       << "    if (!stimPath.empty()) {\n"
       << "        std::ifstream in(stimPath);\n"
       << "        std::string line;\n"
       << "        while (std::getline(in, line)) {\n"
       << "            if (line.empty() || line[0] == '#') continue;\n"
       << "            std::istringstream fields(line);\n"
       << "            long long v;\n"
       << "            while (fields >> v) stim.push_back(v);\n"
       << "        }\n"
       << "    }\n"
       // Runs out of stimulus -> fall back to the RNG, so a short or truncated
       // file degrades to the old behaviour instead of driving zeros.
       << "    auto nextStim = [&](bool* ok) -> long long {\n"
       << "        if (stimAt < stim.size()) { *ok = true; return stim[stimAt++]; }\n"
       << "        *ok = false; return 0;\n"
       << "    };\n\n"
       << "    std::set<std::string> statesSeen;\n\n"
       << "    if (dump) Verilated::traceEverOn(true);\n"
       << "    " << model << "* top = new " << model << ";\n"
       << "    VerilatedVcdC* trace = new VerilatedVcdC;\n"
       << "    if (dump) { top->trace(trace, 99); trace->open(vcdPath.c_str()); }\n"
       << "    ::srandom(seed);\n\n"
       << "    vluint64_t now = 0;\n";

    // anyconst: drawn once, before anything runs. These consume the first
    // entries of the stimulus file, so a fuzzer writes them ahead of cycle 0.
    for (const auto& reg : info.freeRegs) {
        if (reg.kind != "anyconst") continue;
        const auto target = freeRegRef(info.top, reg.name);
        os << scripted(target,
                       driveLine(target, reg.width, specFor(options, reg.name),
                                 "      "),
                       "    ");
    }
    for (const auto& reg : options.hierFreeRegs) {
        if (reg.kind != "anyconst") continue;
        const auto target = "top->rootp->" + reg.flatPath;
        os << scripted(target,
                       driveLine(target, reg.width, specFor(options, reg.name),
                                 "      "),
                       "    ");
    }

    std::vector<frontend::Port> driven;
    for (const auto& port : info.inputs()) {
        if (clock && port.name == *clock) continue;
        if (reset && port.name == reset->signal) continue;
        driven.push_back(port);
    }

    if (clock) {
        os << "\n    top->" << *clock << " = 0;\n";
        if (reset) {
            os << "    top->" << reset->signal << " = " << reset->active << ";\n"
               << "    for (int i = 0; i < " << reset->cycles << "; ++i) {\n"
               << "        top->" << *clock << " = 1; top->eval(); trace->dump(now); now += 5;\n"
               << "        top->" << *clock << " = 0; top->eval(); trace->dump(now); now += 5;\n"
               << "    }\n"
               << "    top->" << reset->signal << " = " << (1 - reset->active) << ";\n";
        }
    }

    os << "\n    for (int cycle = 0; cycle < " << options.cycles << "; ++cycle) {\n";
    // Per cycle, in this order: driven ports, then anyseq free registers.
    // The stimulus file is read flat, so this order is its format.
    for (const auto& port : driven)
        os << scripted("top->" + port.name,
                       driveLine("top->" + port.name, port.width,
                                 specFor(options, port.name), "          "),
                       "        ");
    for (const auto& reg : info.freeRegs) {
        if (reg.kind != "anyseq") continue;
        const auto target = freeRegRef(info.top, reg.name);
        os << scripted(target,
                       driveLine(target, reg.width, specFor(options, reg.name),
                                 "          "),
                       "        ");
    }
    for (const auto& reg : options.hierFreeRegs) {
        if (reg.kind != "anyseq") continue;
        const auto target = "top->rootp->" + reg.flatPath;
        os << scripted(target,
                       driveLine(target, reg.width, specFor(options, reg.name),
                                 "          "),
                       "        ");
    }

    if (clock) {
        // One full clock period per cycle, sampled on both edges so the trace
        // carries the settled values either side of the edge.
        os << "        top->" << *clock << " = 1; top->eval();\n"
           << "        if (dump) trace->dump(now);\n        now += 5;\n"
           << "        top->" << *clock << " = 0; top->eval();\n"
           << "        if (dump) trace->dump(now);\n        now += 5;\n";
    } else {
        os << "        top->eval();\n"
           << "        if (dump) trace->dump(now);\n        now += 10;\n";
    }

    // What the fuzzer scores a sequence by: how many distinct states the run
    // visits. Uniform random stimulus tends to revisit a small corner of a
    // control FSM, and a state never visited cannot appear in a constraint.
    //
    // The vector is the observable state -- output ports and free registers.
    // It is a PROXY: internal registers with no port of their own are only
    // counted where they show through. Widening it means naming internal
    // signals, which ModuleInfo does not currently carry.
    {
        std::vector<std::string> observed;
        if (!options.stateSignals.empty()) {
            // Enumerated from the design, so internal registers count too.
            for (const auto& signal : options.stateSignals)
                observed.push_back(signal.isPort
                                       ? "top->" + signal.name
                                       : freeRegRef(info.top, signal.name));
        } else {
            for (const auto& port : info.ports)
                if (port.dir == "output") observed.push_back("top->" + port.name);
            for (const auto& reg : info.freeRegs)
                observed.push_back(freeRegRef(info.top, reg.name));
        }

        if (!observed.empty()) {
            os << "        {\n"
               << "            std::ostringstream state;\n";
            for (std::size_t i = 0; i < observed.size(); ++i)
                os << "            state << " << (i ? "',' << " : "")
                   << "static_cast<long long>(" << observed[i] << ");\n";
            os << "            statesSeen.insert(state.str());\n"
               << "        }\n";
        }
    }

    os << "    }\n\n"
       << "    if (!statesPath.empty()) {\n"
       << "        std::ofstream out(statesPath);\n"
       << "        out << statesSeen.size() << \"\\n\";\n"
       << "        for (const auto& s : statesSeen) out << s << \"\\n\";\n"
       << "    }\n"
       << "    if (dump) trace->close();\n"
       << "    top->final();\n"
       << "    delete top;\n"
       << "    delete trace;\n"
       << "    return 0;\n"
       << "}\n";

    return os.str();
}

}  // namespace simgen
}  // namespace smart
