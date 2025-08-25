#ifndef MY_MOBILE_APP_GLTFMESHMODELLOADER_H
#define MY_MOBILE_APP_GLTFMESHMODELLOADER_H

#include "MeshModelLoaderBase.h"

#include <string>


class GltfMeshModelLoader : public MeshModelLoaderBase
{
public:

    std::unique_ptr<Model> LoadModel(const std::string &resource_path) override;

};


#endif //MY_MOBILE_APP_GLTFMESHMODELLOADER_H
