#include "HwcbmcFrontend.h"

#include <verilog/verilog_expr.h>
#include <verilog/verilog_language.h>
#include <verilog/verilog_parse_tree.h>
#include <verilog/verilog_types.h>

#include <langapi/language_file.h>

#include <util/arith_tools.h>
#include <util/invariant.h>
#include <util/json.h>
#include <util/message.h>
#include <util/options.h>
#include <util/string2int.h>
#include <util/symbol_table.h>
#include <util/xml.h>

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <map>
#include <fstream>
#include <memory>
#include <stdexcept>

namespace smart {
namespace frontend {
namespace {

// hw-cbmc talks to us through a message_handlert; we only care about errors,
// and only to put them in the exception text.
class CollectingMessageHandler : public message_handlert
{
public:
    void print(unsigned level, const std::string& message) override
    {
        if(level <= 1)  // M_ERROR
        {
            if(!errors.empty())
                errors += "; ";
            errors += message;
        }
    }

    void print(unsigned level, const std::string& message,
               const source_locationt&) override
    {
        print(level, message);
    }

    // Structured output is unused here; hw-cbmc only emits it under
    // --xml-ui / --json-ui, which we never set.
    void print(unsigned, const xmlt&) override {}
    void print(unsigned, const jsont&) override {}

    void flush(unsigned) override {}

