#include "preprocessor.hpp"

// STL Includes:
#include <cctype>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string_view>

// Absolute Includes:
#include "acris/debug/log.hpp"
#include "acris/diagnostic/diagnostic.hpp"

namespace {
using namespace std::string_view_literals;
constexpr auto std_include_path{"/usr/local/include/stdacris/"sv};

constexpr auto include_once{"include_once"sv};
constexpr auto include{"include"sv};

constexpr auto macro_ifdef{"ifdef"sv};
constexpr auto macro_else{"else"sv};
constexpr auto macro_endif{"endif"sv};

constexpr uchar space{' '};
constexpr uchar underscore{'_'};
constexpr uchar newline{'\n'};
constexpr uchar macro_start{'#'};
constexpr uchar unhygienic_specifier{'<'};
} // namespace

namespace preprocessor {
using diagnostic::PreprocessorError;

Preprocessor::Preprocessor(TextStreamPtr t_text)
  : m_text{t_text}, m_nesting_count{0}, m_included{}, m_defined{}
{}

auto Preprocessor::set_defined(DefinedRegister&& t_defined) -> void
{
  m_defined = std::move(t_defined);
}

auto Preprocessor::make_buffer() -> TextBufferPtr
{
  const auto source_file{m_text->source()};

  return std::make_shared<TextBuffer>(source_file);
}

auto Preprocessor::next_if_unhygienic_macro(TextStreamPtr t_text) -> bool
{
  const auto ch{(uchar)t_text->character()};
  const auto peek_opt{t_text->peek()};

  // We need a # followed by a <.
  if(!peek_opt) {
    return false;
  }

  // Directive processing loop.
  const auto peek_ch{peek_opt.value()};
  if(ch == macro_start && peek_ch == unhygienic_specifier) {
    t_text->next(); // Skip #.
    t_text->next(); // Skip <.

    auto line{t_text->line()};
    DBG_VERBOSE("Found unhygienic macro: ", line);

    return true;
  }

  return false;
}

auto Preprocessor::get_id(TextStreamPtr t_text) -> std::string
{
  std::ostringstream oss{};

  const auto start_ch{(uchar)t_text->character()};
  if(std::isalpha(start_ch) == false) {
    // TODO: Error.
  }
  t_text->next();

  while(!t_text->eos()) {
    const auto ch{(uchar)t_text->character()};
    if(std::isalnum(ch) || ch == underscore) {
      break;
    }

    oss << ch;

    t_text->next();
  }

  return oss.str();
}

auto Preprocessor::include_file(TextBufferPtr& t_buffer, const fs::path t_path)
  -> void
{
  using fs::exists;

  if(!exists(t_path)) {
    std::stringstream ss{};

    ss << "File does not exist ";
    ss << std::quoted(t_path.string()) << '!';

    const auto pos{t_buffer->position()};

    throw PreprocessorError{ss.str(), pos};
  }

  std::ifstream ifs{t_path};
  while(ifs.good() && !ifs.eof()) {
    std::string line{};
    std::getline(ifs, line);

    t_buffer->add_line(line);

    // Nested include directive.
    if(!line.empty() && line.front() == macro_start) {
      auto buffer{make_buffer()};

      m_nesting_count++;
      if(m_nesting_count >= MAX_INCLUDE_NESTING) {
        auto pos{t_buffer->end_position()};

        throw PreprocessorError{
          std::format(
            "Exceeded maximum #$include nesting count of {} includes.",
            MAX_INCLUDE_NESTING),
          pos};
      }

      handle_preprocessor(t_buffer, buffer);
      m_nesting_count--;

      // Expanded buffer write to intermediary.
      t_buffer = std::move(buffer);
    }
  }
}

auto Preprocessor::get_include_path(TextStreamPtr t_text) -> IncludePack
{
  std::stringstream ss{};

  uchar term_char{'"'};

  IncludePack pack{};

  const auto ch{(uchar)t_text->character()};
  if(ch == '<' || ch == '"') {
    if(ch == '<') {
      // Library include path.
      pack.m_is_lib = true;
      term_char = '>';
    }
    t_text->next();

    while(!t_text->eos()) {
      const auto ch{(uchar)t_text->character()};
      if(ch == term_char) {
        break;
      }

      if(ch == newline) {
        auto pos{t_text->position()};

        throw PreprocessorError{
          std::format("Newlines are when defining include paths.."), pos};
      }

      ss << ch;

      t_text->next();
    }
  } else {
    // TODO: Throw.
  }

  pack.m_include = ss.str();

  return pack;
}

auto Preprocessor::skip_whitespace(TextStreamPtr t_text)
{
  while(!t_text->eos()) {
    const auto ch{(uchar)t_text->character()};
    if(ch != space) {
      break;
    }

    t_text->next();
  }
}

auto Preprocessor::handle_include_once(TextStreamPtr t_text,
                                       TextBufferPtr& t_buffer) -> void
{
  skip_whitespace(t_text);

  auto [is_lib, original] = get_include_path(t_text);
  DBG_INFO("include_once: ", std::quoted(original), " : ", m_nesting_count);

  const auto prepend{(is_lib) ? std_include_path : ""sv};
  fs::path include{std::format("{}{}", prepend, original)};
  include = fs::absolute(include);

  if(m_included.contains(include) == false) {
    include_file(t_buffer, include);

    m_included.insert(include);
  }
}

auto Preprocessor::handle_include(TextStreamPtr t_text, TextBufferPtr& t_buffer)
  -> void
{
  skip_whitespace(t_text);

  auto [is_lib, original] = get_include_path(t_text);
  DBG_INFO("include: ", std::quoted(original), " : ", m_nesting_count);

  const auto prepend{(is_lib) ? std_include_path : ""sv};
  fs::path include{std::format("{}{}", prepend, original)};
  include = fs::absolute(include);

  include_file(t_buffer, include);
}

auto Preprocessor::handle_ifdef(TextStreamPtr t_text, TextBufferPtr& t_buffer)
  -> void
{
  skip_whitespace(t_text);

  // DBG_INFO("ifdef: ");

  while(!t_text->eos()) {
    // Directive processing loop.
    const auto macro_id{get_id(t_text)};


    // t_buffer->add_line(std::string{t_text->line()});

    t_text->next_line();
  }
}

auto Preprocessor::handle_preprocessor(TextStreamPtr t_text,
                                       TextBufferPtr& t_buffer) -> void
{
  if(m_nesting_count >= MAX_INCLUDE_NESTING) {
    auto pos{t_text->end_position()};

    throw PreprocessorError{
      std::format("Exceeded maximum #include nesting count of {} includes.",
                  MAX_INCLUDE_NESTING),
      pos};
  }

  while(!t_text->eos()) {
    // Directive processing loop.
    if(next_if_unhygienic_macro(t_text)) {
      const auto macro_id{get_id(t_text)};

      if(macro_id == include_once) {
        handle_include_once(t_text, t_buffer);
      } else if(macro_id == include) {
        handle_include(t_text, t_buffer);
      } else if(macro_id == macro_ifdef) {
        handle_ifdef(t_text, t_buffer);
      }
    } else {
      t_buffer->add_line(std::string{t_text->line()});
    }

    t_text->next_line();
  }
}

auto Preprocessor::preprocess() -> TextStreamPtr
{
  auto buffer{make_buffer()};

  handle_preprocessor(m_text, buffer);
  DBG_INFO("buffer: ", *buffer);

  m_text->reset();

  return buffer;
}
} // namespace preprocessor
