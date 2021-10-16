cmake_minimum_required (VERSION 2.8.11)
project (rudp CXX)

set(CMAKE_CXX_STANDARD 17)

include_directories(src)

add_subdirectory (src)

set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(
                            ${CMAKE_PROJECT_NAME} PUBLIC
                            src)