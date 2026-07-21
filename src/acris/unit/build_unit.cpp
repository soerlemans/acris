#include "build_unit.hpp"


// Absolute Includes:
#include "lib/filesystem.hpp"

namespace unit {
BuildUnit::BuildUnit(BackendPtr&& t_backend): m_backend{std::move(t_backend)}
{}

auto BuildUnit::backend_requires_mir() -> bool
{
  return m_backend->requires_mir();
}

auto BuildUnit::compile(codegen::CompileParams& t_params) -> void
{
  m_backend->compile(t_params);
}

// Functions:
auto make_build_unit(BuildUnitParams& t_params) -> BuildUnitPtr
{
  using codegen::BackendContext;
  using codegen::InteropBackend;
  using codegen::select_backend;

  BuildUnitPtr ptr{};

  auto& [backend_type, interop_types, olevel, output_path_opt, build_dir_opt] =
    t_params;


  auto output_path{output_path_opt.value_or("a.out")};

  auto build_dir{build_dir_opt
                   .or_else([]() {
                     return PathOpt{lib::temporary_directory()};
                   })
                   .value()};

  BackendContext ctx{output_path, build_dir};

  // Get pointer to backend.
  auto backend_ptr{select_backend(backend_type)};

  backend_ptr->set_context(ctx);
  backend_ptr->set_optimize(olevel);

  // Instruct backend to add interop backends.
  for(const InteropBackend type : interop_types) {
    backend_ptr->register_interop_backend(type);
  }

  ptr = std::make_shared<BuildUnit>(std::move(backend_ptr));

  return ptr;
}
} // namespace unit
