//
// Created by tony on 28/03/24.
//

#pragma once

#include "config/Export.h"

#include <cstdint>
#include <filesystem>

namespace fs = std::filesystem;

namespace nbe
{
    class KeyGenerator;
    class NodeLibrary;

    class NODEBACKEND_API NodePluginScanner
    {
    public:
        //! Scan the platform-specific plugins directory, 
        //! loading and initialising any plugins found
        //! by registering new Node types with the NodeLibrary.
        //! \note The KeyGenerator can be the same object as the 
        //! NodeLibrary but presenting a narrow interface
        void scan(KeyGenerator& keyGen, NodeLibrary& nodeLib);

        //! \return The total number of nodes found by scanning plugins.
        [[nodiscard]]std::size_t totalNodes() const
        {
            return _totalNodes;
        }
    private:
        std::size_t _totalNodes{0};

        static fs::path pathToPlugins;
        static fs::path pluginExtension;
    };
}