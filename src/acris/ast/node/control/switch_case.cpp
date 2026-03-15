#include "switch_case.hpp"

namespace ast::node::control {
SwitchCase::SwitchCase(TextPosition t_pos, NodeListPtr&& t_clauses,
                       NodeListPtr&& t_body, bool t_has_fallthrough)
  : NodePosition{std::move(t_pos)},
    Body{std::move(t_body)},
    m_clauses{std::move(t_clauses)},
    m_has_fallthrough{t_has_fallthrough}
{}

auto SwitchCase::clauses() const -> NodeListPtr
{
  return m_clauses;
}

auto SwitchCase::has_fallthrough() const -> bool
{
  return m_has_fallthrough;
}
} // namespace ast::node::control
