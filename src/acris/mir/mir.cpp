#include "mir.hpp"

// STL Includes:
#include <format>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>


// Absolute Includes:
#include "lib/check_nullptr.hpp"
#include "lib/overload.hpp"
#include "lib/stdexcept/stdexcept.hpp"
#include "lib/stdprint.hpp"

// Macros:
#define MATCH(t_key, t_value) \
  case Opcode::t_key:         \
    str = t_value;            \
    break

// Internal
namespace {
// TODO: Move to lib/?
template<typename T>
constexpr inline auto to_string(const T& t_elem) -> std::string
{
  std::ostringstream oss{};

  oss << t_elem;

  return oss.str();
}

} // namespace

namespace mir {
// Methods:
// TODO: Move somewhere else.
auto Label::label() const -> std::string_view
{
  CHECK_NULLPTR(m_target);

  return m_target->m_label;
}

// TODO: Move somewhere else.

auto FunctionLabel::handle() -> FunctionPtr
{
  const FunctionPtr target{m_target.lock()};
  CHECK_NULLPTR(target);

  return target;
}

auto FunctionLabel::label() const -> std::string_view
{
  const FunctionPtr target{m_target.lock()};
  CHECK_NULLPTR(target);

  return target->m_name;
}

// Functions:
auto opcode2str(const Opcode t_opcode) -> std::string_view
{
  using lib::stdexcept::throw_invalid_argument;

  std::string_view str{};

  switch(t_opcode) {
    // Literals:
    MATCH(CONST_FLOAT, "const_float");
    MATCH(CONST_INT, "const_int");
    MATCH(CONST_UINT, "const_uint");
    MATCH(CONST_STRING, "const_string");
    MATCH(CONST_BOOL, "const_bool");

    // Integer arithmetic:
    MATCH(IADD, "iadd");
    MATCH(ISUB, "isub");
    MATCH(IMUL, "imul");
    MATCH(IDIV, "idiv");
    MATCH(INEG, "ineg");

    // Integer comparison:
    MATCH(ICMP_LT, "icmp_lt");
    MATCH(ICMP_LTE, "icmp_lte");
    MATCH(ICMP_EQ, "icmp_eq");
    MATCH(ICMP_NE, "icmp_ne");
    MATCH(ICMP_GT, "icmp_gt");
    MATCH(ICMP_GTE, "icmp_gte");

    // Float arithmetic:
    MATCH(FADD, "fadd");
    MATCH(FSUB, "fsub");
    MATCH(FMUL, "fmul");
    MATCH(FDIV, "fdiv");
    MATCH(FNEG, "fneg");

    // Float comparison:
    MATCH(FCMP_LT, "fcmp_lt");
    MATCH(FCMP_LTE, "fcmp_lte");
    MATCH(FCMP_EQ, "fcmp_eq");
    MATCH(FCMP_NE, "fcmp_ne");
    MATCH(FCMP_GT, "fcmp_gt");
    MATCH(FCMP_GTE, "fcmp_gte");

    // Memory handling:
    MATCH(BIND, "bind");
    MATCH(UPDATE, "update");

    MATCH(LOAD, "load");
    MATCH(STORE, "store");
    MATCH(ALLOC, "alloc");
    MATCH(LEA, "lea");

    // Control Flow:
    MATCH(COND_JUMP, "cond_jmp");
    MATCH(JUMP, "jmp");
    MATCH(RETURN, "ret");

    MATCH(PHI, "phi");

    // High level control flow:
    MATCH(LOOP, "loop");
    MATCH(MATCH, "match");
    MATCH(SWITCH, "switch");

    // Struct operations:
    MATCH(CONSTRUCT, "construct");
    MATCH(INSERT, "insert");
    MATCH(EXTRACT, "extract");

    MATCH(CALL, "call");

    MATCH(NOP, "nop");

    default:
      throw_invalid_argument(
        "Opcode could not be converted to std::string_view.");
      break;
  }

  return str;
}
} // namespace mir

auto operator<<(std::ostream& t_os, const mir::Opcode t_op) -> std::ostream&
{
  t_os << mir::opcode2str(t_op);

  return t_os;
}

auto operator<<(std::ostream& t_os, const mir::Literal& t_val) -> std::ostream&
{
  using lib::Overload;

  // clang-format off
  std::visit(Overload{
			[&](std::string&& t_str) {
				t_os << std::quoted(t_str);
			},
				[&](auto&& t_value) {
					t_os << t_value;
				}},
		t_val.m_value);
  // clang-format on

  t_os << ":" << t_val.m_type;

  return t_os;
}

auto operator<<(std::ostream& t_os, const mir::GlobalVar& t_var)
  -> std::ostream&
{
  t_os << std::format("@{}", t_var.m_name);

  return t_os;
}

auto operator<<(std::ostream& t_os, const mir::GlobalVarPtr& t_ptr)
  -> std::ostream&
{
  using lib::stdprint::detail::print_smart_ptr;

  return print_smart_ptr(t_os, t_ptr);
}

auto operator<<(std::ostream& t_os, const mir::GlobalVarVec& t_vec)
  -> std::ostream&
{
  for(const auto& global_var : t_vec) {
    t_os << "\t" << global_var << '\n';
  }

  return t_os;
}

auto operator<<(std::ostream& t_os, const mir::StackVar& t_var) -> std::ostream&
{
  // TODO: Think about conditional printing of the type as well?

  t_os << std::format("&{}", t_var.m_id);

  return t_os;
}

