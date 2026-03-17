#ifndef ACRIS_ACRIS_AST_NODE_NODE_TRAITS_ATTRIBUTE_DATA_HPP
#define ACRIS_ACRIS_AST_NODE_NODE_TRAITS_ATTRIBUTE_DATA_HPP

// STL Include:
#include <iosfwd>
#include <string>
#include <string_view>
#include <vector>

// Absolute Includes:
#include "acris/ast/node/node_interface.hpp"

// Local Includes:
#include "fdecl.hpp"

namespace ast::node::node_traits {
// Forward Declarations:
struct AttributeMetadata;

// Aliases:
using AttributeArgs = std::vector<std::string>;
using AttributeSeq = std::vector<AttributeMetadata>;

// Enums:
enum class AttributeType {
  // clang-format off
  NO_ATTRIBUTE, //!< If no attribute is given.

  // Common builtin
  INLINE,     //!< Enforce inling of function.
  DEPRECATED, //!< Warn when using a deprecated function, or error.
  EXPORT,     //!< Export a function to somewhere external.
  EXTERN,     //!< Define a symbol as being linked externally.

  UNKNOWN //!< Reserved for third party attributes.
	// clang-format off
};

// Structs:
/*!
 * Holds the metadata which we annotate the AST with.
 */
struct AttributeMetadata {
  //! Determines what behavior should be classified.
  AttributeType m_type;

  // Raw data.
  std::string m_identifier;
  AttributeArgs m_args;

  AttributeMetadata();
  explicit AttributeMetadata(std::string_view t_identifier,
                             AttributeArgs&& t_args);

  virtual ~AttributeMetadata() = default;
};

// Classes:
/*!
 * Class for annotating the AST with attribute information.
 * This is necessary, for when need the attribute information on hand.
 * When generating code, or translating to IR.
 */
class AttributeData : virtual public NodeInterface {
  private:
  AttributeSeq m_attr_seq;

  public:
  AttributeData();

  virtual auto add_attribute(const AttributeMetadata&) -> void;
  virtual auto add_attribute(std::string_view t_identifier,
                             AttributeArgs t_args) -> void;

  virtual auto get_attributes() const -> const AttributeSeq&;
  virtual auto no_attributes() const -> bool;

  // AST_ARCHIVE_MAKE_ARCHIVEABLE(AttributeData)
  // {
  //   t_archive(CEREAL_NVP(m_args));
  // }

  AST_VISITOR_VISITABLE_PURE_ACCEPT(visitor::NodeVisitor);

  virtual ~AttributeData() = default;
};

// Functions:
auto attribute_type2str(AttributeType t_type) -> std::string_view;
auto str2attribute_type(std::string_view t_str) -> AttributeType;
} // namespace ast::node::node_traits

// Functions:
auto operator<<(std::ostream& t_os,
                ast::node::node_traits::AttributeType t_type) -> std::ostream&;
auto operator<<(std::ostream& t_os,
                const ast::node::node_traits::AttributeMetadata& t_type)
  -> std::ostream&;
auto operator<<(std::ostream& t_os,
                const ast::node::node_traits::AttributeSeq& t_seq)
  -> std::ostream&;

// Cereal register type:
// REGISTER_ARCHIVEABLE_TYPE(ast::node::node_traits, AttributeData);

#endif // ACRIS_ACRIS_AST_NODE_NODE_TRAITS_ATTRIBUTE_DATA_HPP
