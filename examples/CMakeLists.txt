cmake_minimum_required (VERSION 2.6)

project (examples)
message ("Generating project ${PROJECT_NAME}")

#set(test_executable_name runner)

add_executable(transliteration transliteration_example.c)
add_executable(learning learning_example.c)

target_link_libraries(transliteration ${VARNAM_LIBRARY_NAME})
target_link_libraries(learning ${VARNAM_LIBRARY_NAME})


