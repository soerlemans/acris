#ifndef SWITCH_ELSE_HPP
#define SWITCH_ELSE_HPP

// Includes:
#include "../node_traits/include.hpp"

// Local Includes:
#include "fdecl.hpp"

// TODO: Add enum for unreachable and panic, or possibly use std::variant.

namespace ast::node::control {
// Using Statements:
using container::TextPosition;
using node_traits::Body;
using node_traits::NodePosition;

// Classes:
class SwitchElse : public NodePosition, public Body {
  public:
  SwitchElse(TextPosition t_pos, NodeListPtr&& t_body);

  // AST_ARCHIVE_MAKE_TRAITS_ARCHIVEABLE(SwitchElse, NodePosition, Body)
  AST_VISITOR_MAKE_VISITABLE(visitor::NodeVisitor);

  virtual ~SwitchElse() = default;
};
} // namespace ast::node::control

// Cereal type registration:
// REGISTER_ARCHIVEABLE_TYPE(ast::node::control, SwitchElse);

#endif // SWITCH_ELSE_HPP
