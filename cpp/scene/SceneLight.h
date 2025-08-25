#ifndef MY_MOBILE_APP_SCENELIGHT_H
#define MY_MOBILE_APP_SCENELIGHT_H

#include "glm/glm.hpp"

#define MAX_LIGHTS 5

enum LightType : int
{
    PointLight = 0,
};

struct alignas(16) SceneLight // layout(std140) rules: vec3 pads to vec4
{
    LightType light_type = LightType::PointLight;
    glm::vec3 padding0; // pad each field to 16 bytes
    glm::vec3 light_position = glm::vec3(0.0);
    float padding1;     // pad each field to 16 bytes
    glm::vec3 light_color = glm::vec3(1.0, 1.0, 1.0); // noon/daylight;
    float padding2;     // pad each field to 16 bytes
};

/*
#include "SceneNode.h"

class SceneLight : public SceneNode
{
public:
    enum LightType : int
    {
        PointLight = 0,
    };

    inline LightType GetLightType() const { return _light_type; }
    inline void SetLightType(LightType ligh_type) { _light_type = ligh_type; }

    inline glm::vec3 GetLightPosition() const { return _light_position; }
    inline void SetLightPosition(const glm::vec3& light_position) { _light_position = light_position; }

    inline glm::vec3 GetLightColor() const { return _light_color; }
    inline void SetLightColor(const glm::vec3& light_color) { _light_color = light_color; }

private:
    LightType _light_type = LightType::PointLight;
    glm::vec3 _light_position = glm::vec3(0.0);
    glm::vec3 _light_color = glm::vec3(1.0, 1.0, 1.0); // noon/daylight;;
};
*/

#endif //MY_MOBILE_APP_SCENELIGHT_H
