#include "MeshModelBuilder.h"
#include "GltfMeshModelLoader.h"

#include <memory>


std::unique_ptr <Model> MeshModelBuilder::CreateMeshModel(const std::string &resource_path)
{
    GltfMeshModelLoader model_loader;

    auto mesh_model = model_loader.LoadModel(resource_path);

    return mesh_model;
}
