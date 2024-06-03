// Copyright (c) 2024 Pyarelal Knowles, MIT License

#pragma once

#include <filesystem>
#include <glm/glm.hpp>
#include <rtr/header.hpp>
#include <decodeless/mappedfile.hpp>
#include <any>
#include <memory>
#include <stdexcept>

namespace rtrtool {

class MappedFile {
public:
    struct Error : public std::runtime_error {
        using std::runtime_error::runtime_error;
    };

    MappedFile(MappedFile&& other) = default;
    MappedFile& operator=(MappedFile&& other) = default;
    MappedFile(const std::filesystem::path& input) : m_file(input)
    {
        if(!(*this)->validate())
        {
            throw Error("Failed binary compatibility validation for " + input.string());
        }
    }

    const rtr::RootHeader& operator*() const {
        return *reinterpret_cast<const rtr::RootHeader*>(m_file.data());
    }

    const rtr::RootHeader* operator->() const {
        return reinterpret_cast<const rtr::RootHeader*>(m_file.data());
    }

private:
    decodeless::file m_file;
};

// For owning an arbitrary object without its type. std::any must be copyable
using UniqueAny = std::unique_ptr<void, void (*)(void*)>;

// Utility to create a deleter lambda for UniqueAny
template <class T, class... Args>
    requires std::is_rvalue_reference_v<T&&>
inline UniqueAny MakeUniqueAny(Args&&... args) {
    return UniqueAny(new T(std::move(std::forward<Args>(args)...)),
                     [](void* ptr) { delete reinterpret_cast<T*>(ptr); });
}

// Wrapper to pass around an rtr::RootHeader while abstracting its storage.
class File {
public:
    template <class FileSource>
        requires std::is_rvalue_reference_v<FileSource&&>
    File(FileSource&& source)
        : m_rootHeader(&static_cast<const rtr::RootHeader&>(*source)),
          m_source(MakeUniqueAny<FileSource>(std::move(source))) {
        if (!(*this)->validate()) {
            throw std::runtime_error("rtr::RootHeader validation failed");
        }
    }

    const rtr::RootHeader& operator*() const { return *m_rootHeader; }
    const rtr::RootHeader* operator->() const { return &*m_rootHeader; }

private:
    const rtr::RootHeader* m_rootHeader = nullptr;
    UniqueAny              m_source;
};

}; // namespace rtrtool
