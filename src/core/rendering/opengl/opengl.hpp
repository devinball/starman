// --- LLM WRITTEN ---
// given the importance of this code i should probably rewrite it

#pragma once
#include "core/layers/render/renderer.hpp"
#include "core/resources/mesh.hpp"
#include "core/resources/image.hpp"
#include "core/resources/material.hpp"
#include "core/resources/shader.hpp"
#include "core/math/vector.hpp"

#include "ecs/components/camera.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <stdexcept>
#include <unordered_map>
#include <cstdio>
#include <cstring>

namespace gl_detail {

inline constexpr const char* VERT = R"glsl(
#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aUV;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

out vec3 vNormal;
out vec2 vUV;

void main() {
    vNormal     = mat3(transpose(inverse(uModel))) * aNormal;
    vUV         = aUV;
    gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);
}
)glsl";

inline constexpr const char* FRAG = R"glsl(
#version 330 core
in  vec3 vNormal;
in  vec2 vUV;
out vec4 fragColor;

uniform vec4      uColor;
uniform sampler2D uAlbedo;
uniform bool      uUseTexture;

void main() {
    vec3  L       = normalize(vec3(0.6, 1.0, 0.4));
    float diffuse = max(dot(normalize(vNormal), L), 0.0);
    float light   = 0.25 + 0.75 * diffuse;
    vec4  base    = uUseTexture ? texture(uAlbedo, vUV) : uColor;
    fragColor     = vec4(base.rgb * light, base.a);
}
)glsl";

}

struct OpenGLRenderer : Renderer {
    void init(const WindowConfig& cfg, ResourcePool* p) override {
        pool = p;

        if (!glfwInit())
            throw std::runtime_error("[GL] glfwInit failed");

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_RESIZABLE, cfg.resizable ? GLFW_TRUE : GLFW_FALSE);

        window = glfwCreateWindow(cfg.width, cfg.height, cfg.title, nullptr, nullptr);
        if (!window) throw std::runtime_error("[GL] glfwCreateWindow failed");

        glfwMakeContextCurrent(window);
        glfwSwapInterval(cfg.vsync ? 1 : 0);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
            throw std::runtime_error("[GL] gladLoadGL failed");

