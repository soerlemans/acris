#ifndef ENUM_HPP
#define ENUM_HPP

// STL Includes:
#include <string_view>

// Includes:
#include "../node_traits/include.hpp"

// Local Includes:
#include "fdecl.hpp"

namespace ast::node::user_types {
// Using Statements:
using node_traits::AttributeData;
using node_traits::Body;
using node_traits::Identifier;
using node_traits::TypeAnnotation;
using node_traits::TypeData;

// Classes:
class Enum : public Identifier,
             public Body,
             public TypeAnnotation,
             public AttributeData,
             public TypeData {
  public:
  Enum(std::string_view t_identifier, NodeListPtr&& t_body);

  AST_ARCHIVE_MAKE_TRAITS_ARCHIVEABLE(Enum, Identifier, Body, TypeAnnotation)
  AST_VISITOR_MAKE_VISITABLE(visitor::NodeVisitor);

  virtual ~Enum() = default;
};
} // namespace ast::node::user_types

// Cereal type registration:
REGISTER_ARCHIVEABLE_TYPE(ast::node::user_types, Enum);

#endif // ENUM_HPP
