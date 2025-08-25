#pragma once

#include "MeshModelLoaderBase.h"

#include <string>


class GltfMeshModelLoader : public MeshModelLoaderBase
{
public:

    std::unique_ptr<Model> LoadModel(const std::string &resource_path) override;

};
