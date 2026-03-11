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

auto Preprocessor::preprocessor_error(TextStreamPtr t_src,
                                      std::string_view t_msg) const -> void
{
  auto pos{t_src->position()};

  throw PreprocessorError{t_msg, pos};
}

Preprocessor::Preprocessor(TextStreamPtr t_src)
  : m_src{t_src}, m_nesting_count{0}, m_ireg{}, m_mreg{}
{}

auto Preprocessor::init_default_macros() -> void
{
  // auto insert_if_not_present{
  //   [&](std::string_view t_key, std::string_view t_val) {
  //     if(!m_mreg.contains(std::string{t_key})) {
  //       m_mreg.emplace(std::string{t_val});
  //     }
  //   }};


  // insert_if_not_present("LINUX", "true");
  // m_mreg.contains("LINUX");
  // TODO: Figure out what do next.
}

auto Preprocessor::defines(const MacroRegister& t_mreg) -> void
{
  DBG_INFO("mdefs: ", dump_mreg(t_mreg));

  m_mreg = t_mreg;

  init_default_macros();
}

auto Preprocessor::make_buffer() -> TextBufferPtr
{
  const auto source_file{m_src->source()};

  return std::make_shared<TextBuffer>(source_file);
}

auto Preprocessor::next_if_unhygienic_macro(TextStreamPtr t_src) -> bool
{
  const auto ch{(uchar)t_src->character()};
  const auto peek_opt{t_src->peek()};

  // We need a # followed by a <.
  if(!peek_opt) {
    return false;
  }

  // Directive processing loop.
  const auto peek_ch{peek_opt.value()};
  if(ch == MACRO_START && peek_ch == UNHYGIENIC_SPECIFIER) {
    t_src->next(); // Skip #.
    t_src->next(); // Skip <.

    auto line{t_src->line()};
    DBG_VERBOSE("Found unhygienic macro: ", line);

    return true;
  }

  return false;
}

auto Preprocessor::skip_whitespace(TextStreamPtr t_src)
{
  while(!t_src->eos()) {
    const auto ch{(uchar)t_src->character()};
    if(ch != SPACE) {
      break;
    }

    t_src->next();
  }
}

auto Preprocessor::get_identifier(TextStreamPtr t_src) -> std::string
{
  std::ostringstream oss{};

  const auto start_ch{(uchar)t_src->character()};
  if(!std::isalpha(start_ch)) {
    const auto msg{
      std::format("Macro name must start with alphanum '{}'.", start_ch)};
    preprocessor_error(t_src, msg);
  }
  oss << start_ch;
  t_src->next();

  while(!t_src->eos()) {
    const auto ch{(uchar)t_src->character()};
    if(!(std::isalnum(ch) || ch == UNDERSCORE)) {
      break;
    }

    oss << ch;

    t_src->next();
  }

  const auto identifier{oss.str()};

  DBG_VERBOSE("Macro ID: ", std::quoted(identifier));
  return identifier;
}

auto Preprocessor::include_file(TextBufferPtr& t_dst, const fs::path t_path)
  -> void
{
  using fs::exists;

  if(!fs::exists(t_path)) {
    std::stringstream ss{};

    ss << "File does not exist! ";
    ss << std::quoted(t_path.string());

    throw std::invalid_argument{ss.str()};
  }

  auto buffer{make_buffer()};

  std::ifstream ifs{t_path};
  while(ifs.good() && !ifs.eof()) {
    std::string line{};
    std::getline(ifs, line);

    buffer->add_line(std::move(line));
  }

  // In the future this solution might cost too much memory/be slow.
  // Handle recursive expansion.
  m_nesting_count++;
  handle_preprocessor(t_dst, buffer);
  m_nesting_count--;
}

auto Preprocessor::get_include_path(TextStreamPtr t_src) -> IncludePack
{
  std::stringstream ss{};

  uchar term_char{'"'};

  IncludePack pack{};

  const auto ch{(uchar)t_src->character()};
  if(ch == '<' || ch == '"') {
    if(ch == '<') {
      // Library include path.
      pack.m_is_lib = true;
      term_char = '>';
    }
    t_src->next();

    while(!t_src->eos()) {
      const auto ch{(uchar)t_src->character()};
      if(ch == term_char) {
        break;
      }

      if(ch == NEWLINE) {
        const auto msg{
          std::format("Newlines are when defining include paths..")};

        preprocessor_error(t_src, msg);
      }

      ss << ch;

      t_src->next();
    }
  } else {
    const auto pos{t_src->position()};
    preprocessor_error(pos,
                       "Unexpected character whilst getting include path.");
  }

  pack.m_include = ss.str();

  return pack;
}

