#ifndef MY_MOBILE_APP_MESHMODELLOADERBASE_H
#define MY_MOBILE_APP_MESHMODELLOADERBASE_H

#include "Model.h"

#include <string>

class MeshModelLoaderBase
{
public:

    virtual std::unique_ptr<Model> LoadModel(const std::string &resource_path) = 0;

};


#endif //MY_MOBILE_APP_MESHMODELLOADERBASE_H
