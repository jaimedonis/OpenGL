#include "SceneGraph.h"

static void ComputeBBox(const Model& model,  glm::vec3& bbox_min, glm::vec3& bbox_max)
{
    for(const auto& mesh : model.GetMeshes()) {
        const auto &indices = mesh->_indices;
        const auto &vertices = mesh->_vertices;
        for (const auto &index: indices) {
            auto vertex = mesh->_model_transform * glm::vec4(vertices[index], 1.0);
            bbox_min.x = std::min(bbox_min.x, vertex.x);
            bbox_min.y = std::min(bbox_min.y, vertex.y);
            bbox_min.z = std::min(bbox_min.z, vertex.z);
            bbox_max.x = std::max(bbox_max.x, vertex.x);
            bbox_max.y = std::max(bbox_max.y, vertex.y);
            bbox_max.z = std::max(bbox_max.z, vertex.z);
        }
    }
}

SceneGraph::SceneGraph()
{
    _lights.reserve(MAX_NUM_LIGHTS);
}

void SceneGraph::AddCamera(std::unique_ptr<CameraBaseNode>& camera)
{
    _cameras.emplace_back(std::move(camera));
}

void SceneGraph::AddLight(const SceneLight& light)
{
    assert(_lights.size() < MAX_NUM_LIGHTS);
    _lights.emplace_back(light);
}

void SceneGraph::AddRenderObject(std::unique_ptr<RenderObject>& render_object)
{
    _render_objects.emplace_back(std::move(render_object));
}

void SceneGraph::GetSceneBounds(glm::vec3& scene_center, float& scene_radius) const
{
    glm::vec3 bbox_min;
    glm::vec3 bbox_max;

    bbox_min.x = std::numeric_limits<float>::max();
    bbox_min.y = bbox_min.x;
    bbox_min.z = bbox_min.x;
    bbox_max.x = std::numeric_limits<float>::min();
    bbox_max.y = bbox_max.x;
    bbox_max.z = bbox_max.x;

    for(const auto& render_object : _render_objects ) {
        ComputeBBox(*render_object->GetMeshModel(), bbox_min, bbox_max);
    }

    scene_center = (bbox_min + bbox_max) * 0.5f;
    scene_radius = glm::length(bbox_max - scene_center);
}