auto operator<<(std::ostream& t_os, const mir::StackVarPtr& t_ptr)
  -> std::ostream&
{
  using lib::stdprint::detail::print_smart_ptr;

  return print_smart_ptr(t_os, t_ptr);
}


auto operator<<(std::ostream& t_os, const mir::LocalVar& t_var) -> std::ostream&
{
  // TODO: Think about conditional printing of the type as well?

  t_os << std::format("%{}", t_var.m_id);

  return t_os;
}

auto operator<<(std::ostream& t_os, const mir::LocalVarPtr& t_ptr)
  -> std::ostream&
{
  using lib::stdprint::detail::print_smart_ptr;

  return print_smart_ptr(t_os, t_ptr);
}

auto operator<<(std::ostream& t_os, const mir::Label& t_label) -> std::ostream&
{
  t_os << std::format("<l:{}>", t_label.label());

  return t_os;
}

auto operator<<(std::ostream& t_os, const mir::FunctionLabel& t_label)
  -> std::ostream&
{
  t_os << std::format("<f:{}>", t_label.label());

  return t_os;
}

auto operator<<(std::ostream& t_os, const mir::PhiArgValue& t_val)
  -> std::ostream&
{
  auto print{[&](auto&& t_elem) {
    t_os << t_elem;
  }};

  std::visit(print, t_val);

  return t_os;
}

auto operator<<(std::ostream& t_os, const mir::PhiArg& t_arg) -> std::ostream&
{
  t_os << '[' << t_arg.m_label << ", " << t_arg.m_value << ']';

  return t_os;
}

auto operator<<(std::ostream& t_os, const mir::Operand& t_operand)
  -> std::ostream&
{
  auto print{[&](auto&& t_elem) {
    t_os << t_elem;
  }};

  std::visit(print, t_operand);

  return t_os;
}

auto operator<<(std::ostream& t_os, const mir::Instruction& t_inst)
  -> std::ostream&
{
  using mir::opcode2str;
  using mir::Operand;

  const auto& [id, opcode, operands, result, comment] = t_inst;

  // Format: "{id}: {result =} {opcode} {operand}, {operand}, {etc}"
  std::stringstream ss{};

  // If the result is present, prepend it to the opcode.
  std::string assign_str{};
  if(result) {
    assign_str = std::format("{} = ", to_string(*result));
  }

  const auto opcode_str{opcode2str(opcode)};
  ss << std::format("{}: {}{} ", id, assign_str, opcode_str);

  // Loop over operands and print them.
  std::string_view sep{};
  for(const Operand& operand : operands) {
    ss << sep << operand;

    sep = ", ";
  }

  // Add printing of a comment if not empty.
  if(!comment.empty()) {
    // We left align in a column of 80 characters.
    t_os << std::format("{:<80} ; {}", ss.view(), comment);
  } else {
    t_os << ss.view();
  }

  return t_os;
}

auto operator<<(std::ostream& t_os, const mir::BasicBlock& t_bblock)
  -> std::ostream&
{
  using mir::Instruction;

  const auto& [label, instructions] = t_bblock;

  t_os << std::format("{}:\n", label);

  for(const Instruction& inst : instructions) {
    t_os << "\t" << inst << '\n';
  }

  return t_os;
}

auto operator<<(std::ostream& t_os, const mir::Function& t_fn) -> std::ostream&
{
  using mir::BasicBlock;
  using mir::LocalVarPtr;
  using mir::StackVarPtr;

  const auto& [name, params, return_type, stack_locals, blocks] = t_fn;

  t_os << std::format("function {}", name);

  t_os << '(';
  std::string_view sep{};
  for(const LocalVarPtr& ptr : params) {
    t_os << sep << ptr;

    sep = ", ";
  }
  t_os << ") -> " << return_type << "{\n";

  // Local vars, for forward declare:
	std::size_t stack_idx{0};
  t_os << "stack_locals {\n";
  for(const StackVarPtr& stack_var : stack_locals) {
    t_os << '\t' << stack_var << " : " << stack_var->m_type << '\n';
  }
  t_os << "}\n\n";

  for(const BasicBlock& block : blocks) {
    t_os << block << "\n";
  }

  t_os << "}\n";

  return t_os;
}

auto operator<<(std::ostream& t_os, const mir::FunctionPtr& t_ptr)
  -> std::ostream&
{
  using lib::stdprint::detail::print_smart_ptr;

  return print_smart_ptr(t_os, t_ptr);
}

auto operator<<(std::ostream& t_os, const mir::FunctionWeakPtr& t_ptr)
  -> std::ostream&
{
  using lib::stdprint::detail::print_weak_ptr;

  return print_weak_ptr(t_os, t_ptr);
}

auto operator<<(std::ostream& t_os, const mir::Module& t_mod) -> std::ostream&
{
  using mir::FunctionPtr;

  const auto& [name, globals, functions] = t_mod;

  // Module declaration header.
  t_os << std::format("module {}\n\n", name);

  // Print globals;
  t_os << "globals {\n";
  t_os << globals;
  t_os << "}\n\n";

  // Print the functions part of the module.
  std::string_view sep{};
  for(const FunctionPtr& fn : functions) {
    t_os << sep << fn;

    // Add a newline between functions.
    sep = "\n";
  }

  return t_os;
}

auto operator<<(std::ostream& t_os, const mir::ModulePtr& t_ptr)
  -> std::ostream&
{
  using lib::stdprint::detail::print_smart_ptr;

  return print_smart_ptr(t_os, t_ptr);
}
