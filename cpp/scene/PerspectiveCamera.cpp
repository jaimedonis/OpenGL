#include "PerspectiveCamera.h"


PerspectiveCamera::PerspectiveCamera(
        float fov_deg,
        int viewport_width,
        int viewport_height,
        float near_plane_distance,
        float far_plane_distance)
        : CameraBaseNode(),
        _fov_deg(fov_deg),
        _near_plane_distance(near_plane_distance),
        _far_plane_distance(far_plane_distance)
{
    _viewport_width = viewport_width;
    _viewport_height = viewport_height;
    ResetProjection();
}

void PerspectiveCamera::ResetProjection()
{
    _projection = glm::perspective(
            glm::radians(_fov_deg),
            float(_viewport_width) / float(_viewport_height),
            _near_plane_distance,
            _far_plane_distance);
}