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

message(STATUS "C Flags: ${CMAKE_C_FLAGS}")

file(GLOB SOURCES "src/*.c")

add_executable(Chip8 ${SOURCES})
add_executable(Chip8Tests src/main.c src/chip8.c src/monitor.c src/tests.c)

target_link_libraries(Chip8 raylib)
target_compile_definitions(Chip8Tests PRIVATE RUN_TESTS)

add_custom_command(TARGET Chip8 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:Chip8>/assets
)
