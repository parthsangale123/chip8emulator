cmake_minimum_required(VERSION 3.16)
project(Chip8Emulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# ---------------------------------------------------------------------------
# raylib dependency
# ---------------------------------------------------------------------------
include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)

find_package(raylib 5.0 QUIET)

if(NOT raylib_FOUND)
    FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG        5.5
        GIT_SHALLOW    TRUE
    )

    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(raylib)
endif()

# ---------------------------------------------------------------------------
# Emulator target
# ---------------------------------------------------------------------------
file(GLOB_RECURSE CHIP8_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp")

add_executable(Chip8Emulator ${CHIP8_SOURCES})

target_include_directories(Chip8Emulator PRIVATE
    "${CMAKE_SOURCE_DIR}/include"
)

target_compile_options(Chip8Emulator PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

target_link_libraries(Chip8Emulator PRIVATE raylib)

if(APPLE)
    target_link_libraries(Chip8Emulator PRIVATE
        "-framework IOKit" "-framework Cocoa" "-framework OpenGL"
    )
elseif(UNIX)
    target_link_libraries(Chip8Emulator PRIVATE m pthread dl)
elseif(WIN32)
    target_link_libraries(Chip8Emulator PRIVATE winmm gdi32)
endif()

set_target_properties(Chip8Emulator PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Copy the bundled ROMs next to the built executable for convenience.
add_custom_command(TARGET Chip8Emulator POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/roms"
        "$<TARGET_FILE_DIR:Chip8Emulator>/roms"
)
