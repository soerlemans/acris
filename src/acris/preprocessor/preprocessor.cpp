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
using preprocessor::MacroRegister;

constexpr auto STD_INCLUDE_PATH{"/usr/local/include/stdacris/"sv};

constexpr auto INCLUDE_ONCE{"include_once"sv};
constexpr auto INCLUDE{"include"sv};

constexpr auto IFDEF{"ifdef"sv};
constexpr auto ELSE{"else"sv};
constexpr auto ENDIF{"endif"sv};
constexpr auto ERROR{"error"sv};

constexpr uchar SPACE{' '};
constexpr uchar UNDERSCORE{'_'};
constexpr uchar NEWLINE{'\n'};
constexpr uchar MACRO_START{'#'};
constexpr uchar UNHYGIENIC_SPECIFIER{'<'};

auto dump_mreg(const MacroRegister& t_mreg) -> std::string
{
  std::ostringstream oss{};

  oss << '[';
  std::string_view sep{};
  for(auto&& [name, val] : t_mreg) {
    oss << sep << name << "=" << val;

    sep = ", ";
  }
  oss << ']';

  return oss.str();
}
} // namespace

namespace preprocessor {
using diagnostic::PreprocessorError;

auto Preprocessor::preprocessor_error(TextPosition t_pos,
                                      std::string_view t_msg) const -> void
{
  throw PreprocessorError{t_msg, t_pos};
}

auto Preprocessor::preprocessor_error(TextStreamPtr t_text,
                                      std::string_view t_msg) const -> void
{
  auto pos{t_text->position()};

  throw PreprocessorError{t_msg, pos};
}

Preprocessor::Preprocessor(TextStreamPtr t_text)
  : m_text{t_text}, m_nesting_count{0}, m_ireg{}, m_mreg{}
{}

auto Preprocessor::set_defined(const MacroRegister& t_mreg) -> void
{
  DBG_INFO("mdefs: ", dump_mreg(t_mreg));

  m_mreg = t_mreg;
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
  if(ch == MACRO_START && peek_ch == UNHYGIENIC_SPECIFIER) {
    t_text->next(); // Skip #.
    t_text->next(); // Skip <.

    auto line{t_text->line()};
    DBG_VERBOSE("Found unhygienic macro: ", line);

    return true;
  }

  return false;
}

auto Preprocessor::skip_whitespace(TextStreamPtr t_text)
{
  while(!t_text->eos()) {
    const auto ch{(uchar)t_text->character()};
    if(ch != SPACE) {
      break;
    }

    t_text->next();
  }
}

auto Preprocessor::get_identifier(TextStreamPtr t_text) -> std::string
{
  std::ostringstream oss{};

  const auto start_ch{(uchar)t_text->character()};
  if(!std::isalpha(start_ch)) {
    const auto msg{
      std::format("Macro name must start with alphanum '{}'.", start_ch)};
    preprocessor_error(t_text, msg);
  }
  oss << start_ch;
  t_text->next();

  while(!t_text->eos()) {
    const auto ch{(uchar)t_text->character()};
    if(!(std::isalnum(ch) || ch == UNDERSCORE)) {
      break;
    }

    oss << ch;

    t_text->next();
  }

  const auto identifier{oss.str()};

  DBG_VERBOSE("Macro ID: ", std::quoted(identifier));
  return identifier;
}

auto Preprocessor::include_file(TextBufferPtr& t_buffer, const fs::path t_path)
  -> void
{
  using fs::exists;

  if(!exists(t_path)) {
    std::ostringstream oss{};
    oss << "File does not exist " << std::quoted(t_path.string()) << '!';

    preprocessor_error(t_buffer, oss.view());
  }

  std::ifstream ifs{t_path};
  while(ifs.good() && !ifs.eof()) {
    std::string line{};
    std::getline(ifs, line);

    t_buffer->add_line(line);

    // Nested include directive.
    if(!line.empty() && line.front() == MACRO_START) {
      auto buffer{make_buffer()};

      m_nesting_count++;
      if(m_nesting_count >= MAX_INCLUDE_NESTING) {
        const auto msg{std::format(
          "Exceeded maximum #<include nesting count of {} includes.",
          MAX_INCLUDE_NESTING)};

        preprocessor_error(t_buffer, msg);
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

      if(ch == NEWLINE) {
        const auto msg{
          std::format("Newlines are when defining include paths..")};

        preprocessor_error(t_text, msg);
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

auto Preprocessor::handle_include_once(TextStreamPtr t_text,
                                       TextBufferPtr& t_buffer) -> void
{
  skip_whitespace(t_text);

  auto [is_lib, original] = get_include_path(t_text);
  DBG_INFO("include_once: ", std::quoted(original), " : ", m_nesting_count);

  const auto prepend{(is_lib) ? STD_INCLUDE_PATH : ""sv};
  fs::path include{std::format("{}{}", prepend, original)};
  include = fs::absolute(include);

  if(m_ireg.contains(include) == false) {
    include_file(t_buffer, include);

    m_ireg.insert(include);
  }
}

auto Preprocessor::handle_include(TextStreamPtr t_text, TextBufferPtr& t_buffer)
  -> void
{
  skip_whitespace(t_text);

  auto [is_lib, original] = get_include_path(t_text);
  DBG_INFO("include: ", std::quoted(original), " : ", m_nesting_count);

  const auto prepend{(is_lib) ? STD_INCLUDE_PATH : ""sv};
  fs::path include{std::format("{}{}", prepend, original)};
  include = fs::absolute(include);

  include_file(t_buffer, include);
}

auto Preprocessor::handle_ifdef(TextStreamPtr t_text, TextBufferPtr& t_buffer)
  -> void
{
  skip_whitespace(t_text);
  const auto cond_id{get_identifier(t_text)};

  // Should we emit the current branch.
  bool emit_branch{false};
  if(m_mreg.contains(cond_id)) {
    emit_branch = true;
  }

  DBG_INFO("ifdef: ", cond_id, ", emit_branch: ", emit_branch);

  t_text->next_line(); // Skip ifdef line.
  while(!t_text->eos()) {
    // Directive processing loop.
    if(next_if_unhygienic_macro(t_text)) {
      const auto macro_id{get_identifier(t_text)};

      if(macro_id == ELSE) {
        emit_branch = !emit_branch; // Flip emit_branch state.
      } else if(macro_id == ENDIF) {
        // Quite ifdef statemachine loop.
        t_text->next_line();
        return;
      } else if(emit_branch) {
        // Only expand macros if we are in the branch that should emit.
        match_macro(macro_id, t_text, t_buffer);
      }
    } else {
      t_buffer->add_line(std::string{t_text->line()});
    }

    t_text->next_line();
  }

  if(t_text->eos()) {
    // TODO: Throw endif was never hit till end of file.
    const auto end_pos{t_text->end_position()};
    preprocessor_error(end_pos, "End of file reached and found no #<endif.");
  }
}

auto Preprocessor::match_macro(const std::string_view t_macro_id,
                               TextStreamPtr t_text, TextBufferPtr& t_buffer)
  -> void
{
  const auto pos{t_text->position()};
  if(t_macro_id == INCLUDE_ONCE) {
    handle_include_once(t_text, t_buffer);
  } else if(t_macro_id == INCLUDE) {
    handle_include(t_text, t_buffer);
  } else if(t_macro_id == IFDEF) {
    handle_ifdef(t_text, t_buffer);
  } else if(t_macro_id == ERROR) {
    const auto msg{std::format("#<error: \"{}\".", t_macro_id)};

    preprocessor_error(pos, msg);
  } else {
    const auto msg{std::format("Unknown macro name \"{}\".", t_macro_id)};

    preprocessor_error(pos, msg);
  }
}

auto Preprocessor::handle_preprocessor(TextStreamPtr t_text,
                                       TextBufferPtr& t_buffer) -> void
{
  if(m_nesting_count >= MAX_INCLUDE_NESTING) {
    const auto msg{
      std::format("Exceeded maximum #<include nesting count of {} includes.",
                  MAX_INCLUDE_NESTING)};

    preprocessor_error(t_text, msg);
  }

  while(!t_text->eos()) {
    // Directive processing loop.
    if(next_if_unhygienic_macro(t_text)) {
      const auto macro_id{get_identifier(t_text)};

      match_macro(macro_id, t_text, t_buffer);
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
