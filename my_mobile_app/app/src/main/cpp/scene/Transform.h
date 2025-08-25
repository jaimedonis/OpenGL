#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"

class Transform
{
public:
    inline glm::mat4 GetWorldMatrix() {
        return _world_matrix;
    }
    inline void SetWorldMatrix(const glm::mat4& value) {
        _world_matrix = value;
    }
    glm::vec3 GetWorldPosition() const;

private:
    glm::mat4 _world_matrix = glm::mat4(1.0); // relative to its parent, world transform later...
};
