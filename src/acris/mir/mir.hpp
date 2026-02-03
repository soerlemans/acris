#ifndef ACRIS_ACRIS_MIR_MIR_HPP
#define ACRIS_ACRIS_MIR_MIR_HPP

/*!
 * @file CLIR is short for AcrisLang Intermediate Representation.
 * We implement the IR as a series of structs that closely mimic the AST.
 * And its layout.
 */

// STL Includes:
#include <format>
#include <list>
#include <memory>
#include <ostream>
#include <string_view>
#include <variant>
#include <vector>

// Absolute Includes:
#include "acris/types/core/core.hpp"
#include "lib/stdtypes.hpp"

namespace mir {
// Forward Declarations:
struct Literal;
struct GlobalVar;
struct SsaVar;
struct PhiArg;
struct Label;
struct FunctionLabel;
struct Instruction;
struct BasicBlock;
struct Function;
struct Module;

// Using:
using types::core::NativeType;
using types::core::TypeVariant;

// Local variables and global variables get treated very differently.
using LocalVarHandle = u64;
using GlobalVarHandle = u64;

// Aliases:
using ModulePtr = std::shared_ptr<Module>;

using GlobalVarPtr = std::shared_ptr<GlobalVar>;
using SsaVarPtr = std::shared_ptr<SsaVar>;
using FunctionPtr = std::shared_ptr<Function>;
using FunctionWeakPtr = std::weak_ptr<Function>;
// using BasicBlockPtr = std::shared_ptr<BasicBlock>;
// using BasicBlockWeakPtr = std::shared_ptr<BasicBlock>;

// We use lists for instructions and basic blocks.
// This is to prevent any iterator or reference invalidation.
// During the building of the IR.
// Or the modifying of it afterwards.
using ModuleSeq = std::list<Module>;
using FunctionSeq = std::list<FunctionPtr>;

using BasicBlockSeq = std::list<BasicBlock>;
using InstructionSeq = std::list<Instruction>;

using GlobalVarVec = std::vector<GlobalVarPtr>;
using SsaVarVec = std::vector<SsaVarPtr>;
using CfgSeq = std::list<BasicBlock*>;

// TODO: Support more then just bool, add all other supported native_types.
//! Variant containing all supported literal types.
using LiteralValue = std::variant<uint, int, f64, std::string, bool>;

using PhiArgValue = std::variant<GlobalVarPtr, SsaVarPtr, Literal>;

/*!
 * The @ref FunctionPtr is needed for resolving function calls.
 * The @ref SsaVarPtr is needed for obtaining references to SSA variables.
 * The @ref Literal is needed for obtaining references to literals.
 * The @ref Label is needed for obtaining references to basic blocks.
 */
using Operand =
  std::variant<GlobalVarPtr, SsaVarPtr, Literal, Label, FunctionLabel, PhiArg>;
using OperandSeq = std::vector<Operand>;

using BasicBlockIter = BasicBlockSeq::iterator;
using FunctionIter = FunctionSeq::iterator;

// Enums:
// I have no clue what im doing with this IR, so I just need to write stuff.
// And figure everything out in hindsight.
/*!
 * The opcodes which are supported for IR.
 */
enum class Opcode : u32 {
  // Literals:
  CONST_F32, // %<dest> = const_f32 <float32 literal>
  CONST_F64, // %<dest> = const_f64 <float64 literal>

  CONST_INT, // %<dest> = const_int <integer literal>

  // FIXME: Unused as of writing as we dont support suffix literals yet.
  // CONST_I8,
  // CONST_I16,
  // CONST_I32,
  // CONST_I64,
  // CONST_ISIZE,

  // CONST_UINT,
  // CONST_U8,
  // CONST_U16,
  // CONST_U32,
  // CONST_U64,
  // CONST_USIZE,

  CONST_STRING, // %<dest> = const_string <string literal>
  CONST_BOOL,   // %<dest> = const_bool <true|false>

  // Integer arithmetic:
  IADD, // %<dest> = iadd <src> <div>
  ISUB, // %<dest> = isub <src> <div>
  IMUL, // %<dest> = imul <src> <div>
  IDIV, // %<dest> = idiv <src> <div>
  IMOD, // %<dest> = imod <src> <div>
  INEG, // %<dest> = ineg <src>

