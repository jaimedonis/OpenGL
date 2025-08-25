#pragma once

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
