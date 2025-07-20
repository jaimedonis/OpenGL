#ifndef MY_MOBILE_APP_SCENENODE_H
#define MY_MOBILE_APP_SCENENODE_H

#include "Transform.h"

class SceneNode
{
protected:
    SceneNode() = default;
public:
    virtual ~SceneNode() = default;

public:
    Transform& GetTransform() {
        return _transform;
    }

protected:
    std::string _id;
    Transform _transform;
};


#endif //MY_MOBILE_APP_SCENENODE_H
