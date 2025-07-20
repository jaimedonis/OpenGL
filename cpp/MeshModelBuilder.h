#ifndef MY_MOBILE_APP_MESHMODELBUILDER_H
#define MY_MOBILE_APP_MESHMODELBUILDER_H

#include "MeshModelLoaderBase.h"
#include "Model.h"

#include <memory>

class MeshModelBuilder
{
public:

    static std::unique_ptr<Model> CreateMeshModel(const std::string& resource_path);

};


#endif //MY_MOBILE_APP_MESHMODELBUILDER_H
