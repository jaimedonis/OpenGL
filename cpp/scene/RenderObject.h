#ifndef MY_MOBILE_APP_RENDEROBJECT_H
#define MY_MOBILE_APP_RENDEROBJECT_H

#include "SceneNode.h"
#include "../Model.h"

#include <memory>


class RenderObject : public SceneNode
{
public:
    void ApplyMeshModel(std::unique_ptr<Model> model);
    inline Model* GetMeshModel() const {
        return _model.get();
    }

private:
    std::unique_ptr<Model> _model;
};


#endif //MY_MOBILE_APP_RENDEROBJECT_H
