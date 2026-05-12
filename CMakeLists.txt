cmake_minimum_required(VERSION 3.27)
project(engine VERSION 0.1.0 LANGUAGES CXX)

# =============================================================================
# Global compiler setup
# =============================================================================
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# =============================================================================
# CPM.cmake – package manager
# =============================================================================
set(CPM_DOWNLOAD_VERSION 0.38.2)

if(NOT EXISTS "${CMAKE_BINARY_DIR}/cmake/CPM.cmake")
    file(DOWNLOAD
        https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake
        ${CMAKE_BINARY_DIR}/cmake/CPM.cmake)
endif()

include(${CMAKE_BINARY_DIR}/cmake/CPM.cmake)

# =============================================================================
# Python interpreter (required for GLAD code generation)
# =============================================================================
find_package(Python3 REQUIRED COMPONENTS Interpreter)

# =============================================================================
# GLAD – OpenGL loader generation
# =============================================================================
set(GLAD_ROOT        "${CMAKE_CURRENT_BINARY_DIR}/glad")
set(GLAD_INCLUDE_DIR "${GLAD_ROOT}/include")
set(GLAD_SRC_DIR     "${GLAD_ROOT}/src")

add_custom_command(
    OUTPUT
        "${GLAD_INCLUDE_DIR}/glad/glad.h"
        "${GLAD_INCLUDE_DIR}/KHR/khrplatform.h"
        "${GLAD_SRC_DIR}/glad.c"
    COMMAND
        ${Python3_EXECUTABLE} -m glad
            --api gl=3.3
            --profile core
            --spec gl
            --generator c
            --out-path "${GLAD_ROOT}"
    COMMENT "Generating GLAD (OpenGL 3.3 Core)"
    VERBATIM)

add_custom_target(glad_generate ALL
    DEPENDS
        "${GLAD_INCLUDE_DIR}/glad/glad.h"
        "${GLAD_INCLUDE_DIR}/KHR/khrplatform.h"
        "${GLAD_SRC_DIR}/glad.c")

add_library(glad INTERFACE)
target_include_directories(glad INTERFACE "${GLAD_INCLUDE_DIR}")
add_dependencies(glad glad_generate)

# =============================================================================
# External dependencies (via CPM)
# =============================================================================
CPMAddPackage(
    NAME Boost
    URL "https://github.com/boostorg/boost/releases/download/boost-1.89.0/boost-1.89.0-cmake.tar.gz"
    OPTIONS
        "BOOST_ENABLE_CMAKE ON"
        "BOOST_INCLUDE_LIBRARIES multiprecision")

CPMAddPackage(
    NAME EnTT
    GITHUB_REPOSITORY skypjack/entt
    GIT_TAG v3.11.0
    VERSION 3.11.0)

CPMAddPackage(
    NAME assimp
    GITHUB_REPOSITORY assimp/assimp
    GIT_TAG v6.0.4
    OPTIONS
        "BUILD_SHARED_LIBS OFF"
        "ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF")

CPMAddPackage(
  NAME yaml-cpp
  GITHUB_REPOSITORY jbeder/yaml-cpp
  GIT_TAG 0.9.0
)

# =============================================================================
# Engine library sources
# =============================================================================
file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/engine/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/engine/src/**/*.cpp")

if(NOT ENGINE_SOURCES)
    set(ENGINE_SOURCES "${CMAKE_BINARY_DIR}/empty.cpp")
    file(WRITE "${CMAKE_BINARY_DIR}/empty.cpp" "// placeholder\n")
endif()

# =============================================================================
# Engine static library
# =============================================================================
add_library(engine STATIC ${ENGINE_SOURCES})

target_compile_features(engine PUBLIC cxx_std_23)

target_include_directories(engine PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src")

target_sources(engine PRIVATE "${GLAD_SRC_DIR}/glad.c")

# Compile glad.c as C++ to avoid C compiler issues in Nix
set_source_files_properties("${GLAD_SRC_DIR}/glad.c" PROPERTIES LANGUAGE CXX)

# Ensure GLAD is generated before engine compilation
add_dependencies(engine glad_generate)

find_package(glfw3 REQUIRED)

target_link_libraries(engine PUBLIC
    glad
    glfw
    Boost::multiprecision
    EnTT::EnTT
    #yaml-cpp::yaml-cpp
    assimp
)

# =============================================================================
# Example executable
# =============================================================================
add_executable(example example/main.cpp)
target_link_libraries(example PRIVATE engine)

# =============================================================================
# Developer convenience: copy compile_commands.json to project root
# =============================================================================
add_custom_target(copy_compile_commands ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_BINARY_DIR}/compile_commands.json"
        "${CMAKE_SOURCE_DIR}/compile_commands.json"
    DEPENDS "${CMAKE_BINARY_DIR}/compile_commands.json"
    COMMENT "Copying compile_commands.json to project root")
