#ifndef MY_MOBILE_APP_TRANSFORM_H
#define MY_MOBILE_APP_TRANSFORM_H

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"

class Transform
{
public:
    inline glm::mat4 GetLocalMatrix() {
        return _local_transform;
    }
    inline void SetLocalMatrix(const glm::mat4& value) {
        _local_transform = value;
    }
    glm::vec3 GetPosition() const;

private:
    glm::mat4 _local_transform; // relative to its parent, world transform later...
};


#endif //MY_MOBILE_APP_TRANSFORM_H
