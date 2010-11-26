
cmake_minimum_required (VERSION 2.6)

project(varnam)

set(VARNAM_LIBRARY_NAME "varnam")
set(FOREIGN_LIBRARY_NAME "foreign-functions")

set(VARNAM_VERSION_MAJOR 1)
set(VARNAM_VERSION_MINOR 0)
set(VARNAM_VERSION_PATCH 1)

IF(CMAKE_COMPILER_IS_GNUCC)
        set(CMAKE_C_FLAGS "-g -Wall -ansi -pedantic -Wconversion -Wformat=2 -Wshadow -Wcast-qual -Wwrite-strings")
ENDIF()

IF(MSVC)
        set(CMAKE_C_FLAGS "/Wall /wd\"4255;4100\"")
        # We don't want to use MSVC's more secure versions of functions. So suppressing the warnings
        add_definitions(-DWIN32 -D_CRT_SECURE_NO_WARNINGS)
ENDIF()

add_definitions(-DHAVE_SNPRINTF -DPREFER_PORTABLE_SNPRINTF -DNEED_ASPRINTF)
    
# Append the source files here
list (APPEND VARNAM_SOURCES
  varnam-util.c
  varnam-trie.c
  varnam-strbuf.c
  varnam-tl.c
  varnam.c
  )

# Append the header files here. this will get copied to include directory
list (APPEND VARNAM_INCLUDE_FILES
  varnam.h  
  varnam-result-codes.h  
  varnam-trie.h  
  varnam-types.h  
  varnam-util.h
  )

# Build foreign functions
add_subdirectory(foreign)
        
# Create a shared library libvarnam     
add_library (${VARNAM_LIBRARY_NAME} SHARED ${VARNAM_SOURCES})

SET_TARGET_PROPERTIES(${VARNAM_LIBRARY_NAME} PROPERTIES 
                                             VERSION ${VARNAM_VERSION_MAJOR}.${VARNAM_VERSION_MINOR}.${VARNAM_VERSION_PATCH} 
                                             SOVERSION ${VARNAM_VERSION_MAJOR})

INSTALL ( TARGETS ${VARNAM_LIBRARY_NAME} DESTINATION lib)
INSTALL ( FILES ${VARNAM_INCLUDE_FILES} DESTINATION include/lib${VARNAM_LIBRARY_NAME})
INSTALL ( FILES foreign/sqlite3.h DESTINATION include/lib${VARNAM_LIBRARY_NAME}/foreign)

IF(CMAKE_COMPILER_IS_GNUCC)
        # sqlite requires pthread and dl
        target_link_libraries(${VARNAM_LIBRARY_NAME} ${FOREIGN_LIBRARY_NAME} pthread dl)
ENDIF()

IF(MSVC)
        target_link_libraries(${VARNAM_LIBRARY_NAME} ${FOREIGN_LIBRARY_NAME})
ENDIF()

