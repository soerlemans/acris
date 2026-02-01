#ifndef TEST_UTIL_HPP
#define TEST_UTIL_HPP

// STL Includes:
#include <sstream>
#include <string_view>

// Absolute Includes:
#include "acris/container/text_buffer.hpp"
#include "acris/lexer/lexer.hpp"
#include "acris/parser/acris/acris_parser.hpp"


using namespace parser::acris;

inline auto prep_parser(const std::string_view t_program) -> AcrisParser
{
  using container::TextBuffer;
  using lexer::Lexer;
  using parser::acris::AcrisParser;
  using token::TokenStream;

  // Write program.
  auto stream_ptr{std::make_shared<TextBuffer>()};
  stream_ptr->add_line(std::string{t_program});

  // Create AST from TextBuffer.
  Lexer lexer{stream_ptr};
  AcrisParser parser{lexer.tokenize()};

  return parser;
}

inline auto report_parse_failure(std::string_view t_program) -> std::string
{
  std::ostringstream ss{};

  ss << "Failed to parse " << std::quoted(t_program);

  return ss.str();
}

template<typename T>
inline auto report_exception(std::string_view t_program, T& t_err)
  -> std::string
{
  std::ostringstream ss{};

  ss << "Exception for program " << std::quoted(t_program)
     << " error: " << t_err.what() << '.';

  return ss.str();
}

inline auto report_uncaught_exception(std::string_view t_program) -> std::string
{
  std::ostringstream ss{};

  ss << "Uncaught exception for program " << std::quoted(t_program) << '.';

  return ss.str();
}

#endif // TEST_UTIL_HPP
