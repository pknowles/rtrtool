// Copyright (c) 2024 Pyarelal Knowles, MIT License

#pragma once

#include <decodeless/writer.hpp>
#include <filesystem>

namespace rtrtool {

namespace fs = std::filesystem;

using WriterAllocator = std::pmr::polymorphic_allocator<std::byte>; //typename decodeless::writer::allocator_type;

[[nodiscard]] std::span<uint8_t> convertToKtx(const WriterAllocator& allocator,
                                              const fs::path& path, std::string_view swizzle);

} // namespace rtrtool
