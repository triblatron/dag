//
// Created by tony on 27/04/24.
//

#include "config/config.h"

#include "DynamicLibrary.h"
#include <iostream>

namespace dag
{
    DynamicLibrary* DynamicLibrary::loadLibrary(const std::string& path)
    {
#if defined(__linux__) || defined(__APPLE__)
        // Pass RTLD_NODELETE to never unload the image and potentially trigger an ASAN error such as the vtable
        // for Nodes in the plugin.
        auto handle = dlopen(path.c_str(), RTLD_NOW|RTLD_LOCAL|RTLD_NODELETE);
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
            // Intentionally leak the handle to get similar behaviour to Linux and macOS.
            //FreeLibrary(_handle);
#else
            int errod = dlclose(_handle);
            if (errod != 0)
            {
                std::cerr << "Error from dlclose():" << dlerror() << '\n';
            }
#endif
        }
    }

    void *DynamicLibrary::getProcAddress(const char *name)
    {
#if defined(__linux__) || defined(__APPLE__)
        return dlsym(_handle, name);
#elif defined(_WIN32)
        return GetProcAddress(_handle, name);
#endif // __linux__
    }
} // nbe
