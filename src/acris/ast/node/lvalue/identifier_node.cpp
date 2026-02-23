#include "identifier_node.hpp"

namespace ast::node::lvalue {
// Methods:
IdentifierNode::IdentifierNode(TextPosition t_pos,
                               const std::string_view t_identifier)
  : NodePosition{std::move(t_pos)}, Identifier{t_identifier}
{}
} // namespace ast::node::lvalue
