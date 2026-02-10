#ifndef ACRIS_ACRIS_MIR_MIR_BUILDER_MIR_MODULE_FACTORY_HPP
#define ACRIS_ACRIS_MIR_MIR_BUILDER_MIR_MODULE_FACTORY_HPP

// STL Includes:
#include <memory>
#include <unordered_map>

// Absolute Includes:
#include "acris/mir/mir.hpp"
#include "acris/mir/mir_builder/mir_env_state.hpp"

namespace mir::mir_builder {
// Using:
using types::core::TypeVariant;

// Forward Declarations:
struct LocalVarSite;
template<typename T>
struct MirEntity;
class MirModuleFactory;

// Aliases:
using MirModuleFactoryPtr = std::unique_ptr<MirModuleFactory>;

using FunctionMirEntity = MirEntity<FunctionPtr>;
using GlobalMirEntity = MirEntity<GlobalVarPtr>;

//! Globals can be forward declared, regular variables not.
using GlobalVarMap = std::unordered_map<std::string, GlobalMirEntity>;
using LocalVarEnvState = MirEnvState<LocalVarSite>;
using FunctionEnvState = MirEnvState<FunctionMirEntity>;

// Enums:
// TODO: Similar to @ref SymbolStatus maybe unify?
//! Keep track if a MIR item has only been declared, or defined.
enum class EntityStatus {
  DECLARED,
  DEFINED,
};

// Structs:
/*!
 * We need to keep track of the blocks ssa var's are in for phi merging.
 * Which is also relevant for LLVM IR.
 */
struct LocalVarSite {
  BasicBlock* m_block;
  LocalVarPtr m_var;
};

/*!
 * Functions or structs could be forward declared.
 * For these kind of MirEntities we need something inbetween.
 */
template<typename T>
struct MirEntity {
  EntityStatus m_status;
  T m_entity;
};

// Classes:
// TODO: Implement a functionality similar to LLVM's SetInsertPoint.
/*!
 * Utility class to aid building a CLIR module.
 * When walking the AST.
 * In the @ref MirBuilder.
 * Here we handle all the boilerplate operations for constructing the CLIR.
 *
 * @note Any method starting with `create_` will create that item.
 * But not add it to the current module the factory is building.
 *
 * @note Any method starting with `add_` will create that item.
 * As well as add itto the last item it can be added to.
 *
 * @note Any method starting with `last_` will retrieve the last item.
 * Of that object that was added to the module.
 */
class MirModuleFactory {
  private:
  // This is the result of the factory.
  ModulePtr m_module;

  // Environment for referencing globals.
  GlobalVarMap m_global_map;

  // Semantic pass should prevent any variables and functions from conflicting.
  FunctionEnvState m_fn_env;
  LocalVarEnvState m_var_env;

  // We need to increment these to prevent collisions.
  u64 m_block_id;
  u64 m_instr_id;
  u64 m_global_id;
  u64 m_var_id;

  public:
  MirModuleFactory();

  // Env operations:
  auto push_env() -> void;
  auto pop_env() -> void;
  auto clear_env() -> void;

  auto set_var_env(const LocalVarEnvState& t_env) -> void;
  auto get_var_env() const -> const LocalVarEnvState&;

  // LocalVar operations:
  [[nodiscard("Must use created ssa var.")]]
  auto create_var(TypeVariant t_type) -> LocalVarPtr;
  auto add_result_var(TypeVariant t_type) -> LocalVarPtr;

  /*!
   * Returns the result @ref LocalVarPtr from the last @ref Instruction.
   *
   * @remark Can be a nullptr if last instruction has no result variable.
   */
  auto last_var() -> LocalVarPtr;

  /*!
   * Returns the result @ref LocalVarPtr from the last @ref Instruction.
   *
   * @remark Throws if the last instruction has no result variable.
   */
  auto require_last_var() -> LocalVarPtr;

