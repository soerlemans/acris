#include "settings.hpp"

// STL Includes:
#include <exception>

// Absolute Includes:
#include "acris/debug/log.hpp"
#include "acris/settings/cli.hpp"
#include "acris/settings/toml.hpp"
#include "lib/stdprint.hpp"

namespace settings {
auto get_settings(CliParams& t_params) -> Settings
{
  Settings settings{};

  // try {
  //  If a project settings file was found we should use it.
  const auto opt{find_settings_toml()};
  if(opt) {
    const auto path{opt.value()};

    read_settings_toml(path, settings);
  }

  // CLI settings overwrite TOML settings.
  read_cli_settings(t_params, settings);
  //} catch(std::exception& e) {
  // TODO: Will I use this?
  // }

  DBG_PRINTLN("Settings: ", settings);

  return settings;
}
} // namespace settings

// Functions:
auto operator<<(std::ostream& t_os, const settings::Settings& t_settings)
  -> std::ostream&
{
  using namespace lib::stdprint::vector;
  using namespace lib::stdprint::map;
  using lib::iomanip::cond_nl;

  // clang-format off
  t_os << "Settings{" << cond_nl;
  t_os << "source_paths: " << t_settings.m_source_paths << ", " << cond_nl;
  t_os << "macro_definitions: " << t_settings.m_mdefs << ", " << cond_nl;
  t_os << "backend: " << t_settings.m_backend << ", " << cond_nl;
  t_os << "interop_backends: " << t_settings.m_ibackends << ", " << cond_nl;
  t_os << "optimization_level: " << t_settings.m_olevel << ", " << cond_nl;
  t_os << "loglevel: " << t_settings.m_level << ", " << cond_nl;
  t_os << "}";
  // clang-format on

  return t_os;
}
