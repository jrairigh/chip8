cmake_minimum_required(VERSION 3.10)
project(Chip8)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY
    https://github.com/raysan5/raylib.git
    GIT_TAG
    5.0
)

FetchContent_MakeAvailable(raylib)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  message(STATUS "Building in Debug mode")
  add_definitions(-DDEBUG)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
  message(STATUS "Building in Release mode")
  add_definitions(-DNDEBUG)
endif()

file(GLOB SOURCES "src/*.c")

add_executable(Chip8 ${SOURCES})

target_link_libraries(Chip8 raylib)
