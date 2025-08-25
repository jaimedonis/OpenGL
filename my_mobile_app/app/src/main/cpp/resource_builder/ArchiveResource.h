#pragma once

#include "IResource.h"

struct AAsset;

class ArchiveResource : public IResource
{
public:
    ArchiveResource();
    virtual ~ArchiveResource();

public:

    bool IsOpen() const override;
    bool Open(const std::string& path) override;
    bool ReadText(std::string& text) const override;
    void Close() override;

private:
    void ReleaseResources();

private:
    AAsset* _asset = nullptr;
};
