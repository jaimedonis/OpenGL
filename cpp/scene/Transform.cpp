#include "Transform.h"

glm::vec3 Transform::GetWorldPosition() const
{
    return {_world_matrix[3][0], _world_matrix[3][1], _world_matrix[3][2] };
}