#----------------------------------------------------------------
# Generated CMake target import file for configuration "RelWithDebInfo".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Lua::lua_shared" for configuration "RelWithDebInfo"
set_property(TARGET Lua::lua_shared APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(Lua::lua_shared PROPERTIES
  IMPORTED_IMPLIB_RELWITHDEBINFO "${_IMPORT_PREFIX}/lib/lua_shared.lib"
  IMPORTED_LOCATION_RELWITHDEBINFO "${_IMPORT_PREFIX}/bin/lua_shared.dll"
  )

list(APPEND _cmake_import_check_targets Lua::lua_shared )
list(APPEND _cmake_import_check_files_for_Lua::lua_shared "${_IMPORT_PREFIX}/lib/lua_shared.lib" "${_IMPORT_PREFIX}/bin/lua_shared.dll" )

# Import target "Lua::lua_static" for configuration "RelWithDebInfo"
set_property(TARGET Lua::lua_static APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(Lua::lua_static PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELWITHDEBINFO "C"
  IMPORTED_LOCATION_RELWITHDEBINFO "${_IMPORT_PREFIX}/lib/lua_static.lib"
  )

list(APPEND _cmake_import_check_targets Lua::lua_static )
list(APPEND _cmake_import_check_files_for_Lua::lua_static "${_IMPORT_PREFIX}/lib/lua_static.lib" )

# Import target "Lua::lua" for configuration "RelWithDebInfo"
set_property(TARGET Lua::lua APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(Lua::lua PROPERTIES
  IMPORTED_LOCATION_RELWITHDEBINFO "${_IMPORT_PREFIX}/bin/lua.exe"
  )

list(APPEND _cmake_import_check_targets Lua::lua )
list(APPEND _cmake_import_check_files_for_Lua::lua "${_IMPORT_PREFIX}/bin/lua.exe" )

# Import target "Lua::luac" for configuration "RelWithDebInfo"
set_property(TARGET Lua::luac APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(Lua::luac PROPERTIES
  IMPORTED_LOCATION_RELWITHDEBINFO "${_IMPORT_PREFIX}/bin/luac.exe"
  )

list(APPEND _cmake_import_check_targets Lua::luac )
list(APPEND _cmake_import_check_files_for_Lua::luac "${_IMPORT_PREFIX}/bin/luac.exe" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
