#include "config/config.h"

#include "FileSystemTraverser.h"

namespace nbe
{
    FileSystemTraverser::FileSystemTraverser(fs::path &dir)
    :
    _dir(dir)
    {

    }

    void FileSystemTraverser::eachEntry(std::function<void(const fs::directory_entry &)> f)
    {
        if (fs::exists(_dir) && fs::is_directory(_dir))
        {
            for (const auto& entry : fs::directory_iterator(_dir) )
            {
                f(entry);
            }
        }
    }
}