    std::string errors;
};

std::string readableErrors(const CollectingMessageHandler& handler)
{
    return handler.errors.empty() ? std::string("parse error") : handler.errors;
}

// Verilog identifiers in the symbol table are "Verilog::<module>[.<name>]".
irep_idt moduleSymbol(const std::string& top)
{
    return "Verilog::" + top;
}

// Resolved bit width of an elaborated type, or -1 when it is not a plain
// bit-vector (matching the oracle's "unknown width" = JSON null).
int widthOf(const typet& type)
{
    if(type.id() == ID_bool)
        return 1;

    if(type.id() == ID_unsignedbv || type.id() == ID_signedbv ||
       type.id() == ID_verilog_unsignedbv || type.id() == ID_verilog_signedbv)
    {
        const auto width = type.get_size_t(ID_width);
        return width == 0 ? -1 : static_cast<int>(width);
    }

    return -1;
}

// `(* anyseq *)` / `(* anyconst *)`. Attribute shape in the parse tree:
//   verilog_attributes -> 0: verilog_attribute -> * name: symbol(base_name)
// NOTE this only works with the patch in third_party/patches/ applied;
// unpatched hw-cbmc silently drops single-attribute instances.
bool hasAttribute(const irept& decl, const char* name)
{
    for(const auto& attribute : decl.find(ID_verilog_attributes).get_sub())
    {
        const irept& attribute_name = attribute.find(ID_name);
        if(attribute_name.get(ID_base_name) == name ||
           attribute_name.get(ID_identifier) == name)
            return true;
    }
    return false;
}

bool containsAssume(const irept& irep)
{
    if(irep.id() == ID_verilog_immediate_assume ||
       irep.id() == ID_verilog_assume_property || irep.id() == ID_sva_assume ||
       irep.id() == ID_verilog_smv_assume)
        return true;

    for(const auto& op : irep.get_sub())
        if(containsAssume(op))
            return true;

    for(const auto& named : irep.get_named_sub())
        if(containsAssume(named.second))
            return true;

    return false;
}

const verilog_module_sourcet* findModule(const verilog_parse_treet& parse_tree,
                                         const std::string& top)
{
    for(const auto& item : parse_tree.items)
    {
        if(item.id() != ID_verilog_module)
            continue;

        const auto& module = to_verilog_module_source(item);
        if(top.empty() || id2string(module.base_name()) == top)
            return &module;
    }
    return nullptr;
}

void collectFreeRegs(const verilog_module_sourcet& module, ModuleInfo& info)
{
    for(const auto& module_item : module.module_items())
    {
        if(module_item.id() != ID_decl)
            continue;

        const auto& decl =
            static_cast<const verilog_declt&>(static_cast<const irept&>(module_item));

        const bool anyseq = hasAttribute(decl, "anyseq");
        const bool anyconst = hasAttribute(decl, "anyconst");
        if(!anyseq && !anyconst)
            continue;

        for(const auto& declarator : decl.declarators())
        {
            FreeReg reg;
            reg.kind = anyseq ? "anyseq" : "anyconst";
            reg.width = -1;  // filled from the symbol table below
            reg.name = id2string(declarator.base_name());
            info.freeRegs.push_back(std::move(reg));
        }
    }
}

// Width of an UNELABORATED declaration type. Only constant ranges resolve
// here; `[W-1:0]` cannot, and yields -1 like the oracle's null. Used solely
// on the degraded path — the elaborated symbol table is the real source.
int parseTreeWidth(const typet& type)
{
    if(type.is_nil() || type.id().empty() || type.id() == ID_reg ||
       type.id() == ID_wire || type.id() == ID_bool)
        return 1;

    if(type.id() == ID_verilog_packed_array)
    {
        const auto& range = type.find(ID_range).get_sub();
        if(range.size() != 2)
            return -1;
        const auto msb = string2optional_int(id2string(range[0].get(ID_value)));
        const auto lsb = string2optional_int(id2string(range[1].get(ID_value)));
        if(!msb.has_value() || !lsb.has_value())
            return -1;
        return std::abs(*msb - *lsb) + 1;
    }

    return -1;
}

std::string directionOf(const irep_idt& decl_class)
{
    if(decl_class == ID_input)
        return "input";
    if(decl_class == ID_inout)
        return "inout";
    if(decl_class == ID_output || decl_class == "output_register")
        return "output";
    return "";
}

// Fallback for designs hw-cbmc parses but cannot elaborate: read the ports
// straight off the parse tree. ANSI headers carry the direction on the port
// declaration; non-ANSI ones list bare names in the header and declare the
// directions among the module items, so both are handled.
void fillPortsFromParseTree(const verilog_module_sourcet& module, ModuleInfo& info)
{
    std::map<std::string, std::pair<std::string, int>> declared;  // name -> (dir, width)
    for(const auto& module_item : module.module_items())
    {
        if(module_item.id() != ID_decl)
            continue;
        const auto& decl =
            static_cast<const verilog_declt&>(static_cast<const irept&>(module_item));
        const auto direction = directionOf(decl.get_class());
        if(direction.empty())
            continue;
        for(const auto& declarator : decl.declarators())
            declared[id2string(declarator.get(ID_base_name))] = {
                direction, parseTreeWidth(decl.type())};
    }

    for(const auto& port : module.ports())
    {
        const auto header_direction = directionOf(port.get_class());
        for(const auto& declarator : port.declarators())
        {
            Port p;
            p.name = id2string(declarator.get(ID_base_name));
            p.dir = header_direction;
            p.width = parseTreeWidth(port.type());

            const auto found = declared.find(p.name);
            if(p.dir.empty() && found != declared.end())
            {
                p.dir = found->second.first;
                p.width = found->second.second;
            }
            if(p.dir.empty())
                p.dir = "input";  // Verilog's default for an undeclared port

            info.ports.push_back(std::move(p));
        }
    }
}

// Ports come from the ELABORATED module type, not the parse tree: it
// normalises ANSI and non-ANSI port styles into one ordered list carrying
// direction flags and resolved widths.
void fillPortsFromSymbolTable(const symbol_tablet& symbol_table,
                              const std::string& top, ModuleInfo& info)
{
    const auto* module_symbol = symbol_table.lookup(moduleSymbol(top));
    if(module_symbol == nullptr)
        return;

    const auto& ports =
        static_cast<const exprt&>(module_symbol->type.find(ID_ports)).operands();

    for(const auto& port : ports)
    {
        if(port.id() != ID_symbol)
            continue;

        Port p;
        const bool is_input = port.get_bool(ID_input);
        const bool is_output = port.get_bool(ID_output);
        p.dir = (is_input && is_output) ? "inout" : (is_output ? "output" : "input");
        p.width = widthOf(port.type());
        p.name = id2string(port.get("#name"));
        if(p.name.empty())
        {
            // Fall back to the tail of the qualified identifier.
            const std::string identifier = id2string(to_symbol_expr(port).get_identifier());
            const auto dot = identifier.rfind('.');
            p.name = dot == std::string::npos ? identifier : identifier.substr(dot + 1);
        }
        info.ports.push_back(std::move(p));
    }
}

void fillWidthsAndParams(const symbol_tablet& symbol_table,
                         const std::string& top, ModuleInfo& info)
{
    const irep_idt module_identifier = moduleSymbol(top);
    const std::string prefix = id2string(module_identifier) + ".";

    for(auto& reg : info.freeRegs)
    {
        const auto* symbol = symbol_table.lookup(prefix + reg.name);
        if(symbol != nullptr)
            reg.width = widthOf(symbol->type);
    }

    // Elaborated compile-time constants of the module: `parameter` (port list
    // or body) and `localparam` alike land as is_macro symbols with a
    // constant value. Reporting all of them is a deliberate superset of what
    // the regex oracle could evaluate — see the WP2 divergence report.
    for(const auto& [name, symbol] : symbol_table.symbols)
    {
        if(symbol.module != module_identifier || !symbol.is_macro ||
           symbol.value.is_nil())
            continue;

        const auto value = numeric_cast<mp_integer>(symbol.value);
        if(value.has_value())
            info.params[id2string(symbol.base_name)] = value->to_long();
    }
}

}  // namespace

bool declaresModule(const std::string& path, const std::string& name)
{
    std::ifstream in(path);
    if(!in)
        return false;

    const std::string keyword = "module";
    std::string line;
    while(std::getline(in, line))
    {
        for(std::size_t at = line.find(keyword); at != std::string::npos;
            at = line.find(keyword, at + 1))
        {
            const bool startsWord =
                at == 0 || !(std::isalnum((unsigned char)line[at - 1]) ||
                             line[at - 1] == '_');
            if(!startsWord)
                continue;

            std::size_t after = at + keyword.size();
            if(after >= line.size() || !std::isspace((unsigned char)line[after]))
                continue;
            while(after < line.size() && std::isspace((unsigned char)line[after]))
                ++after;

            const std::size_t nameStart = after;
            while(after < line.size() &&
                  (std::isalnum((unsigned char)line[after]) || line[after] == '_'))
                ++after;
            if(after - nameStart == name.size() &&
               line.compare(nameStart, after - nameStart, name) == 0)
                return true;
        }
    }
    return false;
}

namespace {

// Design files to hand to the frontend: the main one, anything the caller
// named, and — unless disabled — its sibling .sv/.v files, which is where
// multi-file designs like the i2c cores keep their submodules.
std::vector<std::string> designFiles(const std::string& path,
                                     const std::string& top,
                                     const HwcbmcOptions& options)
{
    namespace fs = std::filesystem;

    std::vector<std::string> files{path};
    // Compare by identity, not by spelling: "c17.sv" and "./c17.sv" are the
    // same file, and listing it twice makes hw-cbmc see two modules of that
    // name — it then elaborates neither, and every width goes unresolved.
    auto already_listed = [&files](const std::string& candidate) {
        std::error_code error;
        for(const auto& listed : files)
            if(fs::equivalent(listed, candidate, error))
                return true;
        return false;
    };

    for(const auto& extra : options.extraFiles)
        if(!already_listed(extra))
            files.push_back(extra);

    if(options.autoDiscoverSiblings)
    {
        const auto parent = fs::path(path).parent_path();
        std::vector<std::string> siblings;
        std::error_code error;
        for(const auto& entry :
            fs::directory_iterator(parent.empty() ? fs::path(".") : parent, error))
        {
            if(!entry.is_regular_file())
                continue;
            const auto extension = entry.path().extension().string();
            if(extension != ".sv" && extension != ".v")
                continue;
            const auto candidate = entry.path().string();
            if(already_listed(candidate))
                continue;
            // Never pull in a file that redeclares the top module — `smart`
            // writes <top>_assertion.sv next to its input, and a second run in
            // the same directory must not swallow its own previous output.
            if(!top.empty() && declaresModule(candidate, top))
                continue;
            siblings.push_back(candidate);
        }
        // directory_iterator order is unspecified; keep runs reproducible.
        std::sort(siblings.begin(), siblings.end());
        files.insert(files.end(), siblings.begin(), siblings.end());
    }

    return files;
}

}  // namespace

ModuleInfo parseModuleFile(const std::string& path, const std::string& top,
                           HwcbmcOptions& options)
{
    CollectingMessageHandler message_handler;

    optionst language_options;
    {
        // The design's own directory first, so `include "sibling.sv"` works
        // the way it does for the simulator and for EBMC.
        std::list<std::string> include_paths;
        const auto parent = std::filesystem::path(path).parent_path().string();
        include_paths.push_back(parent.empty() ? "." : parent);
        for(const auto& include_path : options.includePaths)
            include_paths.push_back(include_path);
        language_options.set_option("I", include_paths);

        std::list<std::string> defines(options.defines.begin(), options.defines.end());
        language_options.set_option("defines", defines);
    }

    // language_filest owns the languaget instances and, crucially, typechecks
    // modules in dependency order across all files.
    language_filest language_files;
    // Points into a parse tree owned by language_files; only read before
    // typecheck() runs.
    const verilog_module_sourcet* module = nullptr;

    for(const auto& file : designFiles(path, top, options))
    {
        std::ifstream in(file);
        if(!in)
        {
            if(file == path)
                throw std::runtime_error("cannot open " + path);
            continue;  // a sibling we cannot read is not fatal
        }

        auto& language_file = language_files.add_file(file);
        language_file.filename = file;
        auto language = std::make_unique<verilog_languaget>();
        language->set_language_options(language_options, message_handler);

        if(language->parse(in, file, message_handler))
        {
            if(file == path)
                throw std::runtime_error("hw-cbmc failed to parse " + path + ": " +
                                         readableErrors(message_handler));
            // A sibling that does not parse is dropped rather than sinking
            // the whole elaboration.
            language_files.remove_file(file);
            continue;
        }

        if(module == nullptr)
            module = findModule(language->get_parse_tree(), top);

        language_file.language = std::move(language);
        language_file.get_modules();
    }

    if(module == nullptr)
        throw std::runtime_error("module '" + top + "' not found");

    ModuleInfo info;
    info.top = id2string(module->base_name());

    collectFreeRegs(*module, info);

    for(const auto& module_item : module->module_items())
    {
        if(containsAssume(module_item))
        {
            info.hasAssume = true;
            break;
        }
    }

    // Elaboration: widths, port normalisation, parameter values. A design
    // hw-cbmc cannot elaborate still yields the parse-tree view.
    //
    // CBMC reports internal invariant violations by ABORTING the process,
    // which is not acceptable in a library: designs exist (i2c_slave_wbm,
    // s1196) that trip one during elaboration. cbmc_invariants_should_throwt
    // flips them to `invariant_failedt` exceptions for the duration, so a
    // frontend limitation costs us the widths, not the run.
    symbol_tablet symbol_table;
    CollectingMessageHandler typecheck_messages;
    {
        cbmc_invariants_should_throwt invariants_throw;
        try
        {
            options.elaborated =
                !language_files.typecheck(symbol_table, typecheck_messages);
        }
        catch(const invariant_failedt& e)
        {
            options.elaborated = false;
            options.elaborationError = "hw-cbmc invariant violation: " +
                                       e.what();
        }
        catch(const std::exception& e)
        {
            options.elaborated = false;
            options.elaborationError = e.what();
        }
    }

    if(options.elaborated)
    {
        fillPortsFromSymbolTable(symbol_table, info.top, info);
        fillWidthsAndParams(symbol_table, info.top, info);
    }
    else
    {
        if(options.elaborationError.empty())
            options.elaborationError = readableErrors(typecheck_messages);
        fillPortsFromParseTree(*module, info);
    }

    return info;
}

ModuleInfo parseModuleFile(const std::string& path, const std::string& top)
{
    HwcbmcOptions options;
    return parseModuleFile(path, top, options);
}

}  // namespace frontend
}  // namespace smart
