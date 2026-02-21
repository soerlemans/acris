#include "enum.hpp"

namespace ast::node::user_types {
Enum::Enum(const std::string_view t_identifier, NodePtr&& t_annot,
           NodeListPtr&& t_body)
  : Identifier{t_identifier},
    TypeAnnotation{std::move(t_annot)},
    Body{std::move(t_body)},
    TypeData{},
    AttributeData{}
{}
} // namespace ast::node::user_types
