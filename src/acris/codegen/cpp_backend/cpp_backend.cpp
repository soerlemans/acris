#include "cpp_backend.hpp"

// STL Includes:
#include <format>
#include <fstream>
#include <sstream>
#include <string_view>

// Absolute Includes:
#include "acris/ast/node/include_nodes.hpp"
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
  oss << "\n";

  // FIXME: Temporary input for printing purposes.
  oss << "// Stdacris Includes:\n";
  oss << R"(#include "stdacris/core/linux/core.h")" << "\n";
  oss << R"(#include "stdacris/internal/internal.hpp")" << "\n\n";

  // Loop through the interop backends and add the prologue from each backend.
  for(auto& ptr : m_interop_backends) {
    oss << ptr->prologue();
  }

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

// Public:
CppBackend::CppBackend()
  : m_inv{}, m_interop_backends{}, m_terminate{}, m_id_defer_count{0}
{}

// Control:
auto CppBackend::visit(If* t_if) -> Any
{
  const auto init_expr{resolve(t_if->init_expr(), false)};
  const auto cond{resolve(t_if->condition())};

  const auto then{resolve(t_if->then())};
  const auto alt{resolve(t_if->alt())};

  std::ostringstream oss{};

  oss << std::format("if({} {}) {{\n", init_expr, cond) << then;

  // Dont create else branch if we dont have a statement for it.
  if(!alt.empty()) {
    oss << "} else {\n" << alt;
  }

  oss << "}\n";

  return oss.str();
}

auto CppBackend::visit(Loop* t_loop) -> Any
{
  const auto init_expr{resolve(t_loop->init_expr(), false)};
  const auto cond{resolve(t_loop->condition())};

  const auto post_expr{resolve(t_loop->expr(), false)};
  const auto body{resolve(t_loop->body())};

  std::ostringstream oss{};

  // clang-format off
  oss << std::format("for({};{}; {} )", init_expr, cond, post_expr)
     << "{\n"
     << body
     << "}\n";
  // clang-format on

  return oss.str();
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

  const auto body{resolve(t_defer->body())};

  oss << std::format(
    "const stdinternal::Defer defer_object{}{{ [&](){{ {} }} }};\n",
    m_id_defer_count, body);

  m_id_defer_count++;

  return oss.str();
}

auto CppBackend::visit(Return* t_ret) -> Any
{
  const auto expr{resolve(t_ret->expr(), false)};

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
  using node::node_traits::AttributeType;

  const auto identifier{t_fn->identifier()};

  const auto fn_type{t_fn->get_type().as_function()};
  const auto ret_type{type_spec2cpp({fn_type->m_return_type})};

  std::ostringstream param_ss{};

  auto sep{""sv};
  const auto params{t_fn->params()};
  for(const auto& param : *params) {
    param_ss << sep << resolve(param);

    sep = ", ";
  }

  std::ostringstream oss{};

  // Attribute insertion:
  const auto attrs{t_fn->get_attributes()};
  for(const auto& attr : attrs) {
    if(attr.m_type == AttributeType::INLINE) {
      oss << "inline\n";
    }
  }

  // clang-format off
	// We use regular function syntax instead trailing return type.
	// Cause trailing return
  oss << std::format("{} {}({})\n", ret_type, identifier, param_ss.str())
     << "{\n"
     << resolve(t_fn->body())
     << "}\n";
  // clang-format on

  // FIXME: We should do this by looping through the toplevel.
  // Of the SymbolTable instead.
  // Register function to interop backend.
  for(auto& ptr : m_interop_backends) {
    ptr->register_function(identifier);
  }

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
    const auto argument{resolve(ptr, false)};
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
    const auto init_expr_str{resolve(init_expr, false)};

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
    const auto init_expr_str{resolve(init_expr, false)};

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
  // bodies.
  const auto attrs{t_attr->get_attributes()};
  for(const auto& attr : attrs) {
    switch(attr.m_type) {
      case AttributeType::EXTERN:
        // clang-format off
      oss << R"(extern "C" {)" << "\n"
				 << resolve(body)
				 << "}\n";
        // clang-format on
        break;

      default:
        // Walk the body like normal.
        oss << resolve(body);
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

  const auto left{resolve(t_assign->left(), false)};
  const auto right{resolve(t_assign->right(), false)};

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

  oss << '{';

  std::string_view sep{};
  for(NodePtr& elem : *list) {
    oss << sep << resolve(elem);

    sep = ", ";
  }

  oss << '}' << terminate();

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

  oss << "enum class" << enum_id << " : " << enum_ut << "{";
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
     << resolve(t_meth->body())
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
    const auto argument{resolve(ptr, false)};
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
  oss << resolve(members);


  oss << "// Methods:\n";

  for(auto& [meth_identifier, method] : methods) {
    const auto meth_type{method.as_function()};

    const auto ret_type{type_spec2cpp({meth_type->m_return_type})};

    std::ostringstream param_ss{};

    auto sep{""sv};
    const auto params{meth_type->m_params};
    for(const auto& param : params) {
      param_ss << sep << type_spec2cpp({param});

      sep = ", ";
    }

    oss << std::format("auto {}({}) -> {};\n", meth_identifier, param_ss.str(),
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
  const auto rhs{resolve(t_access->right(), false)};

  return std::format("{}.{}{}", lhs, rhs, terminate());
}

// Misc:
auto CppBackend::visit(List* t_list) -> Any
{
  std::ostringstream oss{};

  for(NodePtr& node : *t_list) {
    oss << resolve(node);
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
auto CppBackend::register_interop_backend(const InteropBackend t_type) -> void
{
  namespace py_interop = cpp_backend::interop::python_backend;

  CppInteropBackendPtr ptr{};

  switch(t_type) {
    case InteropBackend::PYTHON_INTEROP_BACKEND:
      ptr = std::make_shared<py_interop::PythonBackend>();

      // Add compiler flags for compiling python3 support.
      m_inv.add_flags("-shared -fPIC $(python3 -m pybind11 --includes)");

      // TODO: Find a cleaner way to utilize environment
      // variables. Maybe register environment variables for
      // later usage.

      // Set the out file (SRC_STEM is set in
      // ClangFrontendInvoker).
      // clang-format off
      // m_inv.set_out("acris_${SRC_STEM:-cpython}_export$(python3-config --extension-suffix)");
      // clang-format on
      m_inv.set_out("acris_export$(python3-config "
                    "--extension-suffix)");
      break;

      // case InteropBackend::LUA_INTEROP_BACKEND:
      //   break;

      // Fallthrough all the way to default.
    case InteropBackend::C_INTEROP_BACKEND:
      [[fallthrough]];
    case InteropBackend::LUA_INTEROP_BACKEND:
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
  ofs << resolve(t_ast);
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
  const auto& [session, ast, mir, build_dir, source_path] = t_params;

  fs::path stem{source_path.stem()};
  const fs::path tmp_src{build_dir / stem.concat(".cpp")};

  // Log filepath's:
  DBG_INFO("build_dir: ", build_dir);
  DBG_INFO("tmp_src: ", tmp_src);

  // Generate C++ source file.
  codegen(ast, tmp_src);

  // Allow optional toggling libc.
  if(session->no_libc()) {
    // Always link against our static library, it must be installed.
    m_inv.add_flags("-nostdlib");
    m_inv.add_flags("-lstdacris");
  }

  // Invoke clang frontend to generate a binary.
  m_inv.compile(tmp_src);

  // Clear members, for next compilation.
  m_terminate = {};
  m_id_defer_count = 0;
}
} // namespace codegen::cpp_backend
