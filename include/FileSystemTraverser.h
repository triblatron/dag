#pragma once

#include "config/Export.h"

#include <filesystem>
#include <functional>

namespace dag
{
    namespace fs = std::filesystem;

    class DAG_API FileSystemTraverser
    {
    public:
        explicit FileSystemTraverser(fs::path & dir);

        void eachEntry(std::function<void(const fs::directory_entry&)>);
    private:
        fs::path _dir;
    };
}