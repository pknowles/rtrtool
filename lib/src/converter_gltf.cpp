// Copyright (c) 2024 Pyarelal Knowles, MIT License

#include <cgltf.h>
#include <functional>
#include <glm/ext/matrix_transform.hpp>
#include <optional>
#include <rtr/material.hpp>
#include <rtr/mesh.hpp>
#include <rtr/scene.hpp>
#include <rtr/write_mesh.hpp>
#include <rtrtool/converter.hpp>
#include <rtrtool_cgltf.hpp>
#include <rtrtool_ktx.hpp>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>

namespace rtrtool {

struct MeshData {
#define RTR_ARRAY(type, name) std::vector<type> name;
    RTR_COMMON_MESH_FOREACH_ARRAY
#undef RTR_ARRAY
};

using IndexedTexture = std::pair<uint32_t, rtr::common::Texture>;
using TextureCache = std::unordered_map<std::string, IndexedTexture>;

rtr::common::Material convertGltfMaterial(const WriterAllocator& allocator,
                                          const fs::path& basePath, const cgltf_material& material,
                                          TextureCache& textureCache) {
    rtr::common::Material result;
    result.factors = {
        .color = glm::make_vec4(material.pbr_metallic_roughness.base_color_factor),
        .metallic = material.pbr_metallic_roughness.metallic_factor,
        .roughness = material.pbr_metallic_roughness.roughness_factor,
    };
    auto convertTexture = [&allocator, &textureCache, &basePath](cgltf_texture*   cgltfTexture,
                                                                 std::string_view swizzle = {}) {
        rtr::optional_index32 result;
        if (cgltfTexture && cgltfTexture->image && cgltfTexture->image->uri) {
            auto key = (std::string(cgltfTexture->image->uri) + ":") + std::string(swizzle);
            auto [it, created] =
                textureCache.try_emplace(key, IndexedTexture{uint32_t(textureCache.size()), {}});
            auto& [textureIndex, texture] = it->second;
            if (created) {
                std::span<uint8_t> ktxData =
                    convertToKtx(allocator, basePath / cgltfTexture->image->uri, swizzle);
                texture = rtr::common::Texture{
                    .ktx = reinterpret_cast<rtr::ktx::Header*>(ktxData.data())};
                if (!texture.ktx->validateIdentifier())
                    throw std::runtime_error("Converted KTX texture failed validation");
            }
            result = textureIndex;
        } else if (cgltfTexture && cgltfTexture->image && !cgltfTexture->image->uri) {
            fprintf(stderr, "Warning: skipping gltf image with no uri (possibly embedded)\n");
        }
        return result;
    };
    result.textures.color =
        convertTexture(material.pbr_metallic_roughness.base_color_texture.texture);
    result.textures.metallic =
        convertTexture(material.pbr_metallic_roughness.metallic_roughness_texture.texture, "b");
    result.textures.roughness =
        convertTexture(material.pbr_metallic_roughness.metallic_roughness_texture.texture, "g");
    result.textures.normal = convertTexture(material.normal_texture.texture);
    return result;
}

using NodeIterator = decodeless::offset_span<rtr::Node>::iterator;

NodeIterator
writeNodesRecursive(const WriterAllocator& allocator, std::span<const cgltf_node* const> inputs,
                    std::function<void(const cgltf_node&, const NodeIterator&)> visitor,
                    const NodeIterator& parent, NodeIterator output) {
    for (const cgltf_node* const gltfNode : inputs) {
        auto rtrNode = output++;
        *rtrNode = {};
        rtrNode->transform = cgltfTransform(*gltfNode);
        rtrNode->parentOffset = uint32_t(rtrNode - parent);
        std::span children(gltfNode->children, gltfNode->children_count);
        output = writeNodesRecursive(allocator, children, visitor, rtrNode, output);
        rtrNode->descendantCount = uint32_t(output - rtrNode) - 1;
        visitor(*gltfNode, rtrNode);
    }
    return output;
}

rtr::RootHeader* convertFromGltf(const WriterAllocator& allocator, const fs::path& path) {
    cgltf_options    gltfOptions{};
    decodeless::file gltfFile(path);
    std::span        gltfData(reinterpret_cast<const std::byte*>(gltfFile.data()), gltfFile.size());
    cgltf_data*      data = nullptr;
    cgltf_result parseResult = cgltf_parse(&gltfOptions, gltfData.data(), gltfData.size(), &data);
    if (parseResult != cgltf_result_success) {
        throw std::runtime_error(cgltfErrorString(parseResult, data));
    }

    std::vector<decodeless::file> externalBuffers;
    {
        // Duplicate cgltf_load_buffers() functionality
        if (data->buffers_count && data->buffers[0].data == nullptr &&
            data->buffers[0].uri == nullptr && data->bin != nullptr) {
            if (data->bin_size < data->buffers[0].size) {
                throw std::runtime_error("data too short"); // ??
            }

            data->buffers[0].data = const_cast<void*>(data->bin); // DANGER: const_cast
            data->buffers[0].data_free_method = cgltf_data_free_method_none;
        }
        for (const auto& buffer : std::span(data->buffers, data->buffers_count)) {
            if (buffer.data)
                continue;
            if (std::string_view(buffer.uri).starts_with("data:")) {
                throw std::runtime_error("data uri not implemented");
            }
            externalBuffers.emplace_back((path.parent_path() / buffer.uri).string());
            data->buffers[0].data =
                const_cast<void*>(externalBuffers.back().data()); // DANGER: const_cast
            data->buffers[0].data_free_method = cgltf_data_free_method_none;
        }
    }

    std::unordered_map<const cgltf_primitive*, size_t> meshIndices;
    std::unordered_map<const cgltf_material*, size_t>  materialIndices;

    // Build mesh arrays to pass to rtr::common::MeshHeader::create()
    std::vector<rtr::common::Mesh> meshes;
    std::vector<std::string_view>  meshNames;
    std::vector<MeshData>          temporary;
    std::vector<std::string>       meshNamesTmp;
    for (const auto& mesh : std::span(data->meshes, data->meshes_count)) {
        temporary.emplace_back();
        uint32_t primitiveIndex = 0;
        for (const auto& primitive : std::span(mesh.primitives, mesh.primitives_count)) {
            // TODO: sometimes primitives can be duplicated to reference the
            // same mesh with multiple materials. Need a primitive equality
            // operator and hash.
            meshIndices[&primitive] = meshes.size();
            meshes.emplace_back();
            materialIndices.try_emplace(primitive.material, materialIndices.size());
            meshes.back().triangleVertices = rtrtool::convert<uint32_t, const glm::uvec3>(
                *primitive.indices, temporary.back().triangleVertices);
            for (const auto& attrib : std::span(primitive.attributes, primitive.attributes_count)) {
                // Why do I need an explicit "const" here. GCC 13.2.1 bug?
                switch (attrib.type) {
                case cgltf_attribute_type_position:
                    meshes.back().vertexPositions = rtrtool::convert<glm::vec3, const glm::vec3>(
                        *attrib.data, temporary.back().vertexPositions);
                    break;
                case cgltf_attribute_type_normal:
                    meshes.back().vertexNormals = rtrtool::convert<glm::vec3, const glm::vec3>(
                        *attrib.data, temporary.back().vertexNormals);
                    break;
                case cgltf_attribute_type_texcoord:
                    meshes.back().vertexTexCoords0 = rtrtool::convert<glm::vec2, const glm::vec2>(
                        *attrib.data, temporary.back().vertexTexCoords0);
                    break;
                case cgltf_attribute_type_tangent:
                    meshes.back().vertexTangents = rtrtool::convert<glm::vec4, const glm::vec4>(
                        *attrib.data, temporary.back().vertexTangents);
                    break;
                default:
                    // ignore unknown attributes
                    break;
                }
            }
            if (mesh.primitives_count == 1)
                meshNames.push_back(mesh.name);
            else {
                meshNamesTmp.push_back(std::string(mesh.name) + std::to_string(primitiveIndex++));
                meshNames.push_back(meshNamesTmp.back());
            }
        }
    }

    // File root header. Must be the first object allocated!
    rtr::RootHeader* header = decodeless::create::object<rtr::RootHeader>(allocator);
    std::vector<decodeless::offset_ptr<decodeless::Header>> subHeaders;

    // Write meshes
    subHeaders.push_back(rtr::common::createMeshHeader(allocator, meshes, meshNames));

    // Write materials
    rtr::common::MaterialHeader* materialHeader =
        decodeless::create::object<rtr::common::MaterialHeader>(allocator);
    materialHeader->materials =
        decodeless::create::array<rtr::common::Material>(allocator, materialIndices.size());
    TextureCache textureCache;
    for (const auto& [cgltfMaterial, materialIndex] : materialIndices) {
        if (cgltfMaterial) {
            materialHeader->materials[materialIndex] =
                convertGltfMaterial(allocator, path.parent_path(), *cgltfMaterial, textureCache);
        } else {
            // Default material
            materialHeader->materials[materialIndex] = rtr::common::Material{};
        }
    }
    materialHeader->textures =
        decodeless::create::array<rtr::common::Texture>(allocator, textureCache.size());
    for (const auto& [key, value] : textureCache) {
        const auto& [textureIndex, texture] = value;
        materialHeader->textures[textureIndex] = texture;
    }
    subHeaders.push_back(materialHeader);

    rtr::SceneHeader* sceneHeader = decodeless::create::object<rtr::SceneHeader>(allocator);
    subHeaders.push_back(sceneHeader);
    std::span<cgltf_scene> gltfScenes{data->scenes, data->scenes_count};
    sceneHeader->nodes =
        decodeless::create::array<rtr::Node>(allocator, data->nodes_count + data->scenes_count);
    sceneHeader->scenes = decodeless::create::array<decodeless::offset_ptr<rtr::Node>>(
        allocator, data->nodes_count + data->scenes_count);
    std::vector<rtr::Instance>         instances;
    std::vector<rtr::Camera>           cameras;
    std::vector<rtr::offset_string>    cameraNames;
    std::vector<rtr::DirectionalLight> directionalLights;
    std::vector<rtr::PointLight>       pointLights;
    std::vector<rtr::SpotLight>        spotLights;
    std::vector<rtr::MeshLight>        meshLights;
    auto makeAttachments = [&](const cgltf_node& gltfNode, const NodeIterator& rtrNode) {
        uint32_t nodeIndex(rtrNode - sceneHeader->nodes.begin());
        if (gltfNode.mesh) {
            for (auto& primitive :
                 std::span(gltfNode.mesh->primitives, gltfNode.mesh->primitives_count)) {
                instances.push_back(rtr::Instance{
                    .node = nodeIndex,
                    .mesh = uint32_t(meshIndices[&primitive]),
                    .material = uint32_t(materialIndices[primitive.material]),
                });
            }
        }
        if (gltfNode.camera && gltfNode.camera->type == cgltf_camera_type_perspective) {
            cameras.push_back(rtr::Camera{
                .node = nodeIndex,
                .fov = gltfNode.camera->data.perspective.yfov,
                .near = gltfNode.camera->data.perspective.znear,
                .far = gltfNode.camera->data.perspective.has_zfar
                           ? gltfNode.camera->data.perspective.zfar
                           : std::numeric_limits<float>::infinity(),
            });
            cameraNames.push_back(decodeless::create::array<char>(
                allocator, std::string_view(gltfNode.camera->name)));
        }
        if (gltfNode.light) {
            if (gltfNode.light->type == cgltf_light_type_directional) {
                directionalLights.push_back(rtr::DirectionalLight{
                    .illuminance =
                        glm::make_vec3(gltfNode.light->color) * gltfNode.light->intensity,
                    .node = nodeIndex,
                });
            }
            if (gltfNode.light->type == cgltf_light_type_point) {
                pointLights.push_back(rtr::PointLight{
                    .intensity = glm::make_vec3(gltfNode.light->color) * gltfNode.light->intensity,
                    .node = nodeIndex,
                });
            }
            if (gltfNode.light->type == cgltf_light_type_spot) {
                spotLights.push_back(rtr::SpotLight{
                    .intensity = glm::make_vec3(gltfNode.light->color) * gltfNode.light->intensity,
                    .node = nodeIndex,
                    .attenuationMax = gltfNode.light->range,
                    .innerAngle = gltfNode.light->spot_inner_cone_angle,
                    .outerAngle = gltfNode.light->spot_outer_cone_angle,
                });
            }
        }
    };
    auto nextSceneRootPtr = sceneHeader->scenes.begin();
    auto nextSceneRoot = sceneHeader->nodes.begin();
    for (auto& scene : gltfScenes) {
        auto sceneRoot = nextSceneRoot++;
        *sceneRoot = {};
        sceneRoot->transform = glm::identity<glm::mat4>();
        nextSceneRoot = writeNodesRecursive(allocator, std::span(scene.nodes, scene.nodes_count),
                                            makeAttachments, sceneRoot, nextSceneRoot);
        sceneRoot->descendantCount = uint32_t(sceneRoot - nextSceneRoot) - 1;
        *nextSceneRootPtr++ = &*sceneRoot;
    }
    sceneHeader->instances = decodeless::create::array<rtr::Instance>(allocator, instances);
    sceneHeader->cameras = decodeless::create::array<rtr::Camera>(allocator, cameras);
    sceneHeader->cameraNames =
        decodeless::create::array<rtr::offset_string>(allocator, cameraNames);
    sceneHeader->directionalLights =
        decodeless::create::array<rtr::DirectionalLight>(allocator, directionalLights);
    sceneHeader->pointLights = decodeless::create::array<rtr::PointLight>(allocator, pointLights);
    sceneHeader->spotLights = decodeless::create::array<rtr::SpotLight>(allocator, spotLights);
    sceneHeader->meshLights = decodeless::create::array<rtr::MeshLight>(allocator, meshLights);

    // Write sub-headers
    std::ranges::sort(subHeaders, decodeless::RootHeader::HeaderPtrComp());
    header->headers = decodeless::create::array<decodeless::offset_ptr<decodeless::Header>>(
        allocator, subHeaders);

    // TODO: raii
    cgltf_free(data);
    return header;
}

} // namespace rtrtool
