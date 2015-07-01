##
# Copyright (C) Navaneeth.K.N
#
# This is part of libvarnam. See LICENSE.txt for the license
##

cmake_minimum_required (VERSION 2.8.8)

project(varnam)
message ("Generating project ${PROJECT_NAME}")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
	add_definitions(-D_FORTIFY_SOURCE=2)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/deps/cmake_modules/")

set(VARNAM_LIBRARY_NAME "varnam")
set(VARNAM_LIBRARY_NAME_STATIC "varnamstatic")
set(DEPS_LIBRARY_NAME "deps")

set(VARNAM_VERSION_MAJOR 3)
set(VARNAM_VERSION_MINOR 2)
set(VARNAM_VERSION_PATCH 5)

option(EMBED_SQLITE "Embed Sqlite or use the shared library available in the system?" ON)
option(DISABLE_WITHOUT_ROW_ID_OPTIMIZATION "Disable this specific optimization as the SQLite version don't have it" OFF)
option(BUILD_TESTS "Build tests?" OFF)
option(BUILD_EXAMPLES "Build examples?" OFF)
option(BUILD_TOOLS "Build tools?" OFF)

set(CMAKE_C_FLAGS "-Wall -ansi -pedantic -Wno-long-long -Wconversion -Wformat=2 -Wshadow -Wcast-qual -Wwrite-strings -fPIC")
set(CMAKE_C_FLAGS_DEBUG "-g")
set(CMAKE_C_FLAGS_RELEASE "-O2")
set(SQLITE3_LIBRARIES "")

if (NOT EMBED_SQLITE)
	find_package (SQLite3 REQUIRED)
	include_directories (${SQLITE3_INCLUDE_DIRS})
endif ()

if (DISABLE_WITHOUT_ROW_ID_OPTIMIZATION)
	# Added to support older SQLIte versions which don't have WIthout row id support
	# eg: Debian wheezy still uses 3.7.13. This support was added in 3.8.2. 
	# This is required only when EMBED_SQLITE=false
	add_definitions(-DVARNAM_DISABLE_WITHOUT_ROW_ID_OPTIMIZATION)
endif()

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

list (APPEND SUPPORTED_SCHEMES
  ml
  ml-inscript
  mr
  ne
  or
  pa
  sa
  ta
  te
  hi
  kn
  gu
	bn
	as
)

# Append the source files here
list (APPEND VARNAM_SOURCES
  util.c
  lru_cache.c
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

# Generate pkg-config file
configure_file(varnam.pc.in varnam.pc @ONLY)
configure_file(varnamstatic.pc.in varnamstatic.pc @ONLY)

add_definitions(-DSQLITE_CASE_SENSITIVE_LIKE -DSQLITE_ENABLE_FTS3 -DSQLITE_ENABLE_FTS3_PARENTHESIS)

# Create a shared and static library libvarnam
add_library (varnam_core OBJECT ${VARNAM_SOURCES})
add_library (${VARNAM_LIBRARY_NAME_STATIC} STATIC $<TARGET_OBJECTS:${DEPS_LIBRARY_NAME}> $<TARGET_OBJECTS:varnam_core>)
add_library (${VARNAM_LIBRARY_NAME} SHARED $<TARGET_OBJECTS:${DEPS_LIBRARY_NAME}> $<TARGET_OBJECTS:varnam_core>)

SET_TARGET_PROPERTIES(${VARNAM_LIBRARY_NAME} PROPERTIES
                                             VERSION ${VARNAM_VERSION_MAJOR}.${VARNAM_VERSION_MINOR}.${VARNAM_VERSION_PATCH}
                                             SOVERSION ${VARNAM_VERSION_MAJOR})

# a custom target to compile all scheme files
# this is just a highlevel target and there are individual targets which depends on this
add_custom_target (vst)

# Each scheme will have a target to compile
# vst will have a dependency to all these targets so that running vst will compile all the scheme files
foreach(scheme ${SUPPORTED_SCHEMES})
  add_custom_target (${scheme}.vst COMMAND ./varnamc --compile schemes/${scheme})
  add_dependencies (vst "${scheme}.vst")
  install (FILES schemes/${scheme}.vst DESTINATION ${CMAKE_INSTALL_PREFIX}/share/varnam/vst OPTIONAL)
endforeach()

add_custom_target (test command cd tests && ./runtests)

# Makes the distro
add_custom_target (distro COMMAND ./makedistro.sh "${VARNAM_VERSION_MAJOR}.${VARNAM_VERSION_MINOR}.${VARNAM_VERSION_PATCH}")

## The following variable is used to install libraries on multiarch supported environments, like Debian & Ubuntu
## Debian's build script will set the VARNAM_LIB_ARCH environment variable to indicate the architecture specific directory
## https://wiki.debian.org/Multiarch
set(VARNAM_LIB_INSTALL_DIR "")
if(DEFINED ENV{VARNAM_LIB_ARCH})
  set(VARNAM_LIB_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/lib/$ENV{VARNAM_LIB_ARCH})
else()
  set(VARNAM_LIB_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/lib)
endif()

INSTALL ( TARGETS ${VARNAM_LIBRARY_NAME} DESTINATION ${VARNAM_LIB_INSTALL_DIR})
INSTALL ( TARGETS ${VARNAM_LIBRARY_NAME_STATIC} DESTINATION ${VARNAM_LIB_INSTALL_DIR})
INSTALL ( FILES ${VARNAM_INCLUDE_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/lib${VARNAM_LIBRARY_NAME})
INSTALL ( FILES deps/sqlite3.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/lib${VARNAM_LIBRARY_NAME}/deps)
INSTALL ( FILES deps/uthash.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/lib${VARNAM_LIBRARY_NAME}/deps)
INSTALL ( FILES ${CMAKE_BINARY_DIR}/varnam.pc DESTINATION ${VARNAM_LIB_INSTALL_DIR}/pkgconfig)
INSTALL ( FILES ${CMAKE_BINARY_DIR}/varnamstatic.pc DESTINATION ${VARNAM_LIB_INSTALL_DIR}/pkgconfig)
INSTALL ( FILES varnamruby.rb DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
INSTALL_PROGRAMS(/bin FILES varnamc)
INSTALL ( FILES varnamc.1 DESTINATION ${CMAKE_INSTALL_PREFIX}/share/man/man1)

# uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)


IF(MSVC)
  target_link_libraries(${VARNAM_LIBRARY_NAME} ${SQLITE3_LIBRARIES})
else()
        # sqlite requires pthread and dl
        target_link_libraries(${VARNAM_LIBRARY_NAME} pthread dl ${SQLITE3_LIBRARIES})
ENDIF()

