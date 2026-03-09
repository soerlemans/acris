#ifndef ACRIS_ACRIS_SYMBOL_TABLE_NODE_FRAME_NODE_FRAME_FACTORY_HPP
#define ACRIS_ACRIS_SYMBOL_TABLE_NODE_FRAME_NODE_FRAME_FACTORY_HPP

// Absolute Includes:
#include "acris/symbol_table/symbol_tree/symbol_tree_factory.hpp"

// Local Includes:
#include "node_frame.hpp"

namespace symbol_table::node_frame {
// Using:
using ast::node::NodePtr;
using ast::visitor::Any;
using ast::visitor::NodeVisitor;
using symbol_tree::SymbolTreeFactory;
using symbol_tree::SymbolTreePtr;
using symbol_tree::SymbolTreeResult;

namespace node = ast::node;

// Classes:
class NodeFrameFactory : public NodeVisitor {
  private:
  SymbolTreeFactory m_tree_factory;
  NodeRegister m_register;

  public:
  NodeFrameFactory();

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
  auto visit(node::lvalue::IdentifierNode* t_id) -> Any override;

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

  // auto visit(node::operators::Ternary* t_ternary) -> Any override;

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
  virtual auto visit(node::List* t_list) -> Any override;

  auto push_scope() -> void;
  auto pop_scope() -> void;

  auto construct_frame(NodePtr t_ast) -> NodeFramePtr;

  virtual ~NodeFrameFactory() = default;
};
} // namespace symbol_table::node_frame

#endif // ACRIS_ACRIS_SYMBOL_TABLE_NODE_FRAME_NODE_FRAME_FACTORY_HPP
