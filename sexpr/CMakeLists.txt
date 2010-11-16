cmake_minimum_required (VERSION 2.6)

project (sexpr)
message ("Generating project ${PROJECT_NAME}")

# This library has got lot of warnings. Until it compiles cleanly on all platforms
# we need to supress the warnings generated from it.

IF(CMAKE_COMPILER_IS_GNUCC)
        set(CMAKE_C_FLAGS "-Wall -fPIC")
ENDIF()

IF(MSVC)
		# turning off the warnings for sexpr on windows.
        set(CMAKE_C_FLAGS "/W0")

        # We don't want to use MSVC's more secure versions of functions. So suppressing the warnings
        add_definitions(-DWIN32 -D_CRT_SECURE_NO_WARNINGS)
ENDIF()

list (APPEND SEXPR_SOURCES
  cstring.c
  sexp_ops.c
  sexp.c
  faststack.c
  io.c 
  event_temp.c 
  parser.c 
  sexp_vis.c
  )

add_library (${SEXPR_LIBRARY_NAME} STATIC ${SEXPR_SOURCES})

