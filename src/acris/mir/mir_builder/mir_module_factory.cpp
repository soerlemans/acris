#include "mir_module_factory.hpp"

// STL Includes:
#include <algorithm>
#include <iomanip>
#include <memory>
#include <ranges>

// Absolute Includes:
#include "acris/debug/log.hpp"
#include "lib/check_nullptr.hpp"
#include "lib/stdexcept/stdexcept.hpp"
#include "lib/string_util.hpp"

namespace mir::mir_builder {
// Methods:
MirModuleFactory::MirModuleFactory()
  : m_module{std::make_shared<Module>()},

    // Environments:
    m_global_map{},
    m_fn_env{},
    m_var_env{},

    // IDs:
    m_block_id{0},
    m_instr_id{0},
    m_global_id{0},
    m_var_id{0}
{}

auto MirModuleFactory::push_env() -> void
{
  m_var_env.push_env();

  // FIXME: Why did I do this you cant nest functions in IR?
  m_fn_env.push_env();
}

auto MirModuleFactory::pop_env() -> void
{
  m_var_env.pop_env();

  // FIXME: Why did I do this you cant nest functions in IR?
  m_fn_env.pop_env();
}

auto MirModuleFactory::clear_env() -> void
{
  m_global_map.clear();
  m_var_env.clear();
  m_fn_env.clear();
}

auto MirModuleFactory::set_var_env(const LocalVarEnvState& t_env) -> void
{
  m_var_env = t_env;
}

auto MirModuleFactory::get_var_env() const -> const LocalVarEnvState&
{
  return m_var_env;
}

auto MirModuleFactory::create_var(TypeVariant t_type) -> LocalVarPtr
{
  auto ptr{std::make_shared<LocalVar>(m_var_id, t_type)};

  m_var_id++;

  return ptr;
}

auto MirModuleFactory::add_result_var(TypeVariant t_type) -> LocalVarPtr
{
  auto ssa_var{create_var(t_type)};
  auto& instr{last_instruction()};

  // Add the variable to the last instruction.
  instr.m_result = ssa_var;

  return ssa_var;
}

auto MirModuleFactory::last_var() -> LocalVarPtr
{
  auto& instr{last_instruction()};

  return instr.m_result;
}

auto MirModuleFactory::require_last_var() -> LocalVarPtr
{
  auto var{last_var()};
  if(!var) {
    lib::stdexcept::throw_runtime_error(
      "Expected last IR instruction to produce an SSA var.");
  }

  return var;
}

auto MirModuleFactory::create_instruction(const Opcode t_opcode) -> Instruction
{
  // Cookie cutter the creation of an instruction.
  Instruction instr{};

  instr.m_id = m_instr_id;
  instr.m_opcode = t_opcode;
  instr.m_result = nullptr;

  m_instr_id++;

  return instr;
}

auto MirModuleFactory::add_instruction_to(const Opcode t_opcode,
                                          BasicBlock& t_block) -> Instruction&
{
  auto& instructions{t_block.m_instructions};

  auto instr{create_instruction(t_opcode)};
  instructions.push_back(instr);

  return last_instruction();
}

auto MirModuleFactory::add_instruction(const Opcode t_opcode) -> Instruction&
{
  auto& block{last_block()};

  return add_instruction_to(t_opcode, block);
}

auto MirModuleFactory::add_comment(std::string t_comment) -> void
{
  auto& instr{last_instruction()};

  lib::strip_whitespace(t_comment);
  lib::trim_whitespace(t_comment);

  instr.m_comment = std::format(R"("{}".)", t_comment);
}

auto MirModuleFactory::add_literal(NativeType t_type, LiteralValue t_value)
  -> Instruction&
{
  using types::core::nativetype2str;

  Opcode opcode{};

  // Translate native type to literal opcode.
  // TODO: Create a separate helper function for this.
  switch(t_type) {
    case NativeType::F32:
      opcode = Opcode::CONST_F32;
      break;

    case NativeType::F64:
      opcode = Opcode::CONST_F64;
      break;

    case NativeType::INT:
      opcode = Opcode::CONST_INT;
      break;

    case NativeType::CSTR:
      opcode = Opcode::CONST_STRING;
      break;

    case NativeType::BOOL:
      opcode = Opcode::CONST_BOOL;
      break;

    default: {
      using lib::stdexcept::throw_invalid_argument;

      // Be aware nativetyp2str() can also fail for the given type.
      std::stringstream ss{};
      ss << std::format(R"(Given native type is unsupported "{}".)",
                        nativetype2str(t_type));

      throw_invalid_argument(ss.str());
      break;
    }
  }

  // Instruction insertion:
  auto& instr{add_instruction(opcode)};

  // Add the literal as an operand.
  Literal lit{t_type, t_value};
  instr.add_operand(lit);

  auto LocalVar{create_var({t_type})};
  instr.m_result = std::move(LocalVar);

  return instr;
}

auto MirModuleFactory::insert_jump(Instruction t_instr, BasicBlock& t_block,
                                   BasicBlock& t_target) -> Instruction&
{
  auto& instructions{t_block.m_instructions};

  // Create label operand.
  Label label{&t_target};

  // Add label to operands.
  auto& operands{t_instr.m_operands};
  operands.push_back({label});

  // Push back instruction to source block.
  instructions.push_back(t_instr);

  return instructions.back();
}

auto MirModuleFactory::insert_jump(BasicBlock& t_block, BasicBlock& t_target)
  -> Instruction&
{
  auto jmp_instr{create_instruction(Opcode::JUMP)};

  return insert_jump(jmp_instr, t_block, t_target);
}

auto MirModuleFactory::create_var_binding(std::string_view t_name,
                                          LocalVarPtr t_var) -> void
{
  // TODO: Check for errors.
  auto& block{last_block()};
  LocalVarSite site{&block, t_var};

  const auto [iter, inserted] = m_var_env.insert({std::string{t_name}, site});
  if(!inserted) {
    using lib::stdexcept::throw_runtime_error;

    throw_runtime_error("Could not insert ", std::quoted(t_name), ".");
  }
}

auto MirModuleFactory::create_global(std::string_view t_name,
                                     TypeVariant t_type) -> GlobalVarPtr
{
  const auto global_var{
    std::make_shared<GlobalVar>(m_global_id, t_name, t_type)};

  // Update ID value.
  m_global_id++;

  return global_var;
}

auto MirModuleFactory::is_global(const std::string_view t_name) const -> bool
{
  return m_global_map.contains(std::string{t_name});
}

auto MirModuleFactory::add_global_declaration(const std::string_view t_name,
                                              TypeVariant t_type) -> void
{
  const std::string name{t_name};
  const auto iter{m_global_map.find(name)};

  // Only insert if no already present.
  if(iter == m_global_map.end()) {
    const auto global_var{create_global(t_name, t_type)};

    // Add placeholder for current referencing.
    GlobalMirEntity entity{EntityStatus::DECLARED, global_var};
    m_global_map.insert({std::string{t_name}, entity});
  }
}

auto MirModuleFactory::add_variable_definition(const std::string_view t_name,
                                               TypeVariant t_type)
  -> Instruction&
{
  auto& assign_instr{add_instruction(Opcode::INIT)};
  auto result_var{add_result_var(t_type)};

  if(m_var_env.is_toplevel()) {
    const auto global_var{create_global(t_name, t_type)};

    // Insert into global environment tracking.
    GlobalMirEntity entity{EntityStatus::DEFINED, global_var};
    m_global_map.insert({std::string{t_name}, entity});

    // Insert into globals section of module.
    auto& global_vec{m_module->m_globals};
    global_vec.push_back(global_var);
  } else {
    // Bind the source variable name to the ssa var.
    create_var_binding(t_name, result_var);
  }

  return assign_instr;
}

auto MirModuleFactory::add_variable_ref(const std::string_view t_name)
  -> Instruction&
{
  if(is_global(t_name)) {
    auto iter{m_global_map.find(std::string{t_name})};
    const auto global_var{iter->second.m_entity};
    const auto type{global_var->m_type};

    // Construct load instruction.
    auto& load_instr{add_instruction(Opcode::LOAD)};
    load_instr.add_operand(global_var);
    add_result_var(type);

    return load_instr;
  } else {
    // Get the previous ssa variable associated with the name.
    auto prev_var{m_var_env.get_value(t_name).m_var};

    return add_update(t_name, prev_var);
  }
}

auto MirModuleFactory::add_update(std::string_view t_name,
                                  LocalVarPtr t_prev_var) -> Instruction&
{
  auto& update_instr{add_instruction(Opcode::UPDATE)};

  // Add the last usage of the ssa variable associated with the name.
  update_instr.add_operand(t_prev_var);

  const auto type{t_prev_var->m_type};
  auto result_var{add_result_var(type)};

  // Update with the new result var.
  // For the next variable reference.
  auto& block{last_block()};
  LocalVarSite site{&block, result_var};
  m_var_env.update(t_name, site);

  return update_instr;
}

auto MirModuleFactory::add_call(const std::string_view t_name,
                                const LocalVarVec& t_args) -> Instruction&
{
  auto& call_instr{add_instruction(Opcode::CALL)};

  // Get a handle to the function.
  const FunctionMirEntity& entity{m_fn_env.get_value(t_name)};

  // Insert a weak reference to the function as operand.
  // FIXME: But this fails when we only have a declaration of a function.
  // Which occurs when we declare an extern C function.
  // We need to have a fix for this.
  FunctionLabel label{FunctionWeakPtr{entity.m_entity}};
  call_instr.add_operand({label});

  // The rest of the args.
  for(const LocalVarPtr& var : t_args) {
    call_instr.add_operand({var});
  }

  return last_instruction();
}

auto MirModuleFactory::last_instruction() -> Instruction&
{
  auto& block{last_block()};
  auto& instructions{block.m_instructions};

  if(instructions.empty()) {
    using lib::stdexcept::throw_runtime_error;

    throw_runtime_error(
      "There are no instructions in the last basic block, cant retrieve "
      "last instruction.");
  }

  return instructions.back();
}

auto MirModuleFactory::create_block(const std::string_view t_label)
  -> BasicBlock
{
  // Create basic block and set its label.
  // We want all blocks to have their own unique label.
  BasicBlock block{};

  block.m_label = std::format("{}#{}", t_label, m_block_id);

  m_block_id++;

  // Return the just added basic block.
  return block;
}

auto MirModuleFactory::append_block(const BasicBlock& t_block) -> BasicBlock&
{
  auto& fn{last_function()};
  auto& blocks{fn->m_blocks};

  // Add the new block to the last function.
  blocks.push_back(t_block);

  // Return the just added basic block.
  return last_block();
}

auto MirModuleFactory::add_block(const std::string_view t_label) -> BasicBlock&
{
  const auto block{create_block(t_label)};

  return append_block(block);
}

auto MirModuleFactory::find_block(const std::string_view t_label) -> BasicBlock*
{
  BasicBlock* ptr{nullptr};

  auto& fn{last_function()};
  auto& blocks{fn->m_blocks};

  for(BasicBlock& block : blocks) {
    if(block.m_label == t_label) {
      ptr = &block;
      break;
    }
  }

  return ptr;
}

auto MirModuleFactory::last_block() -> BasicBlock&
{
  auto& fn{last_function()};

  if(fn->m_blocks.empty()) {
    using lib::stdexcept::throw_runtime_error;

    throw_runtime_error(
      "There are no basic blocks in the last function, cant retrieve last "
      "basic block.");
  }

  return fn->m_blocks.back();
}

auto MirModuleFactory::add_function_declaration(FunctionPtr t_fn) -> void
{
  const auto fn_name{t_fn->m_name};

  // TODO: Maybe double check semantics?

  // Only insert if the function name if an entry does not already exist.
  // Semantic checking should guarentee proper semantics.
  const auto [iter, exists] = m_fn_env.find(fn_name);
  if(!exists) {
    // Add the placeholder for later declaration.
    // This way we can reference it before, the complete definition.
    FunctionMirEntity entity{EntityStatus::DECLARED, t_fn};
    m_fn_env.insert({fn_name, entity});
  }
}

auto MirModuleFactory::add_function_definition(FunctionPtr t_new_fn) -> void
{
  auto& functions{m_module->m_functions};

  FunctionPtr fn_def{t_new_fn};

  const auto fn_name{t_new_fn->m_name};
  const auto [iter, exists] = m_fn_env.find(fn_name);
  if(exists) {
    // TODO: Maybe double check semantics.
    // TODO: This is brittle, depends on not modifying the inplace FunctionPtr.
    // As it would invalidate any calls that depend on the forward declaration.
    auto& [fn_status, fn_ptr] = iter->second;

    // Change the status inplace to not invalidate any call statements.
    // That reference the current FunctionPtr.
    fn_status = EntityStatus::DEFINED;

    CHECK_NULLPTR(fn_ptr);
    CHECK_NULLPTR(t_new_fn);

    // Copy over contents, as to not invalidate any references.
    *fn_ptr = *t_new_fn;

    // Set the definition to the inplace entry.
    fn_def = fn_ptr;
  } else {
    FunctionMirEntity entity{EntityStatus::DEFINED, t_new_fn};
    m_fn_env.insert({fn_name, entity});
  }

  functions.push_back(fn_def);
}

auto MirModuleFactory::last_function() -> FunctionPtr&
{
  auto& functions{m_module->m_functions};

  if(functions.empty()) {
    using lib::stdexcept::throw_runtime_error;

    throw_runtime_error("There are no functions in the MIR "
                        "module, cant retrieve last function.");
  }

  return functions.back();
}

// TODO: Maybe find a way to optimize the implementation.
auto MirModuleFactory::merge_envs(const LocalVarEnvState& t_env1,
                                  const LocalVarEnvState& t_env2)
  -> LocalVarEnvState
{
  using EnvMap = LocalVarEnvState::BaseEnvState::EnvMap;

  // Loop through layers of both t_env1 and t_env2.
  // And insert phi nodes and update variable binding.
  // For creation and setting of the new env state.

  // TODO:
  // Important to note the nested nature can be used for optimization.
  // As we only create a new env when going into a possible scenario.
  // Where phi merging might be needed.

  // Create a new environment from the base environment.
  LocalVarEnvState merge_env{t_env1};

  // We need to loop through both environments at the same time.
  // Whilst merging the new one.
  auto iter2{t_env2.begin()};
  for(EnvMap& merge_map : merge_env) {
    if(iter2 == t_env2.end()) {
      // TODO: Error handle.
      break;
    }

    // TODO: Maybe we should also check if we looped through all map2
    // elements.
    const EnvMap& map2{*iter2};

    for(auto& [merge_key, merge_site] : merge_map) {
      auto& [merge_block, merge_ssa] = merge_site;

      const auto map_iter{map2.find(merge_key)};
      if(map_iter != map2.end()) {
        const auto& [block2, ssa2] = map_iter->second;

        // If the SSA variables differ for an entry.
        // Then we have variable references in two different branches.
        // And we need to insert a phi instruction.
        if(merge_ssa != ssa2) {
          auto& phi_instr{add_instruction(Opcode::PHI)};

          // Set result variable type.
          const auto type{merge_ssa->m_type};
          const auto phi_result{add_result_var(type)};

          // TODO: Are phi instructions must depend on basic blocks.
          // Add operands, for the phi instruction.
          // phi_instr.add_operand({t_cond});
          // phi_instr.add_operand({merge_ssa});
          // phi_instr.add_operand({ssa2});

          // TODO: Insert type specification for result var.
          // phi_instr.add_operand(PhiArg{merge_block, merge_ssa});
          phi_instr.add_operand(PhiArg{merge_block, merge_ssa});
          phi_instr.add_operand(PhiArg{block2, ssa2});

          // Update the ssa binding to the new result var.
          merge_ssa = phi_result;
        }
      } else {
        // TODO: Throw or report.
        break;
      }
    }

    iter2++;
  }

  // Use to debug:
  // DBG_INFO("env1: ", t_env1);
  // DBG_INFO("env2", t_env2);
  // DBG_INFO("merge: ", merge_env);

  return merge_env;
}

auto MirModuleFactory::set_module_name(std::string_view t_name) -> void
{
  m_module->m_name = t_name;
}

auto MirModuleFactory::get_module() -> ModulePtr
{
  return m_module;
}
} // namespace mir::mir_builder
