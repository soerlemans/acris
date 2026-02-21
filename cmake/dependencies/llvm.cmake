# LLVM (Compiler toolchain libraries):
message(STATUS "[+] Finding LLVM.")

find_package(
	LLVM
	17.0.6
	CONFIG
	REQUIRED
	# COMPONENTS lld
)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Required setup:
include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIR})

# Add macros:
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

# Link libraries:
target_link_libraries(
	${TARGET_ACRIS_LIB}
	PUBLIC LLVM-17
)
