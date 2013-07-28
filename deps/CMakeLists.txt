

# This builds the 3rd party code that we uses. It uses very less level of warnings 
# than the actual code
IF(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_C_FLAGS "-Wall -ansi -pedantic -fPIC")
ENDIF()

IF(MSVC)
    set(CMAKE_C_FLAGS "/Wall")
    # We don't want to use MSVC's more secure versions of functions. So suppressing the warnings
    add_definitions(-DWIN32 -D_CRT_SECURE_NO_WARNINGS)
ENDIF()

add_definitions(-DSQLITE_CASE_SENSITIVE_LIKE -DSQLITE_ENABLE_FTS3 -DSQLITE_ENABLE_FTS3_PARENTHESIS)

list (APPEND DEPS_SOURCES
    snprintf.c
    sqlite3.c
    )

add_library (${DEPS_LIBRARY_NAME} OBJECT ${DEPS_SOURCES})


