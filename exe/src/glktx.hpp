// Copyright (c) 2024 Pyarelal Knowles, MIT License

#pragma once

#include <globjects.hpp>
#include <rtr/ktx.hpp>
#include "../../external/KTX-Software/lib/vk2gl.h"

namespace glraii {

inline Texture uploadTexture(const rtr::ktx::Header& header) {
    if (!header.validateIdentifier())
        throw std::runtime_error("KTX texture failed validation");
    VkFormat vkFormat = static_cast<VkFormat>(header.vkFormat);
    GLenum internalFormat = vkFormat2glInternalFormat(vkFormat);
    GLenum format = vkFormat2glFormat(vkFormat);
    GLenum dataType = vkFormat2glType(vkFormat);
    Texture result(GL_TEXTURE_2D, header.levelCount, internalFormat, header.pixelWidth, header.pixelHeight, header.pixelDepth);
    GLint level = 0;
    for (const auto& raw : header.levelsRaw()) {
        GLsizei levelWidth = std::max(1u, header.pixelWidth >> level);
        GLsizei levelHeight = std::max(1u, header.pixelHeight >> level);
        glTextureSubImage2D(result, level++, 0, 0, levelWidth, levelHeight, format, dataType,
                            raw.data());
    }
    if (header.levelsRaw().size() > 1)
        glTextureParameteri(result, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    else
        glTextureParameteri(result, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(result, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    return result;
}

} // namespace glraii