
cmake_minimum_required (VERSION 2.6)

project (tests)
message ("Generating project ${PROJECT_NAME}")

# Append the files that has tests here
list (APPEND TEST_FILES
  01-transliteration.c  
  02-ml-unicode.c
  )

set(test_executable_name runner)

add_executable(runner test-runner.c ${TEST_FILES})

target_link_libraries(runner ${VARNAM_LIBRARY_NAME})

add_test(transliteration ${test_executable_name} basic-transliteration td-01.vst)
add_test(ml-unicode ${test_executable_name} ml-unicode ../vsf/ml-unicode.vst ml-unicode-input.txt)
    
