// Copyright (c) 2024 Pyarelal Knowles, MIT License

#include <app.hpp>
#include <app_impl.hpp>
#include <battery/embed.hpp>
#include <gldebug.hpp>
#include <globjects.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <implot.h>
#include <stdexcept>

App::Impl::Impl()
    : m_window(1280, 720, "Ready To Render (*.rtr) Tool", nullptr, nullptr) {
    if (!IMGUI_CHECKVERSION()) {
        throw std::runtime_error("ImGUI version mismatch");
    }

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1);  // Enable vsync

    int version = gladLoadGL(glfwGetProcAddress);
    if(version == 0)
    {
        throw std::runtime_error("Failed to initialize OpenGL context\n");
    }
}

void App::Impl::view(rtrtool::File&& file)
{
    std::lock_guard<std::mutex> lock(m_scenesMutex);
    m_scenes.emplace_back(std::move(file));
}

void App::Impl::renderloop()
{
    glraii::Program meshProgram{
        glraii::Shader(GL_VERTEX_SHADER, b::embed<"shaders/raster_mesh.vert">().str()),
        glraii::Shader(GL_GEOMETRY_SHADER, b::embed<"shaders/raster_mesh.geom">().str()),
        glraii::Shader(GL_FRAGMENT_SHADER, b::embed<"shaders/raster_mesh.frag">().str()),
    };

    OrbitCamera             camera;
    PerspectiveProjection   projection;

    glm::vec2 scale;
    glfwGetWindowContentScale(m_window, &scale.x, &scale.y);
    float dpiScale = std::max(scale.x, scale.y);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGui::StyleColorsDark();
    ImGuiIO& io = ImGui::GetIO();
    //io.FontGlobalScale = dpiScale; // blurry
    //io.DisplayFramebufferScale = {scale.x, scale.y}; // does nothing
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init("#version 150");
    auto font = b::embed<"fonts/Roboto-Medium.ttf">();
    io.Fonts->AddFontFromMemoryTTF(const_cast<void*>(static_cast<const void*>(font.data())),
                                   font.size(), 16.0f * dpiScale);

    // During init, enable debug output
    glDebugMessageCallback(defaultDebugCallbackPrintStderr, 0);
    glEnable(GL_DEBUG_OUTPUT);

    while(!glfwWindowShouldClose(m_window))
    {
        glfwPollEvents();

        int display_w, display_h;
        glfwGetFramebufferSize(m_window, &display_w, &display_h);
        projection.aspect = float(display_w) / float(display_h);

        // Camera controls
        const float movePixelsToDistance  = 1.0f / (atanf(projection.fovY * 0.5f) * (display_h * 0.5f));
        const float rotatePixelsToRadians = 10.0f / display_h;
        if(!io.WantCaptureMouse)
        {
        if(io.MouseDown[0])
        {
            camera.yaw(-io.MouseDelta.x * rotatePixelsToRadians);
            camera.pitch(-io.MouseDelta.y * rotatePixelsToRadians);
        }
        if(io.MouseDown[1])
        {
            camera.zoom(-io.MouseDelta.y * movePixelsToDistance);
        }
        if(io.MouseDown[2])
        {
            camera.pan(-io.MouseDelta.x * movePixelsToDistance, io.MouseDelta.y * movePixelsToDistance);
        }
        }

        //glm::mat4 view = camera.worldToEye();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("RTR Window");
        static bool showDemoWindow = false;
        ImGui::Checkbox("Demo Window", &showDemoWindow);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        ImGui::End();

        if (showDemoWindow)
            ImGui::ShowDemoWindow(&showDemoWindow);

        // Rendering
        ImGui::Render();
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.231f, 0.231f, 0.231f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        glUseProgram(meshProgram);
        auto worldToEye = camera.worldToEye();
        meshProgram.setUniform("lightDir", glm::mat3(camera.worldToEye()) * glm::vec3(1.0f));
        for(const auto& scene : m_scenes)
        {
            for (const auto& instance : scene.instances()) {
                auto localToEye = worldToEye * instance.localToWorld;
                meshProgram.setUniform("modelView", localToEye);
                meshProgram.setUniform("modelViewProjection", projection.matrix() * localToEye);
                meshProgram.setUniform("normalMatrix",
                                       glm::inverse(glm::transpose(glm::mat3(localToEye))));

                const glraii::Mesh& mesh = scene.meshes()[instance.meshIndex];
                const rtr::common::Material& material = scene.materials()[instance.materialIndex];
                auto bindTexture = [&meshProgram, &scene](uint32_t bindingIndex, const std::string& uniformHasName, const std::string& uniformSamplerName, rtr::optional_index32 sceneIndex)
                {
                    meshProgram.setUniform(uniformHasName, sceneIndex ? 1 : 0);
                    glActiveTexture(GL_TEXTURE0 + bindingIndex);
                    if(sceneIndex)
                    {
                        glBindTexture(GL_TEXTURE_2D, scene.textures()[*sceneIndex]);
                        meshProgram.setUniform(uniformSamplerName, GLint(bindingIndex));
                    }
                    else
                        glBindTexture(GL_TEXTURE_2D, 0);
                };
                bindTexture(0, "hasColorTexture", "colorTexture", material.textures.color);
                bindTexture(1, "hasMetallicTexture", "metallicTexture", material.textures.metallic);
                bindTexture(2, "hasRoughnessTexture", "roughnessTexture", material.textures.roughness);
                bindTexture(3, "hasNormalTexture", "normalTexture", material.textures.normal);
                meshProgram.setUniform("color", material.factors.color);
                meshProgram.setUniform("metallic", material.factors.metallic);
                meshProgram.setUniform("roughness", material.factors.roughness);
                mesh.draw();
            }
        }

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(m_window);
    }

    std::lock_guard<std::mutex> lock(m_scenesMutex);
}
