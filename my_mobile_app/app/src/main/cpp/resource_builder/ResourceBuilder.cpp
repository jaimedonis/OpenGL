#include "ResourceBuilder.h"
#include "ArchiveResource.h"

#include <cassert>
#include <filesystem>

std::shared_ptr<IResource> ResourceBuilder::Open( const std::string& file_path )
{
    std::shared_ptr<IResource> asset_resource;

    asset_resource = std::make_shared<ArchiveResource>();
    asset_resource->Open(file_path);

    return asset_resource;
}

