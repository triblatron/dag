#pragma once

#include "config/Export.h"
#include <cstdint>
#include "Types.h"

namespace nbe
{
    struct NBE_API Endpoint
    {
        PortID port;
        NodeID node;
    };

    struct NBE_API SignalPathDef
    {
        SignalPathID id;
        Endpoint source;
        Endpoint dest;
        bool removed{false};

        SignalPathDef() = default;

        SignalPathDef(NodeID fromNode, PortID fromPort, NodeID toNode, PortID toPort)
	        :
        id(_nextID++)
        {
            source.node = fromNode;
            source.port = fromPort;
            dest.node = toNode;
            dest.port = toPort;
        }
    private:
        static std::int64_t _nextID;
    };
}
