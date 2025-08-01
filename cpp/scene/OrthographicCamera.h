#ifndef MY_MOBILE_APP_ORTHOGRAPHICCAMERA_H
#define MY_MOBILE_APP_ORTHOGRAPHICCAMERA_H

#include "CameraBaseNode.h"

class OrthographicCamera : public CameraBaseNode
{
public:
    OrthographicCamera(
            int viewport_width,
            int viewport_height,
            float near_plane_distance = -500,
            float far_plane_distance = 500);

    void ResetProjection() override;

private:
    float _near_plane_distance;
    float _far_plane_distance;
};


#endif //MY_MOBILE_APP_ORTHOGRAPHICCAMERA_H
