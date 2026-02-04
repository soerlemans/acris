#include "mir_pass.hpp"

namespace mir::mir_pass {
auto MirPass::on_block(BasicBlock& t_block) -> void
{
  for(Instruction& instr : t_block.m_instructions) {
    on_instruction(instr);
  }
}

auto MirPass::on_function(FunctionPtr& t_fn) -> void
{
  for(BasicBlock& block : t_fn->m_blocks) {
    on_block(block);
  }
}

auto MirPass::on_module(ModulePtr& t_module) -> void
{
  for(FunctionPtr& fn : t_module->m_functions) {
    on_function(fn);
  }
}

auto MirPass::run_pass(MirPassParams t_params) -> void
{
  auto& [module_] = t_params;

  // TODO: Set symboltable?

  on_module(module_);
};
} // namespace mir::mir_pass
