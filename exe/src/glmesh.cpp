// Copyright (c) 2024 Pyarelal Knowles, MIT License

#include <glmesh.hpp>
#include <mikktspace.h>
#include <stdexcept>

namespace glraii {

inline MeshAux& udatMesh(const SMikkTSpaceContext* pContext) {
    return *reinterpret_cast<MeshAux*>(pContext->m_pUserData);
}

MeshAux::MeshAux(const rtr::common::Mesh& mesh)
    : m_mesh(mesh) {
    if (!m_mesh.triangleVertices.size())
        throw std::runtime_error("cannot generate topology");
    if (!m_mesh.vertexPositions.size())
        throw std::runtime_error("cannot generate positions");
    if (!m_mesh.vertexTexCoords0.size()) {
        throw std::runtime_error("cannot generate texture coordinates");
    }
    if (!m_mesh.vertexNormals.size()) {
        m_vertexNormals.resize(m_mesh.vertexPositions.size(), glm::vec3(0.0f));
        for (const auto& tri : m_mesh.triangleVertices) {
            glm::vec3 a = m_mesh.vertexPositions[tri.x];
            glm::vec3 b = m_mesh.vertexPositions[tri.y];
            glm::vec3 c = m_mesh.vertexPositions[tri.z];
            m_vertexNormals[tri.x] += glm::cross(b - a, c - a);
            m_vertexNormals[tri.y] += glm::cross(c - b, a - b);
            m_vertexNormals[tri.z] += glm::cross(a - c, b - c);
        }
        for (auto& normal : m_vertexNormals) {
            normal = glm::normalize(normal);
        }
        m_mesh.vertexNormals = m_vertexNormals;
    }
    if (!m_mesh.vertexTangents.size()) {
        m_vertexTangents.resize(m_mesh.vertexPositions.size(), glm::vec4(0.0f));
        m_mesh.vertexTangents = m_vertexTangents;
        SMikkTSpaceInterface interface{
            .m_getNumFaces =
                [](const SMikkTSpaceContext* pContext) {
                    return int(udatMesh(pContext).m_mesh.triangleVertices.size());
                },
            .m_getNumVerticesOfFace = []([[maybe_unused]] const SMikkTSpaceContext* pContext,
                                         [[maybe_unused]] const int iFace) -> int { return 3; },
            .m_getPosition =
                [](const SMikkTSpaceContext* pContext, float fvPosOut[], const int iFace,
                   const int iVert) {
                    *reinterpret_cast<glm::vec3*>(fvPosOut) =
                        udatMesh(pContext).m_mesh.vertexPositions
                            [udatMesh(pContext).m_mesh.triangleVertices[iFace][iVert]];
                },
            .m_getNormal =
                [](const SMikkTSpaceContext* pContext, float fvNormOut[], const int iFace,
                   const int iVert) {
                    *reinterpret_cast<glm::vec3*>(fvNormOut) =
                        udatMesh(pContext).m_mesh.vertexNormals
                            [udatMesh(pContext).m_mesh.triangleVertices[iFace][iVert]];
                },
            .m_getTexCoord =
                [](const SMikkTSpaceContext* pContext, float fvTexcOut[], const int iFace,
                   const int iVert) {
                    *reinterpret_cast<glm::vec2*>(fvTexcOut) =
                        udatMesh(pContext).m_mesh.vertexTexCoords0
                            [udatMesh(pContext).m_mesh.triangleVertices[iFace][iVert]];
                },
            .m_setTSpaceBasic =
                [](const SMikkTSpaceContext* pContext, const float fvTangent[], const float fSign,
                   const int iFace, const int iVert) {
                    udatMesh(pContext).m_vertexTangents
                        [udatMesh(pContext).m_mesh.triangleVertices[iFace][iVert]] =
                        glm::vec4{glm::make_vec3(fvTangent), fSign};
                },
            .m_setTSpace = nullptr,
        };
        SMikkTSpaceContext context{
            .m_pInterface = &interface,
            .m_pUserData = this,
        };
        if (!genTangSpaceDefault(&context)) {
            throw std::runtime_error("Failed MikkTSpace generation");
        }
    }
}

} // namespace glraii
