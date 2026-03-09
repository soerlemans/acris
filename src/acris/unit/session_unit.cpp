#include "session_unit.hpp"

// Absolute Includes:
#include "acris/settings/settings.hpp"

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

auto SessionUnit::no_libc() const -> bool
{
  return m_no_libc;
}
} // namespace unit
