
cmake_minimum_required (VERSION 2.6)

project(varnam)
message ("Generating project ${PROJECT_NAME}")

set(VARNAM_LIBRARY_NAME "varnam")
set(DEPS_LIBRARY_NAME "deps")

set(VARNAM_VERSION_MAJOR 1)
set(VARNAM_VERSION_MINOR 0)
set(VARNAM_VERSION_PATCH 1)

enable_testing()

IF(CMAKE_COMPILER_IS_GNUCC)
        set(CMAKE_C_FLAGS "-g -Wall -ansi -pedantic -Wconversion -Wformat=2 -Wshadow -Wcast-qual -Wwrite-strings")
ENDIF()

IF(MSVC)
        set(CMAKE_C_FLAGS "/Wall /wd\"4255;4100\"")
        # We don't want to use MSVC's more secure versions of functions. So suppressing the warnings
        add_definitions(-DWIN32 -D_CRT_SECURE_NO_WARNINGS)
ENDIF()

if (RECORD_EXEC_TIME)
   add_definitions(-D_RECORD_EXEC_TIME)
endif (RECORD_EXEC_TIME)

if (VARNAM_VERBOSE)
   add_definitions(-D_VARNAM_VERBOSE)
endif (VARNAM_VERBOSE)

add_definitions(-DHAVE_SNPRINTF -DPREFER_PORTABLE_SNPRINTF -DNEED_ASPRINTF)

# Append the source files here
list (APPEND VARNAM_SOURCES
  util.c
  trie.c
  strbuf.c
  transliterate.c
  symbol-table.c
  words-table.c
  varray.c
  token.c
  vword.c
  learn.c
  rendering.c
  renderer/ml_unicode.c
  varnam.c
  )

# Append the header files here. this will get copied to include directory
list (APPEND VARNAM_INCLUDE_FILES
  varnam.h
  api.h
  result-codes.h
  vtypes.h
  util.h
  varray.h
  )

# Build dependencies
add_subdirectory(deps)

# Build tests
add_subdirectory(tests)

# Create a shared library libvarnam
add_library (${VARNAM_LIBRARY_NAME} SHARED ${VARNAM_SOURCES})

SET_TARGET_PROPERTIES(${VARNAM_LIBRARY_NAME} PROPERTIES
                                             VERSION ${VARNAM_VERSION_MAJOR}.${VARNAM_VERSION_MINOR}.${VARNAM_VERSION_PATCH}
                                             SOVERSION ${VARNAM_VERSION_MAJOR})

INSTALL ( TARGETS ${VARNAM_LIBRARY_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
INSTALL ( FILES ${VARNAM_INCLUDE_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/lib${VARNAM_LIBRARY_NAME})
INSTALL ( FILES deps/sqlite3.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/lib${VARNAM_LIBRARY_NAME}/deps)
INSTALL ( FILES schemes/ml-unicode.vst DESTINATION ${CMAKE_INSTALL_PREFIX}/share/varnam/vst)

IF(CMAKE_COMPILER_IS_GNUCC)
        # sqlite requires pthread and dl
        target_link_libraries(${VARNAM_LIBRARY_NAME} ${DEPS_LIBRARY_NAME} pthread dl)
ENDIF()

IF(MSVC)
        target_link_libraries(${VARNAM_LIBRARY_NAME} ${DEPS_LIBRARY_NAME})
ENDIF()

add_custom_target(vstfiles
                           COMMAND varnam-cli compile vsf/ml-unicode.vsf
                           COMMAND varnam-cli compile tests/td-01.vsf)
