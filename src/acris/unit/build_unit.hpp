#ifndef ACRIS_ACRIS_UNIT_BUILD_UNIT_HPP
#define ACRIS_ACRIS_UNIT_BUILD_UNIT_HPP

// STL Includes:
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

// Absolute Includes:
#include "acris/codegen/backend_interface.hpp"

namespace unit {
// Forward Declarations:
class BuildUnit;

// Aliases:
namespace fs = std::filesystem;

using codegen::Backend;
using codegen::BackendPtr;
using codegen::InteropSelectors;
using codegen::Optimize;

using BuildUnitPtr = std::shared_ptr<BuildUnit>;
using PathOpt = std::optional<fs::path>;

// Structs:
struct BuildUnitParams {
  Backend m_backend_selector;
  InteropSelectors m_interop_selectors;
  Optimize m_olevel;

  // Intended for BackendContext.
  PathOpt m_output_path;
  PathOpt m_build_dir;
};

// Classes:
/*!
 * Stores the configuration for each @ref TranslationUnit.
 * This includes the instantiated backends.
 * And which build dir should be used.
 */
class BuildUnit {
  private:
  BackendPtr m_backend;

  public:
  BuildUnit(BackendPtr&& t_backend);

  auto backend_requires_mir() -> bool;
  auto compile(codegen::CompileParams& t_params) -> void;

  virtual ~BuildUnit() = default;
};

// Functions:
//! Resolve CLI settings and configuration from project.toml.
auto make_build_unit(BuildUnitParams& t_params) -> BuildUnitPtr;
} // namespace unit

#endif // ACRIS_ACRIS_UNIT_BUILD_UNIT_HPP
