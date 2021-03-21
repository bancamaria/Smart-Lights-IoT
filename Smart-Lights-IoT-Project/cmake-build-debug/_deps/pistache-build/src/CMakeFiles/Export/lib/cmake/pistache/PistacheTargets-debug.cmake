#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "pistache_shared" for configuration "Debug"
set_property(TARGET pistache_shared APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(pistache_shared PROPERTIES
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libpistache-0.0.002-git20210107.so.0.0"
  IMPORTED_SONAME_DEBUG "libpistache-0.0.002-git20210107.so.0.0"
  )

list(APPEND _IMPORT_CHECK_TARGETS pistache_shared )
list(APPEND _IMPORT_CHECK_FILES_FOR_pistache_shared "${_IMPORT_PREFIX}/lib/libpistache-0.0.002-git20210107.so.0.0" )

# Import target "pistache_static" for configuration "Debug"
set_property(TARGET pistache_static APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(pistache_static PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libpistache.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS pistache_static )
list(APPEND _IMPORT_CHECK_FILES_FOR_pistache_static "${_IMPORT_PREFIX}/lib/libpistache.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
