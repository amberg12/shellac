cmake_minimum_required(VERSION 4.1)
project(shellac)

set(CMAKE_CXX_STANDARD 17)

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

set(EXE_NAME "${PROJECT_NAME}-${GIT_BRANCH}-${GIT_COMMIT}")

message(STATUS "Executable name: ${EXE_NAME}")

add_executable(${EXE_NAME} src/main.cpp
        src/defs.h
        src/uci.cpp
        src/uci.h
        src/position.h
        src/position.cpp
        src/position.h
        src/types.h
        src/engine.cpp
        src/engine.h
        src/bitboard.cpp
        src/bitboard.h
        src/movegen.cpp
        src/movegen.h
        src/threadpool.cpp
        src/threadpool.h)

target_compile_definitions(${EXE_NAME} PRIVATE BUILD_IDENTIFIER="${GIT_BRANCH}-${GIT_COMMIT}")