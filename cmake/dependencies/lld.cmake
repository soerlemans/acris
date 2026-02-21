# LLD linker (Comes with LLVM needed for linking object files into executable):
message(STATUS "[+] Finding LLD.")

# Needed to find LLDConfig.cmake.
list(APPEND CMAKE_PREFIX_PATH "/usr/lib/llvm-17")

find_package(
	LLD
	17.0.6
	CONFIG
	REQUIRED
)

message(STATUS "[+] lld_include_dirs: ${LLD_INCLUDE_DIRS}.")
include_directories(${LLD_INCLUDE_DIRS})

# # Link lld.
target_link_libraries(
	${TARGET_ACRIS_LIB}
	PUBLIC
	lldCommon
	# lldCOFF
	lldELF
	lldMachO
	# lldMinGW
	lldWasm
)
