#include "attribute_data.hpp"

// STL Includes:
#include <iomanip>

// Absolute Includes:
#include "lib/stdexcept/stdexcept.hpp"
#include "lib/stdprint.hpp"

// Macros:
#define MATCH(t_key, t_value) \
  case AttributeType::t_key:  \
    str = t_value;            \
    break

namespace ast::node::node_traits {
AttributeMetadata::AttributeMetadata()
  : m_type{AttributeType::NO_ATTRIBUTE}, m_identifier{}, m_args{}
{}

AttributeMetadata::AttributeMetadata(const std::string_view t_identifier,
                                     AttributeArgs&& t_args)
  : m_type{str2attribute_type(t_identifier)},
    m_identifier{std::string{t_identifier}},
    m_args{std::move(t_args)}
{}

// Methods:
AttributeData::AttributeData(): m_attr_seq{}
{}

auto AttributeData::add_attribute(const AttributeMetadata& t_attr) -> void
{
  m_attr_seq.push_back(t_attr);
}

auto AttributeData::add_attribute(std::string_view t_identifier,
                                  AttributeArgs t_args) -> void
{
  AttributeMetadata attr{t_identifier, std::move(t_args)};

  m_attr_seq.push_back(std::move(attr));
}

auto AttributeData::get_attributes() const -> const AttributeSeq&
{
  return m_attr_seq;
}

auto AttributeData::no_attributes() const -> bool
{
  return m_attr_seq.empty();
}

// Functions:
auto attribute_type2str(AttributeType t_type) -> std::string_view
{
  std::string_view str{};

  switch(t_type) {
    MATCH(NO_ATTRIBUTE, "no_attribute");
    MATCH(INLINE, "inline");
    MATCH(DEPRECATED, "deprecated");
    MATCH(EXTERN, "extern");
    MATCH(EXPORT, "export");
    MATCH(UNKNOWN, "unknown");

    default: {
      using lib::stdexcept::throw_invalid_argument;

      throw_invalid_argument(
        "Unhandled AttributeType cant be converted to string.");
      break;
    }
  }

  return str;
}

auto str2attribute_type(std::string_view t_str) -> AttributeType
{
  auto type{AttributeType::NO_ATTRIBUTE};

  if(t_str.empty()) {
    // An empty string means no attribute is provided.
    return type;
  }

  // Builtin attributes:
  if(t_str == "inline") {
    type = AttributeType::INLINE;
  } else if(t_str == "deprecated") {
    type = AttributeType::DEPRECATED;
  } else if(t_str == "export") {
    type = AttributeType::EXPORT;
  } else if(t_str == "extern") {
    type = AttributeType::EXTERN;
  } else {
    // If we dont recognize, any of the builtin types
    type = AttributeType::UNKNOWN;
  }

  return type;
}
} // namespace ast::node::node_traits

// Functions:
auto operator<<(std::ostream& t_os,
                const ast::node::node_traits::AttributeType t_type)
  -> std::ostream&
{
  using ast::node::node_traits::attribute_type2str;

  t_os << attribute_type2str(t_type);

  return t_os;
}

auto operator<<(std::ostream& t_os,
                const ast::node::node_traits::AttributeMetadata& t_data)
  -> std::ostream&
{
  const auto& [type, id, args] = t_data;

  t_os << "(" << type << ":" << std::quoted(id) << ")";

  return t_os;
}

auto operator<<(std::ostream& t_os,
                const ast::node::node_traits::AttributeSeq& t_seq)
  -> std::ostream&
{
  using lib::stdprint::detail::print_seq;

  return print_seq(t_os, t_seq);
}
