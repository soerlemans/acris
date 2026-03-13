#include "switch.hpp"

namespace ast::node::control {
Switch::Switch(TextPosition t_pos, NodePtr&& t_condition, NodeListPtr&& t_body)
  : NodePosition{std::move(t_pos)},
    Condition{std::move(t_condition)},
    Body{std::move(t_body)}
{}
} // namespace ast::node::control
