//
// Created by tony on 28/03/24.
//
#include "config/config.h"

#include "NodePluginScanner.h"
#include "FileSystemTraverser.h"
#include "NodeLibrary.h"

#include "DynamicLibrary.h"

namespace nbe
{
#if defined(__linux__)
    fs::path NodePluginScanner::pathToPlugins = "lib/NodePlugins";
    fs::path NodePluginScanner::pluginExtension = ".so";
#elif defined(_WIN32)
    fs::path NodePluginScanner::pathToPlugins = "bin/NodePlugins";
    fs::path NodePluginScanner::pluginExtension = ".dll";
#else
#error "Your platform is not supported by NodePluginScanner";
#endif
    typedef void (*InitFunc)(KeyGenerator&, NodeLibrary&);

    void NodePluginScanner::scan(KeyGenerator& keyGen, NodeLibrary& nodeLib)
    {
        FileSystemTraverser trav(pathToPlugins);

        trav.eachEntry([this, &keyGen, &nodeLib](const fs::directory_entry& entry)
                       {
                           if (entry.is_regular_file() && entry.path().extension() == pluginExtension)
                           {
                               DynamicLibrary* lib = DynamicLibrary::loadLibrary(entry.path().string());
                               if (lib != nullptr)
                               {
                                   InitFunc initFunc = reinterpret_cast<InitFunc>(lib->getProcAddress("init"));

                                   if (initFunc != nullptr)
                                   {
                                       std::size_t numNodesBefore = nodeLib.numNodes();
                                       (*initFunc)(keyGen, nodeLib);
                                       _totalNodes += nodeLib.numNodes() - numNodesBefore;
                                   }
                               }
                           }
                       });
    }
}