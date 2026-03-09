# Install and include dependencies required for the project.

# Include fetch content and configure global options
include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)

# Includes:
# TODO: Figure out a way to include these relatively.
include(cmake/dependencies/tomlplusplus.cmake)
include(cmake/dependencies/rang.cmake)
include(cmake/dependencies/tabulate.cmake)
include(cmake/dependencies/boost.cmake)
include(cmake/dependencies/libassert.cmake)
include(cmake/dependencies/cereal.cmake)
include(cmake/dependencies/cpptrace.cmake)
include(cmake/dependencies/threads.cmake)
include(cmake/dependencies/cli11.cmake)
include(cmake/dependencies/llvm.cmake)
# include(cmake/dependencies/lld.cmake)
include(cmake/dependencies/clang.cmake)
