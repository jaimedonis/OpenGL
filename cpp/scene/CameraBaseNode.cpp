#include "CameraBaseNode.h"

void CameraBaseNode::LookAt(const glm::vec3& eye, const glm::vec3& target)
{
    _eye = eye;
    _target = target;
    _view = glm::lookAt(eye, target, glm::vec3(0.0, 1.0, 0.0));
}

void CameraBaseNode::SetViewPort(int width, int height)
{
    _viewport_width = width;
    _viewport_height = height;
    ResetProjection();
}
