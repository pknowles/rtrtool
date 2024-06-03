// Copyright (c) 2024 Pyarelal Knowles, MIT License

#pragma once

#include <decodeless/writer.hpp>
#include <filesystem>
#include <rtr/header.hpp>
#include <memory_resource>

namespace rtrtool {

namespace fs = std::filesystem;

// It is intended that a linear allocator is used to build a contiguous block of
// memory.
using WriterAllocator = std::pmr::polymorphic_allocator<std::byte>;

[[maybe_unused]] rtr::RootHeader* convertFromGltf(const WriterAllocator& allocator, const fs::path& path);

} // namespace rtrtool
