#include "scope_resolution.hpp"

namespace ast::node::lvalue {
ScopeResolution::ScopeResolution(TextPosition t_pos,
                                 ScopeResolutionPath&& t_path, NodePtr&& t_expr)
  : NodePosition{std::move(t_pos)},
    Expr{std::move(t_expr)},
    m_path{std::move(t_path)}
{}

auto ScopeResolution::path() const -> const ScopeResolutionPath&
{
  return m_path;
}
} // namespace ast::node::lvalue
