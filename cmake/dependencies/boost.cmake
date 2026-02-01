# Boost (C++ utility libraries):
message(STATUS "[+] Finding Boost library.")

if(DEFINED ACRIS_STATIC_BUILD)
	message(STATUS "[+] Linking Boost statically.")
	set(Boost_USE_STATIC_LIBS ON CACHE BOOL "Link Boost statically.")
endif()

find_package(
  Boost
  1.83
  COMPONENTS program_options filesystem iostreams thread
  REQUIRED
)

include_directories(
  SYSTEM
  ${Boost_INCLUDE_DIRS}
)

target_link_libraries(
  ${TARGET_ACRIS_LIB}
  PUBLIC ${Boost_LIBRARIES}
)
