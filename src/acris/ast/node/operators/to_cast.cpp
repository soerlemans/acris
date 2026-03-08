#include "to_cast.hpp"

namespace ast::node::operators {
// Methods:
ToCast::ToCast(TextPosition t_pos, NodePtr&& t_left)
  : NodePosition{std::move(t_pos)}, UnaryOperator{std::move(t_left)}
{}
} // namespace ast::node::operators
