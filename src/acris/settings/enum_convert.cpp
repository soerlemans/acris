#include "enum_convert.hpp"

// STL Includes
#include <exception>
#include <format>
#include <string>

// Internal:
namespace {
using codegen::BackendType;
using codegen::InteropBackendType;
using codegen::OptimizationLevel;
using debug::LogLevel;
using settings::BackendTypeMap;
using settings::InteropBackendTypeMap;
using settings::LogLevelMap;
using settings::OptimizationLevelMap;

LogLevelMap loglevel_convmap{
  {"critical", LogLevel::CRITICAL},
  {   "error",    LogLevel::ERROR},
  { "warning",  LogLevel::WARNING},
  {  "notice",   LogLevel::NOTICE},
  {    "info",     LogLevel::INFO},
  { "verbose",  LogLevel::VERBOSE}
};

OptimizationLevelMap optimization_level_convmap{
  {"fast",            OptimizationLevel::FAST},
  {"size",            OptimizationLevel::SIZE},
  {   "0", OptimizationLevel::NO_OPTIMIZATION},
  {   "1",         OptimizationLevel::LEVEL_1},
  {   "2",         OptimizationLevel::LEVEL_2},
  {   "3",         OptimizationLevel::LEVEL_3},
};

BackendTypeMap backendtype_convmap{
  { "cpp",  BackendType::CPP_BACKEND},
  {"llvm", BackendType::LLVM_BACKEND},
};

InteropBackendTypeMap interopbackendtype_convmap{
  {     "C",      InteropBackendType::C_INTEROP_BACKEND},
  {"python", InteropBackendType::PYTHON_INTEROP_BACKEND},
  {   "lua",    InteropBackendType::LUA_INTEROP_BACKEND},
};

// TODO: Create std::map type concept.
template<typename MapType>
inline auto str2enum(const MapType& t_map, const std::string_view t_key,
                     std::string_view t_origin = "str2enum")
  -> MapType::mapped_type
{
  using MappedType = MapType::mapped_type;

  MappedType value{};
  std::string str{t_key};

  const auto iter{t_map.find(str)};
  if(iter != t_map.end()) {
    value = iter->second;
  } else {
    const auto err_msg{
      std::format("{}() could not convert string to enum value.", t_origin)};

    throw std::invalid_argument{err_msg};
  }

  return value;
}
} // namespace

namespace settings {
auto loglevel_map() -> const LogLevelMap&
{
  return loglevel_convmap;
}

auto optimization_level_map() -> const OptimizationLevelMap&
{
  return optimization_level_convmap;
}

auto backendtype_map() -> const BackendTypeMap&
{
  return backendtype_convmap;
}

auto interopbackendtype_map() -> const InteropBackendTypeMap&
{
  return interopbackendtype_convmap;
}

auto str2loglevel(const std::string_view t_key) -> debug::LogLevel
{
  using debug::LogLevel;

  LogLevel level{};

  const auto& map{loglevel_map()};
  std::string str{t_key};

  auto iter{map.find(str)};
  if(iter != map.end()) {
    level = iter->second;
  } else {
    throw std::invalid_argument{
      "str2loglevel() could not convert string to LogLevel."};
  }

  return level;
}

auto str2optimizationlevel(const std::string_view t_key)
  -> codegen::OptimizationLevel
{
  using codegen::OptimizationLevel;

  // OptimizationLevel level{};

  // const auto& map{optimizationlevel_map()};
  // std::string str{t_key};

  // auto iter{map.find(str)};
  // if(iter != map.end()) {
  //   level = iter->second;
  // } else {
  //   throw std::invalid_argument{
  //     "str2optimizationlevel() could not convert string to
  //     OptimizationLevel."};
  // }

  // return level;

  return str2enum(optimization_level_map(), t_key, "str2optimizationlevel()");
}

// TODO: All these string conversion functions look the same create a helper.
auto str2backendtype(const std::string_view t_key) -> codegen::BackendType
{
  using codegen::BackendType;

  BackendType backend{};

  const auto& map{backendtype_map()};
  std::string str{t_key};

  auto iter{map.find(str)};
  if(iter != map.end()) {
    backend = iter->second;
  } else {
    throw std::invalid_argument{
      "str2backendtype could not convert string to BackendType."};
  }

  return backend;

  return str2enum(backendtype_map(), t_key, "str2backendtype()");
}

auto str2interopbackendtype(std::string_view t_key)
  -> codegen::InteropBackendType
{
  return str2enum(interopbackendtype_map(), t_key, "str2interopbackendtype()");
}
} // namespace settings