        screenW = cfg.width;
        screenH = cfg.height;

        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, [](GLFWwindow* w, int wi, int hi) {
            auto* s = static_cast<OpenGLRenderer*>(glfwGetWindowUserPointer(w));
            s->screenW = wi;
            s->screenH = hi;
        });

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        defaultProg = compileProgram(gl_detail::VERT, gl_detail::FRAG);

        glfwSetWindowTitle(window, cfg.title);

        printf("[GL] %s  %dx%d\n", glGetString(GL_VERSION), cfg.width, cfg.height);
    }

    void shutdown() override {
        for (auto& [_, m]  : meshCache)    freeMesh(m);
        for (auto& [_, t]  : texCache)     glDeleteTextures(1, &t.glId);
        for (auto& [_, s]  : shaderCache)  glDeleteProgram(s.prog);
        for (auto& [_, rt] : rtCache)      freeRT(rt);
        glDeleteProgram(defaultProg);
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void beginFrame() override {
        glfwPollEvents();
    }

    void endFrame  () override {
        glfwSwapBuffers(window);
    }

    bool shouldClose() override {
        return glfwWindowShouldClose(window);
    }

    RenderTarget createRenderTarget(uint32_t w, uint32_t h) override {
        Framebuffer fb{};
        fb.w = w; fb.h = h;

        glGenFramebuffers(1, &fb.fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fb.fbo);

        glGenTextures(1, &fb.colorTex);
        glBindTexture(GL_TEXTURE_2D, fb.colorTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fb.colorTex, 0);

        glGenRenderbuffers(1, &fb.depthRbo);
        glBindRenderbuffer(GL_RENDERBUFFER, fb.depthRbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                                  GL_RENDERBUFFER, fb.depthRbo);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            printf("[GL] incomplete framebuffer %ux%u\n", w, h);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        RenderTarget handle{ ++rtCounter };
        rtCache[handle.id] = fb;
        return handle;
    }

    void destroyRenderTarget(RenderTarget rt) override {
        auto it = rtCache.find(rt.id);
        if (it == rtCache.end()) return;
        freeRT(it->second);
        rtCache.erase(it);
    }

    void render(const std::vector<DrawCommand>& commands, const std::vector<Camera>& cameras, const EvictionList& evictions = {}) override
    {
        processEvictions(evictions);

        sortedCams.assign(cameras.begin(), cameras.end());
        std::sort(sortedCams.begin(), sortedCams.end(),
            [](const Camera& a, const Camera& b){ return a.priority < b.priority; });

        sortedCmds.assign(commands.begin(), commands.end());
        std::sort(sortedCmds.begin(), sortedCmds.end(),
            [](const DrawCommand& a, const DrawCommand& b){ return a.key < b.key; });

        for (const Camera& cam : sortedCams) renderCamera(cam);
    }

    Vector2I getSize() {
        return Vector2I(screenW, screenH);
    }

private:
    struct GpuMesh   { uint32_t vao=0, vbo=0, ebo=0, indexCount=0; };
    struct GpuTex    { uint32_t glId=0; };
    struct GpuShader { uint32_t prog=0; };
    struct Framebuffer {
        uint32_t fbo=0, colorTex=0, depthRbo=0, w=0, h=0;
    };

    struct UniformCache {
        GLint model      = -1;
        GLint view       = -1;
        GLint projection = -1;
        GLint color      = -1;
        GLint useTexture = -1;
    };

    ResourcePool* pool       = nullptr;
    GLFWwindow*   window     = nullptr;
    uint32_t      defaultProg = 0;
    int           screenW = 0, screenH = 0;
    uint32_t      rtCounter  = 0;

    std::unordered_map<std::string, GpuMesh>    meshCache;
    std::unordered_map<std::string, GpuTex>     texCache;
    std::unordered_map<std::string, GpuShader>  shaderCache;
    std::unordered_map<uint32_t,    Framebuffer> rtCache;
    std::unordered_map<uint32_t,    UniformCache> uniformCache;

    std::vector<Camera>      sortedCams;
    std::vector<DrawCommand> sortedCmds;

    void renderCamera(const Camera& cam) {
        if (cam.target.isValid()) {
            auto it = rtCache.find(cam.target.id);
            if (it == rtCache.end()) return;
            auto& fb = it->second;
            glBindFramebuffer(GL_FRAMEBUFFER, fb.fbo);
            glViewport(0, 0, fb.w, fb.h);
        } else {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(0, 0, screenW, screenH);
        }

        if (cam.doClear) {
            glClearColor(cam.clearColor.x, cam.clearColor.y,
                         cam.clearColor.z, cam.clearColor.w);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }

        glm::mat4 view = toGlm(cam.view);
        glm::mat4 proj = toGlm(cam.projection);

        std::string boundMatId = "\x01";
        uint32_t    activeProg = bindDefaultMaterial();
        UniformCache uc = getUniforms(activeProg);

        for (const DrawCommand& cmd : sortedCmds) {
            if (!(cam.layerMask & (1u << cmd.layer))) continue;
            if (!cmd.mesh.isValid()) continue;

            Mesh* mesh = pool->get(cmd.mesh);
            if (!mesh) continue;
            if (!meshCache.count(mesh->getId())) uploadMesh(*mesh);
            GpuMesh& gm = meshCache[mesh->getId()];


            const std::string matId = cmd.material.isValid() ? cmd.material.getId() : "";
            if (matId != boundMatId) {
                activeProg = bindMaterial(cmd.material);
                uc         = getUniforms(activeProg);  // copy, not pointer
                glUniformMatrix4fv(uc.view,       1, GL_FALSE, glm::value_ptr(view));
                glUniformMatrix4fv(uc.projection, 1, GL_FALSE, glm::value_ptr(proj));
                boundMatId = matId;
            }

            glm::mat4 model = toGlm(cmd.transform);
            glUniformMatrix4fv(uc.model, 1, GL_FALSE, glm::value_ptr(model));
            glBindVertexArray(gm.vao);
            glDrawElements(GL_TRIANGLES, gm.indexCount, GL_UNSIGNED_INT, nullptr);
        }

        glBindVertexArray(0);
    }

    const UniformCache& getUniforms(uint32_t prog) {
        auto it = uniformCache.find(prog);
        if (it != uniformCache.end()) return it->second;
        UniformCache uc;
        uc.model      = glGetUniformLocation(prog, "uModel");
        uc.view       = glGetUniformLocation(prog, "uView");
        uc.projection = glGetUniformLocation(prog, "uProjection");
        uc.color      = glGetUniformLocation(prog, "uColor");
        uc.useTexture = glGetUniformLocation(prog, "uUseTexture");
        uniformCache[prog] = uc;
        return uniformCache[prog];
    }

    uint32_t bindMaterial(const Handle<Material>& handle) {
        if (!handle.isValid()) return bindDefaultMaterial();

        Material* mat = pool->get(handle);
        if (!mat) return bindDefaultMaterial();

        uint32_t prog = defaultProg;
        if (mat->shader.isValid()) {
            Shader* shader = pool->get(mat->shader);
            if (shader) {
                if (!shaderCache.count(shader->getId())) uploadShader(*shader);
                prog = shaderCache[shader->getId()].prog;
            }
        }
        glUseProgram(prog);

        for (auto& [name, val] : mat->floats) {
            GLint loc = glGetUniformLocation(prog, name.c_str());
            if (loc >= 0) glUniform1f(loc, val);
        }
        for (auto& [name, val] : mat->vectors) {
            GLint loc = glGetUniformLocation(prog, name.c_str());
            if (loc >= 0) glUniform4f(loc, val.x, val.y, val.z, val.w);
        }

        int  unit   = 0;
        bool hasTex = false;
        for (auto& [name, imgHandle] : mat->textures) {
          if (!imgHandle.isValid()) continue;
          Image* img = pool->get(imgHandle);
          if (!img) continue;
          if (!texCache.count(img->getId())) uploadImage(*img);
          auto it = texCache.find(img->getId());
          if (it == texCache.end()) continue;

          glActiveTexture(GL_TEXTURE0 + unit);
          glBindTexture(GL_TEXTURE_2D, it->second.glId);
          GLint loc = glGetUniformLocation(prog, name.c_str());
          if (loc >= 0) glUniform1i(loc, unit);
          ++unit; hasTex = true;
      }

        if (!mat->vectors.count("uColor")) {
            GLint loc = glGetUniformLocation(prog, "uColor");
            if (loc >= 0) glUniform4f(loc, 1,1,1,1);
        }
        GLint loc = glGetUniformLocation(prog, "uUseTexture");
        if (loc >= 0) glUniform1i(loc, hasTex ? 1 : 0);

        return prog;
    }

    uint32_t bindDefaultMaterial() {
        glUseProgram(defaultProg);
        const UniformCache& uc = getUniforms(defaultProg);
        if (uc.color      >= 0) glUniform4f(uc.color, 1,1,1,1);
        if (uc.useTexture >= 0) glUniform1i(uc.useTexture, 0);
        return defaultProg;
    }

    void uploadMesh(const Mesh& mesh) {
        const size_t n = mesh.vertices.size();
        std::vector<float> verts;
        verts.reserve(n * 8);

        for (size_t i = 0; i < n; ++i) {
            verts.push_back(mesh.vertices[i].x);
            verts.push_back(mesh.vertices[i].y);
            verts.push_back(mesh.vertices[i].z);
            Vector3F nm = i < mesh.normals.size() ? mesh.normals[i] : Vector3F{0,1,0};
            verts.push_back(nm.x); verts.push_back(nm.y); verts.push_back(nm.z);
            Vector2F uv = i < mesh.uvs.size() ? mesh.uvs[i] : Vector2F{0,0};
            verts.push_back(uv.x); verts.push_back(uv.y);
        }

        uint32_t vao, vbo, ebo;
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);
        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(uint32_t), mesh.indices.data(), GL_STATIC_DRAW);

        constexpr GLsizei S = 8 * sizeof(float);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, S, (void*)0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, S, (void*)(3*sizeof(float)));
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, S, (void*)(6*sizeof(float)));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glBindVertexArray(0);

        meshCache[mesh.getId()] = { vao, vbo, ebo, (uint32_t)mesh.indices.size() };
    }

    void uploadImage(const Image& img) {
        uint32_t glId;
        glGenTextures(1, &glId);
        glBindTexture(GL_TEXTURE_2D, glId);

        auto toWrap = [](WrapMode m) -> GLint {
            switch (m) {
                case WrapMode::Clamp:  return GL_CLAMP_TO_EDGE;
                case WrapMode::Mirror: return GL_MIRRORED_REPEAT;
                default:               return GL_REPEAT;
            }
        };
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, toWrap(img.wrapU));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, toWrap(img.wrapV));

        GLint minF = (img.filter == FilterMode::Nearest)
            ? (img.genMips ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST)
            : (img.genMips ? GL_LINEAR_MIPMAP_LINEAR   : GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minF);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
            img.filter == FilterMode::Nearest ? GL_NEAREST : GL_LINEAR);

        GLenum fmt = img.channels == 4 ? GL_RGBA :
                    img.channels == 3 ? GL_RGB  : GL_RED;
        glTexImage2D(GL_TEXTURE_2D, 0, fmt,
            img.width, img.height, 0, fmt, GL_UNSIGNED_BYTE, img.pixels.data());
        if (img.genMips) glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);

        texCache[img.getId()] = { glId };
    }

    void uploadShader(const Shader& shader) {
        shaderCache[shader.getId()] = { compileProgram(shader.vertexSrc, shader.fragmentSrc) };
    }

    void processEvictions(const EvictionList& ev) {
        for (auto& id : ev.meshIds) {
            auto it = meshCache.find(id);
            if (it == meshCache.end()) continue;
            freeMesh(it->second); meshCache.erase(it);
        }
        for (auto& id : ev.imageIds) {
            auto it = texCache.find(id);
            if (it == texCache.end()) continue;
            glDeleteTextures(1, &it->second.glId); texCache.erase(it);
        }
        for (auto& id : ev.shaderIds) {
            auto it = shaderCache.find(id);
            if (it == shaderCache.end()) continue;
            uniformCache.erase(shaderCache[id].prog); // also clear cached uniforms
            glDeleteProgram(it->second.prog); shaderCache.erase(it);
        }
    }

    uint32_t compileProgram(const std::string& vertSrc, const std::string& fragSrc) {
        auto compileStage = [](const std::string& src, GLenum type) -> uint32_t {
            uint32_t id = glCreateShader(type);
            const char* c = src.c_str();
            glShaderSource(id, 1, &c, nullptr);   // was incorrectly glShader()
            glCompileShader(id);
            int ok; glGetShaderiv(id, GL_COMPILE_STATUS, &ok);
            if (!ok) {
                char log[512];
                glGetShaderInfoLog(id, 512, nullptr, log);
                printf("[GL] compile error: %s\n", log);
            }
            return id;
        };

        uint32_t vert = compileStage(vertSrc, GL_VERTEX_SHADER);
        uint32_t frag = compileStage(fragSrc, GL_FRAGMENT_SHADER);
        uint32_t prog = glCreateProgram();
        glAttachShader(prog, vert);
        glAttachShader(prog, frag);
        glLinkProgram(prog);

        int ok; glGetProgramiv(prog, GL_LINK_STATUS, &ok);
        if (!ok) {
            char log[512];
            glGetProgramInfoLog(prog, 512, nullptr, log);
            printf("[GL] link error: %s\n", log);
        }
        glDeleteShader(vert);
        glDeleteShader(frag);
        return prog;
    }

    void freeMesh(const GpuMesh& m) {
        glDeleteVertexArrays(1, &m.vao);
        glDeleteBuffers(1, &m.vbo);
        glDeleteBuffers(1, &m.ebo);
    }

    void freeRT(const Framebuffer& fb) {
        glDeleteFramebuffers (1, &fb.fbo);
        glDeleteTextures     (1, &fb.colorTex);
        glDeleteRenderbuffers(1, &fb.depthRbo);
    }

    glm::mat4 toGlm(const Matrix4x4F& m) const {
        return glm::make_mat4(m.getData().data());
    }
};
