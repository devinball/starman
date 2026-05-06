#pragma once

#include "core/renderer.hpp"
#include "core/resources/mesh.hpp"
#include "core/math/utilities.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

inline constexpr const char* defaultVertexShader = R"glsl(
#version 330 core
layout (location = 0) in vec3 aVertex;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUv;
layout (location = 3) in mat4 aTransform;

out vec3 vNormal;

void main() {
  gl_Position = aTransform * vec4(aVertex, 1.0f);
  vNormal = normalize(mat3(aTransform) * aNormal);;
}
)glsl";

inline constexpr const char* defaultFragmentShader = R"glsl(
#version 330 core
in vec3 vNormal;

out vec4 fragColor;

void main() {
  float ambient = 0.1;
  vec3  L       = normalize(vec3(0.6, 1.0, 0.4));
  float diffuse = max(dot(normalize(vNormal), L), 0.0);
  float light   = ambient + (1 - ambient) * diffuse;
  vec4  base    = vec4(1.0, 1.0, 1.0, 1.0);
  fragColor     = vec4(base.rgb * light, base.a);

  // fragColor = vec4(vNormal.rgb, 1.0);
}
)glsl";

inline constexpr const char* defaultShader = R"glsl(

)glsl";
        
    

glm::mat4 toGlm(Matrix4x4F& mat) {
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

    uint maxInstances = 10000;

    struct GPUMesh {
      uint VBO, VAO, EBO, numVerts, instanceVBO;
    };

    // handle id -> GPUMesh  TODO: i should really just write a hash for handle
    std::unordered_map<std::string, GPUMesh> meshCache;

    // shader cache
    // mesh cache
    // material cache

  public:
    uint shaderProgramId;

    void init(const WindowSettings settings) {
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

      glfwSwapInterval(0);

      glEnable(GL_DEPTH_TEST);
      glEnable(GL_CULL_FACE);
      glCullFace(GL_BACK);
      glFrontFace(GL_CCW);

      // wireframe mode
      // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

      shaderProgramId = compileShaderProgram(defaultVertexShader, defaultFragmentShader);

      printf("Completed Setup\n");
    }

    void beginFrame() {
      processInput();
    }

    uint compileShaderProgram(const char* vertex, const char* fragment) {
      uint vertexShader, fragmentShader, shaderProgram;
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

    // load material, compile shaders if needed
    void bindMaterial(Handle<Material> materialHandle) {
      
    }

    void bindMesh(Handle<Mesh> meshHandle) {
      if (meshCache.count(meshHandle.getId()) == 0) {
        Mesh* mesh = resourcePool->get(meshHandle);

        uint VAO, VBO, EBO;

        std::vector<float> vertices;
        for (uint i = 0; i < mesh->vertices.size(); ++i) {
          vertices.push_back(mesh->vertices[i].x);
          vertices.push_back(mesh->vertices[i].y);
          vertices.push_back(mesh->vertices[i].z);

          vertices.push_back(mesh->normals[i].x);
          vertices.push_back(mesh->normals[i].y);
          vertices.push_back(mesh->normals[i].z);

          vertices.push_back(mesh->uvs[i].x);
          vertices.push_back(mesh->uvs[i].y);
        }

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        
        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indices.size() * sizeof(uint32_t), mesh->indices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3*sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6*sizeof(float)));
        glEnableVertexAttribArray(2);

        uint instanceVBO;
        glGenBuffers(1, &instanceVBO);

        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * maxInstances, nullptr, GL_DYNAMIC_DRAW);

        for (int i = 0; i < 4; ++i) {
          glEnableVertexAttribArray(3 + i);
          glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(i * sizeof(glm::vec4)));
          glVertexAttribDivisor(3 + i, 1);
        }

        // TODO, make sure numVerts should actually be indices and not vertices size
        meshCache[meshHandle.getId()] = GPUMesh{VBO, VAO, EBO, (uint)mesh->indices.size(), instanceVBO};

        // TODO: if shader is the same as the previous one don't rebind
        // TODO: this should really be in render() with the check
        glUseProgram(shaderProgramId);

        printf("Loaded mesh to gpu\n");
      }
    }

    void render() {
      // const std::vector<DrawCommand> &commands
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

      glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

      // compute these matrices only once, if i wanted to i could even cache these in between frames
      // and only update when i know something has changed, but i doubt that would make too much of a
      // difference anyways.
      glm::mat4 projection = glm::perspective(glm::radians(45.f), (float)framebufferWidth/(float)framebufferHeight, 0.1f, 1000.f);
      glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 25.0f, 0.1f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

      // V_clip = M_proj * M_view * M_model * V_local

      sceneGraph->eachModel(identityMatrix4x4F(), [&](Handle<Mesh> meshHandle, Handle<Material> materialHandle, std::vector<Matrix4x4F>& transforms) {
        int numInstances = transforms.size();

        assert(numInstances <= maxInstances);

        // TODO: sort array such that the same meshes/materials are next to each other
        // TODO: only bind if mesh needs to be changed
        // TODO: this should be good now, given the nature of how the scene graph works
        bindMesh(meshHandle);

        GPUMesh& gpuMesh = meshCache[meshHandle.getId()];
        
        std::vector<glm::mat4> glTransforms;
        for (int i = 0; i < numInstances; ++i) {
          glTransforms.push_back(projection * view * toGlm(transforms[i]));
        }

        glBindBuffer(GL_ARRAY_BUFFER, gpuMesh.instanceVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::mat4) * numInstances, glTransforms.data());
        
        glBindVertexArray(gpuMesh.VAO);

        glDrawElementsInstanced(GL_TRIANGLES, gpuMesh.numVerts, GL_UNSIGNED_INT, (void*)0, numInstances);
      });
    }

    void endFrame() {
      glfwSwapBuffers(window);
      glfwPollEvents();
    }

    void shutdown() {
      for (auto& [id, gpuMesh] : meshCache) {
        glDeleteVertexArrays(1, &(gpuMesh.VAO));
        glDeleteBuffers(1, &(gpuMesh.VBO));
      }

      glDeleteProgram(shaderProgramId);

      glfwTerminate();
    }

    Vector2I getSize() {
      return {framebufferWidth, framebufferHeight};
    }

    bool shouldClose() {
      return glfwWindowShouldClose(window);
    }

    void setRenderMode(RenderMode renderMode) {};
};
