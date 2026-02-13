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

using codegen::BackendType;
using codegen::OptimizationLevel;
using debug::LogLevel;

using FileVec = std::vector<fs::path>;
using StringVec = std::vector<std::string>;
using MacroDefs = std::map<std::string, std::string>;
using MacroUndefs = std::set<std::string>;
using InteropBackendTypeVec = std::vector<codegen::InteropBackendType>;

// using InteropBackendVec = std::vector<>;

// Structs:
struct Settings {
  FileVec m_source_paths;
  MacroDefs m_mdefs;

  bool m_no_libc;

  BackendType m_backend;
  InteropBackendTypeVec m_ibackends;
  OptimizationLevel m_optimization_level;

  LogLevel m_level;

  // Methods:
  Settings()
    : m_source_paths{},
      m_mdefs{},
      m_no_libc{false},
      m_backend{BackendType::CPP_BACKEND},
      m_ibackends{},
      m_optimization_level{OptimizationLevel::NO_OPTIMIZATION},
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
