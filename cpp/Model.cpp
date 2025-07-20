#include "Model.h"


void ModelMesh::ComputeTangentSpace()
{
    assert(_tex_coords.size() == _vertices.size());

    if(!_tangents.empty()) {
        // this mesh already has tangents
        return;
    }
    _tangents = std::vector<glm::vec4>(_vertices.size());

    bool hasValidNormals = true;
    if (_normals.empty()) {
        hasValidNormals = false;
        _normals = std::vector<glm::vec3>(_vertices.size());
    }

    auto totalIndices = _indices.size();
    auto totalVertices = _vertices.size();
    // Iterate through every triangle and for each triangle calculate the tangent and bitangent for each vertex.
    // This uses the equations shown in class (Lecture 8).
    std::vector<int> tangentAverager(_vertices.size());
    if (!_indices.empty()) {
        assert(_indices.size() % 3 == 0);
        for (decltype(totalIndices) i = 0; i < totalIndices; i += 3) {
            glm::ivec3 triangle(_indices.at(i), _indices.at(i + 1), _indices.at(i + 2));
            ComputeTangentSpaceHelper(triangle, hasValidNormals, tangentAverager);
        }
    } else {
        assert(_vertices.size() % 3 == 0);
        for (decltype(totalVertices) i = 0; i < totalVertices; i += 3) {
            glm::ivec3 triangle(i, i + 1, i + 2);
            ComputeTangentSpaceHelper(triangle, hasValidNormals, tangentAverager);
        }
    }

    for (decltype(totalVertices) i = 0; i < totalVertices; ++i) {
        auto& tangent_ref = _tangents.at(i);
        tangent_ref = glm::normalize(_tangents.at(i) / static_cast<float>(tangentAverager[i]));
        tangent_ref.w = 1.0;
        if (!hasValidNormals) {
            _normals.at(i) = glm::normalize(_normals.at(i) / static_cast<float>(tangentAverager[i]));
        }
    }
}

void ModelMesh::ComputeTangentSpaceHelper(glm::ivec3 triangleVertexIndices, bool useStoredNormals, std::vector<int>& averager)
{
    for (int x = 0; x < 3; ++x) {
        const int i = triangleVertexIndices[x];
        const int j = triangleVertexIndices[(x + 1) % 3];
        const int k = triangleVertexIndices[(x + 2) % 3];

        glm::vec3 deltaPos21 = glm::vec3(_vertices.at(j) - _vertices.at(i));
        glm::vec3 deltaPos31 = glm::vec3(_vertices.at(k) - _vertices.at(i));

        glm::vec2 deltaUV21 = _tex_coords.at(j) - _tex_coords.at(i);
        glm::vec2 deltaUV31 = _tex_coords.at(k) - _tex_coords.at(i);

        glm::vec3 bitangent = (deltaPos21 * deltaUV31.x - deltaPos31 * deltaUV21.x) / (deltaUV21.y * deltaUV31.x - deltaUV31.y * deltaUV21.x);
        glm::vec3 tangent;
        if (useStoredNormals) {
            glm::vec3 currentNormal = _normals.at(i);
            tangent = glm::cross(bitangent, currentNormal);
        } else {
            tangent = (deltaPos21 - bitangent * deltaUV21.y) / deltaUV21.x;
            _normals.at(i) += glm::cross(tangent, bitangent);
        }

        glm::vec4 add_tangent = glm::vec4(tangent, 0);
        _tangents.at(i) += add_tangent;

        ++averager[i];
    }
}
