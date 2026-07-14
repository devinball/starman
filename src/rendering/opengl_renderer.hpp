#pragma once

#include "rendering/renderer.hpp"
#include "resources/mesh.hpp"
#include "math/utilities.hpp"
#include "resources/shader.hpp"
#include "resources/image.hpp"
#include "resources/material.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>    

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

glm::mat4 toGlm(Matrix4x4F& mat) {
  return glm::make_mat4(mat.getData().data());
}

glm::mat4 toGlm(Matrix4x4& mat) {
  std::vector<float> floatVector(mat.getData().begin(), mat.getData().end());
  return glm::make_mat4(floatVector.data());
}

glm::mat4 createProjMatrix(float fov, float aspectRatio, float near, float far) {
  return glm::perspective(glm::radians(fov), aspectRatio, near, far);
}

glm::mat4 createTransformMatrix(const Vector3D& position, const Vector3F& scale, const QuaternionF& rotation) {
  glm::quat q(rotation.r, rotation.i, rotation.j, rotation.k);
  glm::mat4 rotationMatrix = glm::mat4_cast(q);
  glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(scale.x, scale.y, scale.z));
  glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, position.z));

  return translationMatrix * rotationMatrix * scaleMatrix;
}

glm::mat4 createViewMatrix(const QuaternionF& rotation) {
  glm::quat q(rotation.r, rotation.i, rotation.j, rotation.k);
  glm::mat4 cameraTransform = glm::mat4(1.0f);
  cameraTransform *= glm::mat4_cast(q);

  return glm::inverse(cameraTransform);
}

int framebufferWidth = 0;
int framebufferHeight = 0;

struct OpenGLRenderer : Renderer {
  private:
    GLFWwindow* window;

    std::shared_ptr<ResourceManager> resourceManager;
    std::shared_ptr<SceneGraph> sceneGraph;
    std::shared_ptr<InputBuffer> inputBuffer;

    void processInput() {
      glfwPollEvents();

      if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
      }

