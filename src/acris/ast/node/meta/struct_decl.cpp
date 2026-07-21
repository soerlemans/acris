#include "struct_decl.hpp"

namespace ast::node::meta {
StructDecl::StructDecl(const std::string_view t_identifier)
  : Identifier{t_identifier}, AttributeData{}
{}
} // namespace ast::node::meta
