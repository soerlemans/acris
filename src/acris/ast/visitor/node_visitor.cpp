#include "node_visitor.hpp"

// STL Includes:
#include <cassert>

// Absolute Includes:
#include "acris/ast/node/include_nodes.hpp"
#include "acris/debug/log.hpp"
#include "lib/check_nullptr.hpp"
#include "lib/stdexcept/stdexcept.hpp"


// Macros:
// TODO: Place somewhere appropriate
#define assert_msg(t_msg, t_expr) assert(((void)t_msg, t_expr))

#define STUB(t_type)                                              \
  auto NodeVisitor::visit([[maybe_unused]] t_type* t_ptr) -> Any  \
  {                                                               \
    DBG_CRITICAL("Method for type ", #t_type, " not overriden!"); \
                                                                  \
    return {};                                                    \
  }

namespace ast::visitor {
// Using:
NODE_USING_ALL_NAMESPACES()

// Methods:
//! Traverse all nodes neatly
auto NodeVisitor::traverse(NodePtr t_node) -> Any
{
  Any any{};

  // We cant traverse a nullptr.
  CHECK_NULLPTR(t_node);

  // Kick off the traversal.
  any = t_node->accept(this);

  return any;
}

// Stubs:
STUB(If)
STUB(Loop)
STUB(Switch)
STUB(SwitchCase)
STUB(SwitchElse)
STUB(Fallthrough)

STUB(Continue)
STUB(Break)
STUB(Defer)
STUB(Return)
STUB(Parameter)
STUB(Function)
STUB(FunctionCall)
STUB(ReturnType)

// LValue:
STUB(Let)
STUB(Var)
STUB(IdentifierNode)
STUB(Subscript)
STUB(ScopeResolution)

// Meta:
STUB(Attribute)
STUB(LetDecl)
STUB(VarDecl)
STUB(FunctionDecl)

// BinaryOperator:
STUB(Arithmetic)
STUB(Assignment)
STUB(Comparison)
STUB(Increment)
STUB(Decrement)
STUB(AddressOf)
STUB(Dereference)
STUB(UnaryPrefix)
STUB(ToCast)

STUB(Not)
STUB(And)
STUB(Or)
STUB(Ternary)
STUB(Import)
STUB(ModuleDecl)
STUB(Float)
STUB(Integer)
STUB(Char)
STUB(String)
STUB(ArrayExpr)
STUB(Boolean)

// Builtin Types:
STUB(Pointer)
STUB(Array)
STUB(TypeName)

// User Types:
STUB(EnumField)
STUB(Enum)
STUB(Method)
STUB(MethodCall)
STUB(Interface)
STUB(MemberDecl)
STUB(Struct)
STUB(Self)
STUB(Member)
STUB(MemberAccess)

// Misc:
auto NodeVisitor::visit(List* t_list) -> Any
{
  for(NodePtr& node : *t_list) {
    traverse(node);
  }

  return {};
}

auto NodeVisitor::visit([[maybe_unused]] Nil* t_nil) -> Any
{
  DBG_INFO("Visited a Nil node");

  return {};
}

//! This catches the error case where a node does not have its own method
auto NodeVisitor::visit([[maybe_unused]] NodeInterface* t_ptr) -> Any
{
  assert_msg("NodeVisitor: Received a NodeInterface*", t_ptr);

  return {};
}
} // namespace ast::visitor
