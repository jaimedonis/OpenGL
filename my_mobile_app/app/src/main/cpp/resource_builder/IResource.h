#pragma once

#include <string>

class IResourceManager;

class IResource
{
public:

    virtual bool IsOpen() const = 0;

    virtual bool Open(const std::string& path) = 0;

    virtual bool ReadText(std::string& text) const = 0;

    virtual void Close() = 0;

};
