// STL Includes:
#include <string_view>

// Library Includes:
#include <gtest/gtest.h>

// Absolute Includes:
#include "acris/diagnostic/diagnostic.hpp"
#include "acris/parser/acris/acris_parser.hpp"

// Test Includes:
#include "test_util.hpp"

/*!
 * @file
 *
 */


// Using:
using namespace std::literals::string_view_literals;

using parser::acris::AcrisParser;
using parser::acris::TypeParser;

using TypeExprs = std::vector<std::string_view>;

inline auto type_parse(const TypeExprs& t_exprs)
{
  using diagnostic::SyntaxError;

  for(auto&& program : t_exprs) {
    auto parser{prep_parser(program)};

    try {
      auto node{parser.type_parse([](TypeParser& type) {
        return type.type_expr();
      })};

      EXPECT_TRUE(node != nullptr)
        << "Expression failed to parse: " << std::quoted(program) << '.';
    } catch(SyntaxError& err) {
      FAIL() << report_exception(program, err);
    } catch(std::exception& err) {
      FAIL() << report_exception(program, err);
    } catch(...) {
      FAIL() << report_uncaught_exception(program);
    }
  }
}

// Test Cases:
TEST(TestPrattParser, BasicTypeSpecification)
{
  // User defined types wont work during unit testing yet.
  // clang-format off
  TypeExprs exprs = {
    "void"sv,

    "f32"sv,
    "f64"sv,

    "int"sv,
    "i8"sv,
    "i16"sv,
    "i32"sv,
    "i64"sv,
    "isz"sv,

    "uint"sv,
    "u8"sv,
    "u16"sv,
    "u32"sv,
    "u64"sv,
    "usz"sv,

    "char"sv,
		"cstr"sv,
  };
  // clang-format on

  type_parse(exprs);
}

TEST(TestPrattParser, BasicPointerTypeSpecification)
{
  // User defined types wont work during unit testing yet.
  // clang-format off
  TypeExprs exprs = {
    "*void"sv,

    "*f32"sv,
    "*f64"sv,

    "*int"sv,
    "*i8"sv,
    "*i16"sv,
    "*i32"sv,
    "*i64"sv,
    "*isz"sv,

    "*uint"sv,
    "*u8"sv,
    "*u16"sv,
    "*u32"sv,
    "*u64"sv,
    "*usz"sv,

    "*char"sv,
		"*cstr"sv,
  };
  // clang-format on

  type_parse(exprs);
}

TEST(TestPrattParser, BasicArrayTypeSpecification)
{
  // User defined types wont work during unit testing yet.
  // clang-format off
  TypeExprs exprs = {
    "[void; 5]"sv,

    "[f32; 5]"sv,
    "[f64; 5]"sv,

    "[int; 5]"sv,
    "[i8; 5]"sv,
    "[i16; 5]"sv,
    "[i32; 5]"sv,
    "[i64; 5]"sv,
    "[isz; 5]"sv,

    "[uint; 5]"sv,
    "[u8; 5]"sv,
    "[u16; 5]"sv,
    "[u32; 5]"sv,
    "[u64; 5]"sv,
    "[usz; 5]"sv,

    "[char; 5]"sv,
		"[cstr; 5]"sv,
  };
  // clang-format on

  type_parse(exprs);
}

TEST(TestPrattParser, ComplexTypeSpecification)
{
  // User defined types wont work during unit testing yet.
  // clang-format off
  TypeExprs exprs = {
    "void"sv,
    "*void"sv,
    "**void"sv,
    "***void"sv,
    "*[void; 5]"sv,
    "[*void; 5]"sv,
    "*[*void; 5]"sv,
    "**[**cstr; 5]"sv,
  };
  // clang-format on

  type_parse(exprs);
}
