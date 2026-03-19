#include "clang_frontend_invoker.hpp"

// STL Includes:
#include <cstdio>
#include <cstdlib>
#include <format>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string_view>

// Local Includes:
#include "acris/debug/log.hpp"
#include "lib/stdexcept/stdexcept.hpp"
#include "lib/stdtypes.hpp"
#include "lib/string_util.hpp"

namespace codegen::cpp_backend {
// Using Statements:
using namespace std::literals::string_view_literals;

// Methods:
ClangFrontendInvoker::ClangFrontendInvoker(): m_compiler_flags{}, m_out{}
{
  // Add debug flags.
  add_flags("-g -ggdb"sv);

  // Add C++23 standard flag.
  add_flags("-std=c++23"sv);
}

auto ClangFrontendInvoker::add_flags(const std::string_view t_str) -> void
{
  // TODO: Sanitize this one day as to prevent command injection?
  // Or just dont care?
  std::string str_buf{t_str};

  // Strip any leading and trailing whitespace.
  lib::strip_whitespace(str_buf);
  lib::trim_whitespace(str_buf);

  // Add spaces passed around the passed flags, automatically.
  m_compiler_flags << std::format(" {}", str_buf);
}

auto ClangFrontendInvoker::set_out(const std::string_view t_out) -> void
{
  // TODO: Sanitize this one day as to prevent command injection.
  m_out = t_out;
}

// Public Methods:
auto ClangFrontendInvoker::compile(const fs::path& t_source) -> void
{
  fs::path source{t_source};
  source.make_preferred();

  const auto stem{source.stem()};
  const auto tmp_base{source.parent_path() / stem};

  auto tmp_obj{tmp_base};
  tmp_obj += ".o";

  std::string out{stem};
  out += ".out";
  if(!m_out.empty()) {
    // If m_out is set override the default.
    out = m_out;
  }

  // Logging:
  DBG_INFO("source: ", t_source);
  DBG_INFO("tmp_obj: ", tmp_obj);
  DBG_INFO("out: ", out);

  // Print generated code (lazy implementation).
#ifndef NDEBUG
  DBG_PRINTLN("# C++ codegeneration:");

  const auto cmd_cat{
    std::format("clang-format --style=Google < {}", t_source.c_str())};
  std::system(cmd_cat.c_str());

  DBG_PRINTLN();
#endif // NDEBUG

  DBG_PRINTLN("# Clang Compilation:");

  // TODO: Have the O2 flag or Ofast flag be selected, in BackendInterface.
  // From an enumeration.
  // TODO: Add -O2 flag on release and reldebug builds (not on debug).
  // FIXME: This is a temporary workaround till the programmatic approach works.

  // List version of compiler used.
  // We use G++ at the moment as it supports more of C++23.
  const auto cpp_compiler{R"("${CXX:-g++}")"sv};

  const auto flags{m_compiler_flags.view()};
  const auto cmd{std::format("export SRC_STEM=\"{}\"; {} {} {} -o {}",
                             stem.c_str(), cpp_compiler, t_source.c_str(),
                             flags, out)};

  DBG_NOTICE("Compiler command: ", cmd);
  const auto status_code{std::system(cmd.c_str())};

  if(status_code == 0) {
    std::cout << std::format("Compiled {}!\n", out);
  } else {
    std::cerr << std::format("Failed to compile {}!\n", out);

    // TODO: Throw an exception. Or maybe not?
  }
}

	// TODO: Test/fully implement.
auto shell_exec(std::string t_cmd) -> ProcessResult
{
  using lib::stdexcept::RuntimeError;
  using lib::stdexcept::throwf;

  ProcessResult result{};
  std::array<char, 512> buf{};

  // Do an estimate for now.
  result.m_stdout.reserve(10);

#if defined(_WIN32)
  auto* pipe = _popen(t_cmd.c_str(), "r");
#else
  auto* pipe = popen(t_cmd.c_str(), "r");
#endif

  if(!pipe) {
    throwf<RuntimeError>("Failed to run command: {}", t_cmd);
  }

  std::string line{};
  while(std::fgets(buf.data(), buf.size(), pipe) != nullptr) {
    std::string line{buf.data()};

    if(!line.empty() && line.back() == '\n') {
      line.pop_back();

      // We have a full line push back.
      result.m_stdout.push_back(line);
      line.clear();
    }
  }

#if defined(_WIN32)
  result.m_exit_code = _pclose(pipe);
#else
  int rc = pclose(pipe);
  result.m_exit_code = WEXITSTATUS(rc);
#endif

  return result;
}
} // namespace codegen::cpp_backend
