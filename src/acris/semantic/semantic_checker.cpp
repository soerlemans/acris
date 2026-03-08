#include "semantic_checker.hpp"

// STL Includes:
#include <algorithm>
#include <any>
#include <iomanip>
#include <memory>
#include <ranges>
#include <sstream>

// Library Includes:
#include <libassert/assert.hpp>

// Absolute Includes:
#include "acris/ast/node/include_nodes.hpp"
#include "acris/debug/debug.hpp"
#include "acris/types/semantic/symbol_data.hpp"

namespace semantic {
// Using Statements:
using diagnostic::throw_type_error;
using types::core::NativeType;
using types::symbol::FnTypePtr;
using types::symbol::StructTypePtr;
using types::symbol::SymbolData;
using types::symbol::VarTypePtr;

NODE_USING_ALL_NAMESPACES()

// Protected methods:
auto SemanticChecker::push_env() -> void
{
  m_symbol_state.push_env();
}

auto SemanticChecker::pop_env() -> void
{
  m_symbol_state.pop_env();
}

auto SemanticChecker::clear_env() -> void
{
  // Reset/clear the construction object.
  m_symbol_state.clear();
}

auto SemanticChecker::str2symbol_data(const std::string_view t_type_id)
  -> SymbolData
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

auto SemanticChecker::node2symbol_data(NodePtr t_type_node) -> SymbolData
{
  return m_node_evaluator.evaluate(t_type_node);
}

auto SemanticChecker::annotate_attr(AttributeData* t_node) -> void
{
  for(const auto& attr : m_active_attrs) {
    t_node->add_attribute(attr);
  }
}

// Environment state related methods:
auto SemanticChecker::add_symbol_declaration(const std::string_view t_key,
                                             const SymbolData& t_data) -> void
{
  // First check if the entry exists.
  const auto [iter, exists] = m_symbol_state.find(t_key);
  if(exists) {
    const auto& symbol{iter->second};

    std::stringstream ss{};
    switch(symbol.m_status) {
      case SymbolStatus::DECLARED:
        [[fallthrough]];
      case SymbolStatus::DEFINED: {
        // Check for conflicting.
        if(t_data != symbol.m_data) {
          ss << "Conflicting declarations and definitions for "
             << std::quoted(t_key) << ".";

          throw_type_error(ss.str());
        }
        break;
      }

      default: {
        using lib::stdexcept::throw_invalid_argument;

        throw_invalid_argument("Unhandled SymbolStatus case.");
        break;
      }
    }
  } else {
    Symbol symbol{SymbolStatus::DECLARED, t_data};
    m_symbol_state.insert({std::string{t_key}, symbol});
  }
}

auto SemanticChecker::add_symbol_definition(const std::string_view t_key,
                                            const SymbolData& t_data) -> void
{
  // First check if the entry already exists.
  const auto [iter, exists] = m_symbol_state.find(t_key);
  if(exists) {
    auto& symbol{iter->second};

    std::stringstream ss{};
    switch(symbol.m_status) {
      case SymbolStatus::DECLARED: {
        // Check symbolData equality.
        if(t_data == symbol.m_data) {
          // Change from declaration to definition.
          symbol.m_status = SymbolStatus::DEFINED;
        } else {
          ss << "Conflicting declaration and definition for "
             << std::quoted(t_key) << ".";

          throw_type_error(ss.str());
        }
        break;
      }

      case SymbolStatus::DEFINED: {
        // Redefinition is not allowed so throw.
        ss << "Redefining " << std::quoted(t_key) << " is not allowed.";

        throw_type_error(ss.str());
        break;
      }

      default: {
        using lib::stdexcept::throw_invalid_argument;

        throw_invalid_argument("Unhandled SymbolStatus case.");
        break;
      }
    }
  } else {
    Symbol symbol{SymbolStatus::DEFINED, t_data};
    m_symbol_state.insert({std::string{t_key}, symbol});
  }
}

auto SemanticChecker::add_struct_member_definition(SymbolData& t_struct,
                                                   const MemberSymbol& t_sym)
  -> void
{
  using lib::stdexcept::throw_runtime_error;

  if(!t_struct.is_struct()) {
    std::ostringstream ss{};

    ss << "Symbol data \"" << t_struct << "\" is not a struct.";

    throw_runtime_error(ss.str());
  }

  // Shared pointer so copying is fine.
  const auto struct_ptr{t_struct.as_struct()};
  auto& members{struct_ptr->m_members};

  const auto inserted{members.insert(t_sym).second};
  if(inserted) {
  }
}

auto SemanticChecker::add_struct_method_definition(SymbolData& t_struct,
                                                   const MethodSymbol& t_sym)
  -> void
{
  using lib::stdexcept::throw_runtime_error;

  if(!t_struct.is_struct()) {
    std::ostringstream ss{};

    ss << "Symbol data \"" << t_struct << "\" is not a struct.";

    throw_runtime_error(ss.str());
  }

  // Shared pointer so copying is fine.
  const auto struct_ptr{t_struct.as_struct()};
  auto& methods{struct_ptr->m_methods};

  const auto inserted{methods.insert(t_sym).second};
  if(inserted) {
  }
}

auto SemanticChecker::get_symbol_data_from_env(const std::string_view t_key)
  -> SymbolData&
{
  return m_symbol_state.get_data(t_key);
}

// Type promotion related methods:
auto SemanticChecker::handle_condition(const SymbolData& t_data,
                                       const TextPosition& t_pos) const -> void
{
  std::stringstream ss{};

  if(const auto opt{t_data.native_type()}; opt) {
    if(!is_condition(opt.value())) {
      ss << "Expected a pointer, integer or a boolean for a conditional "
         << "expression.\n\n";

      ss << t_pos;

      throw_type_error(ss.str());
    }
  } else {
    ss << "Non native types can not casted to " << std::quoted("bool")
       << ".\n\n";

    ss << t_pos;

    throw_type_error(ss.str());
  }
}

auto SemanticChecker::get_symbol_data(NodePtr t_ptr) -> SymbolData
{
  using lib::stdexcept::throw_bad_any_cast;

  SymbolData data{};

  const auto any{traverse(t_ptr)};
  if(any.has_value()) {
    try {
      data = std::any_cast<SymbolData>(any);
    } catch(const std::bad_any_cast& err) {
      DBG_CRITICAL(err.what());


      throw_bad_any_cast(err.what());
    }
  }

  return data;
}

auto SemanticChecker::get_resolved_result_type(NodePtr t_ptr) -> SymbolData
{
  return get_symbol_data(t_ptr).resolve_result_type();
}

auto SemanticChecker::get_type_list(NodeListPtr t_list) -> SymbolDataList
{
  SymbolDataList list;

  for(const auto& ptr : *t_list) {
    list.push_back(get_symbol_data(ptr));
  }

  return list;
}

auto SemanticChecker::get_resolved_result_type_list(NodeListPtr t_list)
  -> SymbolDataList
{
  SymbolDataList list;

  for(const auto& ptr : *t_list) {
    list.push_back(get_resolved_result_type(ptr));
  }

  return list;
}

// Public methods:
SemanticChecker::SemanticChecker()
  : m_validator{},
    m_symbol_state{},
    m_node_evaluator{m_symbol_state},
    m_active_attrs{},
    m_active_struct{},
    m_annot_queue{}
{}

// Control:
auto SemanticChecker::visit(If* t_if) -> Any
{
  const auto init_expr{t_if->init_expr()};
  const auto then{t_if->then()};
  const auto alt{t_if->alt()};

  // Init expression must be evaluated before condition.
  if(init_expr) {
    traverse(init_expr);
  }

  // The condition is resolved to a type.
  const auto cond{get_symbol_data(t_if->condition())};
  DBG_INFO("Condition: ", cond);

  handle_condition(cond, t_if->position());

  // Branch traversal:
  traverse(then);

  if(alt) {
    traverse(alt);
  }

  return {};
}

auto SemanticChecker::visit(Loop* t_loop) -> Any
{
  // Init expression must be evaluated before condition.
  traverse(t_loop->init_expr());

  // A loops condition maybe empty, which is an endless loop.
  if(t_loop->condition()) {
    const auto cond{get_symbol_data(t_loop->condition())};

    DBG_INFO("Condition: ", cond);

    handle_condition(cond, t_loop->position());
  }

  traverse(t_loop->body());
  traverse(t_loop->expr());

  return {};
}

AST_VISITOR_STUB(SemanticChecker, Continue)
AST_VISITOR_STUB(SemanticChecker, Break)
AST_VISITOR_STUB(SemanticChecker, Defer)

auto SemanticChecker::visit(Return* t_return) -> Any
{
  // FIXME: Check if type of expression being returned.
  // Matches the functions return type.
  const auto data{get_symbol_data(t_return->expr())};

  return data;
}

// Function:
auto SemanticChecker::visit(Parameter* t_param) -> Any
{
  const auto type{node2symbol_data(t_param->type())};

  t_param->set_type(type.type_variant());

  return SymbolData{type};
}

auto SemanticChecker::visit(Function* t_fn) -> Any
{
  using types::symbol::make_function;
  using types::symbol::make_variable;

  const auto id{t_fn->identifier()};
  const auto ret_type{node2symbol_data(t_fn->type())};
  const auto params{t_fn->params()};
  const auto body{t_fn->body()};

  // Register function type signature to environment.
  const auto params_type_list{get_type_list(params)};
  const SymbolData data{make_function(params_type_list, ret_type)};

  // Add the function and ID to the environment.
  add_symbol_definition(id, data);
  DBG_INFO("Function: ", id, "(", params_type_list, ") -> ", ret_type);

  // Annotate AST.
  m_annot_queue.push({t_fn, data});
  annotate_attr(t_fn);

  // Register parameters to environment.
  push_env();
  for(const auto& node : *params) {
    using types::symbol::Mutability;

    // Gain a raw ptr (non owning).
    // If the AST changes the assertion will be triggered.
    const auto* param{dynamic_cast<Parameter*>(node.get())};
    DEBUG_ASSERT(param != nullptr, R"(Was unable to cast to "*Parameter"!)");

    const auto id{param->identifier()};
    const auto type{node2symbol_data(param->type())};

    const SymbolData data{make_variable(Mutability::IMMUTABLE, type)};

    // Add parameter to environment.
    add_symbol_definition(id, data);
  }

  // Run type checking on the function body.
  traverse(body);
  pop_env();

  return {};
}

auto SemanticChecker::visit(FunctionCall* t_fn_call) -> Any
{
  // TODO: Improve this code to be more generic and clean, error if this is
  // not a function name
  const auto id{t_fn_call->identifier()};

  const auto fn_data{get_symbol_data_from_env(id)};
  const auto args{get_resolved_result_type_list(t_fn_call->args())};

  const auto fn{fn_data.as_function()};
  const auto params{fn->m_params};
  const auto return_type{fn->m_return_type};

  if(args != params) {
    std::stringstream ss;

    ss << "Arguments passed to " << std::quoted(id)
       << " do not match parameters.\n";

    ss << "Function signature: fn " << id << "(" << params << ") -> "
       << return_type << " { ... }\n";

    ss << "FunctionCall signature: " << id << "(" << args << ")";

    throw_type_error(ss.str());
  }

  return fn->m_return_type;
}

auto SemanticChecker::visit(ReturnType* t_rt) -> Any
{
  const auto type{t_rt->type()};

  return SymbolData{node2symbol_data(type)};
}

// Lvalue:
// TODO: FIXME Disallow redeclaration of variables.
auto SemanticChecker::visit(Let* t_let) -> Any
{
  using types::symbol::make_variable;
  using types::symbol::Mutability;

  const auto init_expr{t_let->init_expr()};
  const std::string id{t_let->identifier()};
  const auto type{t_let->type()};
  const auto pos{t_let->position()};

  // Extract the SymbolData if an init expression was given.
  SymbolDataOpt init_opt{};
  if(init_expr) {
    init_opt = get_symbol_data(init_expr);
  }

  SymbolDataOpt type_opt{};
  if(type) {
    type_opt = node2symbol_data(type);
  }

  const BindingExprData data{init_opt, id, type_opt, pos};
  const auto type_data{m_validator.validate_binding_expr(data)};

  const SymbolData var_data{make_variable(Mutability::IMMUTABLE, type_data)};
  add_symbol_definition(id, var_data);

  // Annotate AST.
  m_annot_queue.push({t_let, type_data});
  annotate_attr(t_let);

  return {};
}

auto SemanticChecker::visit(Var* t_var) -> Any
{
  using types::symbol::make_variable;
  using types::symbol::Mutability;

  const auto init_expr{t_var->init_expr()};
  const std::string id{t_var->identifier()};
  const auto type{t_var->type()};
  const auto pos{t_var->position()};

  // Extract the SymbolData if an init expression was given.
  SymbolDataOpt init_opt{};
  if(init_expr) {
    // Resolve the type.
    init_opt = get_symbol_data(init_expr).resolve_result_type();

    DBG_INFO("init_opt", init_opt.value());
  }

  SymbolDataOpt type_opt{};
  if(type) {
    type_opt = node2symbol_data(type);
  }

  const BindingExprData data{init_opt, id, type_opt, pos};
  const auto type_data{m_validator.validate_binding_expr(data)};

  const SymbolData var_data{make_variable(Mutability::MUTABLE, type_data)};
  add_symbol_definition(id, var_data);

  // Annotate AST.
  m_annot_queue.push({t_var, type_data});
  annotate_attr(t_var);

  return {};
}

auto SemanticChecker::visit(IdentifierNode* t_id) -> Any
{
  const auto id{t_id->identifier()};
  const auto var_data{get_symbol_data_from_env(id)};

  DBG_INFO("IdentifierNode ", std::quoted(id), " of type ", var_data);

  // Annotate AST.
  m_annot_queue.push({t_id, var_data});

  return {var_data};
}

auto SemanticChecker::visit(Subscript* t_subscript) -> Any
{
  SymbolData type{};

  const auto var_data{get_symbol_data(t_subscript->left())};
  const auto index_data{get_symbol_data(t_subscript->left())};

  // TODO: Move to type validator.
  // TODO: Check index data if numeric.
  DBG_INFO("Subscript of type ", var_data);

  auto result_data{var_data.resolve_result_type()};
  if(result_data.is_array()) {
    const auto array_ptr{result_data.as_array()};

    type = array_ptr->m_type;
  } else {
    // TODO: Throw.
  }

  // Annotate AST.
  m_annot_queue.push({t_subscript, type});

  // return {var_data};

  return type;
}

auto SemanticChecker::visit(ScopeResolution* t_scope_res) -> Any
{
  SymbolData type{};

  const auto scope_path{t_scope_res->path()};
  auto expr{t_scope_res->expr()};

  DEBUG_ASSERT(!scope_path.empty(), R"(Empty scope path cant be resolved.)");

  // For now only resolve in-module.
  const auto entity_id{scope_path.front()};

  const auto [iter, exists] = m_symbol_state.find(entity_id);
  if(exists) {
    auto& [status, type_data] = iter->second;

    if(type_data.is_enum()) {
      const auto enum_type{type_data.as_enum()};
      auto* id_node{dynamic_cast<IdentifierNode*>(expr.get())};
      DEBUG_ASSERT(id_node != nullptr, R"(Must be an IdentifierNode.)");

      const std::string field_id{id_node->identifier()};
      const auto has_field{enum_type->m_fields.contains(field_id)};
      if(!has_field) {
        const auto msg{
          std::format("Enum {} has no field named {}.", entity_id, field_id)};

        throw_type_error(msg);
      }

      // Annotate Queue.
      m_annot_queue.push({t_scope_res, type_data});

      // Set return value.
      type = type_data;
    }
  }

  return type;
}

// Meta:
auto SemanticChecker::visit(Attribute* t_attr) -> Any
{
  using ast::node::node_traits::AttributeArgs;

  const auto id{t_attr->identifier()};
  const auto params{t_attr->params()};
  const auto body{t_attr->body()};

  // TODO: Resolve parameters, and get them as strings.
  AttributeArgs args{};
  AttributeMetadata attr{id, std::move(args)};

  // Annotate attribute.
  t_attr->add_attribute(attr);

  // Push the current attribute on the context stack.
  // So we can refer to them later.
  m_active_attrs.push_back(attr);

  // Traverse body like normal.
  traverse(body);

  m_active_attrs.pop_back();

  return {};
}

auto SemanticChecker::visit(LetDecl* t_ldecl) -> Any
{
  using types::symbol::make_variable;
  using types::symbol::Mutability;

  const auto id{t_ldecl->identifier()};
  const auto type_data{node2symbol_data(t_ldecl->type())};

  // Create the SymbolData for a variable.
  const SymbolData data{make_variable(Mutability::IMMUTABLE, type_data)};
  add_symbol_declaration(id, data);

  // Annotate AST.
  m_annot_queue.push({t_ldecl, type_data});
  annotate_attr(t_ldecl);

  return {};
}

auto SemanticChecker::visit(VarDecl* t_vdecl) -> Any
{
  using types::symbol::make_variable;
  using types::symbol::Mutability;

  const auto id{t_vdecl->identifier()};
  const auto type_data{node2symbol_data(t_vdecl->type())};

  // Create the SymbolData for a variable.
  const SymbolData data{make_variable(Mutability::MUTABLE, type_data)};
  add_symbol_declaration(id, data);

  // Annotate AST.
  m_annot_queue.push({t_vdecl, type_data});
  annotate_attr(t_vdecl);

  return {};
}

auto SemanticChecker::visit(FunctionDecl* t_fdecl) -> Any
{
  using types::symbol::make_function;

  const auto id{t_fdecl->identifier()};
  const auto ret_type{node2symbol_data(t_fdecl->type())};
  const auto params{t_fdecl->params()};

  // Register function type signature to environment.
  const auto params_type_list{get_type_list(params)};
  const SymbolData data{make_function(params_type_list, ret_type)};

  add_symbol_declaration(id, data);
  DBG_INFO("FunctionDecl: ", id, "(", params_type_list, ") -> ", ret_type);

  // Annotate AST.
  m_annot_queue.push({t_fdecl, data});
  annotate_attr(t_fdecl);

  return {};
}

// Operators:
auto SemanticChecker::visit(Arithmetic* t_arith) -> Any
{
  // const auto op{t_arith->op()};
  const auto lhs{get_symbol_data(t_arith->left())};
  const auto rhs{get_symbol_data(t_arith->right())};
  const auto pos{t_arith->position()};

  DBG_INFO("Typeof lhs: ", lhs);
  DBG_INFO("Typeof rhs: ", rhs);

  BinaryOperationData data{lhs, rhs, pos};
  const auto ret{m_validator.validate_arithmetic(data)};

  // Annotate AST.
  m_annot_queue.push({t_arith, ret});

  return ret;
}

auto SemanticChecker::visit(Assignment* t_assign) -> Any
{
  const auto var{get_symbol_data(t_assign->left())};
  const auto expr{get_resolved_result_type(t_assign->right())};

  const auto pos{t_assign->position()};

  DBG_INFO("Typeof var: ", var);
  DBG_INFO("Typeof expr: ", expr);

  BinaryOperationData data{var, expr, pos};
  const auto ret{m_validator.validate_assignment(data)};

  // Annotate AST.
  m_annot_queue.push({t_assign, ret});

  return ret;
}

auto SemanticChecker::visit(Comparison* t_comp) -> Any
{
  const auto lhs{get_symbol_data(t_comp->left())};
  const auto rhs{get_symbol_data(t_comp->right())};
  const auto pos{t_comp->position()};

  DBG_INFO("Typeof lhs: ", lhs);
  DBG_INFO("Typeof rhs: ", rhs);

  BinaryOperationData data{lhs, rhs, pos};
  const auto ret{m_validator.validate_comparison(data)};

  // TODO: Annotate types.
  m_annot_queue.push({t_comp, ret});

  return ret;
}

auto SemanticChecker::visit(Increment* t_inc) -> Any
{
  const auto left{t_inc->left()};
  const auto left_data{get_symbol_data(left)};

  // Return underlying type.
  if(left_data.is_ptr()) {
    const auto ptr{left_data.as_ptr()};

    return ptr->m_type;
  }

  const auto opt{left_data.native_type()};
  if(!opt) {
    // TODO: Add position
    throw_type_error("Trying to increment a non native type is illegal.");
  }

  if(!is_integer(opt.value())) {
    std::stringstream ss;

    ss << "Trying to increment a variable that is not an integer.";

    throw_type_error(ss.str());
  }

  return SymbolData{opt.value()};
}

auto SemanticChecker::visit(Decrement* t_dec) -> Any
{
  const auto left{t_dec->left()};
  const auto left_data{get_symbol_data(left)};

  // Return underlying type.
  if(left_data.is_ptr()) {
    const auto ptr{left_data.as_ptr()};

    return ptr->m_type;
  }

  const auto opt{left_data.native_type()};
  if(!opt) {
    // TODO: Add position
    throw_type_error("Trying to decrement a non native type is illegal.");
  }

  if(!is_integer(opt.value())) {
    std::stringstream ss{};

    ss << "Trying to decrement a variable that is not an integer.";

    throw_type_error(ss.str());
  }

  return SymbolData{opt.value()};
}

auto SemanticChecker::visit(AddressOf* t_addr_of) -> Any
{
  using types::symbol::make_pointer;

  const auto left{get_symbol_data(t_addr_of->left())};
  auto resolved{left.resolve_result_type()};
  if(resolved.is_array()) {
    // When address off array cast to underlying element type.
    resolved = resolved.as_array()->m_type;
  }

  const bool readonly{left.is_mutable() == false};
  auto pointer{make_pointer(resolved, 1, readonly)};

  // Annotate AST.
  m_annot_queue.push({t_addr_of, pointer});

  return pointer;
}

auto SemanticChecker::visit(Dereference* t_deref) -> Any
{
  using types::symbol::make_pointer;

  const auto left{get_symbol_data(t_deref->left())};

  // Remove variable mapping.
  const auto result_type{left.resolve_result_type()};

  // TODO: Test to make sure if pointer type else error.

  // Get underlying pointer.
  const auto ptr{result_type.as_ptr()};

  return ptr->m_type;
}

auto SemanticChecker::visit(UnaryPrefix* t_up) -> Any
{
  const auto left{get_symbol_data(t_up->left())};

  // TODO: Implement, check if it is a integer type.

  return left;
}

auto SemanticChecker::visit(ToCast* t_cast) -> Any
{
  const auto left{get_symbol_data(t_cast->left())};
  const auto type_data{node2symbol_data(t_cast->right())};

	// Annotate AST.
  m_annot_queue.push({t_cast, type_data});

  // TODO: Check if cast is possible.

  return type_data;
}

// Logical:
auto SemanticChecker::visit(Not* t_not) -> Any
{
  const auto lhs{get_symbol_data(t_not->left())};
  const auto pos{t_not->position()};

  DBG_INFO("Typeof lhs: ", lhs);

  UnaryOperationData data{lhs, pos};

  return m_validator.validate_logical_unop(data);
}

// TODO: Create a helper method for these types of type checks
auto SemanticChecker::visit(And* t_and) -> Any
{
  const auto lhs{get_symbol_data(t_and->left())};
  const auto rhs{get_symbol_data(t_and->right())};
  const auto pos{t_and->position()};

  DBG_INFO("Typeof lhs: ", lhs);
  DBG_INFO("Typeof rhs: ", rhs);

  BinaryOperationData data{lhs, rhs, pos};

  return m_validator.validate_logical_binop(data);
}

auto SemanticChecker::visit(Or* t_or) -> Any
{
  const auto lhs{get_symbol_data(t_or->left())};
  const auto rhs{get_symbol_data(t_or->right())};
  const auto pos{t_or->position()};

  DBG_INFO("Typeof lhs: ", lhs);
  DBG_INFO("Typeof rhs: ", rhs);

  BinaryOperationData data{lhs, rhs, pos};

  return m_validator.validate_logical_binop(data);
}

// Packaging:
AST_VISITOR_STUB(SemanticChecker, Import)
AST_VISITOR_STUB(SemanticChecker, ModuleDecl)

// Rvalue:
auto SemanticChecker::visit([[maybe_unused]] Float* t_float) -> Any
{
  return SymbolData{NativeType::F64};
}

auto SemanticChecker::visit([[maybe_unused]] Integer* t_int) -> Any
{
  return SymbolData{NativeType::INT};
}

auto SemanticChecker::visit([[maybe_unused]] Char* t_ch) -> Any
{
  return SymbolData{NativeType::CHAR};
}

auto SemanticChecker::visit([[maybe_unused]] String* t_str) -> Any
{
  return SymbolData{NativeType::CSTR};
}

auto SemanticChecker::visit([[maybe_unused]] Boolean* t_bool) -> Any
{
  return SymbolData{NativeType::BOOL};
}

auto SemanticChecker::visit([[maybe_unused]] ArrayExpr* t_arr) -> Any
{
  using types::symbol::make_array;

  SymbolData type{};
  NodeListPtr list{t_arr->get()};

  // TODO: Move to type validator.
  if(!list->empty()) {
    // We need to deduce type from the first elem.
    auto iter{list->begin()};
    type = get_symbol_data(*iter);

    // Increment past first entry.
    iter++;
    for(; iter != list->end(); iter++) {
      // TODO: Check if all elements are the same type.
    }
  } else {
    using lib::stdexcept::throw_invalid_argument;

    throw_invalid_argument("ArrayExpr literal list should contain entries.");
  }

  const auto size{list->size()};

  auto symbol_data{make_array(type, size)};

  return symbol_data;
}

// User Types:
auto SemanticChecker::visit(EnumField* t_field) -> Any
{
  // Nothing to semantically validate yet, until iota comes along.
  // The value of an enum should be computed at compile time.

  // Also check for enum overflow values.

  return {};
}

auto SemanticChecker::visit(Enum* t_enum) -> Any
{
  using types::symbol::IdentifierSet;
  using types::symbol::make_enum;

  const std::string enum_id{t_enum->identifier()};
  const auto type{t_enum->type()}; // Underlying type.

  // Default type for enums is an int.
  // TODO: Have enum fields decide the underlying enum type.
  SymbolData enum_ut{NativeType::INT};

  // Underlying type can be optionally specified.
  if(type) {
    enum_ut = node2symbol_data(type);
  }

  const auto enum_body{t_enum->body()};

  IdentifierSet id_set{};
  for(const auto& node : *enum_body) {
    const auto* enum_field{dynamic_cast<EnumField*>(node.get())};
    DEBUG_ASSERT(enum_field != nullptr,
                 R"(Was unable to cast to "*EnumField"!)");

    // TODO: Check if iota, is only used once.

    // Store enum identifier names.
    id_set.emplace(enum_field->identifier());
  }

  const auto enum_data{make_enum(enum_id, enum_ut, id_set)};

  add_symbol_definition(enum_id, enum_data);

  DBG_INFO("Enum: ", enum_data);

  // Annotate AST.
  m_annot_queue.push({t_enum, enum_data});

  return {};
}

auto SemanticChecker::visit(Method* t_meth) -> Any
{
  using types::symbol::make_function;
  using types::symbol::make_variable;
  using types::symbol::MethodSymbol;

  auto recv_type{str2symbol_data(t_meth->get_receiver())};

  const auto id{t_meth->identifier()};
  const auto ret_type{node2symbol_data(t_meth->type())};
  const auto params{t_meth->params()};

  if(!recv_type.is_struct()) {
    throw_type_error("Receiver type for method is not a struct.");
  }

  // // Register function type signature to Struct.
  const auto params_type_list{get_type_list(params)};
  const SymbolData data{make_function(params_type_list, ret_type)};

  // Add the function and ID to the environment.
  MethodSymbol sym{id, data};
  add_struct_method_definition(recv_type, sym);
  DBG_INFO("Method: (", recv_type, ") ", id, "(", params_type_list, ") -> ",
           ret_type);

  // Annotate AST.
  m_annot_queue.push({t_meth, data});
  annotate_attr(t_meth);

  // Register parameters to environment.
  push_env();
  for(const auto& node : *params) {
    using types::symbol::Mutability;

    // Gain a raw ptr (non owning).
    // If the AST changes the assertion will be triggered.
    const auto* param{dynamic_cast<Parameter*>(node.get())};
    DEBUG_ASSERT(param != nullptr, R"(Was unable to cast to "Parameter*"!)");

    const auto id{param->identifier()};
    const auto type{node2symbol_data(param->type())};

    const SymbolData data{make_variable(Mutability::MUTABLE, type)};

    // Add parameter to environment.
    add_symbol_definition(id, data);
  }

  m_active_struct = recv_type.as_struct();

  // Run type checking on the function body.
  traverse(t_meth->body());

  m_active_struct = std::nullopt;
  pop_env();

  return {};
}

AST_VISITOR_STUB(SemanticChecker, Interface)

auto SemanticChecker::visit(MemberDecl* t_member) -> Any
{
  const auto type{node2symbol_data(t_member->type())};

  // Annotate AST.
  m_annot_queue.push({t_member, type});

  return {};
}

auto SemanticChecker::visit(Struct* t_struct) -> Any
{
  using types::symbol::make_struct;
  using types::symbol::MemberMap;

  const std::string struct_id{t_struct->identifier()};
  const auto struct_body{t_struct->body()};

  // First annotate all the member declarations.
  traverse(struct_body);

  // Loop through member declarations and add them to the member map.
  MemberMap members{};
  for(const auto& node : *struct_body) {
    // Gain a raw ptr (non owning).
    // If the AST changes the assertion will be triggered.
    const auto* member_decl{dynamic_cast<MemberDecl*>(node.get())};
    DEBUG_ASSERT(member_decl != nullptr,
                 R"(Was unable to cast to "*MemberDecl"!)");

    const std::string member_id{member_decl->identifier()};
    const SymbolData member_type{node2symbol_data(member_decl->type())};

    members.insert({member_id, member_type});
  }

  const auto struct_data{make_struct(struct_id, members)};

  add_symbol_definition(struct_id, struct_data);

  DBG_INFO("Struct: ", struct_data);

  // Annotate AST.
  m_annot_queue.push({t_struct, struct_data});

  return {};
}

// TODO: Rename SelfAcceptor?
auto SemanticChecker::visit([[maybe_unused]] Self* t_self) -> Any
{
  using lib::stdexcept::throw_runtime_error;

  if(!m_active_struct) {
    throw_runtime_error(
      "Keyword self used without active struct in method context.");
  }

  return SymbolData{m_active_struct.value()};
}

auto SemanticChecker::visit(Member* t_member) -> Any
{
  const auto id{t_member->identifier()};
  // Rewrite:
  // const auto var_data{get_symbol_data_from_env(id)};

  // DBG_INFO("Member ", std::quoted(id), " of type ", var_data);

  // Annotate AST.
  // m_annot_queue.push({t_member, var_data});

  // return {var_data};
  return SymbolData{NativeType::INT}; // FIXME: hardcoded cuz stupid.
}

auto SemanticChecker::visit(MemberAccess* t_access) -> Any
{
  const auto left{t_access->left()};
  const auto right{t_access->right()};
  const auto pos{t_access->position()};

  const auto lhs{get_resolved_result_type(left)};

  // Check type of left side, check if operation on the right side is possible.
  // Always return result of right side operation.

  // Evaluate chain expressions by setting an active access var.

  // dynamic_cast<MethodCall>(right);
  // const auto rhs{get_resolved_result_type(right)};

  // const auto rhs{get_resolved_result_type(right)};
  return SymbolData{NativeType::INT};
}

auto SemanticChecker::check(NodePtr t_ast) -> void
{
  // Semantically check the AST for correctness.
  traverse(t_ast);

  // Annotate AST nodes with type info.
  m_annot_queue.annotate_ast();

  // Clear EnvState and release old SymbolTable ptr.
  clear_env();
}
} // namespace semantic
