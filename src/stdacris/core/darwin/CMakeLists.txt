set(ACRIS_STDLIB_ARCH "${CMAKE_SYSTEM_PROCESSOR}" CACHE STRING "Target CPU architecture.")
string(TOLOWER "${ACRIS_STDLIB_ARCH}" LOCAL_ARCH)

message(STATUS "Architecture: ${LOCAL_ARCH}")

# Find Architecture.
if(LOCAL_ARCH MATCHES "x86_64|amd64")
	set(LOCAL_ARCH "x86_64")
elseif(LOCAL_ARCH MATCHES "aarch64|arm64")
	set(LOCAL_ARCH "aarch64")
elseif(LOCAL_ARCH MATCHES "x86|i[3-6]86|ia32")
	set(LOCAL_ARCH "x86")
else()
	message(FATAL_ERROR "Architecture ${LOCAL_ARCH} unknown!")
endif()

set(ARCH_SOURCE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/${LOCAL_ARCH}_kernel.c")
if(NOT EXISTS "${ARCH_SOURCE_FILE}")
    message(FATAL_ERROR "Architecture ${LOCAL_ARCH} is not supported.")
endif()

target_sources(${TARGET_ACRIS_STDACRIS} PRIVATE
	"${LOCAL_ARCH}_kernel.c"
)
