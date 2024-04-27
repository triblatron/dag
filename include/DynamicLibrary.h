//
// Created by tony on 27/04/24.
//

#pragma once

#include "config/Export.h"

#if defined(HAVE_DLFCN_H)
#include <dlfcn.h>
#endif // HAVE_DLFCN_H

#include <string>

namespace nbe
{

    class DynamicLibrary
    {
    public:
#if defined(__linux__)
        typedef void* Handle;
#endif
    public:
        explicit DynamicLibrary(Handle handle);

        void* getProcAddress(const char* name);

        static DynamicLibrary* loadLibrary(const std::string& path);
    private:
        Handle _handle{nullptr};
    };

} // nbe
