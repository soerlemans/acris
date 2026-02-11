# Set minimum CMake version.
cmake_minimum_required(VERSION 3.22)

# Define the project.
project(
  acris
  VERSION 0.1.0
  DESCRIPTION "Compiler for acris programming language 𓆏."
  LANGUAGES CXX C
)

if(DEFINED ACRIS_STATIC_BUILD)
	# Link dependencies statically.
	message(STATUS "[+] Compiling project statically.")
  set(BUILD_SHARED_LIBS OFF CACHE BOOL "Do not build as shared libs." FORCE)
else()
	# Link dependencies as shared_libraries (default).
  set(BUILD_SHARED_LIBS ON CACHE BOOL "Do build as shared libs." FORCE)
endif()

# Set the target variables.
include(cmake/targets.cmake)

# Define the executable for the acris compiler.
add_library(${TARGET_ACRIS_LIB} STATIC)
add_library(${TARGET_ACRIS_STDACRIS} STATIC)
add_executable(${TARGET_ACRIS})

# Configure include-what-you-use.
#include(cmake/iwyu.cmake)

set_target_properties(
  ${TARGET_ACRIS}
  PROPERTIES
  EXPORT_COMPILE_COMMANDS ON
)

# Configure the compiler.
include(cmake/compiler.cmake)

# Configure include-what-you-use.

# Configure clang-tidy.
include(cmake/clang-tidy.cmake)

# Configure Link time optimization.
include(cmake/lto.cmake)

# Generate the definitions header.
include(cmake/definitions.cmake)

# Add dependencies.
include(cmake/dependencies.cmake)

# Compile sources.
add_subdirectory(src)
target_link_libraries(
  ${TARGET_ACRIS}
  PUBLIC ${TARGET_ACRIS_LIB})

# Build tests:
if(DEFINED ACRIS_BUILD_TESTS)
  include(cmake/dependencies/gtest.cmake)

  add_subdirectory(tests)
endif()
