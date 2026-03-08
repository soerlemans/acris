#include "to_cast.hpp"

namespace ast::node::operators {
// Methods:
ToCast::ToCast(NodePtr&& t_left): UnaryOperator{std::move(t_left)}
{}
} // namespace ast::node::operators
