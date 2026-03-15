#include "switch_else.hpp"

namespace ast::node::control {
SwitchElse::SwitchElse(TextPosition t_pos, NodeListPtr&& t_body)
  : NodePosition{std::move(t_pos)}, Body{std::move(t_body)}
{}
} // namespace ast::node::control
