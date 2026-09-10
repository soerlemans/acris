#include "mir_module_factory.hpp"

// STL Includes:
#include <algorithm>
#include <format>
#include <iomanip>
#include <memory>
#include <ranges>

// Absolute Includes:
#include "acris/debug/log.hpp"
#include "lib/check_nullptr.hpp"
#include "lib/overload.hpp"
#include "lib/stdexcept/stdexcept.hpp"
#include "lib/string_util.hpp"

namespace mir::mir_builder {
// Methods:
MirModuleFactory::MirModuleFactory()
  : m_module{std::make_shared<Module>()},

    // Environments:
    m_global_map{},
    m_stack_map{},
    m_fn_env{},
    m_value_env{},

    // IDs:
    m_block_id{0},
    m_instr_id{0},
    m_global_id{0},
    m_stack_id{0},
    m_value_id{0}
{}

auto MirModuleFactory::push_env() -> void
{
  m_value_env.push_env();

  // FIXME: Why did I do this you cant nest functions in IR?
  m_fn_env.push_env();
}

auto MirModuleFactory::pop_env() -> void
{
  m_value_env.pop_env();

  // FIXME: Why did I do this you cant nest functions in IR?
  m_fn_env.pop_env();

  // TODO: LLVM IR backend should also reset this for every function.
  m_instr_id = 0;
  m_value_id = 0;
  m_stack_id = 0;
}

auto MirModuleFactory::clear_env() -> void
{
  m_global_map.clear();
  m_value_env.clear();
  m_fn_env.clear();
}

auto MirModuleFactory::create_value(TypeVariant t_type) -> ValuePtr
{
  auto ptr{std::make_shared<Value>(m_value_id, t_type)};
  m_value_id++;

  return ptr;
}

auto MirModuleFactory::add_result(TypeVariant t_type) -> ValuePtr
{
  auto val{create_value(t_type)};
  auto& instr{last_instruction()};

  // Add the variable to the last instruction.
  instr.m_result = val;

  return val;
}

auto MirModuleFactory::last_value() -> ValuePtr
{
  auto& instr{last_instruction()};

  return instr.m_result;
}

auto MirModuleFactory::require_last_value() -> ValuePtr
{
  auto var{last_value()};
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

auto MirModuleFactory::to_opcode(NativeType t_type) -> Opcode
{
  using types::core::nativetype2str;

  Opcode opcode{};

  // TODO: Encode exact types later.
  switch(t_type) {
    case NativeType::F32:
      opcode = Opcode::CONST_FLOAT;
      break;

    case NativeType::F64:
      opcode = Opcode::CONST_FLOAT;
      break;

    case NativeType::INT:
      opcode = Opcode::CONST_INT;
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

  return opcode;
}

auto MirModuleFactory::add_literal(NativeType t_type, LiteralValue t_value)
  -> Instruction&
{
  using types::core::nativetype2str;

  auto& fn{last_function()};
  auto& instr{add_instruction(to_opcode(t_type))};
  add_result({t_type});

  // Add the literal as an operand.
  Literal lit{t_type, t_value};
  instr.add_operand(lit);

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

auto MirModuleFactory::stack_alloca(std::string_view t_name, TypeVariant t_type)
  -> void
{
  auto& fn{last_function()};

  // Create stack var entry.
  auto stack_var{std::make_shared<StackSlot>(m_stack_id, t_type)};
  m_stack_id++;

  const auto [iter, inserted] =
    m_stack_map.emplace(std::string{t_name}, stack_var);
  if(!inserted) {
    using lib::stdexcept::throw_runtime_error;

    throw_runtime_error("Could not insert stack slot ", std::quoted(t_name),
                        ".");
  }

  // Insert the stack variable into the function.
  fn->m_stack.push_back(stack_var);
}

auto MirModuleFactory::load(std::string_view t_name) -> Instruction&
{
  // TODO: Check for errors.
  auto& fn{last_function()};

  // Get stack entry to load.
  const auto iter = m_stack_map.find(std::string{t_name});
  if(iter == m_stack_map.end()) {
    using lib::stdexcept::throw_runtime_error;

    throw_runtime_error("Could not find stack variable ", std::quoted(t_name),
                        ".");
  }

  // Add instruction.
  auto& instr{add_instruction(Opcode::LOAD)};

  const auto idx{iter->second->m_id};

  add_result(iter->second->m_type);
  instr.add_operand(fn->m_stack.at(idx));

  return instr;
}

auto MirModuleFactory::store(std::string_view t_name, ValuePtr t_prev_var)
  -> Instruction&
{
  auto& fn{last_function()};

  const auto type{t_prev_var->m_type};

  // Check if that stack var exists.
  const auto iter = m_stack_map.find(std::string{t_name});
  if(iter == m_stack_map.end()) {
    using lib::stdexcept::throw_runtime_error;

    throw_runtime_error("Could not find stack variable ", std::quoted(t_name),
                        ".");
  }

  // Add instruction.
  auto& instr{add_instruction(Opcode::STORE)};

  const auto idx{iter->second->m_id};
  instr.add_operand(fn->m_stack.at(idx));
  instr.add_operand(t_prev_var);

  add_result(type);

  return instr;
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
    add_result(type);

    return load_instr;
  } else {
    return load(t_name);
  }
}

auto MirModuleFactory::add_call(const std::string_view t_name,
                                const ValueVec& t_args) -> Instruction&
{
  auto& call_instr{add_instruction(Opcode::CALL)};

  // Get a handle to the function.
  const FunctionMirEntity& entity{m_fn_env.get_value(t_name)};

	// TODO: We have to resolve the function type now.
  auto resolved_fn{entity.m_entity};
	add_result(resolved_fn->m_return_type);

  // Insert a weak reference to the function as operand.
  // FIXME: But this fails when we only have a declaration
  // of a function. Which occurs when we declare an extern
  // C function. We need to have a fix for this.
  FunctionLabel label{FunctionWeakPtr{entity.m_entity}};
  call_instr.add_operand({label});

  // The rest of the args.
  for(const ValuePtr& var : t_args) {
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

    throw_runtime_error("There are no instructions in the "
                        "last basic block, cant retrieve "
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

  block.m_label = std::format("{}_{}", t_label, m_block_id);

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

    throw_runtime_error("There are no basic blocks in the last function, "
                        "cant retrieve last "
                        "basic block.");
  }

  return fn->m_blocks.back();
}

auto MirModuleFactory::add_function_declaration(FunctionPtr t_fn) -> void
{
  const auto fn_name{t_fn->m_name};

  // TODO: Maybe double check semantics?

  // Only insert if the function name if an entry does not
  // already exist. Semantic checking should guarentee
  // proper semantics.
  const auto [iter, exists] = m_fn_env.find(fn_name);
  if(!exists) {
    // Add the placeholder for later declaration.
    // This way we can reference it before, the complete
    // definition.
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
    // TODO: This is brittle, depends on not modifying the
    // inplace FunctionPtr. As it would invalidate any
    // calls that depend on the forward declaration.
    auto& [fn_status, fn_ptr] = iter->second;

    // Change the status inplace to not invalidate any call
    // statements. That reference the current FunctionPtr.
    fn_status = EntityStatus::DEFINED;

    CHECK_NULLPTR(fn_ptr);
    CHECK_NULLPTR(t_new_fn);

    // Copy over contents, as to not invalidate any
    // references.
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

auto MirModuleFactory::set_module_name(std::string_view t_name) -> void
{
  m_module->m_name = t_name;
}

auto MirModuleFactory::get_module() -> ModulePtr
{
  return m_module;
}
} // namespace mir::mir_builder
