cmake_minimum_required(VERSION 4.1)
project(shellac)

set(CMAKE_CXX_STANDARD 17)

if(EXISTS "${CMAKE_SOURCE_DIR}/.git/HEAD")
    configure_file("${CMAKE_SOURCE_DIR}/.git/HEAD" "${CMAKE_BINARY_DIR}/git-head-hash" COPYONLY)
endif()

execute_process(
        COMMAND git rev-parse --abbrev-ref HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_BRANCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
        COMMAND git rev-parse --short HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_COMMIT
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(OUTPUT_NAME "${PROJECT_NAME}-${GIT_BRANCH}-${GIT_COMMIT}")
message(STATUS "Executable filename will be: ${OUTPUT_NAME}")

add_executable(shellac
        src/main.cpp
        src/defs.h
        src/uci.cpp
        src/uci.h
        src/position.h
        src/position.cpp
        src/types.h
        src/engine.cpp
        src/engine.h
        src/bitboard.cpp
        src/bitboard.h
        src/movegen.cpp
        src/movegen.h
        src/threadpool.cpp
        src/threadpool.h
        src/search.cpp
        src/search.h
        src/evaluate.cpp
)

set_target_properties(shellac PROPERTIES OUTPUT_NAME ${OUTPUT_NAME})

target_compile_definitions(shellac PRIVATE BUILD_IDENTIFIER="${GIT_BRANCH}-${GIT_COMMIT}")