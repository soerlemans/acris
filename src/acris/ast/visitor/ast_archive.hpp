#ifndef ACRIS_ACRIS_AST_VISITOR_AST_ARCHIVE_HPP
#define ACRIS_ACRIS_AST_VISITOR_AST_ARCHIVE_HPP

// STL Includes:
#include <variant>

// Absolute Includes:
#include "lib/overload.hpp"
#include "lib/stdexcept/stdexcept.hpp"
#include "lib/stdtypes.hpp"

// Relative AST Includes:
#include "../node/include_nodes.hpp"

// Local Includes:
#include "node_visitor.hpp"

namespace ast::visitor {
// Using Statements;
using cereal::BinaryInputArchive;
using cereal::BinaryOutputArchive;
using cereal::JSONInputArchive;
using cereal::JSONOutputArchive;
using cereal::PortableBinaryInputArchive;
using cereal::PortableBinaryOutputArchive;
using cereal::XMLInputArchive;
using cereal::XMLOutputArchive;

// Aliases:
// clang-format off
using Archive =
  std::variant<std::monostate,
     JSONOutputArchive, JSONInputArchive,
     XMLOutputArchive, XMLInputArchive,
     BinaryOutputArchive, BinaryInputArchive,
     PortableBinaryOutputArchive, PortableBinaryInputArchive>;
// clang-format on

// Enums:
enum class ArchiveType {
  JSON,
  XML,
  BINARY,
  PORTABLE_BINARY
};

// Classes:
/*!
 * Utility class for Serializing and deserializing the AST.
 * Useful for when compiling multiple files to keep memory footprint low by
 * serializing an AST to disk. Also commonly used for inspecting the AST.
 */
class AstArchive : public NodeVisitor {
  private:
  ArchiveType m_type;
  Archive m_archive;

  protected:
  auto set_archive_out(ArchiveType t_type, std::ostream& t_os) -> void;
  auto set_archive_in(ArchiveType t_type, std::istream& t_is) -> void;

  /*!
   * Helper method for archiving AST nodes.
   * Throws if @ref m_archive is std::monostate.
   *
   * @param[in] t_args Variadic argument of AST nodes to archive.
   *
   */
  template<typename... Args>
  auto archive(Args&&... t_args) -> void
  {
    using lib::Overload;

    const auto fn_archive{[&](auto&& t_archive) {
      t_archive(std::forward<Args>(t_args)...);
    }};

    const auto fn_monostate{[]([[maybe_unused]] std::monostate t_state) {
      lib::stdexcept::throw_exception("Attempted to archive std::monostate.");
    }};

    std::visit(Overload{fn_archive, fn_monostate}, m_archive);
  }

  public:
  AstArchive(ArchiveType t_type);

  // Control:
  auto visit(node::control::If* t_if) -> Any override;
  auto visit(node::control::Loop* t_loop) -> Any override;
  auto visit(node::control::Continue* t_continue) -> Any override;
  auto visit(node::control::Break* t_break) -> Any override;
  auto visit(node::control::Defer* t_defer) -> Any override;
  auto visit(node::control::Return* t_ret) -> Any override;

  // Function:
  auto visit(node::function::Parameter* t_param) -> Any override;
  auto visit(node::function::Function* t_fn) -> Any override;
  auto visit(node::function::FunctionCall* t_fn_call) -> Any override;
  auto visit(node::function::ReturnType* t_rt) -> Any override;

  // Lvalue:
  auto visit(node::lvalue::Let* t_let) -> Any override;
  auto visit(node::lvalue::Var* t_var) -> Any override;
  auto visit(node::lvalue::IdentifierNode* t_id) -> Any override;

  // Meta:
  auto visit(node::meta::Attribute* t_attr) -> Any override;
  auto visit(node::meta::LetDecl* t_ldecl) -> Any override;
  auto visit(node::meta::VarDecl* t_vdecl) -> Any override;
  auto visit(node::meta::FunctionDecl* t_fdecl) -> Any override;

  // Operators:
  auto visit(node::operators::Arithmetic* t_arith) -> Any override;
  auto visit(node::operators::Assignment* t_assign) -> Any override;
  auto visit(node::operators::Comparison* t_comp) -> Any override;

  auto visit(node::operators::Increment* t_inc) -> Any override;
  auto visit(node::operators::Decrement* t_dec) -> Any override;

  auto visit(node::operators::UnaryPrefix* t_up) -> Any override;

  // Logical:
  auto visit(node::operators::Not* t_not) -> Any override;
  auto visit(node::operators::And* t_and) -> Any override;
  auto visit(node::operators::Or* t_or) -> Any override;

  auto visit(node::operators::Ternary* t_ternary) -> Any override;

  // Packaging:
  auto visit(node::packaging::Import* t_import) -> Any override;
  auto visit(node::packaging::ModuleDecl* t_mod) -> Any override;

  // Rvalue:
  auto visit(node::rvalue::Float* t_float) -> Any override;
  auto visit(node::rvalue::Integer* t_int) -> Any override;
  auto visit(node::rvalue::String* t_str) -> Any override;
  auto visit(node::rvalue::Boolean* t_bool) -> Any override;

  // User Types:
  auto visit(node::user_types::Method* t_meth) -> Any override;
  auto visit(node::user_types::Interface* t_ifc) -> Any override;
  auto visit(node::user_types::MemberDecl* t_meth) -> Any override;
  auto visit(node::user_types::Struct* t_struct) -> Any override;
  auto visit(node::user_types::Self* t_self) -> Any override;
  auto visit(node::user_types::Member* t_member) -> Any override;
  auto visit(node::user_types::MemberAccess* t_access) -> Any override;

  // Misc:
  auto visit(node::List* t_list) -> Any override;
  auto visit(node::Nil* t_nil) -> Any override;

  auto out(NodePtr& t_ast, std::ostream& t_os) -> void;
  auto in(NodePtr& t_ast, std::istream& t_is) -> void;

  virtual ~AstArchive() = default;
};
} // namespace ast::visitor

#endif // ACRIS_ACRIS_AST_VISITOR_AST_ARCHIVE_HPP
