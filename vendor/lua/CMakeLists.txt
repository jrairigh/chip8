cmake_minimum_required(VERSION 3.10)
project(lua)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/StaticLibs)

file(GLOB SOURCES "src/l*.[hc]")

add_library(${PROJECT_NAME} STATIC ${SOURCES})
add_executable(luac src/compiler.c ${SOURCES})
add_executable(luarepl src/repl.c ${SOURCES})
