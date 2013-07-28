
cmake_minimum_required (VERSION 2.8)

project(varnam)
message ("Generating project ${PROJECT_NAME}")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/deps/cmake_modules/")

set(VARNAM_LIBRARY_NAME "varnam")
set(VARNAM_LIBRARY_NAME_STATIC "varnamstatic")
set(DEPS_LIBRARY_NAME "deps")

set(VARNAM_VERSION_MAJOR 2)
set(VARNAM_VERSION_MINOR 2)
set(VARNAM_VERSION_PATCH 0)

option(BUILD_TESTS "Build tests?" OFF)
option(BUILD_EXAMPLES "Build examples?" OFF)
option(BUILD_TOOLS "Build tools?" OFF)

IF(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_C_FLAGS "-Wall -ansi -pedantic -Wconversion -Wformat=2 -Wshadow -Wcast-qual -Wwrite-strings")
    set(CMAKE_C_FLAGS_DEBUG "-g")
    set(CMAKE_C_FLAGS_RELEASE "-O2")
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
add_definitions(-DVARNAM_VERSION="${VARNAM_VERSION_MAJOR}.${VARNAM_VERSION_MINOR}.${VARNAM_VERSION_PATCH}")

# Append the source files here
list (APPEND VARNAM_SOURCES
  util.c
  vutf8.c
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
  lang_detection.c
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
  vutf8.h
  langcodes.h
  )

add_subdirectory(deps)

if (BUILD_TESTS)
    # Build tests
    add_subdirectory(tests)
endif ()

if (BUILD_EXAMPLES)
    # Build examples
    add_subdirectory(examples)
endif ()

if (BUILD_TOOLS)
    # Build tools
    add_subdirectory(tools)
endif ()


add_definitions(-DSQLITE_CASE_SENSITIVE_LIKE -DSQLITE_ENABLE_FTS3 -DSQLITE_ENABLE_FTS3_PARENTHESIS)

# Create a shared and static library libvarnam
add_library (varnam_core OBJECT ${VARNAM_SOURCES})
add_library (${VARNAM_LIBRARY_NAME_STATIC} STATIC $<TARGET_OBJECTS:${DEPS_LIBRARY_NAME}> $<TARGET_OBJECTS:varnam_core>)
add_library (${VARNAM_LIBRARY_NAME} SHARED $<TARGET_OBJECTS:${DEPS_LIBRARY_NAME}> $<TARGET_OBJECTS:varnam_core>)

SET_TARGET_PROPERTIES(${VARNAM_LIBRARY_NAME} PROPERTIES
                                             VERSION ${VARNAM_VERSION_MAJOR}.${VARNAM_VERSION_MINOR}.${VARNAM_VERSION_PATCH}
                                             SOVERSION ${VARNAM_VERSION_MAJOR})

file(GLOB vst_files  "schemes/*.vst")

INSTALL ( TARGETS ${VARNAM_LIBRARY_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
INSTALL ( TARGETS ${VARNAM_LIBRARY_NAME_STATIC} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
INSTALL ( FILES ${VARNAM_INCLUDE_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/lib${VARNAM_LIBRARY_NAME})
INSTALL ( FILES deps/sqlite3.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/lib${VARNAM_LIBRARY_NAME}/deps)
INSTALL ( FILES ${vst_files} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/varnam/vst OPTIONAL)
INSTALL_PROGRAMS(/bin FILES varnamc varnamruby.rb)

IF(MSVC)
        target_link_libraries(${VARNAM_LIBRARY_NAME})
else()
        # sqlite requires pthread and dl
        target_link_libraries(${VARNAM_LIBRARY_NAME} pthread dl)
ENDIF()

