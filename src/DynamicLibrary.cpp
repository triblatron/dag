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
#endif // __linux__
        return new DynamicLibrary(handle);
    }

    DynamicLibrary::DynamicLibrary(DynamicLibrary::Handle handle)
    :
    _handle(handle)
    {
        // Do nothing
    }

    void *DynamicLibrary::getProcAddress(const char *name)
    {
#if defined(__linux__)
        return dlsym(_handle, name);
#endif // __linux__
    }
} // nbe