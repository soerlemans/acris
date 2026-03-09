#include "enum_field.hpp"

namespace ast::node::user_types {
EnumField::EnumField(std::string_view t_identifier, NodePtr&& t_expr)
  : Identifier{t_identifier}, Expr{std::move(t_expr)}
{}
} // namespace ast::node::user_types
