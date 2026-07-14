cmake_minimum_required(VERSION 3.27)
project(engine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

find_package(PkgConfig REQUIRED)

find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(GLEW REQUIRED)
find_package(EnTT REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(assimp REQUIRED)

# imgui: nixpkgs' imgui derivation only bundles the backends it was
# built with. You need to have overridden it to include glfw+opengl3,
# e.g. in your nix expression:
#   imgui.override { targets = [ "opengl3" ]; platforms = [ "glfw" ]; }
# Without that override, this find_package call succeeds but the
# imgui_impl_glfw.h / imgui_impl_opengl3.h backend headers won't exist.
find_package(imgui REQUIRED)

set(CPM_DOWNLOAD_VERSION 0.38.2)
if(NOT EXISTS "${CMAKE_BINARY_DIR}/cmake/CPM.cmake")
    file(DOWNLOAD
        https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake
        ${CMAKE_BINARY_DIR}/cmake/CPM.cmake)
endif()
include(${CMAKE_BINARY_DIR}/cmake/CPM.cmake)


#[[
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
]]

CPMAddPackage(
    NAME stb
    GITHUB_REPOSITORY nothings/stb
    GIT_TAG master
    DOWNLOAD_ONLY YES)

if(stb_ADDED)
    add_library(stb INTERFACE)
    target_include_directories(stb INTERFACE "${stb_SOURCE_DIR}")
endif()
set(REFLECTCPP_YAML ON)
CPMAddPackage(
    NAME reflectcpp
    GITHUB_REPOSITORY getml/reflect-cpp
    GIT_TAG v0.25.0)

set(CORRADE_WITH_INTERCONNECT OFF CACHE BOOL "" FORCE)
set(CORRADE_BUILD_TESTS       OFF CACHE BOOL "" FORCE)

CPMAddPackage(
    NAME corrade
    GITHUB_REPOSITORY mosra/corrade
    GIT_TAG v2020.06)

if(corrade_ADDED)
    list(APPEND CMAKE_MODULE_PATH "${corrade_SOURCE_DIR}/modules")
endif()

set(MAGNUM_WITH_SDL2APPLICATION          OFF CACHE BOOL "" FORCE)
set(MAGNUM_WITH_GLFWAPPLICATION          ON  CACHE BOOL "" FORCE)
set(MAGNUM_WITH_WINDOWLESSEGLAPPLICATION OFF CACHE BOOL "" FORCE)
set(MAGNUM_WITH_TRADE                    ON  CACHE BOOL "" FORCE)
set(MAGNUM_WITH_MESHTOOLS                ON  CACHE BOOL "" FORCE)
set(MAGNUM_WITH_SCENEGRAPH               ON  CACHE BOOL "" FORCE)
set(MAGNUM_WITH_SHADERS                  ON  CACHE BOOL "" FORCE)
set(MAGNUM_WITH_AUDIO                    OFF CACHE BOOL "" FORCE)
set(MAGNUM_WITH_VK                       OFF CACHE BOOL "" FORCE)
set(MAGNUM_BUILD_TESTS                   OFF CACHE BOOL "" FORCE)
set(MAGNUM_BUILD_STATIC                  ON  CACHE BOOL "" FORCE)

CPMAddPackage(
    NAME magnum
    GITHUB_REPOSITORY mosra/magnum
    GIT_TAG v2020.06)

if(magnum_ADDED)
    list(APPEND CMAKE_MODULE_PATH "${magnum_SOURCE_DIR}/modules")
endif()

file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/engine/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/engine/src/**/*.cpp")

if(NOT ENGINE_SOURCES)
    set(ENGINE_SOURCES "${CMAKE_BINARY_DIR}/empty.cpp")
    file(WRITE "${CMAKE_BINARY_DIR}/empty.cpp" "// placeholder\n")
endif()

add_library(engine STATIC ${ENGINE_SOURCES})
target_compile_features(engine PUBLIC cxx_std_23)
target_include_directories(engine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")

target_link_libraries(engine PUBLIC
    #glad
    glfw
    glm::glm
    GLEW::GLEW
    assimp::assimp
    EnTT::EnTT
    yaml-cpp::yaml-cpp
    stb
    imgui::imgui
    reflectcpp
    Magnum::Magnum
    Magnum::GL
    Magnum::Trade
    Magnum::MeshTools
    Magnum::SceneGraph
    Magnum::Shaders
    Corrade::Utility
    Corrade::Containers)

add_executable(example example/main.cpp)
target_link_libraries(example PRIVATE engine)

add_custom_target(copy_compile_commands ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_BINARY_DIR}/compile_commands.json"
        "${CMAKE_SOURCE_DIR}/compile_commands.json"
    DEPENDS "${CMAKE_BINARY_DIR}/compile_commands.json"
    COMMENT "Copying compile_commands.json to project root")
    