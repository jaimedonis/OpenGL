#include "OrthographicCamera.h"


OrthographicCamera::OrthographicCamera(
        int viewport_width,
        int viewport_height,
        float near_plane_distance,
        float far_plane_distance)
        : CameraBaseNode(),
        _near_plane_distance(near_plane_distance),
        _far_plane_distance(far_plane_distance)
{
    _viewport_width = viewport_width;
    _viewport_height = viewport_height;
    ResetProjection();
}

void OrthographicCamera::ResetProjection()
{
    _projection = glm::ortho(
            0.0f,
            float(_viewport_width),
            float(_viewport_height),
            0.0f,
            _near_plane_distance,
            _far_plane_distance);
}