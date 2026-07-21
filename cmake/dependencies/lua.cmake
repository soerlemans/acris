# Lua (For build-system and other):

# FIXME: Currently developing on MacOS and this is a pain, so ignore.
message(STATUS "[+] Finding Lua interpreter.")

message(STATUS "[+] Linking Boost statically.")
set(Boost_USE_STATIC_LIBS ON CACHE BOOL "Link Boost statically.")

# TODO: Add minimum version?
find_package(
  Lua
  REQUIRED
)

message(STATUS "[+] Lua found: ${LUA_VERSION_STRING}")

include_directories(
  SYSTEM
  "${LUA_INCLUDE_DIR}"
)

target_link_libraries(
  ${TARGET_ACRIS_LIB}
  PUBLIC ${LUA_LIBRARIES}
)
