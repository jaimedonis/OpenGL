#ifndef MY_MOBILE_APP_PERSPECTIVECAMERA_H
#define MY_MOBILE_APP_PERSPECTIVECAMERA_H

#include "CameraBaseNode.h"

class PerspectiveCamera : public CameraBaseNode
{
public:
    PerspectiveCamera(
            float fov_deg,
            int viewport_width,
            int viewport_height,
            float near_plane_distance = 0.1f,
            float far_plane_distance = 500.0f);

    void ResetProjection() override;

private:
    float _fov_deg;
    float _near_plane_distance;
    float _far_plane_distance;
};


#endif //MY_MOBILE_APP_PERSPECTIVECAMERA_H
