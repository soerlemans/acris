#include "pointer.hpp"

namespace ast::node::builtin_types {
// Methods:
Pointer::Pointer(TextPosition t_pos, NodePtr&& t_left, usz t_indirection,
                 bool t_readonly)
  : NodePosition{std::move(t_pos)},
    UnaryOperator{std::move(t_left)},
    m_indirection{t_indirection},
    m_readonly{t_readonly}
{}

auto Pointer::indirection() const -> usz
{
  return m_indirection;
}

auto Pointer::readonly() const -> bool
{
  return m_readonly;
}
} // namespace ast::node::builtin_types
