// Copyright (c) 2024 Pyarelal Knowles, MIT License

#include <cstddef>
#include <dfd.h>
#include <filesystem>
#include <formats.h>
#include <image.hpp>
#include <imageio.h>
#include <rtrtool_ktx.hpp>
#include <stdexcept>
#include <utility.h>

namespace rtrtool {

namespace fs = std::filesystem;

ktx::KTXTexture2 createTexture(const ImageSpec& target, VkFormat vkFormat) {
    ktxTextureCreateInfo createInfo{
        .glInternalformat = {},
        .vkFormat = vkFormat,
        .pDfd = {},
        .baseWidth = target.width(),
        .baseHeight = target.height(),
        .baseDepth = target.depth(),
        .numDimensions = 2, // 1d/2d/3d texture
        .numLevels = 1,     // mipmap
        .numLayers = 1,     // array texture
        .numFaces = 1,      // cube map
        .isArray = false,
        .generateMipmaps = false,
    };

    ktx::KTXTexture2 texture{nullptr};
    ktx_error_code_e ret =
        ktxTexture2_Create(&createInfo, KTX_TEXTURE_CREATE_ALLOC_STORAGE, texture.pHandle());
    if (KTX_SUCCESS != ret)
        throw std::runtime_error(std::string("ktxTexture2_Create failed with ") +
                                 ktxErrorString(ret));

    KHR_DFDSETVAL(texture->pDfd + 1, PRIMARIES, target.format().primaries());
    KHR_DFDSETVAL(texture->pDfd + 1, TRANSFER, target.format().transfer());
    return texture;
}

// Copied from KTX-Software/tools/ktx/command.h
// Copyright 2022-2023 The Khronos Group Inc.
// Copyright 2022-2023 RasterGrid Kft.
// SPDX-License-Identifier: Apache-2.0
// Cannot include directly due to missing fmt::format_string. Error is:
// 'command.h:121:19: error: ‘args#0’ is not a constant expression'
[[nodiscard]] inline FormatDescriptor createFormatDescriptor(const uint32_t* dfd) {
    const auto* bdfd = dfd + 1;

    FormatDescriptor::basicDescriptor basic;
    basic.model = khr_df_model_e(KHR_DFDVAL(bdfd, MODEL));
    basic.primaries = khr_df_primaries_e(KHR_DFDVAL(bdfd, PRIMARIES));
    basic.transfer = khr_df_transfer_e(KHR_DFDVAL(bdfd, TRANSFER));
    basic.flags = khr_df_flags_e(KHR_DFDVAL(bdfd, FLAGS));
    basic.texelBlockDimension0 = KHR_DFDVAL(bdfd, TEXELBLOCKDIMENSION0);
    basic.texelBlockDimension1 = KHR_DFDVAL(bdfd, TEXELBLOCKDIMENSION1);
    basic.texelBlockDimension2 = KHR_DFDVAL(bdfd, TEXELBLOCKDIMENSION2);
    basic.texelBlockDimension3 = KHR_DFDVAL(bdfd, TEXELBLOCKDIMENSION3);
    basic.bytesPlane0 = KHR_DFDVAL(bdfd, BYTESPLANE0);
    basic.bytesPlane1 = KHR_DFDVAL(bdfd, BYTESPLANE1);
    basic.bytesPlane2 = KHR_DFDVAL(bdfd, BYTESPLANE2);
    basic.bytesPlane3 = KHR_DFDVAL(bdfd, BYTESPLANE3);
    basic.bytesPlane4 = KHR_DFDVAL(bdfd, BYTESPLANE4);
    basic.bytesPlane5 = KHR_DFDVAL(bdfd, BYTESPLANE5);
    basic.bytesPlane6 = KHR_DFDVAL(bdfd, BYTESPLANE6);
    basic.bytesPlane7 = KHR_DFDVAL(bdfd, BYTESPLANE7);

    std::vector<FormatDescriptor::sample> samples;
    for (uint32_t i = 0; i < KHR_DFDSAMPLECOUNT(bdfd); ++i) {
        auto& sample = samples.emplace_back();
        sample.bitOffset = KHR_DFDSVAL(bdfd, i, BITOFFSET);
        sample.bitLength = KHR_DFDSVAL(bdfd, i, BITLENGTH);
        sample.channelType = KHR_DFDSVAL(bdfd, i, CHANNELID);
        const auto dataType = KHR_DFDSVAL(bdfd, i, QUALIFIERS);
        sample.qualifierFloat = (dataType & KHR_DF_SAMPLE_DATATYPE_FLOAT) != 0;
        sample.qualifierSigned = (dataType & KHR_DF_SAMPLE_DATATYPE_SIGNED) != 0;
        sample.qualifierExponent = (dataType & KHR_DF_SAMPLE_DATATYPE_EXPONENT) != 0;
        sample.qualifierLinear = (dataType & KHR_DF_SAMPLE_DATATYPE_LINEAR) != 0;
        sample.samplePosition0 = KHR_DFDSVAL(bdfd, i, SAMPLEPOSITION0);
        sample.samplePosition1 = KHR_DFDSVAL(bdfd, i, SAMPLEPOSITION1);
        sample.samplePosition2 = KHR_DFDSVAL(bdfd, i, SAMPLEPOSITION2);
        sample.samplePosition3 = KHR_DFDSVAL(bdfd, i, SAMPLEPOSITION3);
        sample.lower = KHR_DFDSVAL(bdfd, i, SAMPLELOWER);
        sample.upper = KHR_DFDSVAL(bdfd, i, SAMPLEUPPER);
    }

    return FormatDescriptor{basic, std::move(samples)};
}

[[nodiscard]] inline FormatDescriptor createFormatDescriptor(VkFormat vkFormat) {
    // TODO: avoid temporary malloc?
    const auto dfd = std::unique_ptr<uint32_t[], decltype(std::free)*>(vk2dfd(vkFormat), std::free);
    if (!dfd)
        throw std::runtime_error(std::string("Failed to create format descriptor for: ") + vkFormatString(vkFormat));
    return createFormatDescriptor(dfd.get());
}

template<class T>
std::unique_ptr<Image> makeImageWithChannels(uint32_t channels, uint32_t width, uint32_t height)
{
    if(channels == 1)
        return std::make_unique<ImageT<T, 1>>(width, height);
    else if(channels == 2)
        return std::make_unique<ImageT<T, 2>>(width, height);
    else if(channels == 3)
        return std::make_unique<ImageT<T, 3>>(width, height);
    else if(channels == 4)
        return std::make_unique<ImageT<T, 4>>(width, height);
    else
        throw std::runtime_error("bad channel count");
}

std::span<uint8_t> convertToKtx(const WriterAllocator& allocator, const fs::path& path, std::string_view swizzle)
{
    const auto inputImageFile = ImageInput::open(
        path, nullptr, [](const std::string& w) { printf("Warning: %s\n", w.c_str()); });
    inputImageFile->seekSubimage(0, 0); // Loading multiple subimage from the same input is not supported
    const auto& inputFormat = inputImageFile->spec().format();
    const auto width = inputImageFile->spec().width();
    const auto height = inputImageFile->spec().height();
    const auto inputBitLength = inputFormat.largestChannelBitLength();
    const auto requestBitLength = std::max(imageio::bit_ceil(inputBitLength), 8u);
    VkFormat                vkFormat;
    std::array<VkFormat, 4> vkFormatForChannels; // no nice way to modify formats, e.g. "this format but with 3 channels pls"
    std::function<std::unique_ptr<Image>(uint32_t, uint32_t, uint32_t)> makeSameImage;
    std::unique_ptr<Image> image = nullptr;
    switch (inputImageFile->formatType()) {
    case ImageInputFormatType::exr_uint:
        makeSameImage = makeImageWithChannels<typename rgba32image::Color::value_type>;
        image = std::make_unique<rgba32image>(width, height);
        vkFormat = VK_FORMAT_R32G32B32A32_UINT;
        vkFormatForChannels = {VK_FORMAT_R32_UINT, VK_FORMAT_R32G32_UINT, VK_FORMAT_R32G32B32_UINT,
                               VK_FORMAT_R32G32B32A32_UINT};
        break;
    case ImageInputFormatType::exr_float:
        makeSameImage = makeImageWithChannels<typename rgba32fimage::Color::value_type>;
        image = std::make_unique<rgba32fimage>(width, height);
        vkFormat = VK_FORMAT_R32G32B32A32_SFLOAT;
        vkFormatForChannels = {VK_FORMAT_R32_SFLOAT, VK_FORMAT_R32G32_SFLOAT, VK_FORMAT_R32G32B32_SFLOAT,
                               VK_FORMAT_R32G32B32A32_SFLOAT};
        break;
    case ImageInputFormatType::npbm:
        [[fallthrough]];
    case ImageInputFormatType::jpg:
        [[fallthrough]];
    case ImageInputFormatType::png_l:
        [[fallthrough]];
    case ImageInputFormatType::png_la:
        [[fallthrough]];
    case ImageInputFormatType::png_rgb:
        [[fallthrough]];
    case ImageInputFormatType::png_rgba:
        if (requestBitLength == 8) {
            makeSameImage = makeImageWithChannels<typename rgba8image::Color::value_type>;
            image = std::make_unique<rgba8image>(width, height);
            vkFormat = VK_FORMAT_R8G8B8A8_UNORM;
            vkFormatForChannels = {VK_FORMAT_R8_UNORM, VK_FORMAT_R8G8_UNORM, VK_FORMAT_R8G8B8_UNORM,
                                   VK_FORMAT_R8G8B8A8_UNORM};
            break;
        } else if (requestBitLength == 16) {
            makeSameImage = makeImageWithChannels<typename rgba16image::Color::value_type>;
            image = std::make_unique<rgba16image>(width, height);
            vkFormat = VK_FORMAT_R16G16B16A16_UNORM;
            vkFormatForChannels = {VK_FORMAT_R16_UNORM, VK_FORMAT_R16G16_UNORM,
                                   VK_FORMAT_R16G16B16_UNORM, VK_FORMAT_R16G16B16A16_UNORM};
            break;
        } else {
            throw std::runtime_error("Unsupported input format with channels bit depth " +
                                     std::to_string(requestBitLength));
        }
        break;
    }

    FormatDescriptor loadFormat = createFormatDescriptor(vkFormat);
    inputImageFile->readImage(static_cast<uint8_t*>(*image), image->getByteCount(), 0, 0, loadFormat);

    ImageSpec imageSpec = inputImageFile->spec();
    if(!swizzle.empty())
    {
        if(swizzle.size() < 1 || swizzle.size() > 4)
            throw std::runtime_error("bad swizzle size");
        if(swizzle.size() == image->getComponentCount())
        {
            std::string swizzle4(swizzle);
            for(size_t i = 4; i != swizzle.size(); --i)
                swizzle4 += "0";
            image->swizzle(swizzle);
        }
        else
        {
            // Support for swizzles that resize the image less than 4 components
            auto newImage = makeSameImage(swizzle.size(), width, height);
            if(swizzle.size() == 1)
                image->copyToR(*newImage, std::string(swizzle) + "000");
            else if(swizzle.size() == 2)
                image->copyToRG(*newImage, std::string(swizzle) + "00");
            else if(swizzle.size() == 3)
                image->copyToRGB(*newImage, std::string(swizzle) + "0");
            else if(swizzle.size() == 4)
                image->copyToRGBA(*newImage, swizzle);
            image = std::move(newImage);

            // HACK: replace format on the existing "spec" (this is rather
            // hurriedly written)
            vkFormat = vkFormatForChannels[swizzle.size() - 1];
            imageSpec.format() = createFormatDescriptor(vkFormat);
        }
    }

    ktx::KTXTexture2 texture = createTexture(imageSpec, vkFormat);

    {
        const auto ret =
            ktxTexture_SetImageFromMemory(texture, 0, 0, 0, static_cast<uint8_t*>(*image), image->getByteCount());
        if (KTX_SUCCESS != ret)
            throw std::runtime_error(std::string("ktxTexture_SetImageFromMemory failed with ") +
                                     ktxErrorString(ret));
    }

    // TODO: use a library that doesn't copy the same memory to a bunch of
    // different places before we can put it in its rightful place. Might be
    // able to implement ktxStream callbacks to interface with
    // decodeless::writer, but still have the above texture to deal with.
    auto ktxFree = [](ktx_uint8_t* ptr){ free(ptr); };
    std::unique_ptr<uint8_t, decltype(ktxFree)> unnecessaryTemporaryMemory;
    std::span<uint8_t> sizedKtx;
    {
        ktx_uint8_t* pDstBytes;
        ktx_size_t   size;
        ktx_error_code_e ret = ktxTexture_WriteToMemory(texture, &pDstBytes, &size);
        if (KTX_SUCCESS != ret)
            throw std::runtime_error(std::string("ktxTexture_WriteToMemory failed with ") +
                                     ktxErrorString(ret));
        unnecessaryTemporaryMemory = {pDstBytes, ktxFree};
        sizedKtx = {pDstBytes, size};
    }

    // Allocate aligned memory just in case. KTX doesn't seem to say anything
    // about it. Better safe than sorry.
    auto ptr = allocator.resource()->allocate(sizedKtx.size(), sizeof(std::max_align_t));
    std::span<uint8_t> result(reinterpret_cast<uint8_t*>(ptr), sizedKtx.size());
    std::ranges::copy(sizedKtx, result.begin());
    return result;
}

}