      inputBuffer->forward = glfwGetKey(window, GLFW_KEY_W);
      inputBuffer->backward = glfwGetKey(window, GLFW_KEY_S);
      inputBuffer->left = glfwGetKey(window, GLFW_KEY_A);
      inputBuffer->right = glfwGetKey(window, GLFW_KEY_D);
      inputBuffer->up = glfwGetKey(window, GLFW_KEY_SPACE);
      inputBuffer->down = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT);

      if(glfwGetKey(window, GLFW_KEY_ENTER)) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
      }
    }

    struct GPUMesh {
      uint VBO, VAO, EBO, numVerts, instanceVBO;
    };

    struct GPUTexture {
      uint glTex;
    };

    struct GPUShader {
      uint id;
      uint aVpLoc;
    };

    struct GPUMaterial {
      std::vector<uint> uniformLocations;
    };

    struct GPULight {
      Vector4F posIntensity; // x, y, z | intensity
      Vector4F colorRange; // r, g, b | range
    };

    std::unordered_map<ResourceID, GPUMesh> meshCache;
    std::unordered_map<ResourceID, GPUTexture> textureCache;
    std::unordered_map<ResourceID, GPUShader> shaderCache;
  public:
    void init(const WindowSettings settings) {
      resourceManager = context->resourceManager;
      sceneGraph = context->sceneGraph;
      inputBuffer = context->inputBuffer;

      framebufferWidth = settings.width;
      framebufferHeight = settings.height;

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

      // 0 for vsync off 1 for vsync on
      glfwSwapInterval(0);

      glEnable(GL_DEPTH_TEST);
      glEnable(GL_CULL_FACE);
      glEnable(GL_DEPTH_CLAMP);
      glCullFace(GL_BACK);
      glFrontFace(GL_CCW);

      // wireframe mode
      // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      
      // capture mouse
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
      glfwSetWindowUserPointer(window, this);
      glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos) {
        auto renderer = static_cast<OpenGLRenderer*>(glfwGetWindowUserPointer(window));

        double dx = xpos - renderer->lastX;
        double dy = ypos - renderer->lastY;

        renderer->inputBuffer->dx = -dx;
        renderer->inputBuffer->dy = -dy;

        //printf("%f %f\n", dx, dy);

        renderer->lastX = xpos;
        renderer->lastY = ypos;
      });

      glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset) {
        auto renderer = static_cast<OpenGLRenderer*>(glfwGetWindowUserPointer(window));
        
        renderer->inputBuffer->scroll = yoffset;
      });

      printf("Completed GLFW + OpenGL Setup\n");

      IMGUI_CHECKVERSION();
      ImGui::CreateContext();
      //ImGuiIO& io = ImGui::GetIO();
      //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
      //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
      //io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

      ImGui_ImplGlfw_InitForOpenGL(window, true);
      ImGui_ImplOpenGL3_Init();

      printf("Completed ImGUI Setup\n");
    }

    double lastX, lastY;

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

      //printf("Compiled Shader Program\n");

      uint blockIndex = glGetUniformBlockIndex(shaderProgram, "Lights");
      printf("Lights block index: %u\n", blockIndex);
      if (blockIndex != GL_INVALID_INDEX) {
        glUniformBlockBinding(shaderProgram, blockIndex, 0);
      }

      return shaderProgram;
    }

    void bindShader(Handle<Shader> shaderHandle) {
      if (shaderCache.count(shaderHandle.id)) return;
      //Handle<Shader> shaderHandle = resourceManager->load<Shader>(shaderId);
      Shader* shader = resourceManager->get(shaderHandle);

      uint id = compileShaderProgram(shader->vertexSource.c_str(), shader->fragmentSource.c_str());
      uint aVpLoc = glGetUniformLocation(id, "aVp");

      GPUShader gpuShader{id, aVpLoc};

      shaderCache[shaderHandle.id] = gpuShader;
    }

    void bindTexture(Handle<Image> imageHandle) {
      if (textureCache.count(imageHandle.id)) return;
      Image* image = resourceManager->get(imageHandle);
      
      uint glTex;
      glGenTextures(1, &glTex);
      glBindTexture(GL_TEXTURE_2D, glTex);
      
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image->width, image->height, 0, GL_RGB, GL_UNSIGNED_BYTE, image->data);
      glGenerateMipmap(GL_TEXTURE_2D);

      // TODO: stbi_image_free(data);
      
      textureCache[imageHandle.id] = { glTex };
    }

    void bindMesh(Handle<Mesh> meshHandle) {
      if (meshCache.count(meshHandle.id)) return;
      Mesh* mesh = resourceManager->get(meshHandle);

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

      for (int i = 0; i < 4; ++i) {
        glEnableVertexAttribArray(3 + i);
        glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(i * sizeof(glm::vec4)));
        glVertexAttribDivisor(3 + i, 1);
      }

      // TODO, make sure numVerts should actually be indices and not vertices size
      meshCache[meshHandle.id] = GPUMesh{VBO, VAO, EBO, (uint)mesh->indices.size(), instanceVBO};

      // TODO: if shader is the same as the previous one don't rebind
      // TODO: this should really be in render() with the check
      

      //printf("Loaded mesh to gpu\n");
    }

    uint lightUBO;
    uint maxLights = 64;
    void setupLightig() {
      glGenBuffers(1, &lightUBO);
      glBindBuffer(GL_UNIFORM_BUFFER, lightUBO);
      glBufferData(GL_UNIFORM_BUFFER, sizeof(GPULight) * maxLights, nullptr, GL_DYNAMIC_DRAW);
      glBindBufferBase(GL_UNIFORM_BUFFER, 0, lightUBO);
    }

    void beginFrame() {
      processInput();

      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();
    }

    void render() {
      // TODO: skybox
      /*
      Handle<Image> cubemapHandle = resourceManager->load<Image>("example/textures/sky.jpg");
      bindTexture(cubemapHandle);

      GPUTexture& gpuTexture = textureCache[cubemapHandle.id];

      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
      */

      sceneGraph->eachCamera([&](CameraData& cameraData) {
        glClearColor(cameraData.clearColor.x, cameraData.clearColor.y, cameraData.clearColor.z, cameraData.clearColor.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        //Matrix4x4F a = transformMatrix(toVector3F(cameraData.position), cameraData.scale, cameraData.rotation);
        //toGlm(a);
        glm::mat4 view = createViewMatrix(cameraData.rotation);
        glm::mat4 projection = createProjMatrix(cameraData.fov, (float)framebufferWidth/(float)framebufferHeight, cameraData.near, cameraData.far);
        glm::mat4 vp = projection * view;

        Frustrum frustrum;

        std::vector<GPULight> lights;

        for (auto& pointLight : sceneGraph->pointLights) {
          Vector4F v1 = {
            float(pointLight.position.x - cameraData.position.x),
            float(pointLight.position.y - cameraData.position.y),
            float(pointLight.position.z - cameraData.position.z),
            pointLight.intensity
          };
          Vector4F v2 = {
            pointLight.color.x,
            pointLight.color.y,
            pointLight.color.z,
            pointLight.range
          };
          GPULight light = { v1, v2 };
          lights.push_back(light);
        }

        int lightCount = lights.size();
        if (lightCount > maxLights) {
          lightCount = maxLights;
        }

        glBindBuffer(GL_UNIFORM_BUFFER, lightUBO);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(GPULight) * lightCount, lights.data());
        glBufferSubData(GL_UNIFORM_BUFFER, sizeof(GPULight) * maxLights, sizeof(int), &lightCount);

        sceneGraph->eachModel(frustrum, [&](Handle<Mesh> meshHandle, Handle<Material> materialHandle, std::vector<ModelData>& modelDatas) {
          int numInstances = modelDatas.size();

          // TODO: sort array such that the same meshes/materials are next to each other
          // TODO: only bind if mesh needs to be changed
          // TODO: this should be good now, given the nature of how the scene graph works
          bindMesh(meshHandle);
          GPUMesh& gpuMesh = meshCache[meshHandle.id];

          Material* material = resourceManager->get(materialHandle);

          Handle<Shader> shaderHandle = resourceManager->load<Shader>(material->shaderPath);

          bindShader(shaderHandle);
          GPUShader& gpuShader = shaderCache[shaderHandle.id];

          glUseProgram(gpuShader.id);
          glUniformMatrix4fv(gpuShader.aVpLoc, 1, GL_FALSE, glm::value_ptr(vp));

          // TODO: cache uniform locations in hashmap on GPUShader
          for (auto& [key, value] : material->floats) {
            glUniform1f(glGetUniformLocation(gpuShader.id, key.c_str()), value);
          }

          for (auto& [key, value] : material->vectors) {
            glUniform4f(glGetUniformLocation(gpuShader.id, key.c_str()), value.x, value.y, value.z, value.w);
          }

          uint8_t slot = 0;
          for (auto& [key, value] : material->textures) {
            Handle<Image> imageHandle = resourceManager->load<Image>(value);
            bindTexture(imageHandle);

            GPUTexture& gpuTexture = textureCache[imageHandle.id];

            glActiveTexture(GL_TEXTURE0 + slot);
            glBindTexture(GL_TEXTURE_2D, gpuTexture.glTex);
            glUniform1i(glGetUniformLocation(gpuShader.id, key.c_str()), slot);

            ++slot;
          }
          
          std::vector<glm::mat4> glmodels;
          for (int i = 0; i < numInstances; ++i) {
            ModelData modelData = modelDatas[i];
            Vector3D relativePosition = {
              double(modelData.position.x - cameraData.position.x),
              double(modelData.position.y - cameraData.position.y),
              double(modelData.position.z - cameraData.position.z)
            };
            glmodels.push_back(createTransformMatrix(relativePosition, modelData.scale, modelData.rotation));
          }

          //glBindBuffer(GL_ARRAY_BUFFER, gpuMesh.instanceVBO);
          //glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::mat4) * numInstances, glmodels.data());

          glBindBuffer(GL_ARRAY_BUFFER, gpuMesh.instanceVBO);
          glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * numInstances, glmodels.data(), GL_DYNAMIC_DRAW);
          
          glBindVertexArray(gpuMesh.VAO);

          glDrawElementsInstanced(GL_TRIANGLES, gpuMesh.numVerts, GL_UNSIGNED_INT, (void*)0, numInstances);
        });
      });

      ImGui::Begin("Engine");
      ImGui::Text("Frametime (ms): %f", context->frameTime * 1000);
      ImGui::Text("FPS (Hz): %i", int(1 / context->frameTime));

      //printf("Frametime: %f ms FPS: %i\n", context->frameTime * 1000, int(1 / context->frameTime));
      ImGui::End();
    }

    void endFrame() {     
      ImGui::Render();
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

      glfwSwapBuffers(window);
    }

    void shutdown() {
      for (auto& [id, gpuMesh] : meshCache) {
        glDeleteVertexArrays(1, &(gpuMesh.VAO));
        glDeleteBuffers(1, &(gpuMesh.VBO));
      }

      for (auto& [id, gpuShader] : shaderCache) {
        glDeleteProgram(gpuShader.id);
      }

      ImGui_ImplOpenGL3_Shutdown();
      ImGui_ImplGlfw_Shutdown();
      ImGui::DestroyContext();

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
