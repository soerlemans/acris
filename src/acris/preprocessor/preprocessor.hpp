#ifndef ACRIS_ACRIS_PREPROCESSOR_PREPROCESSOR_HPP
#define ACRIS_ACRIS_PREPROCESSOR_PREPROCESSOR_HPP

// STL Includes:
#include <filesystem>
#include <set>
#include <string>
#include <unordered_set>

// Absolute Includes:
#include "acris/container/text_buffer.hpp"
#include "acris/container/text_stream.hpp"
#include "lib/stdtypes.hpp"

/*!
 * @file Preprocessor is solely used for expanding unhygienic macros.
 * Which should probably also be done after lexing.
 * Which I am not very sure of.
 */

namespace preprocessor {
using container::TextBuffer;
using container::TextBufferPtr;
using container::TextStreamPtr;

namespace fs = std::filesystem;

using IncludedRegister = std::set<std::filesystem::path>;
using DefinedRegister = std::unordered_set<std::string>;

constexpr u8 MAX_INCLUDE_NESTING{255};

struct IncludePack {
  bool m_is_lib;
  std::string m_include;
};

class Preprocessor {
  private:
  TextStreamPtr m_text;

  u16 m_nesting_count;
  IncludedRegister m_included;
  DefinedRegister m_defined;

  public:
  explicit Preprocessor(TextStreamPtr t_text);

	auto set_defined(DefinedRegister&& t_defined) -> void;

  auto make_buffer() -> TextBufferPtr;

  auto next_if_unhygienic_macro(TextStreamPtr t_text) -> bool;
  auto get_id(TextStreamPtr t_text) -> std::string;

  // auto next_ch();

  auto include_file(TextBufferPtr& t_buffer, const fs::path t_path) -> void;
  auto get_include_path(TextStreamPtr t_text) -> IncludePack;
  auto skip_whitespace(TextStreamPtr t_text);

  auto handle_include_once(TextStreamPtr t_text, TextBufferPtr& t_buffer)
    -> void;
  auto handle_include(TextStreamPtr t_text, TextBufferPtr& t_buffer) -> void;

  auto handle_ifdef(TextStreamPtr t_text, TextBufferPtr& t_buffer) -> void;

  auto handle_preprocessor(TextStreamPtr t_text, TextBufferPtr& t_buffer)
    -> void;

  auto preprocess() -> TextStreamPtr;

  virtual ~Preprocessor() = default;
};
} // namespace preprocessor

#endif // ACRIS_ACRIS_PREPROCESSOR_PREPROCESSOR_HPP
