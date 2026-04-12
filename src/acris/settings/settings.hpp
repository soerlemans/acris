#ifndef ACRIS_ACRIS_SETTINGS_SETTINGS_HPP
#define ACRIS_ACRIS_SETTINGS_SETTINGS_HPP

// STL Includes:
#include <filesystem>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

// Absolute Includes:
#include "acris/codegen/backend_interface.hpp"
#include "acris/debug/loglevel.hpp"
#include "acris/unit/session_unit.hpp"

// Local Includes:
#include "cli.hpp"
#include "toml.hpp"

namespace settings {
// Using:
namespace fs = std::filesystem;

using codegen::Backend;
using codegen::Optimize;
using debug::LogLevel;
using unit::MacroDefs;
using unit::MacroUndefs;

using PathOpt = std::optional<fs::path>;
using FileVec = std::vector<fs::path>;
using StringVec = std::vector<std::string>;
using InteropBackendVec = std::vector<codegen::InteropBackend>;

// using InteropBackendVec = std::vector<>;

// Structs:
struct Settings {
  FileVec m_source_paths;
  PathOpt m_output_path;
  MacroDefs m_mdefs;

  bool m_no_libc;

  Backend m_backend;
  InteropBackendVec m_ibackends;
  Optimize m_olevel;

  LogLevel m_level;

  // Methods:
  Settings()
    : m_source_paths{},
      m_output_path{},
      m_mdefs{},
      m_no_libc{false},
      m_backend{Backend::CPP_BACKEND},
      m_ibackends{},
      m_olevel{Optimize::NONE},
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
