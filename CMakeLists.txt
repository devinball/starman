cmake_minimum_required(VERSION 3.27)
project(engine VERSION 0.1.0 LANGUAGES CXX)

# ============================================================
# Global settings
# ============================================================
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Suppress verbose find_package output from SDL and other deps
set(CMAKE_MESSAGE_LOG_LEVEL "WARNING" CACHE STRING "" FORCE)

# ============================================================
# CPM bootstrap
# ============================================================
set(CPM_VERSION "0.40.2")
set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_VERSION}.cmake")

if(NOT EXISTS "${CPM_DOWNLOAD_LOCATION}")
    # Temporarily restore normal logging just for this message
    message(STATUS "Downloading CPM.cmake ${CPM_VERSION}...")
    file(DOWNLOAD
        "https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_VERSION}/CPM.cmake"
        "${CPM_DOWNLOAD_LOCATION}"
        # Make sure to double check this hash!
        EXPECTED_HASH SHA256=c8cdc32c03816538ce22781ed72964dc864b2a34a310d3b7104812a5ca2d835d
    )
endif()

include("${CPM_DOWNLOAD_LOCATION}")

if(NOT DEFINED CPM_SOURCE_CACHE)
    set(CPM_SOURCE_CACHE "${CMAKE_SOURCE_DIR}/.cpm-cache"
        CACHE PATH "CPM package source cache")
endif()

# ============================================================
# Dependencies
# ============================================================

# --- SDL3 ---
CPMAddPackage(
    NAME BGFX
    GITHUB_REPOSITORY bkaradzic/bgfx.cmake
    GIT_TAG v1.142.9197-527
)

# --- EnTT ---
CPMAddPackage(
    NAME EnTT
    GITHUB_REPOSITORY skypjack/entt
    GIT_TAG v3.14.0
)

# Restore normal logging for our own project output
set(CMAKE_MESSAGE_LOG_LEVEL "STATUS" CACHE STRING "" FORCE)

# ============================================================
# Engine library
# ============================================================

# Only glob .cpp files — headers are pulled in automatically by the compiler
# via includes. Passing .hpp to add_library causes CMake to try to compile them.
file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/**/*.cpp"
)

# If there are no .cpp files yet, CMake will error on add_library(STATIC).
# This dummy lets the engine target exist before any .cpp files are written.
if(NOT ENGINE_SOURCES)
    set(ENGINE_SOURCES "${CMAKE_BINARY_DIR}/empty.cpp")
    file(WRITE "${CMAKE_BINARY_DIR}/empty.cpp" "// placeholder\n")
endif()

add_library(engine STATIC ${ENGINE_SOURCES})
target_compile_features(engine PUBLIC cxx_std_23)

target_include_directories(engine PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(engine PUBLIC
    EnTT::EnTT
    SDL3::SDL3-static
)

# ============================================================
# Compiler warnings
# ============================================================
if(MSVC)
    target_compile_options(engine PRIVATE /W4 /WX)
else()
    target_compile_options(engine PRIVATE
        -Wall -Wextra -Wpedantic
        -Wno-unused-parameter
    )
endif()

# ============================================================
# Sanitizers (debug only)
# ============================================================
option(ENGINE_SANITIZE "Enable ASan + UBSan in debug builds" OFF)
if(ENGINE_SANITIZE AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(engine PUBLIC -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(engine PUBLIC -fsanitize=address,undefined)
endif()

# ============================================================
# Example executable
# ============================================================
add_executable(example example/main.cpp)
target_link_libraries(example PRIVATE engine)

# Copy compile_commands.json to source root for clangd
add_custom_target(copy_compile_commands ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_BINARY_DIR}/compile_commands.json"
        "${CMAKE_SOURCE_DIR}/compile_commands.json"
    DEPENDS "${CMAKE_BINARY_DIR}/compile_commands.json"
    COMMENT "Copying compile_commands.json to source root"
)