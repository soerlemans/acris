#include "enum.hpp"

namespace ast::node::user_types {
Enum::Enum(const std::string_view t_identifier, NodeListPtr&& t_body)
  : Identifier{t_identifier}, Body{std::move(t_body)}, TypeData{}
{}
} // namespace ast::node::user_types