auto Preprocessor::handle_include_once(TextBufferPtr& t_dst,
                                       TextStreamPtr t_src) -> void
{
  skip_whitespace(t_src);

  auto [is_lib, original] = get_include_path(t_src);
  DBG_INFO("include_once: ", std::quoted(original), " : ", m_nesting_count);

  const auto prepend{(is_lib) ? STD_INCLUDE_PATH : ""sv};
  fs::path include{std::format("{}{}", prepend, original)};
  include = fs::absolute(include);

  if(m_ireg.contains(include) == false) {
    include_file(t_dst, include);

    m_ireg.insert(include);
  }
}

auto Preprocessor::handle_include(TextBufferPtr& t_dst, TextStreamPtr t_src)
  -> void
{
  skip_whitespace(t_src);

  auto [is_lib, original] = get_include_path(t_src);
  DBG_INFO("include: ", std::quoted(original), " : ", m_nesting_count);

  const auto prepend{(is_lib) ? STD_INCLUDE_PATH : ""sv};
  fs::path include{std::format("{}{}", prepend, original)};
  include = fs::absolute(include);

  include_file(t_dst, include);
}

auto Preprocessor::handle_ifdef(TextBufferPtr& t_dst, TextStreamPtr t_src)
  -> void
{
  skip_whitespace(t_src);
  const auto cond_id{get_identifier(t_src)};

  // Should we emit the current branch.
  bool emit_branch{false};
  if(m_mreg.contains(cond_id)) {
    emit_branch = true;
  }

  DBG_INFO("ifdef: ", cond_id, ", emit_branch: ", emit_branch);

  t_src->next_line(); // Skip ifdef line.
  while(!t_src->eos()) {
    // Directive processing loop.
    if(next_if_unhygienic_macro(t_src)) {
      const auto macro_id{get_identifier(t_src)};

      if(macro_id == ELSE) {
        emit_branch = !emit_branch; // Flip emit_branch state.
      } else if(macro_id == ENDIF) {
        // Quite ifdef statemachine loop.
        t_src->next_line();
        return;
      } else if(emit_branch) {
        // Only expand macros if we are in the branch that should emit.
        match_macro(macro_id, t_dst, t_src);
      }
    } else {
      t_dst->add_line(std::string{t_src->line()});
    }

    t_src->next_line();
  }

  if(t_src->eos()) {
    const auto end_pos{t_src->end_position()};
    preprocessor_error(end_pos, "End of file reached and found no #<endif.");
  }
}

auto Preprocessor::match_macro(const std::string_view t_macro_id,
                               TextBufferPtr& t_dst, TextStreamPtr t_src)
  -> void
{
  const auto pos{t_src->position()};
  if(t_macro_id == INCLUDE_ONCE) {
    handle_include_once(t_dst, t_src);
  } else if(t_macro_id == INCLUDE) {
    handle_include(t_dst, t_src);
  } else if(t_macro_id == IFDEF) {
    handle_ifdef(t_dst, t_src);
  } else if(t_macro_id == ERROR) {
    const auto msg{std::format("#<error: \"{}\".", t_macro_id)};

    preprocessor_error(pos, msg);
  } else {
    const auto msg{std::format("Unknown macro name \"{}\".", t_macro_id)};

    preprocessor_error(pos, msg);
  }
}

auto Preprocessor::handle_preprocessor(TextBufferPtr& t_dst,
                                       TextStreamPtr t_src) -> void
{
  if(m_nesting_count >= MAX_INCLUDE_NESTING) {
    const auto msg{
      std::format("Exceeded maximum #<include nesting count of {} includes.",
                  MAX_INCLUDE_NESTING)};

    preprocessor_error(t_src, msg);
  }

  while(!t_src->eos()) {
    // Directive processing loop.
    if(next_if_unhygienic_macro(t_src)) {
      const auto macro_id{get_identifier(t_src)};

      match_macro(macro_id, t_dst, t_src);
    } else {
      t_dst->add_line(std::string{t_src->line()});
    }

    t_src->next_line();
  }
}

auto Preprocessor::preprocess() -> TextStreamPtr
{
  auto dst{make_buffer()};

  handle_preprocessor(dst, m_src);
  DBG_INFO("dst: ", *dst);

  // Reset anything sanity check.
  m_src->reset();
  dst->reset();

  return dst;
}
} // namespace preprocessor
