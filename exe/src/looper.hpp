// Copyright (c) 2024 Pyarelal Knowles, MIT License

#pragma once

#include <functional>

class Looper {
public:
    template <class Function, class... Args>
    explicit Looper(Function&& f, Args&&... args)
        : m_thread(std::forward<Function>(f), std::forward<Args>(args)...) {}
    ~Looper() {
        {
            std::lock_guard<std::mutex> lock(m_loopMutex);
            m_running = false;
            m_loopCV.notify_one();
        }
        m_thread.join();
    };
    void callOnce() {
        std::lock_guard<std::mutex> lock(m_loopMutex);
        m_once = true;
        m_loopCV.notify_one();
    }
    void callAlways(bool enabled) {
        std::lock_guard<std::mutex> lock(m_loopMutex);
        if (m_always != enabled) {
            m_always = enabled;
            m_loopCV.notify_one();
        }
    }
    bool runningWait() {
        std::unique_lock<std::mutex> lock(m_loopMutex);
        m_loopCV.wait(lock, [this] { return !m_running || m_always || m_once; });
        m_once = false;
        return m_running;
    }

private:
    bool                    m_running = true;
    bool                    m_always = true;
    bool                    m_once = false;
    std::thread             m_thread;
    std::mutex              m_loopMutex;
    std::condition_variable m_loopCV;
};