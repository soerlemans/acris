#ifndef SWITCH_CASE_HPP
#define SWITCH_CASE_HPP

// Includes:
#include "../node_traits/include.hpp"

// Local Includes:
#include "fdecl.hpp"

namespace ast::node::control {
// Using Statements:
using container::TextPosition;
using node_traits::Body;
using node_traits::NodePosition;

// Classes:
class SwitchCase : public NodePosition, public Body {
  private:
  NodeListPtr m_clauses;
  bool m_has_fallthrough;

  public:
  SwitchCase(TextPosition t_pos, NodeListPtr&& t_clauses, NodeListPtr&& t_body,
             bool t_has_fallthrough);

  auto clauses() const -> NodeListPtr;
  auto has_fallthrough() const -> bool;

  // AST_ARCHIVE_MAKE_TRAITS_ARCHIVEABLE(SwitchCase, NodePosition, Condition)
  AST_VISITOR_MAKE_VISITABLE(visitor::NodeVisitor);

  virtual ~SwitchCase() = default;
};
} // namespace ast::node::control

// Cereal type registration:
// REGISTER_ARCHIVEABLE_TYPE(ast::node::control, SwitchCase);

#endif // SWITCH_CASE_HPP
