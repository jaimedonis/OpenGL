#pragma once

#include "MeshModelLoaderBase.h"
#include "Model.h"

#include <memory>

class MeshModelBuilder
{
public:

    static std::unique_ptr<Model> CreateMeshModel(const std::string& resource_path);

};
