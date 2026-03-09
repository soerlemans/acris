#ifndef ACRIS_ACRIS_PARSER_ACRIS_CONTEXT_CONTEXT_STORE_HPP
#define ACRIS_ACRIS_PARSER_ACRIS_CONTEXT_CONTEXT_STORE_HPP

// STL Includes:
#include <string_view>
#include <unordered_map>

namespace parser::acris::context {
// Enums:
//! Possible contexts to be parsing in.
enum class Context {
  ENUM,
  METHOD,
  LOOP,
};

// Aliases:
using ContextMap = std::unordered_map<Context, bool>;

// Classes:
class ContextStore {
  private:
  ContextMap m_store;

  public:
  ContextStore();

  auto set(Context t_context) -> void;
  auto clear(Context t_context) -> void;

  auto get(Context t_context) -> bool;

  virtual ~ContextStore() = default;
};

//! Parsing context of course.
auto context2str(Context t_ctx) -> std::string_view;
} // namespace parser::acris::context

#endif // ACRIS_ACRIS_PARSER_ACRIS_CONTEXT_CONTEXT_STORE_HPP
