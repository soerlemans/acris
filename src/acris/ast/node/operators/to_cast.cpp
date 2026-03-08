#include "to_cast.hpp"

namespace ast::node::operators {
// Methods:
ToCast::ToCast(TextPosition t_pos, NodePtr&& t_left, NodePtr&& t_right)
  : NodePosition{std::move(t_pos)},
    BinaryOperator{std::move(t_left), std::move(t_right)}
{}
} // namespace ast::node::operators
