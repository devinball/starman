cmake_minimum_required(VERSION 3.27)
project(engine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Corrade/Magnum (v2020.06) predate CMake's modern policy minimums.
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

find_package(PkgConfig REQUIRED)

# ---------------------------------------------------------------------
# Nix-provided dependencies
#
# These are expected to be in your shell.nix / flake.nix buildInputs.
# Nix's cmake setup hook adds each package's store path to
# CMAKE_PREFIX_PATH automatically, so find_package should resolve them
# as long as this is configured from inside nix-shell / nix develop.
# ---------------------------------------------------------------------

find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(GLEW REQUIRED)

find_package(entt CONFIG QUIET)
if(NOT entt_FOUND)
    pkg_check_modules(entt REQUIRED IMPORTED_TARGET entt)
    add_library(entt::entt ALIAS PkgConfig::entt)
endif()

find_package(yaml-cpp CONFIG QUIET)
if(NOT yaml-cpp_FOUND)
    pkg_check_modules(yaml-cpp REQUIRED IMPORTED_TARGET yaml-cpp)
    add_library(yaml-cpp::yaml-cpp ALIAS PkgConfig::yaml-cpp)
endif()

find_package(assimp CONFIG QUIET)
if(NOT assimp_FOUND)
    pkg_check_modules(ASSIMP REQUIRED IMPORTED_TARGET assimp)
    add_library(assimp::assimp ALIAS PkgConfig::ASSIMP)
endif()

# imgui: nixpkgs' imgui derivation only bundles the backends it was
# built with. You need to have overridden it to include glfw+opengl3,
# e.g. in your nix expression:
#   imgui.override { targets = [ "opengl3" ]; platforms = [ "glfw" ]; }
# Without that override, this find_package call succeeds but the
# imgui_impl_glfw.h / imgui_impl_opengl3.h backend headers won't exist.
find_package(imgui REQUIRED)

# stb: nixpkgs' stb package ships headers only, with no CMake config
# file — so it's located manually rather than via find_package.
find_path(STB_INCLUDE_DIR stb_image.h)
if(NOT STB_INCLUDE_DIR)
    message(FATAL_ERROR "stb headers not found — check that 'stb' is in your Nix buildInputs")
endif()
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE "${STB_INCLUDE_DIR}")

# ---------------------------------------------------------------------
# CPM-fetched dependencies (not provided by Nix)
# ---------------------------------------------------------------------

set(CPM_DOWNLOAD_VERSION 0.38.2)
if(NOT EXISTS "${CMAKE_BINARY_DIR}/cmake/CPM.cmake")
    file(DOWNLOAD
        https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake
        ${CMAKE_BINARY_DIR}/cmake/CPM.cmake)
endif()
include(${CMAKE_BINARY_DIR}/cmake/CPM.cmake)

# Must be set before CPMAddPackage(reflectcpp) to actually take effect
set(REFLECTCPP_YAML ON)
CPMAddPackage(
    NAME reflectcpp
    GITHUB_REPOSITORY getml/reflect-cpp
    GIT_TAG v0.25.0)

# --- Corrade (Magnum's base/support library) ---
set(CORRADE_WITH_INTERCONNECT OFF CACHE BOOL "" FORCE)
set(CORRADE_BUILD_TESTS       OFF CACHE BOOL "" FORCE)

CPMAddPackage(
    NAME corrade
    GITHUB_REPOSITORY mosra/corrade
    GIT_TAG v2020.06)

if(corrade_ADDED)
    list(APPEND CMAKE_MODULE_PATH "${corrade_SOURCE_DIR}/modules")
endif()

# --- Magnum (graphics engine) ---
# Forced (not plain OPTIONS) so these can't be silently ignored by a
# stale cache entry from an earlier configure.
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

# ---------------------------------------------------------------------
# Engine library
# ---------------------------------------------------------------------

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
    glfw
    glm::glm
    GLEW::GLEW
    assimp::assimp
    EnTT::EnTT
    yaml-cpp::yaml-cpp
    Boost::boost
    stb
    imgui::imgui
    reflectcpp
    Magnum::Magnum
    Magnum::GL
    Magnum::GlfwApplication
    Magnum::Trade
    Magnum::MeshTools
    Magnum::SceneGraph
    Magnum::Shaders)

# --- Example executable ---
add_executable(example example/main.cpp)
target_link_libraries(example PRIVATE engine)

# --- Copy compile_commands.json to project root for editor tooling ---
add_custom_target(copy_compile_commands ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_BINARY_DIR}/compile_commands.json"
        "${CMAKE_SOURCE_DIR}/compile_commands.json"
    DEPENDS "${CMAKE_BINARY_DIR}/compile_commands.json"
    COMMENT "Copying compile_commands.json to project root")
    