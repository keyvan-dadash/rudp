set(BINARY ${CMAKE_PROJECT_NAME})

file(GLOB_RECURSE SOURCES LIST_DIRECTORIES true *.hpp *.cpp)

find_package(Threads REQUIRED)

set(SOURCES ${SOURCES})

add_executable(${BINARY} ${SOURCES})

target_compile_options(${BINARY} PRIVATE -Wall -Wextra -Wpedantic -g)

target_link_libraries(${BINARY} Threads::Threads)
