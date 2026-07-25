// WP2A link spike: prove that hw-cbmc's Verilog frontend can be LINKED into
// our binary (rather than shelled out to) and that the parse tree carries
// everything ModuleInfo needs: module name, ports (dir/width/name), params,
// `(* anyseq *)` / `(* anyconst *)` free registers, and assume statements.
//
// Usage: hwcbmc_spike <file.sv> [top]
// Prints a human-readable summary; exit 0 on success.
//
// Build (see docs/PLAN-cpp-single-binary.md WP2A for the recorded link set).

#include <verilog/verilog_expr.h>
#include <verilog/verilog_language.h>
#include <verilog/verilog_parse_tree.h>

#include <util/cout_message.h>
#include <util/mathematical_types.h>
#include <util/std_expr.h>

#include <fstream>
#include <iostream>
#include <string>

namespace {

// Does this declaration carry the Verilog attribute `name`
// (i.e. was it written `(* name *) reg foo;`)?
bool has_attribute(const irept &decl, const std::string &name)
{
  // Shape (hw-cbmc parse tree):
  //   verilog_attributes
  //     0: verilog_attribute
  //          * name: symbol   * identifier/base_name: anyseq
  const irept &attributes = decl.find(ID_verilog_attributes);

  for(const auto &attribute : attributes.get_sub())
  {
    const irept &attribute_name = attribute.find(ID_name);
    if(attribute_name.get(ID_base_name) == name ||
       attribute_name.get(ID_identifier) == name)
      return true;
  }

  return false;
}

// Best-effort constant width of a declaration type, without elaboration.
// Returns -1 when the width is not a resolvable integer constant here; the
// real adapter (WP2) falls back to the elaborated symbol table for those.
int width_of(const typet &type)
{
  if(type.is_nil() || type.id().empty())
    return 1; // implicit 1-bit net/reg

  if(type.id() == ID_bool)
    return 1;

  if(type.id() == ID_unsignedbv || type.id() == ID_signedbv)
    return atoi(type.get(ID_width).c_str());

  if(type.id() == ID_verilog_unsignedbv || type.id() == ID_verilog_signedbv)
    return atoi(type.get(ID_width).c_str());

  return -1;
}

const char *decl_class(const verilog_declt &decl)
{
  const auto cls = decl.get_class();
  return cls.empty() ? "(none)" : cls.c_str();
}

// Walk the whole irep tree looking for immediate/concurrent assumes.
bool contains_assume(const irept &irep)
{
  if(irep.id() == ID_verilog_immediate_assume ||
     irep.id() == ID_verilog_assume_property || irep.id() == ID_sva_assume ||
     irep.id() == ID_verilog_smv_assume)
    return true;

  for(const auto &op : irep.get_sub())
    if(contains_assume(op))
      return true;

  for(const auto &named : irep.get_named_sub())
    if(contains_assume(named.second))
      return true;

  return false;
}

} // namespace

int main(int argc, char *argv[])
{
  if(argc < 2)
  {
    std::cerr << "usage: hwcbmc_spike <file.sv> [top]\n";
    return 1;
  }

  const std::string path = argv[1];
  const std::string wanted_top = argc > 2 ? argv[2] : "";

  std::ifstream in(path);
  if(!in)
  {
    std::cerr << "cannot open " << path << '\n';
    return 1;
  }

  console_message_handlert message_handler;
  message_handler.set_verbosity(1); // errors only

  verilog_languaget language;

  if(language.parse(in, path, message_handler))
  {
    std::cerr << "PARSE FAILED\n";
    return 2;
  }

  const auto &parse_tree = language.get_parse_tree();

  for(const auto &item : parse_tree.items)
  {
    if(item.id() != ID_verilog_module)
      continue;

    const auto &module = to_verilog_module_source(item);
    const std::string base_name = id2string(module.base_name());

    if(!wanted_top.empty() && base_name != wanted_top)
      continue;

    std::cout << "module " << base_name << '\n';

    // --- parameter port list (#(parameter W = 8)) ---
    for(const auto &declarator : module.parameter_port_list())
    {
      std::cout << "  param " << id2string(declarator.base_name()) << " = ";
      const auto &value = declarator.value();
      if(value.is_not_nil() && value.id() == ID_constant)
        std::cout << id2string(to_constant_expr(value).get_value());
      else
        std::cout << "<expr>";
      std::cout << '\n';
    }

    // --- ports ---
    std::cout << "  ports " << module.ports().size() << '\n';
    for(const auto &port : module.ports())
    {
      for(const auto &declarator : port.declarators())
      {
        std::cout << "    " << decl_class(port) << ' ' << width_of(port.type())
                  << ' ' << id2string(declarator.base_name()) << '\n';
      }
    }

    // --- module items: non-port params, free regs, assumes ---
    bool has_assume = false;
    for(const auto &module_item : module.module_items())
    {
      if(module_item.id() == ID_decl)
      {
        const auto &decl = static_cast<const verilog_declt &>(
          static_cast<const irept &>(module_item));
        const bool anyseq = has_attribute(decl, "anyseq");
        const bool anyconst = has_attribute(decl, "anyconst");

        for(const auto &declarator : decl.declarators())
        {
          if(anyseq || anyconst)
            std::cout << "    free " << (anyseq ? "anyseq" : "anyconst") << ' '
                      << width_of(decl.type()) << ' '
                      << id2string(declarator.base_name()) << '\n';
          else if(decl.get_class() == ID_parameter_decl ||
                  decl.get_class() == ID_local_parameter_decl)
            std::cout << "  param " << id2string(declarator.base_name())
                      << " = <decl>\n";
        }
      }

      if(!has_assume && contains_assume(module_item))
        has_assume = true;
    }

    std::cout << "  hasAssume " << (has_assume ? "1" : "0") << '\n';
  }

  return 0;
}
