#ifndef ACRIS_ACRIS_AST_VISITOR_NODE_VISITOR_HPP
#define ACRIS_ACRIS_AST_VISITOR_NODE_VISITOR_HPP

// Absolute Includes::
#include "acris/ast/node/fdecl.hpp"

// Local Includes:
#include "visitable.hpp"

// TODO: Rename NodeVisitor to AstVisitor.
// Its more clear and the other visitor constructs as also named like this.
// Renaming the visitor folder to ast_visitor could also be a good idea.

// Macros:
#define AST_VISITOR_STUB(t_class, t_type)                    \
  /* NOLINTBEGIN */                                          \
  auto t_class::visit([[maybe_unused]] t_type* t_ptr) -> Any \
  {                                                          \
    return {};                                               \
  }

/* NOLINTBEGIN */
namespace ast::visitor {
// Using:
using node::NodePtr;

// Classes:
/*!
 * Implementation of the Visitor design pattern for Nodes
 * This creates a recursive dependency on the different kind of nodes
 * So all nodes need to be forward declared in this header
 */
class NodeVisitor {
  protected:
  virtual auto traverse(NodePtr t_node) -> Any;

  public:
  NodeVisitor() = default;

  // Control:
  virtual auto visit(node::control::If* t_if) -> Any;
  virtual auto visit(node::control::Loop* t_loop) -> Any;
  virtual auto visit(node::control::Switch* t_sw) -> Any;
  virtual auto visit(node::control::SwitchCase* t_case) -> Any;
  virtual auto visit(node::control::Fallthrough* t_ft) -> Any;

  virtual auto visit(node::control::Continue* t_continue) -> Any;
  virtual auto visit(node::control::Break* t_break) -> Any;
  virtual auto visit(node::control::Defer* t_defer) -> Any;
  virtual auto visit(node::control::Return* t_return) -> Any;

  // Function:
  virtual auto visit(node::function::Parameter* t_param) -> Any;
  virtual auto visit(node::function::Function* t_fn) -> Any;
  virtual auto visit(node::function::FunctionCall* t_fn_call) -> Any;
  virtual auto visit(node::function::ReturnType* t_rt) -> Any;

  // Lvalue:
  virtual auto visit(node::lvalue::Let* t_let) -> Any;
  virtual auto visit(node::lvalue::Var* t_var) -> Any;
  virtual auto visit(node::lvalue::IdentifierNode* t_id) -> Any;
  virtual auto visit(node::lvalue::Subscript* t_subscript) -> Any;
  virtual auto visit(node::lvalue::ScopeResolution* t_scope_res) -> Any;

  // Meta:
  virtual auto visit(node::meta::Attribute* t_attr) -> Any;
  virtual auto visit(node::meta::LetDecl* t_ldecl) -> Any;
  virtual auto visit(node::meta::VarDecl* t_vdecl) -> Any;
  virtual auto visit(node::meta::FunctionDecl* t_fdecl) -> Any;

  // Operators:
  virtual auto visit(node::operators::Arithmetic* t_arith) -> Any;
  virtual auto visit(node::operators::Assignment* t_assign) -> Any;
  virtual auto visit(node::operators::Comparison* t_comp) -> Any;

  virtual auto visit(node::operators::Increment* t_inc) -> Any;
  virtual auto visit(node::operators::Decrement* t_dec) -> Any;

  virtual auto visit(node::operators::AddressOf* t_addr_of) -> Any;
  virtual auto visit(node::operators::Dereference* t_deref) -> Any;
  virtual auto visit(node::operators::UnaryPrefix* t_up) -> Any;
  virtual auto visit(node::operators::ToCast* t_to) -> Any;

  // Logical:
  virtual auto visit(node::operators::Not* t_not) -> Any;
  virtual auto visit(node::operators::And* t_and) -> Any;
  virtual auto visit(node::operators::Or* t_or) -> Any;

  virtual auto visit(node::operators::Ternary* t_ternary) -> Any;

  // Packaging:
  virtual auto visit(node::packaging::Import* t_import) -> Any;
  virtual auto visit(node::packaging::ModuleDecl* t_mod) -> Any;

  // Rvalue:
  virtual auto visit(node::rvalue::Float* t_float) -> Any;
  virtual auto visit(node::rvalue::Integer* t_int) -> Any;
  virtual auto visit(node::rvalue::Char* t_ch) -> Any;
  virtual auto visit(node::rvalue::String* t_str) -> Any;
  virtual auto visit(node::rvalue::ArrayExpr* t_arr) -> Any;
  virtual auto visit(node::rvalue::Boolean* t_bool) -> Any;

  // Builtin Types:
  virtual auto visit(node::builtin_types::Pointer* t_ptr) -> Any;
  virtual auto visit(node::builtin_types::Array* t_array) -> Any;
  virtual auto visit(node::builtin_types::TypeName* t_type) -> Any;

  // User Types:
  virtual auto visit(node::user_types::EnumField* t_field) -> Any;
  virtual auto visit(node::user_types::Enum* t_enum) -> Any;
  virtual auto visit(node::user_types::Method* t_meth) -> Any;
  virtual auto visit(node::user_types::MethodCall* t_meth_call) -> Any;
  virtual auto visit(node::user_types::Interface* t_ifc) -> Any;
  virtual auto visit(node::user_types::MemberDecl* t_meth) -> Any;
  virtual auto visit(node::user_types::Struct* t_struct) -> Any;
  virtual auto visit(node::user_types::Self* t_self) -> Any;
  virtual auto visit(node::user_types::Member* t_member) -> Any;
  virtual auto visit(node::user_types::MemberAccess* t_access) -> Any;

  // Misc:
  virtual auto visit(node::List* t_list) -> Any;
  virtual auto visit(node::Nil* t_nil) -> Any;

  virtual auto visit(node::NodeInterface* t_ptr) -> Any;

  virtual ~NodeVisitor() = default;
};
} // namespace ast::visitor

/* NOLINTEND */

#endif // ACRIS_ACRIS_AST_VISITOR_NODE_VISITOR_HPP
