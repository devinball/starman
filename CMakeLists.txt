cmake_minimum_required(VERSION 3.27)
project(engine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

find_package(PkgConfig REQUIRED)


set(CPM_DOWNLOAD_VERSION 0.38.2)
if(NOT EXISTS "${CMAKE_BINARY_DIR}/cmake/CPM.cmake")
    file(DOWNLOAD
        https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake
        ${CMAKE_BINARY_DIR}/cmake/CPM.cmake)
endif()
include(${CMAKE_BINARY_DIR}/cmake/CPM.cmake)


find_package(Python3 REQUIRED COMPONENTS Interpreter)
find_package(glfw3 REQUIRED)

# --- GLAD (OpenGL 3.3 core loader, generated at configure time) ---
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


find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(GLEW REQUIRED)
find_package(EnTT REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(assimp REQUIRED)
#find_package(imgui REQUIRED)


set(Boost_NO_BOOST_CMAKE ON)
find_package(Boost REQUIRED)


CPMAddPackage(
    NAME stb
    GITHUB_REPOSITORY nothings/stb
    GIT_TAG master
    DOWNLOAD_ONLY YES)

if(stb_ADDED)
    add_library(stb INTERFACE)
    target_include_directories(stb INTERFACE "${stb_SOURCE_DIR}")
endif()


CPMAddPackage(
    NAME imgui
    GITHUB_REPOSITORY ocornut/imgui
    GIT_TAG v1.92.8)

add_library(imgui_lib STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp)
target_include_directories(imgui_lib PUBLIC ${imgui_SOURCE_DIR})
target_compile_features(imgui_lib PUBLIC cxx_std_23)

add_library(imgui_backend STATIC
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp)
target_include_directories(imgui_backend PUBLIC ${imgui_SOURCE_DIR})
target_link_libraries(imgui_backend PUBLIC imgui_lib glfw)

add_library(imgui ALIAS imgui_backend)

# Must be set before CPMAddPackage(reflectcpp) to actually take effect
set(REFLECTCPP_YAML ON)
CPMAddPackage(
    NAME reflectcpp
    GITHUB_REPOSITORY getml/reflect-cpp
    GIT_TAG v0.25.0)


# --- Engine library ---
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

target_sources(engine PRIVATE "${GLAD_SRC_DIR}/glad.c")
set_source_files_properties("${GLAD_SRC_DIR}/glad.c" PROPERTIES LANGUAGE CXX)
add_dependencies(engine glad_generate)

target_link_libraries(engine PUBLIC
    glad
    glfw
    Boost::boost
    EnTT::EnTT
    reflectcpp
    yaml-cpp
    assimp
    stb
    imgui)

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