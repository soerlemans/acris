#include "llvm_backend.hpp"

// STL Includes:
#include <format>
#include <iostream>
#include <optional>
#include <vector>

// Library Includes:
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Verifier.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/CodeGen.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>

// Absolute Includes:
#include "acris/debug/log.hpp"
#include "acris/mir/mir.hpp"
#include "lib/filesystem.hpp"
#include "lib/stdexcept/stdexcept.hpp"
#include "lib/stdtypes.hpp"

// Local Includes:
#include "type2llvm.hpp"

namespace codegen::llvm_backend {
// Using:
using mir::Label;

using llvm::dyn_cast;

using lib::stdexcept::InvalidArgument;
using lib::stdexcept::throw_invalid_argument;
using lib::stdexcept::throwf;

// Methods:
LlvmBackend::LlvmBackend()
  : m_context{std::make_shared<llvm::LLVMContext>()},
    m_builder{std::make_shared<llvm::IRBuilder<>>(*m_context)},
    m_module{std::make_shared<llvm::Module>("Module", *m_context)},
    m_literals{},
    m_globals{},
    m_locals{},
    m_last_bblock{nullptr}
{}

auto LlvmBackend::set_last_bblock(llvm::BasicBlock* t_bblock) -> void
{
  m_last_bblock = t_bblock;
}

auto LlvmBackend::last_bblock() -> llvm::BasicBlock*
{
  return m_last_bblock;
}

auto LlvmBackend::native_type2llvm(const NativeType t_type) -> llvm::Type*
{
  switch(t_type) {
    case NativeType::VOID:
      return llvm::Type::getVoidTy(*m_context);

    // Floats:
    case NativeType::F32:
      return {llvm::Type::getFloatTy(*m_context)};

    case NativeType::F64:
      return {llvm::Type::getDoubleTy(*m_context)};

    // Integers:
    // LLVM Has no concept of unsigned.
    // Instead the right unsigned instructions should be used.
    // MIR generation should properly generate this.
    case NativeType::UINT:
      [[fallthrough]];
    case NativeType::INT:
      return llvm::Type::getInt32Ty(*m_context);

    case NativeType::U8:
      [[fallthrough]];
    case NativeType::I8:
      return llvm::Type::getInt8Ty(*m_context);

    case NativeType::U16:
      [[fallthrough]];
    case NativeType::I16:
      return llvm::Type::getInt16Ty(*m_context);

    case NativeType::U32:
      [[fallthrough]];
    case NativeType::I32:
      return llvm::Type::getInt32Ty(*m_context);

    case NativeType::U64:
      [[fallthrough]];
    case NativeType::I64:
      return llvm::Type::getInt64Ty(*m_context);

    case NativeType::USIZE:
      [[fallthrough]];
    case NativeType::ISIZE:
      break;

    // String:
    case NativeType::CHAR:
      return llvm::Type::getInt8Ty(*m_context);

    // case NativeType::CSTR: {
    //   auto str{std::get<std::string>(t_literal.m_value)};

    //   value =
    //     (llvm::Value*)llvm::ConstantDataArray::getString(*m_context, str,
    //     true);
    //   break;
    // }

    // Boolean:
    case NativeType::BOOL:
      return llvm::Type::getInt1Ty(*m_context);

    default:
      // TOOD: Error out.
      break;
  }

  return {};
}

auto LlvmBackend::type2llvm(TypeVariant& t_type) -> llvm::Value*
{
  TODO("Must still implement.");


  return {};
}

auto LlvmBackend::operand2llvm(const Operand& t_operand,
                               const std::string_view t_id) -> llvm::Value*
{
  DBG_WARNING(t_id, t_operand.index());

  llvm::Value* val{nullptr};
  if(std::holds_alternative<Literal>(t_operand)) {
    auto lit(std::get<Literal>(t_operand));

    val = literal2llvm(lit);
  } else if(std::holds_alternative<LocalVarPtr>(t_operand)) {
    auto var(std::get<LocalVarPtr>(t_operand));
    auto* stored_val = m_locals.at(var->m_id);

    if(auto* alloca = llvm::dyn_cast<llvm::AllocaInst>(stored_val); alloca) {
      auto load_str{std::format("{}_load", t_id)};

      // Dereference stack address if its the case.
      val = m_builder->CreateLoad(alloca->getAllocatedType(), alloca, load_str);
    } else {
      val = stored_val;
    }
  } else if(std::holds_alternative<Label>(t_operand)) {
    auto label(std::get<Label>(t_operand));
    auto& target_block{label.m_target};

    val = m_bblocks.at(target_block->m_label);
  } else {
    throwf<InvalidArgument>("Unsupported operand to llvm Value conversion.");
  }

  return val;
}

auto LlvmBackend::phi_arg_val2llvm(const PhiArgValue& t_phi_arg,
                                   std::string_view t_id) -> llvm::Value*
{
  return std::visit(
    [&](auto&& t_val) {
      Operand operand{t_val};

      return operand2llvm(operand);
    },
    t_phi_arg);
}

auto LlvmBackend::literal2llvm(const Literal& t_literal) -> llvm::Value*
{
  llvm::Value* value{nullptr};

  switch(t_literal.m_type) {
    case NativeType::VOID:
      value = (llvm::Value*)llvm::Type::getVoidTy(*m_context);
      break;

    // Floats:
    case NativeType::F32: {
      auto lit_val{std::get<f64>(t_literal.m_value)};
      auto* ftype{llvm::Type::getFloatTy(*m_context)};

      value = (llvm::Value*)llvm::ConstantFP::get(ftype, lit_val);
      break;
    }

    case NativeType::F64: {
      auto lit_val{std::get<f64>(t_literal.m_value)};
      auto* ftype{llvm::Type::getDoubleTy(*m_context)};

      value = (llvm::Value*)llvm::ConstantFP::get(ftype, lit_val);
      break;
    }

    // Integers:
    case NativeType::INT: {
      auto lit_val{std::get<int>(t_literal.m_value)};

      value = (llvm::Value*)llvm::ConstantInt::get(
        llvm::Type::getInt32Ty(*m_context), lit_val);
      break;
    }

    case NativeType::I8: {
      auto lit_val{std::get<int>(t_literal.m_value)};

      value = (llvm::Value*)llvm::ConstantInt::get(
        llvm::Type::getInt8Ty(*m_context), lit_val);
      break;
    }

    case NativeType::I16: {

      auto lit_val{std::get<int>(t_literal.m_value)};

      value = (llvm::Value*)llvm::ConstantInt::get(
        llvm::Type::getInt16Ty(*m_context), lit_val);
      break;
    }

    case NativeType::I32: {

      auto lit_val{std::get<int>(t_literal.m_value)};

      value = (llvm::Value*)llvm::ConstantInt::get(
        llvm::Type::getInt32Ty(*m_context), lit_val);
      break;
    }

    case NativeType::I64: {
      auto lit_val{std::get<int>(t_literal.m_value)};

      value = (llvm::Value*)llvm::ConstantInt::get(
        llvm::Type::getInt64Ty(*m_context), lit_val);
      break;
    }

    case NativeType::ISIZE:
      break;

      // LLVM Has no concept of unsigned, so deal with this later.
      // MIR generation should have resolved all of this.
    case NativeType::UINT:
      break;
    case NativeType::U8:
      break;
    case NativeType::U16:
      break;
    case NativeType::U32:
      break;
    case NativeType::U64:
      break;
    case NativeType::USIZE:
      break;

    // String:
    case NativeType::CHAR:
      break;
    case NativeType::CSTR: {
      auto str{std::get<std::string>(t_literal.m_value)};

      value =
        (llvm::Value*)llvm::ConstantDataArray::getString(*m_context, str, true);
      break;
    }

    // Boolean:
    case NativeType::BOOL: {
      auto lit_val{std::get<bool>(t_literal.m_value)};

      value = (llvm::Value*)llvm::ConstantInt::get(
        llvm::Type::getInt1Ty(*m_context), (lit_val) ? 0x1 : 0x0);
      break;
    }

    default:
      // TOOD: Error out.
      break;
  }

  return value;
}

auto LlvmBackend::on_isub(Instruction& t_instr) -> void
{
  const auto& [id, opcode, operands, result, comment] = t_instr;
  const auto result_id{result->m_id};

  auto& first{operands.front()};
  auto& second{operands.at(1)};

  llvm::Value* lhs{operand2llvm(first)};
  llvm::Value* rhs{operand2llvm(second)};

  llvm::Value* sub_result = m_builder->CreateSub(lhs, rhs, "sub_result");

  m_locals.emplace(result_id, sub_result);
}

auto LlvmBackend::on_icmp_gt(Instruction& t_instr) -> void
{
  const auto& [id, opcode, operands, result, comment] = t_instr;
  const auto result_id{result->m_id};

  auto first{std::get<LocalVarPtr>(operands.front())->m_id};
  auto second{std::get<LocalVarPtr>(operands.at(1))->m_id};

  auto* lhs{m_locals.at(first)};
  auto* rhs{m_locals.at(second)};

  llvm::Value* cmp_result = m_builder->CreateICmpSGT(lhs, rhs, "sgt_tmp");

  m_locals.emplace(result_id, cmp_result);
}

auto LlvmBackend::on_icmp_gte(Instruction& t_instr) -> void
{
  const auto& [id, opcode, operands, result, comment] = t_instr;
  const auto result_id{result->m_id};

  auto first{std::get<LocalVarPtr>(operands.front())->m_id};
  auto second{std::get<LocalVarPtr>(operands.at(1))->m_id};

  auto* lhs{m_locals.at(first)};
  auto* rhs{m_locals.at(second)};

  llvm::Value* cond = m_builder->CreateICmpSGE(lhs, rhs, "sge_tmp");

  m_locals.emplace(result_id, cond);
}

auto LlvmBackend::on_bind(Instruction& t_instr) -> void
{
  const auto& [id, opcode, operands, result, comment] = t_instr;

  auto& first{operands.front()};

  llvm::Value* val{nullptr};
  if(std::holds_alternative<Literal>(first)) {
    Literal lit(std::get<Literal>(first));

    val = literal2llvm(lit);
  }

  const auto result_id{result->m_id};
  const auto var_id{std::format("v{}", result_id)};

  // Allocate memory for a local integer variable test.
  llvm::AllocaInst* alloc{
    new llvm::AllocaInst(val->getType(), 0, var_id, last_bblock())};

  // Initialize the variable with the value 42 for now.
  m_builder->CreateStore(val, alloc);

  m_locals.emplace(result_id, alloc);
}

auto LlvmBackend::on_update(Instruction& t_instr) -> void
{
  const auto& [id, opcode, operands, result, comment] = t_instr;

  auto& first{operands.front()};

  llvm::Value* val{nullptr};
  if(std::holds_alternative<LocalVarPtr>(first)) {
    auto local_var(std::get<LocalVarPtr>(first));

    val = m_locals.at(local_var->m_id);
  }

  auto result_id{result->m_id};
  m_locals.emplace(result_id, val);
}

auto LlvmBackend::on_cond_jmp(Instruction& t_instr) -> void
{
  const auto& [id, opcode, operands, result, comment] = t_instr;
  const auto result_id{result->m_id};

  auto& first{operands.front()};
  auto var_ptr(std::get<LocalVarPtr>(first));
  auto* val = m_locals.at(var_ptr->m_id);

  auto second_label{std::get<mir::Label>(operands.at(1)).m_target->m_label};
  auto third_label{std::get<mir::Label>(operands.at(2)).m_target->m_label};

  auto true_bblock{m_bblocks.at(second_label)};
  auto false_bblock{m_bblocks.at(third_label)};

  // TODO: Figure this shit out.

  m_builder->CreateCondBr(val, true_bblock, false_bblock);
}

auto LlvmBackend::on_jmp(Instruction& t_instr) -> void
{
  const auto& [id, opcode, operands, result, comment] = t_instr;

  auto label{std::get<mir::Label>(operands.front()).m_target->m_label};
  auto bblock{m_bblocks.at(label)};

  m_builder->CreateBr(bblock);
}

auto LlvmBackend::on_return(Instruction& t_instr) -> void
{
  const auto& [id, opcode, operands, result, comment] = t_instr;

  auto& first{operands.front()};

  llvm::Value* val{operand2llvm(first)};

  m_builder->CreateRet(val);
}

auto LlvmBackend::on_phi(Instruction& t_instr) -> void
{
  const auto& [id, opcode, operands, result, comment] = t_instr;
  auto result_id{result->m_id};

  llvm::PHINode* phi = m_builder->CreatePHI(m_builder->getInt32Ty(), 2, "phi");

  for(const Operand& operand : operands) {
    auto arg{std::get<PhiArg>(operand)};
    auto& [label, value] = arg;

    auto target_block{label.m_target};
    auto* bblock{m_bblocks.at(target_block->m_label)};

    llvm::Value* val{phi_arg_val2llvm(value)};
    phi->addIncoming(val, bblock);
  }

  m_locals.emplace(result_id, phi);
}

auto LlvmBackend::on_instruction(Instruction& t_instr) -> void
{
  using mir::Opcode;

  const auto& [id, opcode, operands, result, comment] = t_instr;

  // Create a global variable to hold the constant string
  // llvm::GlobalVariable* globalVar = new llvm::GlobalVariable(
  //     module,
  //     constString->getType(),
  //     true,  // isConstant
  //     llvm::GlobalValue::PrivateLinkage,
  //     constString,
  //     "myConstString"
  // );

  // case Opcode::CONST_STRING: {
  //   Literal literal{std::get<mir::Literal>(operands.front())};
  //   auto str{std::get<std::string>(literal.m_value)};

  //   llvm::Constant* constant{
  //     llvm::ConstantDataArray::getString(*m_context, str, true)};

  //   const auto result_id{result->m_id};
  //   m_literals.emplace(result_id, (llvm::Value*)constant);
  //   break;
  // }

  switch(opcode) {
    case Opcode::IADD:
      break;

    case Opcode::ISUB:
      on_isub(t_instr);
      break;

    case Opcode::IMUL:
      break;
    case Opcode::IDIV:
      break;
    case Opcode::IMOD:
      break;
    case Opcode::INEG:
      break;
    case Opcode::ICMP_LT:
      break;
    case Opcode::ICMP_LTE:
      break;
    case Opcode::ICMP_EQ:
      break;
    case Opcode::ICMP_NQ:
      break;
    case Opcode::ICMP_GT:
      on_icmp_gt(t_instr);
      break;

    case Opcode::ICMP_GTE:
      on_icmp_gte(t_instr);
      break;

    case Opcode::FADD:
      break;
    case Opcode::FSUB:
      break;
    case Opcode::FMUL:
      break;
    case Opcode::FDIV:
      break;
    case Opcode::FNEG:
      break;
    case Opcode::FCMP_LT:
      break;
    case Opcode::FCMP_LTE:
      break;
    case Opcode::FCMP_EQ:
      break;
    case Opcode::FCMP_NQ:
      break;
    case Opcode::FCMP_GT:
      break;
    case Opcode::FCMP_GTE:
      break;

    case Opcode::BIND:
      on_bind(t_instr);
      break;

    case Opcode::UPDATE:
      on_update(t_instr);
      break;

    case Opcode::LOAD:
      break;
    case Opcode::STORE:
      break;
    case Opcode::ALLOC:
      break;
    case Opcode::LEA:
      break;

    case Opcode::COND_JUMP:
      on_cond_jmp(t_instr);
      break;

    case Opcode::JUMP:
      on_jmp(t_instr);
      break;

    case Opcode::CONTINUE:
      break;
    case Opcode::BREAK:
      break;
    case Opcode::RETURN:
      on_return(t_instr);
      break;

    case Opcode::PHI:
      on_phi(t_instr);
      break;

    case Opcode::LOOP:
      break;
    case Opcode::CALL:
      break;
    case Opcode::NOP:
      break;

    default:
      // For now we log as we are still implementing the IR.
      DBG_ERROR("Unhandled opcode: ", opcode);
      break;
  }
}

auto LlvmBackend::on_block(BasicBlock& t_block) -> void
{
  for(Instruction& instr : t_block.m_instructions) {
    on_instruction(instr);
  }
}

auto LlvmBackend::on_function(FunctionPtr& t_fn) -> void
{
  const auto fn_name{t_fn->m_name};

  auto llvm_params{std::vector<llvm::Type*>()};
  const auto params{t_fn->m_params};
  for(const auto& param : params) {
    const auto opt{param->m_type.native_type()};
    if(!opt) {
      DBG_ERROR("Cancelling function LLVM IR generation, cause return type is "
                "note resolvalbe to native type.");
      return;
    }
    const auto native_type{opt.value()};
    auto* llvm_type{native_type2llvm(native_type)};

    llvm_params.push_back(llvm_type);
  }

  // FIXME: We only support native types right now.
  auto return_type{t_fn->m_return_type};

  // TODO: make a function for this.
  const auto opt{return_type.native_type()};
  if(!opt) {
    DBG_ERROR("Cancelling function LLVM IR generation, cause return type is "
              "note resolvalbe to native type.");
    return;
  }
  const auto native_type{opt.value()};
  auto* llvm_return_type{native_type2llvm(native_type)};

  auto* fn_type{llvm::FunctionType::get(llvm_return_type, llvm_params, false)};

  auto* fn{llvm::Function::Create(fn_type, llvm::Function::ExternalLinkage,
                                  fn_name, m_module.get())};

  // Codegen for the body
  for(BasicBlock& block : t_fn->m_blocks) {
    auto block_label{block.m_label};
    auto* basic_block{llvm::BasicBlock::Create(*m_context, block_label, fn)};

    m_bblocks.emplace(block_label, basic_block);
  }

  // Walk through body.
  for(BasicBlock& block : t_fn->m_blocks) {
    auto block_label{block.m_label};

    m_last_bblock = m_bblocks.at(block_label);
    m_builder->SetInsertPoint(m_last_bblock);

    // Set and update current bblock.
    on_block(block);

    m_last_bblock = nullptr;
  }

  llvm::verifyFunction(*fn);

  // Cleanup resources.
  m_locals.clear();
  m_bblocks.clear();
}

auto LlvmBackend::on_module(ModulePtr& t_module) -> void
{
  for(FunctionPtr& fn : t_module->m_functions) {
    on_function(fn);
  }
}

auto LlvmBackend::initialize_target() -> void
{
  const auto target{llvm::sys::getDefaultTargetTriple()};

  m_module->setTargetTriple(target);

  // Initialize all target stuff:
  llvm::InitializeAllTargetInfos();
  llvm::InitializeAllTargets();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmParsers();
  llvm::InitializeAllAsmPrinters();
}

auto LlvmBackend::dump_ir(std::ostream& t_os) -> void
{
  std::string str;
  llvm::raw_string_ostream oss(str);

  m_module->print(oss, nullptr);

  t_os << str;
}

auto LlvmBackend::requires_mir() -> bool
{
  return true;
}

//! FIXME: For now we do nothing with the @ref SymbolTable
auto LlvmBackend::compile(CompileParams& t_params) -> void
{
  using namespace llvm;
  using namespace llvm::sys::fs;

  using mir::mir_pass::MirPassParams;

  const auto& [ast, mir_module, build_dir, source_path] = t_params;

  // FIXME: Check mir_module for nullptr.

  fs::path stem{source_path.stem()};
  const fs::path tmp_src{build_dir / stem.concat(".ll")};

  // Log filepath's:
  DBG_INFO("build_dir: ", build_dir);
  DBG_INFO("tmp_src: ", tmp_src);

  // Initialize the LLVM target.
  initialize_target();

  // Obtain filehandle to destination file
  const auto filename{tmp_src.c_str()};
  std::error_code err_code{};
  raw_fd_ostream dest{filename, err_code, sys::fs::OF_None};

  if(err_code) {
    errs() << "Could not open file: " << err_code.message();
    return;
  }

  // Resolve target:
  const auto target_str{m_module->getTargetTriple()};
  DBG_INFO("LLVM target: ", target_str);

  std::string err{};
  auto target{TargetRegistry::lookupTarget(target_str, err)};
  if(!target) {
    errs() << err << '\n';

    return; // TODO: Fix
  }

  // Set target machine:
  const auto cpu{"generic"};
  const auto features{""};

  TargetOptions opt{};
  std::optional<Reloc::Model> reloc_model{};
  auto target_machine{
    target->createTargetMachine(target_str, cpu, features, opt, reloc_model)};

  // Write object file:
  legacy::PassManager pass{};
  const auto fype{CGFT_ObjectFile};
  if(target_machine->addPassesToEmitFile(pass, dest, nullptr, fype)) {
    errs() << "target_machine can't emit a file of this type";
    return; // TODO: Fix
  }

  // Traverse ast to generate LLVM IR:
  MirPassParams params{mir_module};
  run_pass(params);

  dump_ir(std::cout);

  if(llvm::verifyModule(*m_module, &llvm::errs())) {
    llvm::errs() << "Error: The module is invalid!\n";
    return; // Exit or handle the error appropriately
  }

  //
  pass.run(*m_module);
  dest.flush();

  // Close so that the permissions can be set
  dest.close();

  // Make object file executable:
  const perms permissions{others_write | all_read | all_exe};
  err_code = setPermissions(tmp_src.c_str(), permissions);

  errs() << err_code.message() << " Done...\n";

  if(err_code) {
    errs() << "Could not change file permissions: " << err_code.message();
    return;
  }
}
} // namespace codegen::llvm_backend
