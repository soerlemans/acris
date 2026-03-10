#include "ast_printer.hpp"

// STL Includes:
#include <iomanip>
#include <ios>
#include <sstream>

// Absolute Includes:
#include "lib/stdprint.hpp"

// Macros:
/*!
 * Instantiates a @ref CountGuard object.
 * Which will use RAII to manage indentation.
 */
#define COUNTG_INIT() \
  CountGuard guard__  \
  {                   \
    m_counter         \
  }

/*!
 * Defines a @ref AstPrinter method, that will print all traits.
 *
 * @param[in] t_type Type of Node to accept.
 */
#define DEFINE_PRINTER_METHOD(t_type)                           \
  auto AstPrinter::visit([[maybe_unused]] t_type* t_ptr) -> Any \
  {                                                             \
    COUNTG_INIT();                                              \
                                                                \
    print_hl(#t_type);                                          \
    print_traits(t_ptr);                                        \
                                                                \
    return {};                                                  \
  }

namespace ast::visitor {
// Using statements:
NODE_USING_ALL_NAMESPACES()

// Friend classes:
namespace {
class CountGuard {
  private:
  int& m_counter;

  public:
  CountGuard(int& t_counter): m_counter{t_counter}
  {
    m_counter++;
  }

  ~CountGuard()
  {
    m_counter--;
  }
};
} // namespace

// Methods:
auto AstPrinter::print_if(std::string_view t_str, NodePtr t_ptr) -> void
{
  if(t_ptr) {
    print(t_str);
    traverse(t_ptr);
  }
}

AstPrinter::AstPrinter(std::ostream& t_os): m_os{t_os}
{}

// Control:
DEFINE_PRINTER_METHOD(If)
DEFINE_PRINTER_METHOD(Loop)
DEFINE_PRINTER_METHOD(Continue)
DEFINE_PRINTER_METHOD(Break)
DEFINE_PRINTER_METHOD(Defer)
DEFINE_PRINTER_METHOD(Return)

// Function:
DEFINE_PRINTER_METHOD(Parameter)
DEFINE_PRINTER_METHOD(Function)

auto AstPrinter::visit(FunctionCall* t_fn_call) -> Any
{
  COUNTG_INIT();

  print_hl("FunctionCall");
  print("| Identifier: ", t_fn_call->identifier());
  print_if("Arguments: ", t_fn_call->args());

  return {};
}

DEFINE_PRINTER_METHOD(ReturnType)

// Lvalue:
DEFINE_PRINTER_METHOD(Let)
DEFINE_PRINTER_METHOD(Var)

auto AstPrinter::visit(IdentifierNode* t_id) -> Any
{
  COUNTG_INIT();

  print_hl("IdentifierNode: ", t_id->identifier());
  print_traits(t_id);

  return {};
}

DEFINE_PRINTER_METHOD(Subscript)

auto AstPrinter::visit(ScopeResolution* t_scope_res) -> Any
{
  using lib::stdprint::detail::print_seq;

  COUNTG_INIT();

  std::ostringstream oss{};
  print_seq(oss, t_scope_res->path());

  print_hl("ScopeResolution");
  print("| Path: ", oss.view());
  print_traits(t_scope_res);

  return {};
}

// Meta:
auto AstPrinter::visit(Attribute* t_attr) -> Any
{
  COUNTG_INIT();

  print_hl("Attribute: ", t_attr->identifier());
  print_traits(t_attr);

  return {};
}

auto AstPrinter::visit(LetDecl* t_ldecl) -> Any
{
  COUNTG_INIT();

  print_hl("LetDecl: ", t_ldecl->identifier());
  print_traits(t_ldecl);

  return {};
}

auto AstPrinter::visit(VarDecl* t_vdecl) -> Any
{
  COUNTG_INIT();

  print_hl("VarDecl: ", t_vdecl->identifier());
  print_traits(t_vdecl);

  return {};
}

auto AstPrinter::visit(FunctionDecl* t_fdecl) -> Any
{
  COUNTG_INIT();

  print_hl("FunctionDecl: ", t_fdecl->identifier());
  print_traits(t_fdecl);

  return {};
}

// Operators:
auto AstPrinter::visit(Arithmetic* t_arith) -> Any
{
  COUNTG_INIT();

  const auto str{t_arith->op2str()};

  print_hl("Arithmetic");
  print("| Op: ", str); // TODO: Make part of print_traits.
  print_traits(t_arith);

  return {};
}

auto AstPrinter::visit(Assignment* t_assign) -> Any
{
  COUNTG_INIT();

  const auto str{t_assign->op2str()};

  print_hl("Assignment");
  print("| Op: ", str); // TODO: Make part of print_traits.
  print_traits(t_assign);

  return {};
}

auto AstPrinter::visit(Comparison* t_comp) -> Any
{
  COUNTG_INIT();

  const auto str{t_comp->op2str()};

  print_hl("Comparison");
  print("| Op: ", str); // TODO: Make part of print_traits.
  print_traits(t_comp);

  return {};
}

auto AstPrinter::visit(Increment* t_inc) -> Any
{
  COUNTG_INIT();

  print_hl("Increment");
  print_traits(t_inc);

  return {};
}

auto AstPrinter::visit(Decrement* t_dec) -> Any
{
  COUNTG_INIT();

  print_hl("Decrement");
  print_traits(t_dec);

  return {};
}

auto AstPrinter::visit(AddressOf* t_addr_of) -> Any
{
  COUNTG_INIT();

  print_hl("AddressOf");
  print_traits(t_addr_of);

  return {};
}

auto AstPrinter::visit(Dereference* t_deref) -> Any
{
  COUNTG_INIT();

  print_hl("Dereference");
  print_traits(t_deref);

  return {};
}

auto AstPrinter::visit(UnaryPrefix* t_up) -> Any
{
  COUNTG_INIT();

  const auto str{t_up->op2str()};

  print_hl("UnaryPrefix");
  print("| Op: ", str); // TODO: Make part of print_traits.
  print_traits(t_up);

  return {};
}

DEFINE_PRINTER_METHOD(ToCast)

// Logical:
DEFINE_PRINTER_METHOD(Not)
DEFINE_PRINTER_METHOD(And)
DEFINE_PRINTER_METHOD(Or)

auto AstPrinter::visit(Ternary* t_ternary) -> Any
{
  COUNTG_INIT();

  print_hl("Ternary");
  print_traits(t_ternary);

  return {};
}

// Packaging:
auto AstPrinter::visit(Import* t_import) -> Any
{
  COUNTG_INIT();

  print_hl("Import");

  for(const auto& pair : t_import->imports()) {
    std::stringstream ss;
    if(pair.second) {
      ss << " Identifier: " << pair.second.value();
    }

    print("| Pkg: ", std::quoted(pair.first), ss.str());
  }

  return {};
}

DEFINE_PRINTER_METHOD(ModuleDecl)

// Rvalue:
auto AstPrinter::visit(Float* t_float) -> Any
{
  COUNTG_INIT();

  print_hl("Float: ", t_float->get());

  return {};
}

auto AstPrinter::visit(Integer* t_int) -> Any
{
  COUNTG_INIT();

  print_hl("Integer: ", t_int->get());

  return {};
}

auto AstPrinter::visit(Char* t_ch) -> Any
{
  COUNTG_INIT();

  print_hl("Char: ", t_ch->get());

  return {};
}

auto AstPrinter::visit(String* t_str) -> Any
{
  COUNTG_INIT();

  print_hl("String: ", t_str->get());

  return {};
}

auto AstPrinter::visit(ArrayExpr* t_arr) -> Any
{
  COUNTG_INIT();

  print_hl("ArrayExpr");
  print_traits(t_arr);

  return {};
}

auto AstPrinter::visit(Boolean* t_bool) -> Any
{
  COUNTG_INIT();

  print_hl("Boolean: ", t_bool->get());

  return {};
}

// Builtin Types:
auto AstPrinter::visit(Pointer* t_ptr) -> Any
{
  COUNTG_INIT();

  print_hl("Pointer");
  print_traits(t_ptr);

  return {};
}

auto AstPrinter::visit(Array* t_arr) -> Any
{
  COUNTG_INIT();

  print_hl("Array");
  print_traits(t_arr);
  print("| ArraySize: ", t_arr->size());

  return {};
}

auto AstPrinter::visit(TypeName* t_type) -> Any
{
  COUNTG_INIT();

  print_hl("TypeName");
  print_traits(t_type);

  return {};
}

// User Types:
DEFINE_PRINTER_METHOD(EnumField)
DEFINE_PRINTER_METHOD(Enum)

auto AstPrinter::visit(Method* t_meth) -> Any
{
  COUNTG_INIT();

  print_hl("Method");
  print("| ReceiverType: ", t_meth->get_receiver());
  print_traits(t_meth);

  return {};
}

auto AstPrinter::visit(MethodCall* t_meth_call) -> Any
{
  COUNTG_INIT();

  print_hl("MethodCall");
  print_traits(t_meth_call);

  return {};
}

DEFINE_PRINTER_METHOD(Interface)
DEFINE_PRINTER_METHOD(MemberDecl)
DEFINE_PRINTER_METHOD(Struct)

auto AstPrinter::visit(Self* t_self) -> Any
{
  COUNTG_INIT();

  print_hl("Self");
  print_traits(t_self);

  return {};
}

DEFINE_PRINTER_METHOD(Member)
DEFINE_PRINTER_METHOD(MemberAccess)

// Misc:
auto AstPrinter::visit(List* t_list) -> Any
{
  COUNTG_INIT();

  print_hl("List");
  for(NodePtr& node : *t_list) {
    traverse(node);
  }

  return {};
}

auto AstPrinter::visit([[maybe_unused]] Nil* t_nil) -> Any
{
  COUNTG_INIT();

  print_hl("Nil");

  return {};
}

auto AstPrinter::print(NodePtr t_ast) -> void
{
  traverse(t_ast);
}
} // namespace ast::visitor
