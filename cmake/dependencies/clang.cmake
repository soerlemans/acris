# LLD linker (Comes with LLVM needed for linking object files into executable):
message(STATUS "[+] Finding Clang driver.")

# Needed to find LLDConfig.cmake.
list(APPEND CMAKE_PREFIX_PATH "/usr/lib/llvm-17")

find_package(
	Clang
	17.0.6
	CONFIG
	REQUIRED
)

include_directories(${CLANG_INCLUDE_DIRS})

# # Link clang driver.
target_link_libraries(
	${TARGET_ACRIS_LIB}
	PUBLIC
	clangDriver        # The orchestrator.
  clangFrontend      # For Diagnostic printers.
  clangBasic         # Source locations and error IDs.
)
