#ifndef SCOPE_RESOLUTION_NODE_HPP
#define SCOPE_RESOLUTION_NODE_HPP

// STL Includes:
#include <vector>

// Includes:
#include "../node_traits/include.hpp"

// Local Includes:
#include "fdecl.hpp"

namespace ast::node::lvalue {
// Using Statements:
using container::TextPosition;
using node_traits::Expr;
using node_traits::NodePosition;
using node_traits::TypeData;

// TODO: Exchange with some kind of node, so we can handle generic expressions.
using ScopeResolutionPath = std::vector<std::string>;

// Classes:
class ScopeResolution : public NodePosition, public Expr, public TypeData {
  private:
  ScopeResolutionPath m_path;

  public:
  ScopeResolution(TextPosition t_pos, ScopeResolutionPath&& t_path,
                  NodePtr&& t_expr);

  auto path() const -> const ScopeResolutionPath&;

  // AST_ARCHIVE_MAKE_TRAITS_ARCHIVEABLE(ScopeResolution, Node)
  AST_VISITOR_MAKE_VISITABLE(visitor::NodeVisitor);

  virtual ~ScopeResolution() = default;
};
} // namespace ast::node::lvalue

// Cereal type registration:
// REGISTER_ARCHIVEABLE_TYPE(ast::node::lvalue, ScopeResolution);


#endif // SCOPE_RESOLUTION_NODE_HPP