  // Integer comparison:
  ICMP_LT,  // %<dest> = icmp_lt <lhs> <rhs>
  ICMP_LTE, // %<dest> = icmp_lte <lhs> <rhs>
  ICMP_EQ,  // %<dest> = icmp_eq <lhs> <rhs>
  ICMP_NQ,  // %<dest> = icmp_nq <lhs> <rhs>
  ICMP_GT,  // %<dest> = icmp_gt <lhs> <rhs>
  ICMP_GTE, // %<dest> = icmp_gte <lhs> <rhs>

  // Float Arithmetic:
  FADD, // %<dest> = fadd <src> <div>
  FSUB, // %<dest> = fsub <src> <div>
  FMUL, // %<dest> = fmul <src> <div>
  FDIV, // %<dest> = fdiv <src> <div>
  FNEG, // %<dest> = fneg <src>

  // Float Comparison:
  FCMP_LT,  // %<dest> = fcmp_lt <lhs> <rhs>
  FCMP_LTE, // %<dest> = fcmp_lte <lhs> <rhs>
  FCMP_EQ,  // %<dest> = fcmp_eq <lhs> <rhs>
  FCMP_NQ,  // %<dest> = fcmp_nq <lhs> <rhs>
  FCMP_GT,  // %<dest> = fcmp_gt <lhs> <rhs>
  FCMP_GTE, // %<dest> = fcmp_gte <lhs> <rhs>

  // Trunc, // Trunc an integer, maybe use??

  // Memory handling:
  // clang-format off
  INIT,      // %<dest> = init <src> ; dest = src. Adds a comment For the in source variable that is instantiated.
  UPDATE,    // %<dest> = update <src> ; dest = src. Adds a comment of the in source variable referenced.

  LOAD,      // %<dest> = load <src> ; dest = *src.
  STORE,     // %<dest> = store <src> ; *dest = src.
  ALLOCA,    // %<dest> = alloca <count>; Allocate memory on the heap.
  LEA,       // %<dest> = lea <src> ; dest = &src Load a calculated address, like load effective address.
  // clang-format on

  // Control Flow:
  COND_JUMP, // %<dest> = cond_jmp <condition> <label_true> <label_false>
  JUMP,      // jmp <label>
  CONTINUE,  // continue
  BREAK,     // break
  RETURN,    // ret %<var>

  // SSA specific, select value based on the control path.
  PHI, // %<dest> = phi <condition> <value_true> <value_false>

  // High level control flow:
  LOOP, // loop <cond>.
  MATCH,
  SWITCH, // switch <index> ; Jump table.

  // Struct operations:
  CONSTRUCT,
  INSERT,
  EXTRACT,

  // Call a function.
  CALL, // %<dest> = call <function reference> <arguments...>

  NOP, // nop ; Short for "No Operation", does nothing.
};

// TODO: Some of these structs, are starting to gain in size.
// So we must likely move them somewhere else.

// Structs:
struct Literal {
  NativeType m_type;
  LiteralValue m_value;

  Literal(const NativeType t_type, const LiteralValue t_value)
    : m_type{t_type}, m_value{t_value}
  {}

  virtual ~Literal() = default;
};

// TODO: Figure this out.
struct GlobalVar {
  GlobalVarHandle m_id;
  std::string m_name;
  TypeVariant m_type;

  GlobalVar(u64 t_id, const std::string_view t_name, TypeVariant t_type)
    : m_id{t_id}, m_name{t_name}, m_type{t_type}
  {}

  virtual ~GlobalVar() = default;
};

// Rename to LocalVar.
struct SsaVar {
  LocalVarHandle m_id;
  TypeVariant m_type;

  SsaVar(u64 t_id, TypeVariant t_type): m_id{t_id}, m_type{t_type}
  {}

  virtual ~SsaVar() = default;
};

//! Used for jump operations.
struct Label {
  BasicBlock* m_target;

  Label(BasicBlock* t_target): m_target{t_target}
  {}

  auto label() const -> std::string_view;

  virtual ~Label() = default;
};

struct FunctionLabel {
  FunctionWeakPtr m_target;

  FunctionLabel(FunctionWeakPtr t_target): m_target{t_target}
  {}

  auto label() const -> std::string_view;
  auto handle() -> FunctionPtr;

