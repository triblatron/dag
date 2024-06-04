//
// Created by tony on 03/06/24.
//

#pragma once

#include "config/Export.h"
#include "Types.h"

#include <string>

namespace nbe
{
    class NodeEditorFrontEnd;

    class NBE_API NodeEditorBackEnd
    {
    public:
        virtual ~NodeEditorBackEnd() = default;

        virtual void createNode(const std::string& className, const std::string& name) = 0;

        virtual void deleteNode(const std::string& name) = 0;

        virtual void addPort(const std::string& nodeName, const std::string& portClass, PortType type, PortDirection direction) = 0;

        virtual void deletePort(const std::string& nodeName, size_t portIndex) = 0;
    };
}
