#ifndef ACRIS_ACRIS_AST_NODE_BUILTIN_TYPES_POINTER_HPP
#define ACRIS_ACRIS_AST_NODE_BUILTIN_TYPES_POINTER_HPP


// Includes:
#include "../node_traits/include.hpp"

// Local Includes:
#include "fdecl.hpp"

namespace ast::node::builtin_types {
// Aliases:
using container::TextPosition;
using node_traits::NodePosition;
using node_traits::UnaryOperator;

// Classes:
class Pointer : public NodePosition, public UnaryOperator {
  private:
  usz m_indirection;
  bool m_readonly;

  public:
  Pointer(TextPosition t_pos, NodePtr&& t_left, usz t_indirection,
          bool t_readonly);

	auto indirection() const -> usz;
	auto readonly() const -> bool;

  // AST_ARCHIVE_MAKE_TRAITS_ARCHIVEABLE(Pointer, NodePosition)
  AST_VISITOR_MAKE_VISITABLE(visitor::NodeVisitor);

  virtual ~Pointer() = default;
};
} // namespace ast::node::builtin_types

// Cereal type registration:
// REGISTER_ARCHIVEABLE_TYPE(ast::node::user_types, Pointer);

#endif // ACRIS_ACRIS_AST_NODE_BUILTIN_TYPES_POINTER_HPP
