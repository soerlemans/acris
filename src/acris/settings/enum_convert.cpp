#include "enum_convert.hpp"

// STL Includes
#include <exception>
#include <format>
#include <string>

// Internal:
namespace {
using codegen::Backend;
using codegen::InteropBackend;
using codegen::Optimize;
using debug::LogLevel;
using settings::BackendMap;
using settings::InteropBackendMap;
using settings::LogLevelMap;
using settings::OptimizeMap;

LogLevelMap loglevel_convmap{
  {"critical", LogLevel::CRITICAL},
  {   "error",    LogLevel::ERROR},
  { "warning",  LogLevel::WARNING},
  {  "notice",   LogLevel::NOTICE},
  {    "info",     LogLevel::INFO},
  { "verbose",  LogLevel::VERBOSE}
};

OptimizeMap optimize_convmap{
  {"none",    Optimize::NONE},
  {"size",    Optimize::SIZE},
  {   "1", Optimize::LEVEL_1},
  {   "2", Optimize::LEVEL_2},
  {   "3", Optimize::LEVEL_3},
};

BackendMap backend_convmap{
  { "cpp",  Backend::CPP_BACKEND},
  {"llvm", Backend::LLVM_BACKEND},
};

InteropBackendMap interopbackend_convmap{
  {     "C",      InteropBackend::C_INTEROP_BACKEND},
  {"python", InteropBackend::PYTHON_INTEROP_BACKEND},
  {   "lua",    InteropBackend::LUA_INTEROP_BACKEND},
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

auto optimize_map() -> const OptimizeMap&
{
  return optimize_convmap;
}

auto backend_map() -> const BackendMap&
{
  return backend_convmap;
}

auto interopbackend_map() -> const InteropBackendMap&
{
  return interopbackend_convmap;
}

auto str2loglevel(const std::string_view t_key) -> debug::LogLevel
{
  return str2enum(loglevel_map(), t_key, "str2loglevel()");
}

auto str2optimize(const std::string_view t_key) -> codegen::Optimize
{
  return str2enum(optimize_map(), t_key, "str2optimize()");
}

// TODO: All these string conversion functions look the same create a helper.
auto str2backend(const std::string_view t_key) -> codegen::Backend
{
  return str2enum(backend_map(), t_key, "str2backend()");
}

auto str2interopbackend(std::string_view t_key) -> codegen::InteropBackend
{
  return str2enum(interopbackend_map(), t_key, "str2interopbackend()");
}
} // namespace settings
