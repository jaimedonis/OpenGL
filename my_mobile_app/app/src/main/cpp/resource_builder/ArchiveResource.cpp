#include "ArchiveResource.h"

#include <android/asset_manager.h>

#include <cassert>

extern AAssetManager* g_asset_manager;

ArchiveResource::ArchiveResource()
{
    assert(g_asset_manager);
}

ArchiveResource::~ArchiveResource()
{
    ReleaseResources();
}

bool ArchiveResource::IsOpen() const
{
    return _asset != nullptr;
}

bool ArchiveResource::Open(const std::string& path)
{
    _asset = AAssetManager_open(
            g_asset_manager,
            path.c_str(),
            AASSET_MODE_UNKNOWN );
    if( _asset == nullptr )
    {
        // LOG_WARN("can't open file '%s'", m_fileName.c_str());
    }
    return (_asset != nullptr);
}

bool ArchiveResource::ReadText(std::string &text) const
{
    auto total_len = AAsset_getLength(_asset);
    if(total_len <= 0) {
        return false;
    }
    auto sz_buffer = (char*)AAsset_getBuffer(_asset);
    if(sz_buffer == nullptr) {
        return false;
    }
    text = sz_buffer;

    return !text.empty();
}

void ArchiveResource::Close()
{
    ReleaseResources();
}

void ArchiveResource::ReleaseResources()
{
    if( _asset )
    {
        AAsset_close(_asset);
        _asset = nullptr;
    }
}