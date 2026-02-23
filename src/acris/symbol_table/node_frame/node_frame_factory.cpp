#include "node_frame_factory.hpp"

// Absolute Includes:
#include "acris/ast/node/include_nodes.hpp"

namespace symbol_table::node_frame {
NODE_USING_ALL_NAMESPACES()

// Public methods:
// FIXME: We just lazily reserve 100 entries for now in the register.
NodeFrameFactory::NodeFrameFactory()
  : NodeVisitor{}, m_tree_factory{}, m_register{100}
{}

// Control:
auto NodeFrameFactory::visit(If* t_if) -> Any
{
  const auto then{t_if->then()};
  const auto alt{t_if->alt()};

  traverse(then);

  if(alt) {
    traverse(alt);
  }

  return {};
}

auto NodeFrameFactory::visit(Loop* t_loop) -> Any
{
  const auto body{t_loop->body()};

  traverse(body);

  return {};
}

AST_VISITOR_STUB(NodeFrameFactory, Continue)
AST_VISITOR_STUB(NodeFrameFactory, Break)
AST_VISITOR_STUB(NodeFrameFactory, Defer)
AST_VISITOR_STUB(NodeFrameFactory, Return)

// Function:
auto NodeFrameFactory::visit(Parameter* t_param) -> Any
{
  const auto id{t_param->identifier()};

  // const auto symbol_id{m_tree_factory.insert(id)};
  // m_register.insert(symbol_id, t_param);

  return {};
}

auto NodeFrameFactory::visit(Function* t_fn) -> Any
{
  const auto id{t_fn->identifier()};

  const auto params{t_fn->params()};
  const auto body{t_fn->body()};

  // const auto symbol_id{m_tree_factory.insert(id)};
  // m_register.insert(symbol_id, t_fn);

  push_scope();
  // Add parameters to nested scope.
  traverse(params);

  // Add body in nested scope.
  traverse(body);
  pop_scope();

  return {};
}

AST_VISITOR_STUB(NodeFrameFactory, FunctionCall)
AST_VISITOR_STUB(NodeFrameFactory, ReturnType)

// Lvalue:
auto NodeFrameFactory::visit(Let* t_let) -> Any
{
  const auto id{t_let->identifier()};

  // const auto symbol_id{m_tree_factory.insert(id)};
  // m_register.insert(symbol_id, t_let);

  return {};
}

auto NodeFrameFactory::visit(Var* t_var) -> Any
{
  const auto id{t_var->identifier()};

  // const auto symbol_id{m_tree_factory.insert(id)};
  // m_register.insert(symbol_id, t_var);

  return {};
}

AST_VISITOR_STUB(NodeFrameFactory, IdentifierNode)

// Operators:
AST_VISITOR_STUB(NodeFrameFactory, Arithmetic)
AST_VISITOR_STUB(NodeFrameFactory, Assignment)
AST_VISITOR_STUB(NodeFrameFactory, Comparison)

AST_VISITOR_STUB(NodeFrameFactory, Increment)
AST_VISITOR_STUB(NodeFrameFactory, Decrement)

AST_VISITOR_STUB(NodeFrameFactory, UnaryPrefix)

// Logical:
AST_VISITOR_STUB(NodeFrameFactory, Not)
AST_VISITOR_STUB(NodeFrameFactory, And)
AST_VISITOR_STUB(NodeFrameFactory, Or)

// Packaging:
AST_VISITOR_STUB(NodeFrameFactory, Import)
AST_VISITOR_STUB(NodeFrameFactory, ModuleDecl)

// Rvalue:
AST_VISITOR_STUB(NodeFrameFactory, Float)
AST_VISITOR_STUB(NodeFrameFactory, Integer)
AST_VISITOR_STUB(NodeFrameFactory, String)
AST_VISITOR_STUB(NodeFrameFactory, Boolean)

// User Types:
AST_VISITOR_STUB(NodeFrameFactory, Method)
AST_VISITOR_STUB(NodeFrameFactory, Interface)
AST_VISITOR_STUB(NodeFrameFactory, MemberDecl)
AST_VISITOR_STUB(NodeFrameFactory, Struct)
AST_VISITOR_STUB(NodeFrameFactory, Self)
AST_VISITOR_STUB(NodeFrameFactory, MemberAccess)

auto NodeFrameFactory::visit(node::List* t_list) -> Any
{
  // TODO: Consider using std::enable_shared_from_this.

  const auto insert{[&](const std::string_view t_id, const NodePtr& t_node) {
    const auto symbol_id{m_tree_factory.insert(t_id)};
    m_register.insert(symbol_id, t_node);
  }};

  // Extract shared_ptr's prematurely, before visitation.
  for(NodePtr& node : *t_list) {
    if(auto ptr{std::dynamic_pointer_cast<Parameter>(node)}; ptr) {
      const auto id{ptr->identifier()};

      insert(id, ptr);
    } else if(auto ptr{std::dynamic_pointer_cast<Function>(node)}; ptr) {
      const auto id{ptr->identifier()};

      insert(id, ptr);

      const auto body{ptr->body()};
      traverse(body);
    } else if(auto ptr{std::dynamic_pointer_cast<Let>(node)}; ptr) {
      const auto id{ptr->identifier()};

      insert(id, ptr);
    } else if(auto ptr{std::dynamic_pointer_cast<Var>(node)}; ptr) {
      const auto id{ptr->identifier()};

      insert(id, ptr);
    } else {
      traverse(node);
    }
  }

  return {};
}

auto NodeFrameFactory::push_scope() -> void
{
  m_tree_factory.push_scope();
}

auto NodeFrameFactory::pop_scope() -> void
{
  m_tree_factory.pop_scope();
}

auto NodeFrameFactory::construct_frame(NodePtr t_ast) -> NodeFramePtr
{
  m_tree_factory.clear();

  // Construct the frame.
  traverse(t_ast);

  const auto tree{m_tree_factory.retrieve()};
  const auto node_frame{std::make_shared<NodeFrame>(tree, m_register)};

  return node_frame;
}
} // namespace symbol_table::node_frame
