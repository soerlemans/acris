#ifndef ACRIS_ACRIS_PREPROCESSOR_PREPROCESSOR_HPP
#define ACRIS_ACRIS_PREPROCESSOR_PREPROCESSOR_HPP

// STL Includes:
#include <filesystem>
#include <map>
#include <set>
#include <stack>
#include <string>
#include <string_view>
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
using container::TextPosition;
using container::TextStreamPtr;

namespace fs = std::filesystem;

using IncludedRegister = std::set<std::filesystem::path>;
using MacroRegister = std::map<std::string, std::string>;

constexpr u8 MAX_INCLUDE_NESTING{255};

struct IncludePack {
  bool m_is_lib;
  std::string m_include;
};

class Preprocessor {
  private:
  TextStreamPtr m_src;

  u16 m_nesting_count;
  IncludedRegister m_ireg;
  MacroRegister m_mreg;

  protected:
  auto preprocessor_error(TextPosition t_pos, std::string_view t_msg) const
    -> void;
  auto preprocessor_error(TextStreamPtr t_src, std::string_view t_msg) const
    -> void;

  public:
  explicit Preprocessor(TextStreamPtr t_src);

  auto init_default_macros() -> void;
  auto defines(const MacroRegister& t_mdefs) -> void;
  auto make_buffer() -> TextBufferPtr;

  auto next_if_unhygienic_macro(TextStreamPtr t_src) -> bool;
  auto skip_whitespace(TextStreamPtr t_src);
  auto get_identifier(TextStreamPtr t_src) -> std::string;

  // auto next_ch();

  auto include_file(TextBufferPtr& t_dst, const fs::path t_path) -> void;
  auto get_include_path(TextStreamPtr t_src) -> IncludePack;

  auto handle_include_once(TextBufferPtr& t_dst, TextStreamPtr t_src) -> void;
  auto handle_include(TextBufferPtr& t_dst, TextStreamPtr t_src) -> void;
  auto handle_ifdef(TextBufferPtr& t_dst, TextStreamPtr t_src) -> void;
  auto match_macro(std::string_view t_macro_id, TextBufferPtr& t_dst,
                   TextStreamPtr t_src) -> void;

  auto handle_preprocessor(TextBufferPtr& t_dst, TextStreamPtr t_src) -> void;

  auto preprocess() -> TextStreamPtr;

  virtual ~Preprocessor() = default;
};
} // namespace preprocessor

#endif // ACRIS_ACRIS_PREPROCESSOR_PREPROCESSOR_HPP
