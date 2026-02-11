#ifndef ACRIS_ACRIS_MIR_OPERAND_HPP
#define ACRIS_ACRIS_MIR_OPERAND_HPP

// STL Includes:
#include <memory>
#include <variant>
#include <vector>

namespace mir {
// Using:
using types::core::NativeType;
using types::core::TypeVariant;

// Forward Declarations:
struct Literal;
struct GlobalVar;
struct LocalVar;
struct Label;
struct FunctioNLabel;
struct FunctioNLabel;

class Operand;

// Using:
using GlobalVarPtr = std::shared_ptr<GlobalVar>;
using LocalVarPtr = std::shared_ptr<LocalVar>;

using OperandVariant = std::variant<GlobalVarPtr, LocalVarPtr, Literal, Label,
                                    FunctionLabel, PhiArg>;
using OperandSeq = std::vector<Operand>;

// Classes:
class Operand {
  private:
  OperandVariant m_operand_types;

  public:
};

} // namespace mir

#endif // ACRIS_ACRIS_MIR_OPERAND_HPP
