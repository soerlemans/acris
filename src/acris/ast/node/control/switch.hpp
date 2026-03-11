#ifndef SWITCH_HPP
#define SWITCH_HPP

// Includes:
#include "../node_traits/include.hpp"

// Local Includes:
#include "fdecl.hpp"

namespace ast::node::control {
// Using Statements:
using container::TextPosition;
using node_traits::Condition;
using node_traits::NodePosition;

// Classes:
class Switch : public NodePosition, public Condition {
  public:
  Switch(TextPosition t_pos, NodePtr&& t_condition);

  // AST_ARCHIVE_MAKE_TRAITS_ARCHIVEABLE(Switch, NodePosition, Condition)
  AST_VISITOR_MAKE_VISITABLE(visitor::NodeVisitor);

  virtual ~Switch() = default;
};
} // namespace ast::node::control

// Cereal type registration:
// REGISTER_ARCHIVEABLE_TYPE(ast::node::control, Switch);

#endif // SWITCH_HPP
