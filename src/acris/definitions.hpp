#ifndef ACRIS_VERSION_HPP
#define ACRIS_VERSION_HPP

// STL Includes:
#include <string_view>


// Macros:
//! Cmake substitutes the version numbers specified in the root CMakeLists.txt.
#define ACRIS_PROJECT_VERSION_MAJOR @acris_VERSION_MAJOR @
#define ACRIS_PROJECT_VERSION_MINOR @acris_VERSION_MINOR @
#define ACRIS_PROJECT_VERSION_PATCH @acris_VERSION_PATCH @

#define ACRIS_PROJECT_VERSION "0.1.0"

//! Absolute path to the root of the acris project.
#define ACRIS_PROJECT_ROOT_DIRECTORY "/home/hackerman/.bmarks/acris"

//! Absolute path to the source directory of the acris project.
#define ACRIS_PROJECT_SRC_DIRECTORY "/home/hackerman/.bmarks/acris/src"

//! Absolute path to the test directory of the acris project.
#define ACRIS_PROJECT_TEST_DIRECTORY "/home/hackerman/.bmarks/acris/test"

/*!
 * Convert @ref __FILE__ into a relative path.
 * If this is not possible return the absolute path.
 */
#define ACRIS_PROJECT_RELATIVE_PATH                                           \
  ([]() consteval noexcept {                                                 \
    constexpr std::string_view file{__FILE__};                               \
    constexpr std::string_view acris_src_dir{ACRIS_PROJECT_SRC_DIRECTORY "/"}; \
                                                                             \
    if constexpr(file.starts_with(acris_src_dir)) {                           \
      constexpr std::string_view relative_path{                              \
        file.substr(acris_src_dir.size())};                                   \
                                                                             \
      return relative_path;                                                  \
    }                                                                        \
                                                                             \
    return file;                                                             \
  }())

/*!
 * Makes a path absolute from the acris project root.
 *
 * @param[in] t_in .
 */
#define ACRIS_PROJECT_ROOT_ABSOLUTE_PATH(t_path)                               \
  ([]() consteval noexcept {                                                  \
    constexpr std::string_view path{t_path};                                  \
    constexpr std::string_view acris_src_dir{ACRIS_PROJECT_ROOT_DIRECTORY "/"}; \
                                                                              \
    return acris_src_dir + path;                                               \
  }())

#endif // ACRIS_PROJECT_VERSION_HPP
