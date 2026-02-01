#ifndef ACRIS_ACRIS_AST_NODE_OPERATORS_ADDRESS_OF_HPP
#define ACRIS_ACRIS_AST_NODE_OPERATORS_ADDRESS_OF_HPP

// Absolute Includes:
#include "acris/token/token_type.hpp"

// Includes:
#include "../node_traits/include.hpp"

// Local Includes:
#include "fdecl.hpp"

namespace ast::node::operators {
// Using Statements:
using node_traits::TypeData;
using node_traits::UnaryOperator;

// Classes:
class AddressOf : public UnaryOperator, public TypeData {
  public:
  AddressOf(NodePtr&& t_left);

  AST_ARCHIVE_MAKE_TRAITS_ARCHIVEABLE(AddressOf, UnaryOperator)
  AST_VISITOR_MAKE_VISITABLE(visitor::NodeVisitor);

  virtual ~AddressOf() = default;
};
} // namespace ast::node::operators

// Cereal type registration:
REGISTER_ARCHIVEABLE_TYPE(ast::node::operators, AddressOf);

#endif // ACRIS_ACRIS_AST_NODE_OPERATORS_ADDRESS_OF_HPP
