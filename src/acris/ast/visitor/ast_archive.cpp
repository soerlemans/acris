#include "ast_archive.hpp"

// Absolute Includes:
#include "acris/debug/log.hpp"
#include "lib/stdexcept/stdexcept.hpp"


/*!
 * Defines a @ref AstArchive method, that will call,
 * @ref AstArchive::archive() on its argument.
 *
 * @param[in] t_type Type of Node to accept.
 */
#define DEFINE_SERIALIZE_METHOD(t_type)                         \
  auto AstArchive::visit([[maybe_unused]] t_type* t_ptr) -> Any \
  {                                                             \
    archive(*t_ptr);                                            \
                                                                \
    return {};                                                  \
  }

namespace ast::visitor {
// Using statements:
NODE_USING_ALL_NAMESPACES()

// Methods:
// Protected Methods:
auto AstArchive::set_archive_out(const ArchiveType t_type, std::ostream& t_os)
  -> void
{
  switch(t_type) {
    case ArchiveType::JSON:
      m_archive.emplace<JSONOutputArchive>(t_os);
      break;

    case ArchiveType::XML:
      m_archive.emplace<XMLOutputArchive>(t_os);
      break;

    case ArchiveType::BINARY:
      m_archive.emplace<BinaryOutputArchive>(t_os);
      break;

    case ArchiveType::PORTABLE_BINARY:
      m_archive.emplace<PortableBinaryOutputArchive>(t_os);
      break;

    default:
      lib::stdexcept::throw_invalid_argument(
        "Unrecognized output archive format.");
      break;
  }
}

auto AstArchive::set_archive_in(const ArchiveType t_type, std::istream& t_is)
  -> void
{
  switch(t_type) {
    case ArchiveType::JSON:
      m_archive.emplace<JSONInputArchive>(t_is);
      break;

    case ArchiveType::XML:
      m_archive.emplace<XMLInputArchive>(t_is);
      break;

    case ArchiveType::BINARY:
      m_archive.emplace<BinaryInputArchive>(t_is);
      break;

    case ArchiveType::PORTABLE_BINARY:
      m_archive.emplace<PortableBinaryInputArchive>(t_is);
      break;

    default:
      lib::stdexcept::throw_invalid_argument(
        "Unrecognized input archive format.");
      break;
  }
}

// Public: Methods:
AstArchive::AstArchive(const ArchiveType t_type)
  : m_type{t_type}, m_archive{std::monostate{}}
{}

// Control:
DEFINE_SERIALIZE_METHOD(If);
DEFINE_SERIALIZE_METHOD(Loop);
DEFINE_SERIALIZE_METHOD(Continue);
DEFINE_SERIALIZE_METHOD(Break);
DEFINE_SERIALIZE_METHOD(Defer);
DEFINE_SERIALIZE_METHOD(Return);

// Function:
DEFINE_SERIALIZE_METHOD(Parameter);
DEFINE_SERIALIZE_METHOD(Function);
DEFINE_SERIALIZE_METHOD(FunctionCall);
DEFINE_SERIALIZE_METHOD(ReturnType);

// Lvalue:
DEFINE_SERIALIZE_METHOD(Let);
DEFINE_SERIALIZE_METHOD(Var);
DEFINE_SERIALIZE_METHOD(IdentifierNode);

// Meta:
auto AstArchive::visit([[maybe_unused]] Attribute* t_attr) -> Any
{
  // archive(*t_ptr);

  // TODO: Implement.
  RUNTIME_ERROR("Attribute node not supported for serialization yet.");

  return {};
}

// DEFINE_SERIALIZE_METHOD(Attribute);
DEFINE_SERIALIZE_METHOD(LetDecl);
DEFINE_SERIALIZE_METHOD(VarDecl);
DEFINE_SERIALIZE_METHOD(FunctionDecl);

// Operators:
DEFINE_SERIALIZE_METHOD(Arithmetic);
DEFINE_SERIALIZE_METHOD(Assignment);
DEFINE_SERIALIZE_METHOD(Comparison);
DEFINE_SERIALIZE_METHOD(Increment);
DEFINE_SERIALIZE_METHOD(Decrement);
DEFINE_SERIALIZE_METHOD(UnaryPrefix);

// Logical:
DEFINE_SERIALIZE_METHOD(Not);
DEFINE_SERIALIZE_METHOD(And);
DEFINE_SERIALIZE_METHOD(Or);
DEFINE_SERIALIZE_METHOD(Ternary);

// Packaging:
DEFINE_SERIALIZE_METHOD(Import);
DEFINE_SERIALIZE_METHOD(ModuleDecl);

// Rvalue:
DEFINE_SERIALIZE_METHOD(Float);
DEFINE_SERIALIZE_METHOD(Integer);
DEFINE_SERIALIZE_METHOD(String);
DEFINE_SERIALIZE_METHOD(Boolean);

// User Types:
DEFINE_SERIALIZE_METHOD(Method);
DEFINE_SERIALIZE_METHOD(Interface);
DEFINE_SERIALIZE_METHOD(MemberDecl);
DEFINE_SERIALIZE_METHOD(Struct);
DEFINE_SERIALIZE_METHOD(Self);
DEFINE_SERIALIZE_METHOD(Member);
DEFINE_SERIALIZE_METHOD(MemberAccess);

// Misc:
DEFINE_SERIALIZE_METHOD(List);
DEFINE_SERIALIZE_METHOD(Nil);

auto AstArchive::out(NodePtr& t_ast, std::ostream& t_os) -> void
{
  set_archive_out(m_type, t_os);

  archive(t_ast);

  m_archive = std::monostate{};
}

auto AstArchive::in(NodePtr& t_ast, std::istream& t_is) -> void
{
  set_archive_in(m_type, t_is);

  // We do not use traverse as this fails on nullptr.
  archive(t_ast);

  m_archive = std::monostate{};
}
} // namespace ast::visitor
