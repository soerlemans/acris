#ifndef ACRIS_ACRIS_CODEGEN_CPP_BACKEND_TYPE2CPP_HPP
#define ACRIS_ACRIS_CODEGEN_CPP_BACKEND_TYPE2CPP_HPP

// STL Includes:
#include <string_view>

// Absolute Includes:
#include "acris/ast/node/fdecl.hpp"
#include "acris/types/core/core.hpp"

namespace codegen::cpp_backend {
// Using Statements:
using types::core::TypeVariant;

// TODO: Improve API for converting a type specification into C++.
// Currently the way for having to pass the identifier for arrays is awkward.
/*!
 * Some types in C++ make the identifier part of the type expression.
 */
struct TypeSpec {
  const TypeVariant& m_type;
	// FIXME: This shit is stupid.
};

/*!
 * Convert the @ref TypeVariant to an actual C++ type.
 */
auto type_spec2cpp(const TypeSpec t_spec) -> std::string;
} // namespace codegen::cpp_backend

#endif // ACRIS_ACRIS_CODEGEN_CPP_BACKEND_TYPE2CPP_HPP
