cmake_minimum_required(VERSION 3.27)
project(engine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CPM_DOWNLOAD_VERSION 0.38.2)
if(NOT EXISTS "${CMAKE_BINARY_DIR}/cmake/CPM.cmake")
    file(DOWNLOAD
        https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake
        ${CMAKE_BINARY_DIR}/cmake/CPM.cmake
    )
endif()
include(${CMAKE_BINARY_DIR}/cmake/CPM.cmake)

CPMAddPackage(
  NAME EnTT
  GITHUB_REPOSITORY skypjack/entt
  GIT_TAG v3.11.0
  VERSION 3.11.0
)

# DiligentCore is the actual library repo — the DiligentEngine repo is just
# an umbrella of submodules and doesn't wire up CPM targets properly.
CPMAddPackage(
    NAME DiligentCore
    GITHUB_REPOSITORY DiligentGraphics/DiligentCore
    GIT_TAG v2.5.6
    OPTIONS
        "DILIGENT_BUILD_TESTS OFF"
        "DILIGENT_NO_GLSLANG OFF"   # set ON to skip GLSL->SPIRV compilation if you don't need it
)

file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/**/*.cpp"
)

if(NOT ENGINE_SOURCES)
    set(ENGINE_SOURCES "${CMAKE_BINARY_DIR}/empty.cpp")
    file(WRITE "${CMAKE_BINARY_DIR}/empty.cpp" "// placeholder\n")
endif()

add_library(engine STATIC ${ENGINE_SOURCES})
target_compile_features(engine PUBLIC cxx_std_23)

target_include_directories(engine PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

# Link against DiligentCore's actual exported targets.
# Diligent-GraphicsEngine is the main interface target.
# Add platform-specific backends as needed.
target_link_libraries(engine PUBLIC
    xcb
    EnTT::EnTT
    Diligent-GraphicsEngine
    Diligent-GraphicsEngineVk-shared   # or -static; swap for Vulkan/D3D11/D3D12/Metal
)

target_include_directories(engine PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${DiligentCore_SOURCE_DIR}"   # <-- this one line is all you need
)

add_executable(example example/main.cpp)
target_link_libraries(example PRIVATE engine)

add_custom_target(copy_compile_commands ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_BINARY_DIR}/compile_commands.json"
        "${CMAKE_SOURCE_DIR}/compile_commands.json"
    DEPENDS "${CMAKE_BINARY_DIR}/compile_commands.json"
    COMMENT "Copying compile_commands.json to source root"
)