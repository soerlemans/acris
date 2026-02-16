#ifndef ENUM_FIELD_HPP
#define ENUM_FIELD_HPP

// Includes:
#include "../node_traits/include.hpp"

// Local Includes:
#include "fdecl.hpp"

namespace ast::node::user_types {
// Using Statements:
using node_traits::Expr;
using node_traits::Identifier;
using node_traits::TypeData;

// Classes:
class EnumField : public Identifier, public Expr, public TypeData {
  public:
  EnumField(std::string_view t_identifier, NodePtr&& t_expr);

  AST_ARCHIVE_MAKE_TRAITS_ARCHIVEABLE(EnumField, Identifier, Expr)
  AST_VISITOR_MAKE_VISITABLE(visitor::NodeVisitor);

  virtual ~EnumField() = default;
};
} // namespace ast::node::user_types

// Cereal type registration:
REGISTER_ARCHIVEABLE_TYPE(ast::node::user_types, EnumField);

#endif // ENUM_FIELD_HPP
