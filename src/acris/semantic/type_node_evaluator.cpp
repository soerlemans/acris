#include "type_node_evaluator.hpp"

// Absolute Includes:
#include "acris/ast/node/include_nodes.hpp"
#include "acris/diagnostic/diagnostic.hpp"
#include "lib/stdexcept/stdexcept.hpp"

namespace semantic {
// Using:
using diagnostic::throw_type_error;

NODE_USING_ALL_NAMESPACES()

auto TypeNodeEvaluator::resolve(NodePtr t_node) -> SymbolData
{
  SymbolData type{};

  const auto any{traverse(t_node)};

  try {
    type = std::any_cast<SymbolData>(any);
  } catch(std::bad_any_cast& exception) {
    using lib::stdexcept::throw_bad_any_cast;

    throw_bad_any_cast(exception.what());
  }

  return type;
}

TypeNodeEvaluator::TypeNodeEvaluator(SymbolEnvState& t_symbol_state)
  : m_symbol_state{t_symbol_state}
{}

auto TypeNodeEvaluator::str2type(const std::string_view t_type_id) -> SymbolData
{
  const auto quoted_name{std::quoted(t_type_id)};

  // First check for native type.
  const auto opt{str2nativetype_opt(t_type_id)};
  if(opt) {
    return {opt.value()};
  }

  // Then check for user defined types.
  // If this type does not exist, we should error.
  const auto [iter, exists] = m_symbol_state.find(t_type_id);
  if(exists) {
    const auto& symbol{iter->second};
    const auto& symbol_data{symbol.m_data};

    // Make sure this is a symbol data object related to type info.
    if(symbol_data.is_struct()) {
      return symbol_data;
    } else {
      const auto msg{
        std::format(R"(Given specifier is not a type "{}".)", t_type_id)};

      throw_type_error(msg);
    }
  } else {
    const auto msg{
      std::format(R"(No type found for specifier "{}".)", t_type_id)};

    throw_type_error(msg);
  }

  return {};
}

auto TypeNodeEvaluator::visit(Pointer* t_ptr) -> Any
{
  const auto left{t_ptr->left()};
  const auto pointer_to{resolve(left)};

  const auto indirection{t_ptr->indirection()};
  const auto readonly{t_ptr->readonly()};

  return SymbolData{
    types::symbol::make_pointer(pointer_to, indirection, readonly)};
}

auto TypeNodeEvaluator::visit(Array* t_arr) -> Any
{
  const auto left{t_arr->left()};
  const auto array_of{resolve(left)};
  const auto array_size{t_arr->size()};

  return SymbolData{types::symbol::make_array(array_of, array_size)};
}

auto TypeNodeEvaluator::visit(TypeName* t_type) -> Any
{
  return SymbolData{str2type(t_type->identifier())};
}

auto TypeNodeEvaluator::evaluate(NodePtr t_ast) -> SymbolData
{
  return resolve(t_ast);
}
} // namespace semantic
