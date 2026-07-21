#ifndef ACRIS_ACRIS_CODEGEN_LLVM_BACKEND_LLVM_BACKEND_HPP
#define ACRIS_ACRIS_CODEGEN_LLVM_BACKEND_LLVM_BACKEND_HPP

// STL Includes:
#include <filesystem>
#include <memory>
#include <unordered_map>

// Library Includes:
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

// Absolute Includes:
#include "acris/ast/visitor/node_visitor.hpp"
#include "acris/codegen/backend_interface.hpp"
#include "acris/mir/mir.hpp"
#include "acris/mir/mir_pass/mir_pass.hpp"

/*!
 * @file
 *
 * FIXME: As of now the @ref LlvmBackend is in a half broken state.
 * It should be properly implemented and fixed one day.
 */


namespace codegen::llvm_backend {
// Using:
namespace fs = std::filesystem;

using mir::BasicBlock;
using mir::BasicBlockHandle;
using mir::FunctionHandle;
using mir::FunctionPtr;
using mir::GlobalVar;
using mir::GlobalVarHandle;
using mir::GlobalVarPtr;
using mir::Instruction;
using mir::InstructionHandle;
using mir::Literal;
using mir::LocalVar;
using mir::LocalVarHandle;
using mir::LocalVarPtr;
using mir::ModulePtr;
using mir::Operand;
using mir::PhiArg;
using mir::PhiArgValue;
using mir::VarHandle;
using mir::mir_pass::MirPass;
using types::core::NativeType;
using types::core::TypeVariant;

using LlvmContextPtr = std::shared_ptr<llvm::LLVMContext>;
using LlvmIrBuilderPtr = std::shared_ptr<llvm::IRBuilder<>>;
using LlvmModulePtr = std::shared_ptr<llvm::Module>;

using LlvmTypePtr = std::unique_ptr<llvm::Type>;

// Literals are assigned.
// FIXME: Globals and such share id/handle space in this case watchout.
using LiteralMap = std::unordered_map<VarHandle, llvm::Value*>;

using GlobalVarMap = std::unordered_map<GlobalVarHandle, llvm::GlobalVariable*>;
using LocalVarMap = std::unordered_map<LocalVarHandle, llvm::Value*>;

using BasicBlockMap = std::unordered_map<BasicBlockHandle, llvm::BasicBlock*>;
using FunctionMap = std::unordered_map<FunctionHandle, llvm::Function*>;

// Classes:
class LlvmBackend : public MirPass, public BackendInterface {
  private:
  BackendContext m_ctx;

  LlvmContextPtr m_context;
  LlvmIrBuilderPtr m_builder;
  LlvmModulePtr m_module;

  LiteralMap m_literals;

  GlobalVarMap m_globals;
  LocalVarMap m_locals;

  BasicBlockMap m_bblocks;

  llvm::BasicBlock* m_entry_bblock;

  Optimize m_olevel;

  public:
  LlvmBackend();

  llvm::BasicBlock* entry_bblock();

  auto native_type2llvm(NativeType t_type) -> llvm::Type*;

  auto literal2llvm(const Literal& t_literal) -> llvm::Value*;
  auto type2llvm(TypeVariant& t_type) -> llvm::Value*;
  auto operand2llvm(const Operand& t_operand, std::string_view t_id = "")
    -> llvm::Value*;
  auto phi_arg_val2llvm(const PhiArgValue& t_phi_arg,
                        std::string_view t_id = "phi") -> llvm::Value*;

	// Opcodes:
  auto on_const_int(Instruction& t_instr) -> void;

  auto on_iadd(Instruction& t_instr) -> void;
  auto on_isub(Instruction& t_instr) -> void;
  auto on_imul(Instruction& t_instr) -> void;
  auto on_idiv(Instruction& t_instr) -> void;
  auto on_imod(Instruction& t_instr) -> void;
  auto on_ineg(Instruction& t_instr) -> void;

  auto on_icmp_lt(Instruction& t_instr) -> void;
  auto on_icmp_lte(Instruction& t_instr) -> void;
  auto on_icmp_eq(Instruction& t_instr) -> void;
  auto on_icmp_ne(Instruction& t_instr) -> void;
  auto on_icmp_gt(Instruction& t_instr) -> void;
  auto on_icmp_gte(Instruction& t_instr) -> void;

  auto on_bind(Instruction& t_instr) -> void;
  auto on_update(Instruction& t_instr) -> void;
  auto on_cond_jmp(Instruction& t_instr) -> void;
  auto on_jmp(Instruction& t_instr) -> void;
  auto on_return(Instruction& t_instr) -> void;
  auto on_phi(Instruction& t_instr) -> void;

  auto on_instruction(Instruction& t_instr) -> void override;
  auto on_block(BasicBlock& t_block) -> void override;
  auto on_function(FunctionPtr& t_fn) -> void override;
  auto on_module(ModulePtr& t_module) -> void override;

  // Util:
  auto initialize_target() -> void;
  auto dump_ir(std::ostream& t_os) -> void;

  auto set_context(BackendContext t_ctx) -> void override;

  //! LLVM backend as of writing supports no interop.
  auto register_interop_backend([[maybe_unused]] InteropBackend t_type)
    -> void override {};

  auto set_optimize(Optimize t_olevel) -> void override;
  auto set_passess() -> void;

  auto requires_mir() -> bool override;

	auto invoke_clang_driver(const char* t_tmp_obj, const char* t_out) -> void;
  auto compile(CompileParams& t_params) -> void override;

  virtual ~LlvmBackend() = default;
};
} // namespace codegen::llvm_backend

#endif // ACRIS_ACRIS_CODEGEN_LLVM_BACKEND_LLVM_BACKEND_HPP
