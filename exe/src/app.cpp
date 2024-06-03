// Copyright (c) 2024 Pyarelal Knowles, MIT License

#include <app.hpp>
#include <app_impl.hpp>

App::App() : m_impl(std::make_unique<App::Impl>()) {}

App::~App() = default;

void App::view(rtrtool::File&& file)
{
    m_impl->view(std::move(file));
}

void App::renderloop()
{
    m_impl->renderloop();
}
