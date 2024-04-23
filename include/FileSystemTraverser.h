#pragma once

#include "config/Export.h"

#include <filesystem>
#include <functional>

namespace nbe
{
    namespace fs = std::filesystem;

    class NODEBACKEND_API FileSystemTraverser
    {
    public:
        explicit FileSystemTraverser(fs::path & dir);

        void eachEntry(std::function<void(const fs::directory_entry&)>);
    private:
        fs::path _dir;
    };
}