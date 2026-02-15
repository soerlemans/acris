#include "toml.hpp"

// STL Includes:
#include <filesystem>
#include <functional>
#include <string_view>

// Library Includes:
#include <toml++/toml.hpp>

// Absolute Includes:
#include "acris/debug/log.hpp"
#include "acris/settings/enum_convert.hpp"
#include "acris/settings/settings.hpp"

// Internal:
namespace {
// Aliases:
namespace fs = std::filesystem;
using settings::Settings;
using settings::str2backend;
using settings::str2interopbackend;

template<typename T>
using NodeView = toml::node_view<T>;

template<typename T>
using Vec = std::vector<T>;

// /*!
//  * Quite a complex construction but basically extract any values
//  */
// template<typename NodeType, typename ElemType,
//          typename FnType = std::function<ElemType(std::string&)>>
// inline auto toml_extract(NodeView<NodeType>&& t_node,
//                          FnType t_transform = std::identity{}) -> ElemType
// {
//   using toml::array;

//   // Closure to extract values from an array.
//   // Reused for single values as well.
//   const auto closure{[&](auto&& t_elem) noexcept {
//     using toml::is_string;

//     if constexpr(is_string<decltype(t_elem)>) {
//       auto str{t_elem.get()};
//       auto elem{t_transform(str)};

//       t_vector.push_back(elem);
//     }
//   }};

//   // Type checking:
//   if(const auto opt{t_node.template value<std::string>()}; opt) {
//     auto str{opt.value()};
//     auto elem{t_transform(str)};

//     t_vector.push_back(elem);
//   } else {
//     DBG_WARNING("Toml invalid value for field.");
//   }
// }

// FIXME: Only supports strings now.
/*!
 * Quite a complex construction but basically extract any values from a field.
 * And throw them into a vector.
 */
template<typename NodeType, typename ElemType,
         typename FnType = std::function<ElemType(std::string&)>>
inline auto toml_extract(NodeView<NodeType>&& t_node, Vec<ElemType>& t_vector,
                         FnType t_transform = std::identity{}) -> void
{
  using toml::array;

  // Closure to extract values from an array.
  // Reused for single values as well.
  const auto closure{[&](auto&& t_elem) noexcept {
    using toml::is_string;

    if constexpr(is_string<decltype(t_elem)>) {
      auto str{t_elem.get()};
      auto elem{t_transform(str)};

      t_vector.push_back(elem);
    }
  }};

  // Type checking:
  if(const auto opt{t_node.template value<std::string>()}; opt) {
    auto str{opt.value()};
    auto elem{t_transform(str)};

    t_vector.push_back(elem);
  } else if(array* arr{t_node.as_array()}; arr) {
    arr->for_each(closure);
  } else {
    DBG_WARNING("Toml invalid value for field.");
  }
}

/*!
 * Helper function for extracting a TOML array that is the same type.
 * For now assume that every item is an array.
 */
template<typename NodeType, typename VecType>
auto toml_extract_array(toml::node_view<NodeType>&& t_node,
                        std::vector<VecType>& t_vector) -> void
{
  if(toml::array* arr = t_node.as_array(); arr) {
    const auto lambda{[&](auto&& t_elem) noexcept {
      if constexpr(toml::is_string<decltype(t_elem)>) {
        auto elem{t_elem.get()};

        DBG_INFO("Toml extracted: ", elem);
        t_vector.push_back(elem);
      } else {
        DBG_WARNING("Non string value in toml array.");
      }
    }};

    arr->for_each(lambda);
  } else {
    DBG_WARNING("Toml array does not exist");
  }
}

auto toml_project_section(toml::table& t_table, Settings& t_settings) -> void
{
  using namespace std::literals;
  using toml::is_string;

  auto project{t_table["project"]};

  auto to_path{[](std::string& t_elem) {
    return fs::path{t_elem};
  }};

  toml_extract(project["sources"], t_settings.m_source_paths, to_path);

  // t_settings.m_backend = project["backend"].value_or("cpp"sv);
  if(auto backend{t_table["backend"].value<std::string>()}; backend) {
    t_settings.m_backend = str2backend(backend.value());
  }

  // Extract interop backend, by enum.
  toml_extract(project["interop_backends"], t_settings.m_ibackends,
               str2interopbackend);
}

auto toml_debug_section(toml::table& t_table, Settings& t_settings) -> void
{
  auto debug{t_table["debug"]};
}
} // namespace

namespace settings {
// Functions:
auto find_settings_toml(const std::string_view t_filename) -> PathOpt
{
  using fs::exists;
  using fs::is_regular_file;

  PathOpt path{};
  const auto filename_quoted{std::quoted(t_filename)};

  fs::path current{fs::current_path()};
  while(true) {
    auto candidate{current / t_filename};
    if(exists(candidate) && is_regular_file(candidate)) {
      DBG_INFO("Found ", filename_quoted, " at ", candidate, ".");

      path = candidate;
      break;
    }

    // Check if we are at the root directory.
    if(current == current.root_path()) {
      DBG_VERBOSE("Could not find ", filename_quoted);
      break;
    }

    // Check parent path in next iteration.
    current = current.parent_path();
  }

  return path;
}

auto read_settings_toml(const fs::path t_filepath, Settings& t_settings) -> void
{
  using toml::parse_error;
  using namespace std::literals;

  try {
    auto table{toml::parse_file(t_filepath.native())};

    toml_project_section(table, t_settings);
    toml_debug_section(table, t_settings);

    // config["project"]["sources"]
    //  std::cout << config["debug"]["loglevel"].value_or("empty"sv);
  } catch(const parse_error& err) {
    const auto description{err.description()};

    DBG_ERROR("Failed to parse ", t_filepath, " results in ", description);

    // Rethrow the error to handle properly.
    throw err;
  }
}
} // namespace settings
