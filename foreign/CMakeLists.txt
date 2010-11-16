cmake_minimum_required (VERSION 2.6)

project (foreign_functions)
message ("Generating project ${PROJECT_NAME}")

# This builds the 3rd party code that we uses. It uses very less level of warnings 
# than the actual code

IF(CMAKE_COMPILER_IS_GNUCC)
        set(CMAKE_C_FLAGS "-Wall -ansi -pedantic -fPIC")
ENDIF()

IF(MSVC)
		# turning off the warnings for sexpr on windows.
        set(CMAKE_C_FLAGS "/Wall")

        # We don't want to use MSVC's more secure versions of functions. So suppressing the warnings
        add_definitions(-DWIN32 -D_CRT_SECURE_NO_WARNINGS)
ENDIF()

list (APPEND FOREIGN_SOURCES
  snprintf.c
  sqlite3.c
  )

add_library (${FOREIGN_LIBRARY_NAME} STATIC ${FOREIGN_SOURCES})

