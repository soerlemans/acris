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

using codegen::BackendPtr;
using codegen::BackendType;
using codegen::InteropSelectors;

using BuildUnitPtr = std::shared_ptr<BuildUnit>;
using PathOpt = std::optional<fs::path>;

// Structs:
struct BuildUnitParams {
  BackendType m_backend_selector;
  InteropSelectors m_interop_selectors;
  PathOpt m_build_dir_opt;
};

struct Config {
  // Other build configuration options.
  // Maybe I will need a configuration_unit later down the line.
  // For all of the different types of settings needed at various points.
  // In the build process.
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
  fs::path m_build_dir;

  public:
  BuildUnit(BackendPtr&& t_backend, fs::path t_build_dir);

  auto backend_requires_mir() -> bool;
  auto compile(codegen::CompileParams& t_params) -> void;
  auto build_dir() -> const fs::path&;

  virtual ~BuildUnit() = default;
};

// Functions:
//! Resolve CLI settings and configuration from project.toml.
auto make_build_unit(BuildUnitParams& t_params) -> BuildUnitPtr;
} // namespace unit

#endif // ACRIS_ACRIS_UNIT_BUILD_UNIT_HPP
