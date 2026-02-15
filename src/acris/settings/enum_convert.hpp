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
using OptimizeMap = std::unordered_map<std::string, codegen::Optimize>;
using BackendMap = std::unordered_map<std::string, codegen::Backend>;
using InteropBackendMap = std::map<std::string_view, codegen::InteropBackend>;

// Functions:
[[nodiscard("Pure function must use result.")]]
auto loglevel_map() -> const LogLevelMap&;

[[nodiscard("Pure function must use result.")]]
auto optimize_map() -> const OptimizeMap&;

[[nodiscard("Pure function must use result.")]]
auto backend_map() -> const BackendMap&;

[[nodiscard("Pure function must use result.")]]
auto interopbackend_map() -> const InteropBackendMap&;

[[nodiscard("Pure function must use result.")]]
auto str2loglevel(std::string_view t_key) -> debug::LogLevel;

[[nodiscard("Pure function must use result.")]]
auto str2optimize(std::string_view t_key) -> codegen::Optimize;

[[nodiscard("Pure function must use result.")]]
auto str2backend(std::string_view t_key) -> codegen::Backend;

[[nodiscard("Pure function must use result.")]]
auto str2interopbackend(std::string_view t_key) -> codegen::InteropBackend;
} // namespace settings

#endif // ACRIS_ACRIS_SETTINGS_ENUM_CONVERT_HPP
