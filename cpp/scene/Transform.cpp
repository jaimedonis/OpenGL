#include "Transform.h"

glm::vec3 Transform::GetPosition() const
{
    return {_local_transform[3][0], _local_transform[3][1], _local_transform[3][2] };
}