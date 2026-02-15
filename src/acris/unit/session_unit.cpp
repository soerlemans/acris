#include "session_unit.hpp"

namespace unit {
SessionUnit::SessionUnit(MacroDefs t_mdefs, bool t_no_libc)
  : m_mdefs{std::move(t_mdefs)}, m_no_libc{t_no_libc}
{
  if(m_no_libc) {
    m_mdefs.emplace("NO_LIBC", "true");
  }
}

auto SessionUnit::macro_defs() const -> const MacroDefs&
{
  return m_mdefs;
}

auto make_session_unit(const Settings& t_settings) -> SessionUnitPtr
{
  using unit::SessionUnit;

  return make_shared<SessionUnit>(t_settings.m_mdefs, t_settings.m_no_libc);
}
} // namespace unit
