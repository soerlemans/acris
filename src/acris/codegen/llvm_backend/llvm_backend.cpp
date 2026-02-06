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
#include "lib/stdtypes.hpp"

// Local Includes:
#include "type2llvm.hpp"

namespace codegen::llvm_backend {
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


  return {};
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

  llvm::AllocaInst* alloc{nullptr};
  if(std::holds_alternative<LocalVarPtr>(first)) {
    auto local_var(std::get<LocalVarPtr>(first));

    auto iter = m_locals.find(local_var->m_id);
    alloc = iter->second;
  }

  auto result_id{result->m_id};
  m_locals.emplace(result_id, alloc);
}

auto LlvmBackend::on_return(Instruction& t_instr) -> void
{
  const auto& [id, opcode, operands, result, comment] = t_instr;

  auto& first{operands.front()};


  llvm::Value* val{nullptr};
  if(std::holds_alternative<Literal>(first)) {
    auto lit(std::get<Literal>(first));

    val = literal2llvm(lit);
  } else if(std::holds_alternative<LocalVarPtr>(first)) {
    auto var_ptr(std::get<LocalVarPtr>(first));

    auto opt{var_ptr->m_type.native_type()};
    // val = native_type2llvm(opt.value());

    auto iter = m_locals.find(var_ptr->m_id);
    auto* alloca = iter->second;
    // val = iter->second;

    // Dereference return value.
    val = m_builder->CreateLoad(alloca->getAllocatedType(), alloca, "ret_tmp");
  }

  m_builder->CreateRet(val);
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
      break;
    case Opcode::ICMP_GTE:
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
      break;
    case Opcode::JUMP:
      break;
    case Opcode::CONTINUE:
      break;
    case Opcode::BREAK:
      break;
    case Opcode::RETURN:
      on_return(t_instr);
      break;

    case Opcode::PHI:
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
    m_builder->SetInsertPoint(basic_block);

    m_last_bblock = basic_block;

    // Set and update current bblock.
    on_block(block);

    m_last_bblock = nullptr;
  }

  llvm::verifyFunction(*fn);

  // Clear local variables.
  m_locals.clear();
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
