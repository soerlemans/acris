#ifndef ACRIS_ACRIS_UNIT_TRANSLATION_UNIT_HPP
#define ACRIS_ACRIS_UNIT_TRANSLATION_UNIT_HPP

// STL Includes:
#include <filesystem>
#include <iosfwd>

// Absolute Includes:
#include "acris/ast/node/fdecl.hpp"
#include "acris/codegen/backend_interface.hpp"
#include "acris/container/text_buffer.hpp"
#include "acris/mir/mir.hpp"
#include "acris/symbol_table/node_frame/node_frame_factory.hpp"
#include "acris/token/token_stream.hpp"
#include "acris/unit/build_unit.hpp"

namespace unit {
// Using Statements:
using ast::node::NodePtr;
using codegen::CompileParams;
using container::TextStreamPtr;
using mir::ModulePtr;
using std::filesystem::path;
using token::TokenStream;

// Enums:
/*!
 * Enumeration keeping track of the current phase of processing.
 * The @ref TranslationUnit is in.
 */
enum class TranslationUnitPhase {
  IDLE,
  PREPROCESSING,
  LEXING,
  PARSING,
  SEMANTIC_ANALYSIS,
  MIR_GENERATION,
  CODE_GENERATION,
  COMPLETED,
  FAILED
};

// Classes:
// TODO: Maybe use the State or Command pattern?
// We could possibly also have ModuleUnit or ConfigurationUnit?
// And these would need to be processed.
/*!
 * Keeps track of all relevant data within a single translation unit.
 * Invokes all the steps needed to go from source code to a compiled binary.
 * For now each @ref TranslationUnit references on source file.
 */
class TranslationUnit {
  private:
  TranslationUnitPhase m_phase;

  // TODO: Settings should be passed to TranslationUnit as a shared_ptr.
  // So we can actually start passing CLI options to other phases of
  // compilation.
	// For example passing through options to clang, via -X.

  // Config:
  BuildUnitPtr m_build_unit;

  // Data:
  path m_source_file;
  TextStreamPtr m_text_stream;
  TokenStream m_token_stream;
  NodePtr m_ast;
  ModulePtr m_mir;

  public:
  TranslationUnit(BuildUnitPtr t_build_unit, path t_source_file);

  //! Run the preprocessortoken_stream.
  virtual auto preprocess(const TextStreamPtr& t_text_stream) -> TextStreamPtr;

  //! Tokenize the text buffer.
  virtual auto lex(const TextStreamPtr& t_text_stream) -> TokenStream;

  //! Parse  the tokenized stream.
  virtual auto parse(const TokenStream& t_token_stream) -> NodePtr;

  //! Print the AST, only available if @ref NDEBUG is not defined.
  virtual auto print_ast(NodePtr t_ast) const -> void;

  //! Analyse the semantics of the AST.
  virtual auto semantic(NodePtr t_ast) -> void;

  //! Translate the AST to CLIR.
  virtual auto mir(NodePtr t_ast) -> ModulePtr;

  //! Execute the codegeneration backend.
  virtual auto backend(CompileParams& t_params) -> void;

  //! Acris compiler regular compilation flow.
  virtual auto execute() -> void;

  virtual ~TranslationUnit() = default;
};
} // namespace unit

auto operator<<(std::ostream& t_os, unit::TranslationUnitPhase t_phase)
  -> std::ostream&;
auto operator<<(std::ostream& t_os, unit::TranslationUnit t_unit)
  -> std::ostream&;

#endif // ACRIS_ACRIS_UNIT_TRANSLATION_UNIT_HPP
