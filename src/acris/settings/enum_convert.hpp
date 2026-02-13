#ifndef ACRIS_ACRIS_SETTINGS_ENUM_CONVERT_HPP
#define ACRIS_ACRIS_SETTINGS_ENUM_CONVERT_HPP

/*!
 * @file Convert strings to enum types.
 * Used for configuration string to enum purposes.
 */

// STL Includes:
#include <map>
#include <string_view>
#include <unordered_map>

// Absolute Includes:
#include "acris/codegen/backend_interface.hpp"
#include "acris/debug/loglevel.hpp"

namespace settings {
// Aliases:
using LogLevelMap = std::unordered_map<std::string, debug::LogLevel>;
using OptimizationLevelMap =
  std::unordered_map<std::string, codegen::OptimizationLevel>;
using BackendTypeMap = std::unordered_map<std::string, codegen::BackendType>;
using InteropBackendTypeMap =
  std::map<std::string_view, codegen::InteropBackendType>;

// Functions:
[[nodiscard("Pure function must use result.")]]
auto loglevel_map() -> const LogLevelMap&;

[[nodiscard("Pure function must use result.")]]
auto optimization_level_map() -> const OptimizationLevelMap&;

[[nodiscard("Pure function must use result.")]]
auto backendtype_map() -> const BackendTypeMap&;

[[nodiscard("Pure function must use result.")]]
auto interopbackendtype_map() -> const InteropBackendTypeMap&;

[[nodiscard("Pure function must use result.")]]
auto str2loglevel(std::string_view t_key) -> debug::LogLevel;

[[nodiscard("Pure function must use result.")]]
auto str2optimizationlevel(std::string_view t_key)
  -> codegen::OptimizationLevel;

[[nodiscard("Pure function must use result.")]]
auto str2backendtype(std::string_view t_key) -> codegen::BackendType;

[[nodiscard("Pure function must use result.")]]
auto str2interopbackendtype(std::string_view t_key)
  -> codegen::InteropBackendType;
} // namespace settings

#endif // ACRIS_ACRIS_SETTINGS_ENUM_CONVERT_HPP
