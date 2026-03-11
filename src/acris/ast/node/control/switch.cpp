#include "switch.hpp"

namespace ast::node::control {
Switch::Switch(TextPosition t_pos, NodePtr&& t_condition)
  : NodePosition{std::move(t_pos)}, Condition{std::move(t_condition)}
{}
} // namespace ast::node::control
