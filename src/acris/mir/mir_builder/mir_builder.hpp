#ifndef ACRIS_ACRIS_MIR_MIR_BUILDER_MIR_BUILDER_HPP
#define ACRIS_ACRIS_MIR_MIR_BUILDER_MIR_BUILDER_HPP

// Absolute Includes:
#include "acris/ast/visitor/node_visitor.hpp"
#include "acris/mir/mir_builder/mir_module_factory.hpp"

namespace mir::mir_builder {
// Aliases:
namespace node = ast::node;

using ast::node::NodePtr;
using ast::visitor::Any;
using ast::visitor::NodeVisitor;

// Structs:
/*!
 */
struct MirTranslateParams {
  NodePtr m_ast;
  // SymbolTablePtr m_symbol_table;
};

// Classes:
// TODO: Globals should be implemented as a .
/*!
 * So we convert the AST to the IR that we defined.
 * Our strategy involves assuming that every given
 */
class MirBuilder : public NodeVisitor {
  private:
  MirModuleFactoryPtr m_factory;

  /*!
   * Traverse a node in a new environemnt
   */
  auto traverse_in_new_env(NodePtr t_node) -> LocalVarEnvState;

  public:
  MirBuilder();

  // Control:
  auto visit(node::control::If* t_if) -> Any override;
  auto visit(node::control::Loop* t_loop) -> Any override;
  auto visit(node::control::Continue* t_continue) -> Any override;
  auto visit(node::control::Break* t_break) -> Any override;
  auto visit(node::control::Defer* t_defer) -> Any override;
  auto visit(node::control::Return* t_return) -> Any override;

  // Function:
  auto visit(node::function::Parameter* t_param) -> Any override;
  auto visit(node::function::Function* t_fn) -> Any override;
  auto visit(node::function::FunctionCall* t_fn_call) -> Any override;
  auto visit(node::function::ReturnType* t_rt) -> Any override;

  // Lvalue:
  auto visit(node::lvalue::Let* t_let) -> Any override;
  auto visit(node::lvalue::Var* t_var) -> Any override;
  auto visit(node::lvalue::Variable* t_var) -> Any override;

  // Meta:
  auto visit(node::meta::Attribute* t_attr) -> Any override;
  auto visit(node::meta::LetDecl* t_ldecl) -> Any override;
  auto visit(node::meta::VarDecl* t_vdecl) -> Any override;
  auto visit(node::meta::FunctionDecl* t_fdecl) -> Any override;

  // Operators:
  auto visit(node::operators::Arithmetic* t_arith) -> Any override;
  auto visit(node::operators::Assignment* t_assign) -> Any override;
  auto visit(node::operators::Comparison* t_comp) -> Any override;

  auto visit(node::operators::Increment* t_inc) -> Any override;
  auto visit(node::operators::Decrement* t_dec) -> Any override;

  auto visit(node::operators::UnaryPrefix* t_up) -> Any override;

  // Logical:
  auto visit(node::operators::Not* t_not) -> Any override;
  auto visit(node::operators::And* t_and) -> Any override;
  auto visit(node::operators::Or* t_or) -> Any override;

  auto visit(node::operators::Ternary* t_ternary) -> Any override;

  // Packaging:
  auto visit(node::packaging::Import* t_import) -> Any override;
  auto visit(node::packaging::ModuleDecl* t_mod) -> Any override;

  // Rvalue:
  auto visit(node::rvalue::Float* t_float) -> Any override;
  auto visit(node::rvalue::Integer* t_int) -> Any override;
  auto visit(node::rvalue::String* t_str) -> Any override;
  auto visit(node::rvalue::Boolean* t_bool) -> Any override;

  // User Types:
  auto visit(node::user_types::Method* t_meth) -> Any override;
  auto visit(node::user_types::Interface* t_ifc) -> Any override;
  auto visit(node::user_types::MemberDecl* t_meth) -> Any override;
  auto visit(node::user_types::Struct* t_struct) -> Any override;
  auto visit(node::user_types::Self* t_self) -> Any override;
  auto visit(node::user_types::MemberAccess* t_dot_expr) -> Any override;

  // Misc:
  auto visit(node::List* t_list) -> Any override;

  // Implementation:
  //! Return all the SSA vars that are needed for a call.
  auto get_call_args(node::NodeListPtr t_list) -> LocalVarVec;

  //! Translate the AST to a CLIR module.
  auto translate(NodePtr t_ast) -> ModulePtr;

  virtual ~MirBuilder() = default;
};


} // namespace mir::mir_builder

#endif // ACRIS_ACRIS_MIR_MIR_BUILDER_MIR_BUILDER_HPP
