#pragma once

#include "core/layers/render/renderer.hpp"
#include "core/resources/mesh.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

inline constexpr const char* defaultVertexShader = R"glsl(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 vNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
  gl_Position = projection * view * model * vec4(aPos, 1.0f);
  vNormal = aNormal;
}
)glsl";

inline constexpr const char* defaultFragmentShader = R"glsl(
#version 330 core
in vec3 vNormal;

out vec4 FragColor;

void main() {
  vec3 L = normalize(vec3(0.6, 1.0, 0.4));
  float light = 0.25 + 0.75 * max(dot(normalize(vNormal), L), 0.0);
  vec4 base = vec4(0.2, 1.0, 0.1, 1.0);
  FragColor = vec4(vNormal.rgb * light, base.a);
}
)glsl";

glm::mat4 toGlm(Matrix4x4F mat) {
  return glm::make_mat4(mat.getData().data());
}

int framebufferWidth = 0;
int framebufferHeight = 0;

struct OpenGLRenderer : Renderer {
  private:
    GLFWwindow* window;

    void processInput() {
      if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
      }
    }

    // shader cache
    // mesh cache
    // material cache


  public:
    ResourcePool* resourcePool;
    CommandBuffer* commandBuffer;

    void init(const WindowSettings settings, ResourcePool *pool) {
      framebufferWidth = settings.width;
      framebufferHeight = settings.height;
      
      glfwInit();
      glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
      glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
      glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
      glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

      window = glfwCreateWindow(settings.width, settings.height, settings.title.c_str(), nullptr, nullptr);
      if (window == NULL) {
        printf("Failed to create GLFW window\n");
        glfwTerminate();
        assert(false && "Could not create window");
      }

      glfwMakeContextCurrent(window);

      // very glad that this does all this stuff for me
      if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        assert(false && "Failed to initialize GLAD");
      }

      // set size of opengl viewport, could make this smaller
      // than the window if i wanted to draw stuff outsize
      // the viewport
      glViewport(0, 0, framebufferWidth, framebufferHeight);

      glfwSetFramebufferSizeCallback(window, [/*&framebufferWidth, &framebufferHeight*/](GLFWwindow* window, int width, int height){
        framebufferWidth = width;
        framebufferHeight = height;
        glViewport(0, 0, framebufferWidth, framebufferHeight);
      });

      setupthings();
    }

    void beginFrame() {
      processInput();
    }

    uint fragmentShader, vertexShader, shaderProgramId;
    uint VBO, VAO, EBO;
    uint numVerts;

    uint compileShaderProgram(const char* vertex, const char* fragment) {
      uint shaderProgram;
      int success;
      char infoLog[512];
    
      vertexShader = glCreateShader(GL_VERTEX_SHADER);
      glShaderSource(vertexShader, 1, &vertex, nullptr);
      glCompileShader(vertexShader);

      glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
      if (!success) {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        printf("VERTEX SHADER COMPILE ERROR: %s\n", infoLog);
      }

      
      fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
      glShaderSource(fragmentShader, 1, &fragment, nullptr);
      glCompileShader(fragmentShader);

      glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
      if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        printf("FRAGMENT SHADER COMPILE ERROR: %s\n", infoLog);
      }

      shaderProgram = glCreateProgram();
      glAttachShader(shaderProgram, vertexShader);
      glAttachShader(shaderProgram, fragmentShader);
      glLinkProgram(shaderProgram);

      glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
      if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        printf("SHADER PROGRAM COMPILE ERROR: %s\n", infoLog);
      }

      glDeleteShader(vertexShader);
      glDeleteShader(fragmentShader);

      printf("Compiled Shader Program\n");

      return shaderProgram;
    }


    uint projectionLocation, viewLocation, modelLocation, albedoLocation;
    void setupthings() {
      glEnable(GL_DEPTH_TEST);
      glEnable(GL_CULL_FACE);
      glCullFace(GL_FRONT);
      glFrontFace(GL_CW);

      shaderProgramId = compileShaderProgram(defaultVertexShader, defaultFragmentShader);

      Handle<Mesh> meshHandle = resourcePool->load<Mesh>("example/models/frog.stl");

      Mesh* mesh = resourcePool->get(meshHandle);

      float scale = 1.f;

      // mesh vertices is a vector of Vector3F, need to convert it into something OpenGL can understand
      std::vector<float> vertices;

      for (uint i = 0; i < mesh->vertices.size(); ++i) {
        vertices.push_back(mesh->vertices[i].x);
        vertices.push_back(mesh->vertices[i].y);
        vertices.push_back(mesh->vertices[i].z);

        vertices.push_back(mesh->normals[i].x);
        vertices.push_back(mesh->normals[i].y);
        vertices.push_back(mesh->normals[i].z);
      }

      numVerts = vertices.size();

      glGenVertexArrays(1, &VAO);
      glGenBuffers(1, &VBO);
      glGenBuffers(1, &EBO);
      
      glBindVertexArray(VAO);

      glBindBuffer(GL_ARRAY_BUFFER, VBO);
      glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indices.size() * sizeof(uint32_t), mesh->indices.data(), GL_STATIC_DRAW);

      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
      glEnableVertexAttribArray(0);
      glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3*sizeof(float)));
      glEnableVertexAttribArray(1);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);

      // wireframe mode
      // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

      projectionLocation = glGetUniformLocation(shaderProgramId, "projection");
      viewLocation = glGetUniformLocation(shaderProgramId, "view");
      modelLocation = glGetUniformLocation(shaderProgramId, "model");

      printf("Completed Setup\n");
    }

    // load mesh, create VAO for it
    void bindMesh(Handle<Mesh> meshHandle) {
      Mesh* mesh = resourcePool->get(meshHandle);
    }

    // load material, compile shaders if needed
    void bindMaterial(Handle<Material> materialHandle) {
      
    }

    float t = 0.0;

    void render() {
      //const std::vector<DrawCommand> &commands
      // before anything else, the render system
      // and render layer need to construct
      // the command buffer, which includes
      // meshes, materials, shaders, lights
      // gui elements
      // if a bunch of models have the same mesh
      // handle and material then they should
      // automatically use instancing

      // we look through all the commands first
      // if there is anything not loaded we load it
      // this include meshes, materials, images, etc
      // then we compile shaders
      // then we render the meshes, we batch things
      // so that it is faster.
      // then we do the defered lighting pass, where
      // things like shadows are rendered.
      // then we do post processing

      /*
      for (auto command : commandBuffer->getCommands()) {
        uint vertexArray;
        if (true) { // if mesh is not already cached
          Mesh* mesh = resourcePool->get(command.mesh);
          vertexArray = 0; // something
        }

        // load these from the mesh?
        vertices = []
      }
      */

      glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

      glUseProgram(shaderProgramId);

      t += 0.25;
      
      glBindVertexArray(VAO);

      glm::mat4 projection = glm::perspective(glm::radians(90.f), (float)framebufferWidth/(float)framebufferHeight, 0.1f, 1000.f);
      glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 15.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

      for (int x = -10; x < 10; x++) {
        for (int y = -10; y < 10; y++) {
          glm::mat4 model = glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(-x * 2, 0.0, -y * 2)), glm::radians(t + x + y), glm::vec3(1.0, 1.0, 1.0));

          glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));
          glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));
          glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
          
          glDrawElements(GL_TRIANGLES, numVerts, GL_UNSIGNED_INT, 0);
        }
      }
      //glBindVertexArray(0);
    }

    void endFrame() {
      glfwSwapBuffers(window);
      glfwPollEvents();
    }

    void shutdown() {
      glDeleteVertexArrays(1, &VAO);
      glDeleteBuffers(1, &VBO);
      glDeleteProgram(shaderProgramId);

      glfwTerminate();
    }

    Vector2I getSize() {
      return {framebufferWidth, framebufferHeight};
    }

    bool shouldClose() {
      return glfwWindowShouldClose(window);
    }
};
