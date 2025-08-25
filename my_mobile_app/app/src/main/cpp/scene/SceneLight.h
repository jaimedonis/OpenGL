#pragma once

#include "glm/glm.hpp"

enum LightType : int
{
    Point = 0,
    Directional = 1,
};

static constexpr int MAX_NUM_LIGHTS = 4;

struct alignas(16) SceneLight {
    // ( std140 shader alignment rule requires each field bellow to be padded to 16 bytes )
    float light_type;
    glm::vec3 padding0;
    glm::vec3 light_position;
    float padding1 = 0;
    glm::vec3 light_color;
    float padding2 = 0;
};