  // Instruction operations:
  [[nodiscard("Must use created instruction.")]]
  auto create_instruction(Opcode t_opcode) -> Instruction;
  auto add_instruction_to(Opcode t_opcode, BasicBlock& t_block) -> Instruction&;
  auto add_instruction(Opcode t_opcode) -> Instruction&;
  auto add_comment(std::string t_comment) -> void;

  auto to_opcode(NativeType t_type) -> Opcode;

  //! Add a literal which also creates the proper instruction for it.
  auto add_literal(NativeType t_type, LiteralValue t_value) -> Instruction&;

  /*!
   * We usually add a jump statement in retrospect between two blocks.
   * So we need to supply everything manually to fix instruction numbering.
   * (in some cases).
   */
  auto insert_jump(Instruction t_instr, BasicBlock& t_block,
                   BasicBlock& t_target) -> Instruction&;
  auto insert_jump(BasicBlock& t_block, BasicBlock& t_target) -> Instruction&;

  /*!
   * Bind a source variable name to an IR var.
   * For later reference/usage.
   */
  auto var_bind(std::string_view t_name, LocalVarPtr t_var) -> void;

  [[nodiscard("Must use created global.")]]
  auto create_global(std::string_view t_name, TypeVariant t_type)
    -> GlobalVarPtr;

  /*!
   * Check if a variable name is a global.
   */
  auto is_global(std::string_view t_name) const -> bool;

  // TODO: Implement and move somewhere proper.
  // auto add_external_function(std::string_view t_name, TypeVariant t_type)

  /*!
   * Add a placeholder variable pointer, for later definition.
   * Variables, are only allowed to be declared if they are globals.
   */
  auto add_global_declaration(std::string_view t_name, TypeVariant t_type)
    -> void;

  /*!
   * Bind a variable name to a result var for later reference.
   * By its corrseponding variable name.
   */
  auto add_variable_bind(std::string_view t_name, Instruction& t_instr)
    -> Instruction&;

  /*!
   * Adds an instruction, which returns a result to reference the variable by.
   * This is used to reference a source variable name.
   * This method can load either a global or insert an update statement.
   */
  auto add_variable_ref(std::string_view t_name) -> Instruction&;

  /*!
   * Adds an instruction, which returns a result to reference the variable by.
   * This instruction is always an update instruction.
   * So we can reference the last SSA var.
   */
  auto add_update(std::string_view t_name, LocalVarPtr t_prev_var)
    -> Instruction&;

  /*!
   * Create a call instruction.
   * @param t_name Name of the function to call.
   * @param t_args Arguments to pass to the function.
   */
  auto add_call(std::string_view t_name, const LocalVarVec& t_args)
    -> Instruction&;

  auto last_instruction() -> Instruction&;

  // Block operations:
  [[nodiscard("Must use created block.")]]
  auto create_block(std::string_view t_label) -> BasicBlock;
  auto append_block(const BasicBlock& t_block) -> BasicBlock&;
  auto add_block(std::string_view t_label) -> BasicBlock&;

  /*!
   * @return nullptr if there is no basicblock associated with the given label.
   */
  auto find_block(std::string_view t_label) -> BasicBlock*;
  auto last_block() -> BasicBlock&;

  // Function operations:
  auto add_function_declaration(FunctionPtr t_fn) -> void;
  auto add_function_definition(FunctionPtr t_fn) -> void;

  /*!
   * Get a function by name.
   *
   * @note semantic analysis should ensure that a function exists.
   * So if we cant get the function we should throw.
   */
  // auto get_function(std::string_view t_key) -> Function&;

  auto last_function() -> FunctionPtr&;

  /*!
   * Insert phi nodes where necessary.
   * Merge two @ref LocalVarEnvState's into a single one.
   */
  auto merge_envs(const LocalVarEnvState& t_env1,
                  const LocalVarEnvState& t_env2) -> LocalVarEnvState;

  // Module operations:
  auto set_module_name(std::string_view t_name) -> void;

  auto get_module() -> ModulePtr;

  virtual ~MirModuleFactory() = default;
};
} // namespace mir::mir_builder

#endif // ACRIS_ACRIS_MIR_MIR_BUILDER_MIR_MODULE_FACTORY_HPP
