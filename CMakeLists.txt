cmake_minimum_required(VERSION 3.10)
project(Chip8)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_subdirectory(vendor/raylib)

file(GLOB SOURCES "src/*")
add_executable(Chip8 ${SOURCES})
add_executable(Chip8Tests src/main.c src/chip8.c src/monitor.c src/tests.c)

message(STATUS "C Flags: ${CMAKE_C_FLAGS}")

foreach(FLAG ${BUILD_FLAGS})
    message(STATUS "Adding compile definition: ${FLAG}")
    target_compile_definitions(Chip8 PRIVATE ${FLAG})
    target_compile_definitions(Chip8Tests PRIVATE ${FLAG})
endforeach()

target_compile_definitions(Chip8Tests PRIVATE RUN_TESTS)
target_link_libraries(Chip8 raylib)

add_custom_command(TARGET Chip8 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:Chip8>/assets
)
