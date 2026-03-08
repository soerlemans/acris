#ifndef TO_TYPE_HPP
#define TO_TYPE_HPP

// Includes:
#include "../node_traits/include.hpp"

// Local Includes:
#include "fdecl.hpp"

namespace ast::node::operators {
// Using Statements:
using container::TextPosition;
using node_traits::NodePosition;
using node_traits::TypeData;
using node_traits::UnaryOperator;

// Classes:
class ToCast : public NodePosition, public UnaryOperator, public TypeData {
  public:
  ToCast(TextPosition t_pos, NodePtr&& t_left);

  AST_ARCHIVE_MAKE_TRAITS_ARCHIVEABLE(ToCast, UnaryOperator)
  AST_VISITOR_MAKE_VISITABLE(visitor::NodeVisitor);

  virtual ~ToCast() = default;
};
} // namespace ast::node::operators

// Cereal type registration:
// REGISTER_ARCHIVEABLE_TYPE(ast::node::operators, ToCast);

#endif // TO_TYPE_HPP
