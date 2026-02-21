#ifndef ACRIS_ACRIS_TYPES_SEMANTIC_SYMBOL_HPP
#define ACRIS_ACRIS_TYPES_SEMANTIC_SYMBOL_HPP

// STL Includes:
#include <iosfwd>
#include <list>
#include <memory>
#include <optional>

namespace types::symbol {
// Forward Declarations:
class SymbolData;

struct EnumType;
struct StructType;
struct FnType;
struct PointerType;
struct ArrayType;
struct VarType;

// Aliases:
using EnumTypePtr = std::shared_ptr<EnumType>;
using StructTypePtr = std::shared_ptr<StructType>;
using FnTypePtr = std ::shared_ptr<FnType>;
using PointerTypePtr = std::shared_ptr<PointerType>;
using ArrayTypePtr = std::shared_ptr<ArrayType>;
using VarTypePtr = std::shared_ptr<VarType>;

using SymbolDataList = std::list<SymbolData>;
} // namespace types::symbol

// Functions:
auto operator<<(std::ostream& t_os, types::symbol::EnumTypePtr t_enum)
  -> std::ostream&;
auto operator<<(std::ostream& t_os, types::symbol::StructTypePtr t_struct)
  -> std::ostream&;
auto operator<<(std::ostream& t_os, types::symbol::FnTypePtr t_fn)
  -> std::ostream&;
auto operator<<(std::ostream& t_os, types::symbol::PointerTypePtr t_ptr)
  -> std::ostream&;
auto operator<<(std::ostream& t_os, types::symbol::ArrayTypePtr t_arr)
  -> std::ostream&;
auto operator<<(std::ostream& t_os, types::symbol::VarTypePtr t_var)
  -> std::ostream&;

auto operator<<(std::ostream& t_os, const types::symbol::SymbolDataList& t_list)
  -> std::ostream&;

#endif // ACRIS_ACRIS_TYPES_SEMANTIC_SYMBOL_HPP
