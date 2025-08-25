#pragma once

#include "RenderObject.h"
#include "SceneLight.h"
#include "CameraBaseNode.h"

#include <memory>
#include <vector>

class SceneGraph
{
public:
    SceneGraph();
    virtual ~SceneGraph() = default;

public:

    // Add a camera the the scene
    void AddCamera(std::unique_ptr<CameraBaseNode>& camera);
    inline std::vector<std::unique_ptr<CameraBaseNode>>& GetCameras() {
        return _cameras;
    }

    inline CameraBaseNode* GetCurrentCamera() const {
        return _cameras[0].get(); // TODO
    }

    // Add a scene light
    void AddLight(const SceneLight& light);
    inline std::vector<SceneLight>& GetLights() {
        return _lights;
    }

    // Add a render mesh model object.
    void AddRenderObject(std::unique_ptr<RenderObject>& render_object);
    inline std::vector<std::unique_ptr<RenderObject>>& GetRenderObjects() {
        return _render_objects;
    }

    // Computes the scene bounds encompassing all scene meshes.
    void GetSceneBounds(glm::vec3& scene_center, float& scene_radius) const;

private:

    std::vector<std::unique_ptr<CameraBaseNode>> _cameras;
    std::vector<SceneLight> _lights;
    std::vector<std::unique_ptr<RenderObject>> _render_objects;
};
