#pragma once

#include "rendering/renderer.hpp"

#include <Corrade/Containers/Optional.h>
#include <Magnum/GL/Buffer.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/Math/Color.h>
#include <Magnum/Math/Vector2.h>
#include <Magnum/Platform/GLContext.h>
#include <Magnum/Shaders/VertexColor.h>

#include <GLFW/glfw3.h>

struct MagnumRenderer : Renderer {
  GLFWwindow* window;

  Corrade::Containers::Optional<Magnum::Platform::GLContext> _context;
  Corrade::Containers::Optional<Magnum::GL::Mesh> _mesh;
  Corrade::Containers::Optional<Magnum::Shaders::VertexColor2D> _shader;


  void init(const WindowSettings settings) {
    glfwSetErrorCallback([](int code, const char* description){
        printf("Failed to create GLFW window: %d\n %s\n", code, description);
    });

    glfwInit();

    printf("Platform: %d\n", glfwGetPlatform());

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window = glfwCreateWindow(settings.width, settings.height, settings.title.c_str(), nullptr, nullptr);
    if (window == NULL) {
        glfwTerminate();
        assert(false && "Could not create window");
    }

    glfwMakeContextCurrent(window);

    // 0 for vsync off 1 for vsync on
    glfwSwapInterval(0);

    Magnum::GL::Renderer::setClearColor(Magnum::Color3{0x1f / 255.0f, 0x1f / 255.0f, 0x1f / 255.0f});

    setupTriangle();

  }

  void setupTriangle() {
    struct Vertex {
        Magnum::Vector2 position;
        Magnum::Color3 color;
    };
 
    const Vertex data[]{
        {{-0.5f, -0.5f}, Magnum::Color3{1.0f, 0.0f, 0.0f}}, // bottom-left, red
        {{ 0.5f, -0.5f}, Magnum::Color3{0.0f, 1.0f, 0.0f}}, // bottom-right, green
        {{ 0.0f,  0.5f}, Magnum::Color3{0.0f, 0.0f, 1.0f}}  // top, blue
    };
 
    Magnum::GL::Buffer buffer;
    buffer.setData(data);
 
    _mesh.emplace();
    _mesh->setCount(3)
          .addVertexBuffer(std::move(buffer), 0,
              Magnum::Shaders::VertexColor2D::Position{},
              Magnum::Shaders::VertexColor2D::Color3{});
 
    _shader.emplace();
  }

  void shutdown() {

  }

  void beginFrame() {

  }

  void render() {

  }
  
  void endFrame() {

  };

  Vector2I getSize() {

  }

  bool shouldClose() {

  }

  void setRenderMode(RenderMode renderMode) {

  }

  void submitCamera(bool doClear, int id, int priority, float fov, float near, float far, Color clearColor, Vector3N position, QuaternionF rotation) {}
  void submitModel(Handle<Mesh> meshHandle, Handle<Material> materialHandle, Vector3N position, Vector3F scale, QuaternionF rotation) {}
  void submitPointLight(float intensity, float range, Vector3N pos, Color color) {}
};
