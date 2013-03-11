
cmake_minimum_required (VERSION 2.6)

project (tools)
message ("Generating project ${PROJECT_NAME}")

add_executable(print-tokens print-tokens.c)
target_link_libraries(print-tokens ${VARNAM_LIBRARY_NAME})


