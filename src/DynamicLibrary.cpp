//
// Created by tony on 27/04/24.
//

#include "config/config.h"

#include "DynamicLibrary.h"

namespace nbe
{
    DynamicLibrary* DynamicLibrary::loadLibrary(const std::string& path)
    {
#if defined(__linux__)
        auto handle = dlopen(path.c_str(), RTLD_NOW|RTLD_LOCAL);
#elif defined(_WIN32)
        auto handle = LoadLibrary(path.c_str());
#endif // __linux__
        return new DynamicLibrary(handle);
    }

    DynamicLibrary::DynamicLibrary(DynamicLibrary::Handle handle)
    :
    _handle(handle)
    {
        // Do nothing
    }

    DynamicLibrary::~DynamicLibrary()
    {
        if (_handle != nullptr)
        {
#if defined(_WIN32)
            FreeLibrary(_handle);
#endif
        }
    }

    void *DynamicLibrary::getProcAddress(const char *name)
    {
#if defined(__linux__)
        return dlsym(_handle, name);
#elif defined(_WIN32)
        return GetProcAddress(_handle, name);
#endif // __linux__
    }
} // nbe