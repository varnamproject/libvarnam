##
# Copyright (C) Navaneeth.K.N
#
# This is part of libvarnam. See LICENSE.txt for the license
##


cmake_minimum_required (VERSION 2.8)

project (tests)
message ("Generating project ${PROJECT_NAME}")

find_package (Check)

include_directories(${CHECK_INCLUDE_DIRS})

file(REMOVE_RECURSE output)
file(MAKE_DIRECTORY output)

# Append the files that has tests here
list (APPEND TEST_FILES
  initialization.c
  transliteration.c
  learning.c
  testcases.c
  vst-compilation.c
  export-words.c
  strbuftest.c
  varnamc_tests.c
  stemmer_tests.c
  )

set(test_executable_name runtests)

add_executable(${test_executable_name} test-runner.c ${TEST_FILES})

target_link_libraries(${test_executable_name} ${VARNAM_LIBRARY_NAME} ${ARGTABLE_LIBRARY_NAME} ${CHECK_LIBRARIES} m)

