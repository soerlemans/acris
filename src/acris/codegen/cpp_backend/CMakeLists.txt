# Add source files
target_sources(${TARGET_ACRIS_LIB} PRIVATE
	cpp_backend.cpp
	type_cpp_conv.cpp
	clang_frontend_invoker.cpp
)

# Add subdirectories
add_subdirectory(interop)
