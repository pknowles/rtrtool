// Copyright (c) 2024 Pyarelal Knowles, MIT License

// Must be first, before imgui
#include <glad/gl.h>

#include <GLFW/glfw3.h>
#include <app.hpp>
#include <camera.hpp>
#include <condition_variable>
#include <glktx.hpp>
#include <glmesh.hpp>
#include <looper.hpp>
#include <memory>
#include <mutex>
#include <rtr/material.hpp>
#include <rtr/mesh.hpp>
#include <rtr/scene.hpp>
#include <rtrtool/file.hpp>
#include <stdexcept>
#include <thread>
#include <vector>

class Scene {
public:
    Scene(rtrtool::File&& file)
        : m_file(std::move(file)),
          m_meshHeader(m_file->findSupported<rtr::common::MeshHeader>()),
          m_materialHeader(m_file->findSupported<rtr::common::MaterialHeader>()),
          m_sceneHeader(m_file->findSupported<rtr::SceneHeader>()) {
        if(!m_meshHeader || !m_materialHeader || !m_sceneHeader)
            throw std::runtime_error("file missing required rtr headers");
        for (const auto& mesh : m_meshHeader->meshes) {
            m_meshes.emplace_back(mesh);
        }
        for (const auto& texture : m_materialHeader->textures) {
            m_textures.emplace_back(glraii::uploadTexture(*texture.ktx));
        }
        m_instances.reserve(m_sceneHeader->instances.size());
        for(auto& instance : m_sceneHeader->instances)
        {
            auto*     node = &m_sceneHeader->nodes[instance.node];
            glm::mat4 transform = node->transform;
            // Inefficient traversal to the root for each instance
            while (node->parentOffset) {
                node -= *node->parentOffset;
                transform = node->transform * transform;
            }
            // Ignore instances from anything other than the first scene by
            // checking if the instance's node's root is the first scene's root.
            if (node != &*m_sceneHeader->scenes[0])
                continue;
            m_instances.push_back({.meshIndex = instance.mesh,
                                   .materialIndex = instance.material,
                                   .localToWorld = transform});
        }
    }

    struct Instance
    {
        uint32_t meshIndex;
        uint32_t materialIndex;
        glm::mat4 localToWorld;
    };

    std::span<const glraii::Mesh>          meshes() const { return m_meshes; }
    std::span<const glraii::Texture>       textures() const { return m_textures; }
    std::span<const rtr::common::Material> materials() const { return m_materialHeader->materials; }
    std::span<const Instance>              instances() const { return m_instances; }

private:
    std::vector<glraii::Mesh>    m_meshes;
    std::vector<glraii::Texture> m_textures;
    std::vector<Instance>        m_instances;
    rtrtool::File                m_file;
    rtr::common::MeshHeader*     m_meshHeader;
    rtr::common::MaterialHeader* m_materialHeader;
    rtr::SceneHeader*            m_sceneHeader;
};

namespace glfw_scoped {

class Initialize {
public:
    Initialize() {
        glfwSetErrorCallback(errorCallback);
        if (!glfwInit())
            throw std::runtime_error("glfwInit() failed");

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
#if defined(__APPLE__)
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required on Mac
#endif
    }
    ~Initialize() { glfwTerminate(); }

private:
    static void errorCallback(int error, const char* description) {
        fprintf(stderr, "GLFW Error %d: %s\n", error, description);
    }
};

class Window {
public:
    Window(int width, int height, const char* title, GLFWmonitor* monitor, GLFWwindow* share)
        : m_window(glfwCreateWindow(width, height, title, monitor, share)){};
    operator GLFWwindow* const&() const { return m_window; }

private:
    GLFWwindow* m_window;
};

}; // namespace glfw_scoped

class App::Impl {
public:
    Impl();
    ~Impl() = default;
    void view(rtrtool::File&& file);
    void renderloop();

private:
    std::vector<Scene>      m_scenes;
    std::mutex              m_scenesMutex;
    glfw_scoped::Initialize m_glfwInit;
    glfw_scoped::Window     m_window;
};
