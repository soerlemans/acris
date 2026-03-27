#ifndef ACRIS_ACRIS_CONTAINER_TEXT_BUFFER_HPP
#define ACRIS_ACRIS_CONTAINER_TEXT_BUFFER_HPP

// STL Includes:
#include <string>
#include <string_view>
#include <vector>

// Local Includes:
#include "text_stream.hpp"

namespace container {
class TextBuffer;

using TextBufferPtr = std::shared_ptr<TextBuffer>;

class TextBuffer : public TextStream {
  private:
  std::vector<std::string> m_buffer;

  std::string m_source;

  // Keep track of current position in the filebuffer
  mutable std::size_t m_lineno;
  mutable std::size_t m_columnno;

  public:
  TextBuffer();
  TextBuffer(const std::string_view t_source);

  auto add_line(std::string t_line) -> void;

  auto next_line() const -> void override;
  auto prev_line() const -> void;

  auto next() const -> void override;
  auto prev() const -> void override;

  auto peek() const -> CharOpt override;

  auto source() const -> std::string_view override;
  auto character() const -> char override;
  auto line() const -> std::string_view override;

  auto eos() const -> bool override;
  auto reset() -> void override;

  auto position() const -> TextPosition override;
  auto end_position() const -> TextPosition override;

  friend auto operator<<(std::ostream& t_os, const TextBuffer& t_tb)
    -> std::ostream&;

  virtual ~TextBuffer() = default;
};
} // namespace container

#endif // ACRIS_ACRIS_CONTAINER_TEXT_BUFFER_HPP
