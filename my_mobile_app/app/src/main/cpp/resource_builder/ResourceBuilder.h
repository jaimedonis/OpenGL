#pragma once

#include "IResource.h"

#include <string>
#include <map>
#include <memory>

class ResourceBuilder
{
public:

    static std::shared_ptr<IResource> Open( const std::string& file_path );

};

