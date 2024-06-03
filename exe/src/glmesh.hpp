// Copyright (c) 2024 Pyarelal Knowles, MIT License

#pragma once

#include <glm/geometric.hpp>
#include <glm/glm.hpp>
#include <globjects.hpp>
#include <rtr/mesh.hpp>
#include <stdexcept>
#include <vector>

namespace glraii {

struct MeshAux {
public:
    MeshAux(const rtr::common::Mesh& mesh);
    const rtr::common::Mesh& operator*() const { return m_mesh; };
    const rtr::common::Mesh* operator->() const { return &m_mesh; };

private:
#define RTR_ARRAY(type, name) std::vector<type> m_##name;
    RTR_COMMON_MESH_FOREACH_ARRAY
#undef RTR_ARRAY

    rtr::common::Mesh m_mesh;
};

class Mesh {
public:
    Mesh(const rtr::common::Mesh& mesh)
        : m_mesh(mesh),
          m_elementBuffer(m_mesh->triangleVertices),
          m_vertexPositions(m_mesh->vertexPositions),
          m_vertexTexCoords0(m_mesh->vertexTexCoords0),
          m_vertexNormals(m_mesh->vertexNormals),
          m_vertexTangents(m_mesh->vertexTangents),
          m_vertexArray(
              m_elementBuffer,
              {
                  VertexArray::Attrib::contiguous<decltype(*m_mesh->vertexPositions.data())>(
                      m_vertexPositions, 0),
                  VertexArray::Attrib::contiguous<decltype(*m_mesh->vertexTexCoords0.data())>(
                      m_vertexTexCoords0, 1),
                  VertexArray::Attrib::contiguous<decltype(*m_mesh->vertexNormals.data())>(
                      m_vertexNormals, 2),
                  VertexArray::Attrib::contiguous<decltype(*m_mesh->vertexTangents.data())>(
                      m_vertexTangents, 3),
              }),
          m_triangleCount(uint32_t(mesh.triangleVertices.size())) {}
    void draw() const {
        glBindVertexArray(m_vertexArray);
        glDrawElements(GL_TRIANGLES, m_triangleCount * 3, GL_UNSIGNED_INT, (void*)0);
        glBindVertexArray(0);
    }

private:
    MeshAux     m_mesh;
    Buffer      m_elementBuffer;
    Buffer      m_vertexPositions;
    Buffer      m_vertexTexCoords0;
    Buffer      m_vertexNormals;
    Buffer      m_vertexTangents;
    VertexArray m_vertexArray;
    GLuint      m_triangleCount = 0;
};

} // namespace glraii
