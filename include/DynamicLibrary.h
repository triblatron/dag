//
// Created by tony on 27/04/24.
//

#pragma once

#include "config/Export.h"

#if defined(HAVE_DLFCN_H)
#include <dlfcn.h>
#endif // HAVE_DLFCN_H

#include <string>

#if defined(HAVE_WINDOWS_H)
#define NOMINMAX
#include <Windows.h>
#endif // HAVE_WINDOWS_H

namespace nbe
{

    class DynamicLibrary
    {
    public:
#if defined(__linux__)
        typedef void* Handle;
#elif defined(_WIN32)
        typedef HINSTANCE Handle;
#endif
    public:
        explicit DynamicLibrary(Handle handle);

        ~DynamicLibrary();

        void* getProcAddress(const char* name);

        static DynamicLibrary* loadLibrary(const std::string& path);
    private:
        Handle _handle{nullptr};
    };

} // nbe
