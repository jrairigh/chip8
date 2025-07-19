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

if(CMAKE_GENERATOR STREQUAL "Visual Studio 16 2019" OR CMAKE_GENERATOR STREQUAL "Visual Studio 17 2022")
    set_target_properties(Chip8 PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/assets/rom")
endif()

add_custom_command(TARGET Chip8 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:Chip8>/assets
)
