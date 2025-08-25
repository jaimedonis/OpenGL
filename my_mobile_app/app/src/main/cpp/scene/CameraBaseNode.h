#pragma once

#include "SceneNode.h"

class CameraBaseNode : public SceneNode
{
protected:
    CameraBaseNode() = default;
public:
    ~CameraBaseNode() override = default;

public:
    inline glm::mat4 GetViewMatrix() {
        return _view;
    }
    inline glm::mat4 GetProjectionMatrix() {
        return _projection;
    }

    void SetViewPort(int width, int height);

    void LookAt(const glm::vec3& eye, const glm::vec3& target);
    inline glm::vec3 GetEye() { return _eye; }
    inline glm::vec3 GetTarget() { return _target; }

    virtual void ResetProjection() = 0;

protected:
    glm::mat4 _view = glm::mat4(1.0);
    glm::mat4 _projection = glm::mat4(1.0);
    int _viewport_width = 0;
    int _viewport_height = 0;
    glm::vec3 _eye;
    glm::vec3 _target;
};

