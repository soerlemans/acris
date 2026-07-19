#include "cpp_backend.hpp"

// STL Includes:
#include <algorithm>
#include <format>
#include <fstream>
#include <sstream>
#include <string_view>

// Library Includes:
#include <libassert/assert.hpp>

// Absolute Includes:
#include "acris/ast/node/include_nodes.hpp"
#include "acris/codegen/cpp_backend/interop/lua_backend/lua_backend.hpp"
#include "acris/codegen/cpp_backend/interop/python_backend/python_backend.hpp"
#include "acris/debug/log.hpp"
#include "lib/stdexcept/stdexcept.hpp"
#include "lib/stdtypes.hpp"

// Local Includes:
#include "clang_frontend_invoker.hpp"
#include "type2cpp.hpp"

namespace codegen::cpp_backend {
// Using:
using namespace ast::visitor;
using namespace std::literals::string_view_literals;

NODE_USING_ALL_NAMESPACES()

// Methods:
// Protected:
auto CppBackend::prologue() -> std::string
{
  std::ostringstream oss{};

  // Acris's native types often translate.
  // To C++ fixed width integers and floats.
  oss << "// STL Includes:\n";
  // oss << "#include <stdfloat>\n"; // TODO: Uncommnet when supported by clang.
  // We include cstdio for FILE struct as we cant forward declare it.
  // oss << "#include <cstdio>\n";
  oss << "\n";

  oss << "// Stdacris Includes:\n";
  if(no_libc()) {
    oss << R"(#include "stdacris/core/linux/core.h")" << "\n";
  }
  oss << R"(#include "stdacris/internal/internal.hpp")" << "\n\n";

  // Loop through the interop backends and add the prologue from each backend.
  for(auto& ptr : m_interop_backends) {
    oss << ptr->prologue();
  }

  oss << "\n\n";

  // TODO: Consider using this?
  oss << "// Macros:\n";
  oss << "#define acris_self (*this)\n";
  oss << "\n\n";

  oss << "// Aliases:\n";
  oss << "namespace stdinternal = stdlibacris::internal;\n";


  return oss.str();
}

auto CppBackend::epilogue() -> std::string
{
  std::ostringstream oss{};

  // Loop through the interop backends and add the epilogue.
  for(auto& ptr : m_interop_backends) {
    oss << ptr->epilogue();
  }

  return oss.str();
}

auto CppBackend::should_terminate() -> bool
{
  return m_terminate.top();
}

auto CppBackend::terminate() -> std::string_view
{
  auto terminate{";\n"sv};

  // Remove semicolon if we should not terminate.
  [[unlikely]]
  if(!should_terminate()) {
    // Dont print anything.
    terminate = "";
  }

  return terminate;
}

// TODO: Add inline option for direct resolution.
auto CppBackend::resolve(NodePtr t_ptr, const bool t_terminate) -> std::string
{
  std::ostringstream oss{};

  if(t_ptr) {
    // Keep track of if the current node we are traversing should be terminated.
    m_terminate.push(t_terminate);
    const auto any{traverse(t_ptr)};
    m_terminate.pop();

    try {
      oss << std::any_cast<std::string>(any);
    } catch(std::bad_any_cast& exception) {
      lib::stdexcept::throw_bad_any_cast(exception.what());
    }
  }

  return oss.str();
}

auto CppBackend::resolve_list(NodeListPtr t_list, const bool t_terminate)
  -> std::string
{
  std::ostringstream oss{};

  for(NodePtr& node : *t_list) {
    oss << resolve(node, t_terminate);
  }

  return oss.str();
}

auto CppBackend::handle_attribute_export() -> void
{}

// Public:
CppBackend::CppBackend()
  : m_inv{},
    m_ctx{},
    m_session{},
    m_interop_backends{},
    m_terminate{},
    m_id_defer_count{0}
{}

// Control:
auto CppBackend::visit(If* t_if) -> Any
{
  const auto init_expr{resolve(t_if->init_expr())};
  const auto cond{resolve(t_if->condition())};

  const auto then{resolve(t_if->then(), true)};
  const auto alt{resolve(t_if->alt(), true)};

  std::ostringstream oss{};

  oss << std::format("if({}; {}) {{\n", init_expr, cond) << then;

  // Dont create else branch if we dont have a statement for it.
  if(!alt.empty()) {
    oss << "} else {\n" << alt;
  }

  oss << "}\n";

  return oss.str();
}

auto CppBackend::visit(Loop* t_loop) -> Any
{
  const auto init_expr{resolve(t_loop->init_expr())};
  const auto cond{resolve(t_loop->condition())};

  const auto post_expr{resolve(t_loop->expr())};
  const auto body{resolve_list(t_loop->body(), true)};

  std::ostringstream oss{};

  // clang-format off
  oss << std::format("for({};{}; {} )", init_expr, cond, post_expr)
     << "{\n"
     << body
     << "}\n";
  // clang-format on

  return oss.str();
}

auto CppBackend::visit(Switch* t_sw) -> Any
{
  const auto cond{resolve(t_sw->condition())};
  const auto body{resolve_list(t_sw->body(), true)};

  std::ostringstream oss{};

  // clang-format off
  oss << std::format("switch({})", cond)
     << "{\n"
     << body
     << "}\n";
  // clang-format on

  return oss.str();
}

auto CppBackend::visit(SwitchCase* t_case) -> Any
{
  const auto clauses{t_case->clauses()};
  const auto body{resolve_list(t_case->body(), true)};

  std::ostringstream oss{};

  // We support multiple clauses for a single case.
  std::string_view sep{};
  for(auto& clause : *clauses) {
    oss << sep << std::format("case {}:", resolve(clause));

    sep = "[[fallthrough]];\n";
  }

  oss << "{\n" << body;
  if(t_case->has_fallthrough() == false) {
    oss << "break;\n";
  }
  oss << "}\n";

  return oss.str();
}

auto CppBackend::visit(SwitchElse* t_else) -> Any
{
  const auto body{resolve_list(t_else->body(), true)};

  std::ostringstream oss{};

  // clang-format off
  oss << "default: {\n"
			<< body
			<< "break;\n"
			<< "}\n";
  // clang-format on

  return oss.str();
}

auto CppBackend::visit([[maybe_unused]] Fallthrough* t_ft) -> Any
{
  return std::string{"[[fallthrough]];\n"};
}

auto CppBackend::visit([[maybe_unused]] Continue* t_continue) -> Any
{
  return std::format("continue;\n");
}

auto CppBackend::visit([[maybe_unused]] Break* t_break) -> Any
{
  return std::format("break;\n");
}

auto CppBackend::visit(Defer* t_defer) -> Any
{
  std::ostringstream oss{};

  const auto body{resolve_list(t_defer->body(), true)};

  oss << std::format(
    "const stdinternal::Defer defer_object{}{{ [&](){{\n {}\n }} }};\n",
    m_id_defer_count, body);

  m_id_defer_count++;

  return oss.str();
}

auto CppBackend::visit(Return* t_ret) -> Any
{
  const auto expr{resolve(t_ret->expr())};

  return std::format("return {};\n", expr);
}

// Functions:
auto CppBackend::visit(Parameter* t_param) -> Any
{
  const auto id{t_param->identifier()};
  const auto type{type_spec2cpp({t_param->get_type()})};

  return std::format("{} {}", type, id);
}

auto CppBackend::visit(Function* t_fn) -> Any
{
  using lib::stdexcept::InvalidArgument;
  using lib::stdexcept::throwf;
  using node::node_traits::AttributeType;

  const auto identifier{t_fn->identifier()};
  const auto attributes{t_fn->get_attributes()};

  const auto fn_type{t_fn->get_type().as_function()};
  const auto ret_type{type_spec2cpp({fn_type->m_return_type})};

  std::ostringstream oss{};

  // Attribute insertion:
  // TODO: Move to a  generic function implementation (Method will need to use
  // this as well).
  for(const auto& attr : attributes) {
    switch(attr.m_type) {
      case AttributeType::INLINE:
        oss << "inline\n";
        break;

      case AttributeType::EXPORT: {
        auto& args{attr.m_args};
        if(args.size() != 1) {
          throwf<InvalidArgument>(
            "Attribute: Export expects exactly one parameter "
            "denoting the language to export to.");
        }

        const auto& target_lang{args.front()};
        for(auto& iback : m_interop_backends) {
          const auto iback_id{iback->backend_id()};

          if(target_lang == iback_id) {
            // FIXME: We should do this by looping through the toplevel.
            // Of the SymbolTable instead.
            // As the symboltable should also have attribute data.
            // And this is expensive and shitty.

            // We only expect a single target language for now so lazy is good.
            iback->register_function(identifier, fn_type);
            break;
          }
        }

        break;
      }

      default:
        // Unhandled ignore.
        break;
    }
  }

  std::ostringstream param_ss{};

  auto sep{""sv};
  const auto params{t_fn->params()};
  for(const auto& param : *params) {
    param_ss << sep << resolve(param);

    sep = ", ";
  }

  // clang-format off
	// We use regular function syntax instead trailing return type.
	// Cause trailing return
  oss << std::format("{} {}({})\n", ret_type, identifier, param_ss.str())
     << "{\n"
		 << resolve_list(t_fn->body(), true)
     << "}\n";
  // clang-format on

  return oss.str();
}

auto CppBackend::visit(FunctionCall* t_call) -> Any
{
  // FIXME: When get non shitty import resolution.
  // const auto identifier{t_call->identifier()};
  auto identifier{t_call->identifier()};
  const auto args{t_call->args()};

  // FIXME: This wont work for a raw function or method call.
  // As when we assign it directly to a variable it will work but not else.
  // As we need to append a semicolon.

  // FIXME: Figure out a way to detect if this function call is inline.
  // Or if this function is being called without as a statement.
  // (I hope this is doable).

  std::ostringstream oss{};
  std::string_view sep{""};

  for(const auto& ptr : *args) {
    const auto argument{resolve(ptr)};
    oss << sep << argument;

    sep = ", ";
  }

  const auto arguments{oss.str()};

  return std::format("{}({}){}", identifier, arguments, terminate());
}

auto CppBackend::visit([[maybe_unused]] ReturnType* t_rt) -> Any
{
  std::ostringstream oss{};

  // Currently we do not dynamically compute return types.
  // So for now converting a type_spec2cpp({) is good enough.

  return oss.str();
}

// Lvalue:
// TODO: Reduce code duplication between the Let and Var methods.
auto CppBackend::visit(Let* t_let) -> Any
{
  const auto id{t_let->identifier()};
  const auto init_expr{t_let->init_expr()};

  const auto type_variant{t_let->get_type()};
  const auto type{type_spec2cpp({type_variant})};

  const auto terminate_str{terminate()};

  if(init_expr) {
    const auto init_expr_str{resolve(init_expr)};

    return std::format("{} const {} = {}{}", type, id, init_expr_str,
                       terminate_str);
  } else {
    return std::format("{} const {}{{}}{}", type, id, terminate_str);
  }
}

auto CppBackend::visit(Var* t_var) -> Any
{
  const auto id{t_var->identifier()};
  const auto init_expr{t_var->init_expr()};

  const auto type_variant{t_var->get_type()};
  const auto type{type_spec2cpp({type_variant})};

  const auto terminate_str{terminate()};

  if(init_expr) {
    const auto init_expr_str{resolve(init_expr)};

    return std::format("{} {} = {}{}", type, id, init_expr_str, terminate_str);
  } else {
    return std::format("{} {}{{}}{}", type, id, terminate_str);
  }
}

auto CppBackend::visit(IdentifierNode* t_id) -> Any
{
  const auto identifier{t_id->identifier()};

  return std::format("{}", identifier);
}

auto CppBackend::visit(Subscript* t_subscript) -> Any
{
  const auto expr{resolve(t_subscript->left())};
  const auto index_expr{resolve(t_subscript->right())};

  return std::format("{}[{}]", expr, index_expr);
}

auto CppBackend::visit(ScopeResolution* t_scope_res) -> Any
{
  const auto scope_path{t_scope_res->path()};
  const auto expr{t_scope_res->expr()};

  DEBUG_ASSERT(!scope_path.empty(), R"(Empty scope path cant be resolved.)");

  std::ostringstream oss{};

  std::string_view sep{""};
  for(auto&& elem : scope_path) {
    oss << sep << elem;

    sep = "::";
  }

  oss << "::" << resolve(expr);

  return oss.str();
}

// Meta:
auto CppBackend::visit(Attribute* t_attr) -> Any
{
  using node::node_traits::AttributeType;

  std::ostringstream oss{};

  const auto id{t_attr->identifier()};
  const auto params{t_attr->params()};
  const auto body{t_attr->body()};

  // TODO: This should probably be somewhere else.
  // Also we should not allow the extern attribute, inside of function
  // bodies, for example attributes for variables.
  const auto attrs{t_attr->get_attributes()};
  for(const auto& attr : attrs) {
    switch(attr.m_type) {
      case AttributeType::EXTERN:
        // clang-format off
			  oss << R"(extern "C" {)" << "\n"
			  		<< resolve_list(body, true)
			  	  << "}\n";
        // clang-format on
        break;

      default:
        // Walk the body like normal.
        oss << resolve_list(body, true);
        break;
    }
  }

  return oss.str();
}

auto CppBackend::visit(LetDecl* t_ldecl) -> Any
{
  const auto identifier{t_ldecl->identifier()};

  const auto type_variant{t_ldecl->get_type()};
  const auto type{type_spec2cpp({type_variant})};

  return std::format("extern const {} {};\n", type, identifier);
}

auto CppBackend::visit(VarDecl* t_vdecl) -> Any
{
  const auto identifier{t_vdecl->identifier()};

  const auto type_variant{t_vdecl->get_type()};
  const auto type{type_spec2cpp({type_variant})};

  return std::format("extern {} {};\n", type, identifier);
}

auto CppBackend::visit(FunctionDecl* t_fdecl) -> Any
{
  const auto identifier{t_fdecl->identifier()};

  const auto fn_type{t_fdecl->get_type().as_function()};
  const auto ret_type{type_spec2cpp({fn_type->m_return_type})};

  std::ostringstream param_ss{};

  auto sep{""sv};
  const auto params{t_fdecl->params()};
  for(const auto& param : *params) {
    param_ss << sep << resolve(param);

    sep = ", ";
  }

  return std::format("{} {}({});\n", ret_type, identifier, param_ss.str());
}

auto CppBackend::visit(StructDecl* t_sdecl) -> Any
{
  const auto struct_id{t_sdecl->identifier()};

  const auto attrs{t_sdecl->get_attributes()};
  for(const auto& attr : attrs) {
    switch(attr.m_type) {
      case AttributeType::NO_CODEGEN:
        // No generation means no code for statement.
        return std::string{};

      case AttributeType::OVERRIDE_CODEGEN: {
        // TODO: Fix this shitty hack.
        if(attr.m_args.size() != 2) {
          // TODO: Throw.
        }
        const auto& target_backend{attr.m_args.front()};
        if(target_backend == "cpp") {
          const auto& override_code{attr.m_args[1]};

          // Replace regular generated code.
          return override_code;
        }
      }

      default:
        // TODO: error.
        break;
    }
  }

  // Need to have attributes affect StructDecl, and skip forward declaration.
  // On MacOS X as it breaks for forward declaring FILE in extern C context.
  // See libc.ac.
  return std::format("struct {}{}", struct_id, terminate());
}

// Operators:
auto CppBackend::visit(Arithmetic* t_arith) -> Any
{
  const auto op{t_arith->op2str()};

  const auto left{resolve(t_arith->left())};
  const auto right{resolve(t_arith->right())};

  // We surround the sub expressions in parenthesis to enforce the
  // precedence, Of Acris over C++.
  return std::format("({}) {} ({})", left, op, right);
}

auto CppBackend::visit(Assignment* t_assign) -> Any
{
  const auto op{t_assign->op2str()};

  const auto left{resolve(t_assign->left())};
  const auto right{resolve(t_assign->right())};

  return std::format("{} {} {};\n", left, op, right);
}

auto CppBackend::visit(Comparison* t_comp) -> Any
{
  const auto op{t_comp->op2str()};

  const auto left{resolve(t_comp->left())};
  const auto right{resolve(t_comp->right())};

  return std::format("({}) {} ({})", left, op, right);
}

auto CppBackend::visit(Increment* t_inc) -> Any
{
  const auto left{resolve(t_inc->left())};
  const auto terminate_str{terminate()};

  return std::format("{}++{}", left, terminate_str);
}

auto CppBackend::visit(Decrement* t_dec) -> Any
{
  const auto left{resolve(t_dec->left())};
  const auto terminate_str{terminate()};

  return std::format("{}--{}", left, terminate_str);
}

auto CppBackend::visit(AddressOf* t_addr_of) -> Any
{
  auto left{t_addr_of->left()};
  const auto elem{resolve(left)};

  if(auto var_ptr{dynamic_cast<IdentifierNode*>(left.get())}; var_ptr) {
    // For arrays we need to access .data().
    auto type_var{var_ptr->get_type().as_var()};
    if(type_var->m_type.is_array()) {
      return std::format("({}.m_data)", elem);
    }
  }

  return std::format("&({})", elem);
}

auto CppBackend::visit(Dereference* t_deref) -> Any
{
  const auto left{resolve(t_deref->left())};

  return std::format("*({})", left);
}

auto CppBackend::visit(UnaryPrefix* t_up) -> Any
{
  const auto op{t_up->op2str()};
  const auto left{resolve(t_up->left())};

  return std::format("({}{})", op, left);
}

auto CppBackend::visit(ToCast* t_cast) -> Any
{
  // Do not terminate expression.
  const auto left{resolve(t_cast->left())};

  const auto type_variant{t_cast->get_type()};
  const auto type{type_spec2cpp({type_variant})};

  return std::format("static_cast<{}>({})", type, left);
}

// Logical:
auto CppBackend::visit(Not* t_not) -> Any
{
  const auto left{resolve(t_not->left())};

  return std::format("(!{})", left);
}

auto CppBackend::visit(And* t_and) -> Any
{
  const auto left{resolve(t_and->left())};
  const auto right{resolve(t_and->right())};

  return std::format("({}) && ({})", left, right);
}

auto CppBackend::visit(Or* t_or) -> Any
{
  const auto left{resolve(t_or->left())};
  const auto right{resolve(t_or->right())};

  return std::format("({}) || ({})", left, right);
}

auto CppBackend::visit([[maybe_unused]] Ternary* t_ternary) -> Any
{
  // const auto left{resolve(t_or->left())};
  // const auto right{resolve(t_or->right())};

  //  return std::format("({}) ? ({}) : ({})", left, right);

  return std::string{};
}

// Packaging:
AST_VISITOR_STUB(CppBackend, Import)

auto CppBackend::visit([[maybe_unused]] ModuleDecl* t_module) -> Any
{
  // Have prototypegenerator handle this?

  // TODO: Do something with this.
  // Wrap everything in namespace?
  // Also make main a special module?

  // DBG_WARNING("CppBackend: ModuleDecl needs to be implemented.");

  return std::string{};
}

// RValue:
auto CppBackend::visit([[maybe_unused]] Float* t_float) -> Any
{
  const auto value{t_float->get()};

  return std::format("{}", value);
}

auto CppBackend::visit(Integer* t_int) -> Any
{
  const auto value{t_int->get()};

  return std::format("{}", value);
}

auto CppBackend::visit([[maybe_unused]] Char* t_ch) -> Any
{
  const auto value{t_ch->get()};

  // We print chars as a hex for codegen.
  return std::format("(char)(0x{:x})", value);
}

auto CppBackend::visit([[maybe_unused]] String* t_str) -> Any
{
  const auto value{t_str->get()};

  return std::format("\"{}\"", value);
}

auto CppBackend::visit(ArrayExpr* t_arr) -> Any
{
  std::ostringstream oss{};

  const auto list{t_arr->get()};

  const auto type_variant{t_arr->get_type()};
  const auto elem_type{type_spec2cpp({type_variant})};

  oss << "stdlibacris::internal::InitList((" << elem_type << "[]){";

  std::string_view sep{};
  for(NodePtr& elem : *list) {
    oss << sep << resolve(elem);

    sep = ", ";
  }

  oss << "}, " << list->size() << ")" << terminate();

  return oss.str();
}

auto CppBackend::visit([[maybe_unused]] Boolean* t_bool) -> Any
{
  const auto value{t_bool->get()};

  return std::format("{}", value);
}

// User Types:
auto CppBackend::visit(EnumField* t_field) -> Any
{
  const auto field_id{t_field->identifier()};
  const auto field_expr{t_field->expr()};

  std::ostringstream oss{};

  oss << field_id;

  if(field_expr) {
    // Resolve assignment expression.
    oss << " = " << resolve(field_expr);
  }

  oss << ", ";

  return oss.str();
}

auto CppBackend::visit(Enum* t_enum) -> Any
{
  const auto enum_id{t_enum->identifier()};
  const auto enum_type{t_enum->get_type().as_enum()};
  const auto enum_ut{type_spec2cpp({enum_type->m_underlying_type})};
  const auto enum_body{t_enum->body()};

  std::ostringstream oss{};

  oss << "enum class " << enum_id << " : " << enum_ut << "{";
  oss << resolve(enum_body);
  oss << "};";

  return oss.str();
}

auto CppBackend::visit(Method* t_meth) -> Any
{
  using node::node_traits::AttributeType;

  const auto identifier{t_meth->identifier()};
  const auto receiver_type{t_meth->get_receiver()};

  const auto meth_type{t_meth->get_type().as_function()};
  const auto ret_type{type_spec2cpp({meth_type->m_return_type})};

  std::ostringstream param_ss{};

  auto sep{""sv};
  const auto params{t_meth->params()};
  for(const auto& param : *params) {
    param_ss << sep << resolve(param);

    sep = ", ";
  }

  std::ostringstream oss{};

  // Attribute insertion:
  const auto attrs{t_meth->get_attributes()};
  for(const auto& attr : attrs) {
    if(attr.m_type == AttributeType::INLINE) {
      oss << "inline\n";
    }
  }

  // clang-format off
  oss << std::format("auto {}::{}({}) -> {}\n", receiver_type, identifier, param_ss.str(), ret_type)
     << "{\n"
		 << resolve_list(t_meth->body(), true)
     << "}\n";
  // clang-format on

  // FIXME: We should do this by looping through the toplevel.
  // Of the SymbolTable instead.
  // Register function to interop backend.
  // for(auto& ptr : m_interop_backends) {
  //   ptr->register_function(identifier);
  // }

  return oss.str();
}

auto CppBackend::visit(MethodCall* t_meth_call) -> Any
{
  // FIXME: When get non shitty import resolution.
  // const auto identifier{t_meth_call->identifier()};
  auto identifier{t_meth_call->identifier()};
  const auto args{t_meth_call->args()};

  // FIXME: This wont work for a raw function or method call.
  // As when we assign it directly to a variable it will work but
  // not else. As we need to append a semicolon.

  // FIXME: Figure out a way to detect if this function call is
  // inline. Or if this function is being called without as a
  // statement. (I hope this is doable).

  std::ostringstream oss{};
  std::string_view sep{""};

  for(const auto& ptr : *args) {
    const auto argument{resolve(ptr)};
    oss << sep << argument;

    sep = ", ";
  }

  const auto arguments{oss.str()};

  return std::format("{}({}){}", identifier, arguments, terminate());
}


AST_VISITOR_STUB(CppBackend, Interface)

auto CppBackend::visit(MemberDecl* t_member) -> Any
{
  const auto identifier{t_member->identifier()};
  const auto type{type_spec2cpp({t_member->get_type()})};

  return std::format("{} {};\n", type, identifier);
}

auto CppBackend::visit(Struct* t_struct) -> Any
{
  const auto identifier{t_struct->identifier()};
  const auto members{t_struct->body()};

  const auto struct_type{t_struct->get_type().as_struct()};
  const auto& methods{struct_type->m_methods};

  std::ostringstream oss{};

  oss << std::format("struct {} {{\n", identifier);
  oss << resolve_list(members, true);


  oss << "// Methods:\n";

  for(auto& [meth_id, method] : methods) {
    const auto meth_type{method.as_function()};
    const auto ret_type{type_spec2cpp({meth_type->m_return_type})};

    std::ostringstream param_ss{};

    auto sep{""sv};
    const auto params{meth_type->m_params};
    for(const auto& param : params) {
      param_ss << sep << type_spec2cpp({param});

      sep = ", ";
    }

    oss << std::format("auto {}({}) -> {};\n", meth_id, param_ss.str(),
                       ret_type);
  }

  oss << "};\n";

  return oss.str();
}

auto CppBackend::visit([[maybe_unused]] Self* t_self) -> Any
{
  return std::string{"(*this)"};
}

auto CppBackend::visit(Member* t_member) -> Any
{
  const auto identifier{t_member->identifier()};

  return std::format("{}", identifier);
}

auto CppBackend::visit(MemberAccess* t_access) -> Any
{
  const auto lhs{resolve(t_access->left())};
  const auto rhs{resolve(t_access->right())};

  return std::format("{}.{}{}", lhs, rhs, terminate());
}

// Misc:
auto CppBackend::visit(List* t_list) -> Any
{
  std::ostringstream oss{};

  for(NodePtr& node : *t_list) {
    // Forward termination status.
    oss << resolve(node, should_terminate());
  }

  return oss.str();
}

auto CppBackend::visit([[maybe_unused]] NodeInterface* t_node) -> Any
{
  // We only call this method if we have not overriden.
  // The visit method for that AST node.
  RUNTIME_ERROR("Unimplemented visit() method for AST node.");

  return {};
}

// Util:
auto CppBackend::set_context(BackendContext t_ctx) -> void
{
  m_ctx = std::move(t_ctx);
}

auto CppBackend::register_interop_backend(const InteropBackend t_type) -> void
{
  namespace py_interop = cpp_backend::interop::python_backend;
  namespace lua_interop = cpp_backend::interop::lua_backend;

  CppInteropBackendPtr ptr{};

  const auto module_name{m_ctx.m_output_path.stem().native()};
  DBG_INFO("Module name: ", module_name);

  switch(t_type) {
    case InteropBackend::PYTHON_INTEROP_BACKEND: {
      ptr = std::make_shared<py_interop::PythonBackend>();

      // Add compiler flags for compiling python3 support.
      const auto includes{shell_getline("python3 -m pybind11 --includes")};
      m_inv.add_flags(std::format("-shared -fPIC {}", includes));

      const auto python_ldflags{shell_getline("python3-config --ldflags")};
      m_inv.add_flags(python_ldflags);

      // TODO: Use this for MACOS.
      auto framework_prefix{shell_getline(
        "python3 -c \"import sysconfig; "
        "print(sysconfig.get_config_var('PYTHONFRAMEWORKPREFIX'))\"")};
      auto framework_name{
        shell_getline("python3 -c \"import sysconfig; "
                      "print(sysconfig.get_config_var('PYTHONFRAMEWORK'))\"")};

      // Need to gete actual outpath to properly generate this and not depend on
      // SRC_STEM or similar.
      const auto pybind11_extension_suffix{
        shell_getline("python3 -m pybind11 --extension-suffix")};
      m_inv.set_out(
        std::format("{}{}", module_name, pybind11_extension_suffix));
      break;
    }

      // case InteropBackend::LUA_INTEROP_BACKEND:
      //   break;

      // Fallthrough all the way to default.
    case InteropBackend::LUA_INTEROP_BACKEND: {
      ptr = std::make_shared<lua_interop::LuaBackend>();

      const auto lua_flags{shell_getline("pkg-config --cflags --libs lua5.4")};
      m_inv.add_flags(std::format("-shared -fPIC {}", lua_flags));

      m_inv.set_out(std::format("{}.so", module_name));
      break;
    }

    case InteropBackend::C_INTEROP_BACKEND:
      [[fallthrough]];
    case InteropBackend::JS_INTEROP_BACKEND: {
      const auto err_msg{std::format("Unsupported interopability backend "
                                     "\"{}\" for C++ backend.",
                                     interopbackend2str(t_type))};
      throw std::invalid_argument{err_msg};
      break;
    }

    default: {
      const auto err_msg{std::format("Unknown interopability backend \"{}\" "
                                     "for C++ backend",
                                     interopbackend2str(t_type))};
      throw std::invalid_argument{err_msg};
      break;
    }
  }

  // Backends have their own set of constraints for module names.
  ptr->register_module(module_name);

  // Add the backend at the end.
  m_interop_backends.emplace_back(ptr);
}

auto CppBackend::set_optimize(const Optimize t_level) -> void
{
  using lib::stdexcept::InvalidArgument;
  using lib::stdexcept::throwf;

  switch(t_level) {
    case Optimize::NONE:
      m_inv.add_flags("-O0");
      break;

    case Optimize::SIZE:
      m_inv.add_flags("-Os");
      break;

    case Optimize::LEVEL_1:
      m_inv.add_flags("-O1");
      break;

    case Optimize::LEVEL_2:
      m_inv.add_flags("-O2");
      break;

    case Optimize::LEVEL_3:
      m_inv.add_flags("-O3");
      break;

    default:
      throwf<InvalidArgument>("Unhandled optimization level.");
      break;
  }
}

auto CppBackend::no_libc() const -> bool
{
  return m_session->no_libc();
}

auto CppBackend::codegen(NodePtr t_ast, const fs::path& t_out) -> void
{
  std::ofstream ofs{t_out};

  // Generate header includes basic typedefinitions and
  // similar.
  ofs << "// Prologue:\n";
  ofs << prologue() << '\n';
  ofs << '\n';

  // Generate forward declarations, to make code position
  // independent. ofs << "// Protoypes:\n"; ofs << "// TODO:
  // Implement." << '\n';

  // Generate C++ code.
  ofs << "// C++ code:\n";
  ofs << resolve(t_ast, true);
  ofs << '\n';

  ofs << "// Epilogue:\n";
  ofs << epilogue() << '\n';
}

auto CppBackend::requires_mir() -> bool
{
  return false;
}

auto CppBackend::compile(CompileParams& t_params) -> void
{
  const auto& [session, ast, mir, source_path] = t_params;

  // TODO: Check for nullptr?
  m_session = session;

  fs::path stem{source_path.stem()};

  const fs::path build_dir{m_ctx.m_build_dir};
  const fs::path tmp_src{build_dir / stem.concat(".cpp")};

  // Log filepath's:
  DBG_INFO("build_dir: ", build_dir);
  DBG_INFO("tmp_src: ", tmp_src);

  // Generate C++ source file.
  codegen(ast, tmp_src);

  // Allow optional toggling libc.
  if(no_libc()) {
    // Always link against our static library, it must be installed.
    m_inv.add_flags("-nostdlib");
    m_inv.add_flags("-lstdacris");
  }

  // Invoke clang frontend to generate a binary.
  m_inv.compile(tmp_src);

  // Clear members, for next compilation.
  m_session.reset();
  m_terminate = {};
  m_id_defer_count = 0;
}
} // namespace codegen::cpp_backend
