cmake_minimum_required(VERSION 3.10)
project(Chip8)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CHIP8_TARGET ${PROJECT_NAME})
set(TEST_TARGET Chip8Tests)

add_subdirectory(vendor/raylib)
add_subdirectory(vendor/lua)

add_executable(${CHIP8_TARGET} src/main.c src/chip8.c src/monitor.c src/renderer.c src/chip8.h src/codes.h src/monitor.h src/program.h src/renderer.h)
add_executable(${TEST_TARGET} src/main.c src/chip8.c src/monitor.c src/tests.c)

include_directories(vendor/lua/src)
message(STATUS "C Flags: ${CMAKE_C_FLAGS}")

foreach(FLAG ${BUILD_FLAGS})
    message(STATUS "Adding compile definition: ${FLAG}")
    target_compile_definitions(${CHIP8_TARGET} PRIVATE ${FLAG})
    target_compile_definitions(${TEST_TARGET} PRIVATE ${FLAG})
endforeach()

find_package(X11)
if(X11_FOUND)
    target_include_directories(${CHIP8_TARGET} PRIVATE ${X11_INCLUDE_DIR})
    target_link_libraries(${CHIP8_TARGET} ${X11_LIBRARIES})
endif()

target_compile_definitions(${TEST_TARGET} PRIVATE RUN_TESTS)
target_link_libraries(${CHIP8_TARGET} raylib)

add_custom_command(TARGET ${CHIP8_TARGET} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:${CHIP8_TARGET}>/assets
)
