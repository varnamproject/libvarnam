
cmake_minimum_required (VERSION 2.6)

project(varnam)

set(VARNAM_LIBRARY_NAME "varnam")
set(SEXPR_LIBRARY_NAME "sexpr")
set(FOREIGN_LIBRARY_NAME "foreign-functions")

IF(CMAKE_COMPILER_IS_GNUCC)
        set(CMAKE_C_FLAGS "-g -Wall -ansi -pedantic -Wconversion -Wformat=2 -Wshadow -Wcast-qual -Wwrite-strings")
ENDIF()

IF(MSVC)
        set(CMAKE_C_FLAGS "/Wall /wd\"4255;4100\"")
        # We don't want to use MSVC's more secure versions of functions. So suppressing the warnings
        add_definitions(-DWIN32 -D_CRT_SECURE_NO_WARNINGS)
ENDIF()

add_definitions(-DHAVE_SNPRINTF -DPREFER_PORTABLE_SNPRINTF)
    
# Append the source files here
list (APPEND VARNAM_SOURCES
  compile.c
  util.c
  trie.c
  strbuf.c
  lex.c
  parser.c
  vstgen.c
  tl.c
  )

# Build foreign functions
add_subdirectory(foreign)

# Build the sexpr library
add_subdirectory(sexpr)

# Create a shared library libvarnam
add_library (${VARNAM_LIBRARY_NAME} SHARED ${VARNAM_SOURCES})

IF(CMAKE_COMPILER_IS_GNUCC)
        # sqlite requires pthread and dl
        target_link_libraries(${VARNAM_LIBRARY_NAME} ${SEXPR_LIBRARY_NAME} ${FOREIGN_LIBRARY_NAME} pthread dl)
ENDIF()

IF(MSVC)
        target_link_libraries(${VARNAM_LIBRARY_NAME} ${SEXPR_LIBRARY_NAME} ${FOREIGN_LIBRARY_NAME})
ENDIF()

