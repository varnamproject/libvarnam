##
# Copyright (C) Navaneeth.K.N
#
# This is part of libvarnam. See LICENSE.txt for the license
##

cmake_minimum_required (VERSION 2.8)

project (examples)
message ("Generating project ${PROJECT_NAME}")

#set(test_executable_name runner)

add_executable(transliteration transliteration.c)
add_executable(learning learning.c)

target_link_libraries(transliteration ${VARNAM_LIBRARY_NAME})
target_link_libraries(learning ${VARNAM_LIBRARY_NAME})