  virtual ~FunctionLabel() = default;
};

//! Used for keeping track of which.
struct PhiArg {
  Label m_label;
  PhiArgValue m_value; // Literal/global var/ssa var to prefer.

  PhiArg(Label t_label, PhiArgValue t_value): m_label{t_label}, m_value{t_value}
  {}

  virtual ~PhiArg() = default;
};

struct Instruction {
  u64 m_id;
  Opcode m_opcode;
  OperandSeq m_operands;

  SsaVarPtr m_result;

  std::string m_comment;

  auto add_operand(Operand t_operand) -> void
  {
    m_operands.emplace_back(std::move(t_operand));
  };

  virtual ~Instruction() = default;
};

struct BasicBlock {
  std::string m_label;
  InstructionSeq m_instructions;

  // TODO: The control flow graphs, should maybe be a map keyed by label?
  // CfgSeq m_successors;
  // CfgSeq m_predecessors;

  BasicBlock() = default;

  virtual ~BasicBlock() = default;
};

struct Function {
  std::string m_name;
  SsaVarVec m_params;
  BasicBlockSeq m_blocks;
  TypeVariant m_return_type;

  Function() = default;

  virtual ~Function() = default;
};

struct Module {
  std::string m_name;
  // TODO: Add a field for struct definitions, and type aliases.
  // m_externals; // For out of module declarations or references.

  // For C symbols we need interop with (these must own the FunctionPtr.).
  // Or any other language.
  // m_external_ffi;
  GlobalVarVec m_globals;
  FunctionSeq m_functions;

  Module() = default;

  virtual ~Module() = default;
};

// Functions:
auto opcode2str(Opcode t_opcode) -> std::string_view;
} // namespace mir

// Functions:
// clang-format off
auto operator<<(std::ostream& t_os, const mir::Opcode t_op) -> std::ostream&;

auto operator<<(std::ostream& t_os, const mir::Literal& t_lit) -> std::ostream&;
auto operator<<(std::ostream& t_os, const mir::GlobalVar& t_var) -> std::ostream&;
auto operator<<(std::ostream& t_os, const mir::GlobalVarPtr& t_ptr) -> std::ostream&;
auto operator<<(std::ostream& t_os, const mir::GlobalVarVec& t_vec) -> std::ostream&;
auto operator<<(std::ostream& t_os, const mir::SsaVar& t_var) -> std::ostream&;
auto operator<<(std::ostream& t_os, const mir::SsaVarPtr& t_ptr) -> std::ostream&;
auto operator<<(std::ostream& t_os, const mir::Label& t_label) -> std::ostream&;
auto operator<<(std::ostream& t_os, const mir::FunctionLabel& t_label) -> std::ostream&;
auto operator<<(std::ostream& t_os, const mir::PhiArgValue& t_val) -> std::ostream&;
auto operator<<(std::ostream& t_os, const mir::PhiArg& t_arg) -> std::ostream&;
auto operator<<(std::ostream& t_os, const mir::Operand& t_operand) -> std::ostream&;

auto operator<<(std::ostream& t_os, const mir::Instruction& t_inst) -> std::ostream&;
auto operator<<(std::ostream& t_os, const mir::BasicBlock& t_bblock) -> std::ostream&;
auto operator<<(std::ostream& t_os, const mir::Function& t_fn) -> std::ostream&;
auto operator<<(std::ostream& t_os, const mir::FunctionPtr& t_ptr) -> std::ostream&;
auto operator<<(std::ostream& t_os, const mir::FunctionWeakPtr& t_ptr) -> std::ostream&;
auto operator<<(std::ostream& t_os, const mir::Module& t_mod) -> std::ostream&;
auto operator<<(std::ostream& t_os, const mir::ModulePtr& t_mod) -> std::ostream&;
// clang-format on

// Format specializations:
// struct std::formatter<mir::SsaVar> { // Doesnt work for MacOS.
template<>
struct std::formatter<mir::SsaVar> : std::formatter<std::string_view> {
  template<typename FormatContext>
  auto format(const mir::SsaVar& t_var, FormatContext& ctx)
    -> std::formatter<std::string_view>
  {
    // Reuse operator<<()
    std::ostringstream oss{};
    oss << t_var.m_id;

    return std::formatter<std::string_view>::format(oss.view(), ctx);
  }
};

#endif // ACRIS_ACRIS_MIR_MIR_HPP
