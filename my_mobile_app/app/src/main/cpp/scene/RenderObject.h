#pragma once

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
