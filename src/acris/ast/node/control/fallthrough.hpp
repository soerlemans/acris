#ifndef FALLTHROUGH_HPP
#define FALLTHROUGH_HPP

// Includes:
#include "../node_interface.hpp"

// Local Includes:
#include "fdecl.hpp"

namespace ast::node::control {
class Fallthrough : public NodeInterface {
  public:
  Fallthrough() = default;

  AST_ARCHIVE_DEFINE_SERIALIZE_METHOD_NIL()
  AST_VISITOR_MAKE_VISITABLE(visitor::NodeVisitor);

  virtual ~Fallthrough() = default;
};
} // namespace ast::node::control

// Cereal type registration:
REGISTER_ARCHIVEABLE_TYPE(ast::node::control, Fallthrough);


#endif // FALLTHROUGH_HPP
