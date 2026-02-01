#include "type2cpp.hpp"

// STL Includes:
#include <memory>

// Absolute Includes:
#include "acris/ast/node/include_nodes.hpp"
#include "lib/overload.hpp"
#include "lib/stdexcept/stdexcept.hpp"

// Macros:
#define MATCH(t_key, t_value) \
  case NativeType::t_key:     \
    str = t_value;            \
    break

namespace {
// Using:
using codegen::cpp_backend::type_spec2cpp;
using types::core::ArrayTypePtr;
using types::core::FnTypePtr;
using types::core::NativeType;
using types::core::nativetype2str;
using types::core::PointerTypePtr;
using types::core::StructTypePtr;
using types::core::VarTypePtr;

// Functions:
/*!
 * @warning Make sure that <cstdint> is included for the fixed width integers.
 */
inline auto native_type2cpp(const NativeType t_type) -> std::string
{
  using lib::stdexcept::throw_invalid_argument;

  std::string str{};

  // FIXME: Currently the C++ fixed width floating point types.
  // Are not yet supported by clang libc++.
  // So for now just error out, on these.
  // Possibly print currently unsupported?
  switch(t_type) {
    MATCH(VOID, "void");

    MATCH(INT, "int_t");
    MATCH(I8, "i8_t");
    MATCH(I16, "i16_t");
    MATCH(I32, "i32_t");
    MATCH(I64, "i64_t");
    MATCH(ISIZE, "isize_t");

    MATCH(UINT, "uint_t");
    MATCH(U8, "u8_t");
    MATCH(U16, "u16_t");
    MATCH(U32, "u32_t");
    MATCH(U64, "u64_t");
    MATCH(USIZE, "usize_t");

    MATCH(CHAR, "char");
    MATCH(CSTR, "const char*");

    MATCH(BOOL, "bool");

    default: {
      const auto msg{
        std::format("NativeType could not be converted to C++ type: ",
                    nativetype2str(t_type))};

      throw_invalid_argument(msg);
      break;
    }
  }

  return str;
}

inline auto struct2cpp(const StructTypePtr& t_struct) -> std::string
{
  return t_struct->m_identifier;
}

// TODO: Figure out how to deal with this one.
inline auto fn2cpp([[maybe_unused]] const FnTypePtr& t_type) -> std::string
{
  // TODO: Throw?

  return {};
}

inline auto pointer2cpp(const PointerTypePtr& t_ptr) -> std::string
{
  std::ostringstream oss{};

  if(t_ptr->m_readonly) {
    oss << "const ";
  }

  auto base_type{type_spec2cpp({t_ptr->m_type})};
  oss << base_type;

  for(int index{0}; index < t_ptr->m_indirection; index++) {
    oss << '*';
  }


  // return std::format("{}*", base_type);
  return oss.str();
}

inline auto array2cpp(const ArrayTypePtr& t_arr) -> std::string
{
  using codegen::cpp_backend::type_spec2cpp;

  auto base_type{type_spec2cpp({t_arr->m_type})};
  auto size{t_arr->m_size};

  return std::format("stdlibacris::internal::Array<{} ,{}>", base_type, size);
}

// Could be used for decltype() or similar.
inline auto var2cpp(const VarTypePtr& t_var) -> std::string
{
  using codegen::cpp_backend::type_spec2cpp;

  auto base_type{type_spec2cpp({t_var->m_type})};

  return std::format("{}", base_type);
}
} // namespace

namespace codegen::cpp_backend {
// Using Statements:
NODE_USING_ALL_NAMESPACES()

auto type_spec2cpp(const TypeSpec t_spec) -> std::string
{
  using lib::Overload;

  auto& [type] = t_spec;

  // clang-format off
  return std::visit(
					Overload{
						[&](const NativeType t_native) {
							return native_type2cpp(t_native);
						},
						[&](const StructTypePtr& t_struct) {
							return struct2cpp(t_struct);
						},
						[&](const FnTypePtr& t_fn) {
							return fn2cpp(t_fn);
						},
						[&](const PointerTypePtr& t_ptr) {
							return pointer2cpp(t_ptr);
						},
						[&](const ArrayTypePtr& t_arr) {
							return array2cpp(t_arr);
						},
						[&](const VarTypePtr& t_var) {
							return var2cpp(t_var);
						}},
					type);
	// clang-format off
}
} // namespace codegen::cpp_backend
