#pragma once

#include "Model.h"

#include <string>

class MeshModelLoaderBase
{
public:

    virtual std::unique_ptr<Model> LoadModel(const std::string &resource_path) = 0;

};
