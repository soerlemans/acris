#ifndef ACRIS_ACRIS_SETTINGS_SETTINGS_HPP
#define ACRIS_ACRIS_SETTINGS_SETTINGS_HPP

// STL Includes:
#include <filesystem>
#include <sstream>
#include <string>
#include <vector>

// Absolute Includes:
#include "acris/codegen/backend_interface.hpp"
#include "acris/debug/loglevel.hpp"

// Local Includes:
#include "cli.hpp"
#include "toml.hpp"

namespace settings {
// Using:
namespace fs = std::filesystem;

using FileVec = std::vector<fs::path>;
using StringVec = std::vector<std::string>;
using MacroDefinitionsMap = std::map<std::string, std::string>;
using InteropBackendTypeVec = std::vector<codegen::InteropBackendType>;

// using InteropBackendVec = std::vector<>;

// Structs:
struct Settings {
  using BackendType = codegen::BackendType;
  using LogLevel = debug::LogLevel;

  FileVec m_source_paths;
  MacroDefinitionsMap m_macro_definitions;

  codegen::BackendType m_backend;
  InteropBackendTypeVec m_interop_backends;

  debug::LogLevel m_level;

  // Methods:
  Settings()
    : m_source_paths{},
      m_backend{BackendType::CPP_BACKEND},
      m_interop_backends{},
      m_level{LogLevel::VERBOSE}
  {}

  Settings(const Settings&) = default;

  // Operators:
  auto operator=(Settings&&) noexcept -> Settings& = default;

  virtual ~Settings() = default;
};

// Functions:
//! Read compiler settings from CLI options or project.toml.
auto get_settings(CliParams& t_params) -> Settings;
} // namespace settings

// Functions:
auto operator<<(std::ostream& t_os, const settings::Settings& t_settings)
  -> std::ostream&;

#endif // ACRIS_ACRIS_SETTINGS_SETTINGS_HPP
