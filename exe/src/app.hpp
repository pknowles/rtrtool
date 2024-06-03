// Copyright (c) 2024 Pyarelal Knowles, MIT License

#pragma once

#include <rtrtool/file.hpp>
#include <memory>

class App {
public:
    App();
    ~App();
    void view(rtrtool::File&& file);
    void renderloop();

private:
    // PIMPL to avoid dragging in glfw, vulkan, imgui etc.
    class Impl;
    std::unique_ptr<Impl> m_impl;
};
