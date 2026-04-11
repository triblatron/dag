//
// Created by tony on 18/02/24.
//

#pragma once

#include "config/Export.h"

#include "Types.h"
#include "Port.h"

#include <cstdint>

namespace dagbase
{
    class InputStream;
    class OutputStream;
}

namespace dag
{
    class NodeLibrary;

    class DAG_API SignalPath
    {
    public:
        enum Flags : std::uint32_t
        {
            FLAGS_NONE = 0x0,
            REMOVED_BIT = 1<<0
        };
    public:
        SignalPath(Port* source, Port* dest)
        :
        _id(_nextID++),
        _source(source),
        _dest(dest),
        _flags(FLAGS_NONE)
        {
            // Do nothing.
        }

        SignalPath(dagbase::InputStream& str, NodeLibrary& nodeLib, dagbase::Lua& lua);

        [[nodiscard]]SignalPathID id() const
        {
            return _id;
        }

        void setSource(Port* source)
        {
            _source = source;
        }

        Port* source()
        {
            return _source;
        }

        void setDest(Port* dest)
        {
            _dest = dest;
        }

        Port* dest()
        {
            return _dest;
        }

        [[nodiscard]]Node* sourceNode()
        {
            if (_source!=nullptr)
            {
                return _source->parent();
            }

            return nullptr;
        }

        [[nodiscard]]Node* destNode()
        {
            if (_dest!=nullptr)
            {
                return _dest->parent();
            }

            return nullptr;
        }

        void markRemoved()
        {
            _flags = static_cast<Flags>(_flags | REMOVED_BIT);
        }

        void clearRemoved()
        {
            _flags = static_cast<Flags>(_flags &~REMOVED_BIT);
        }

        [[nodiscard]]bool isRemoved() const
        {
            return (_flags & REMOVED_BIT)!=0x0;
        }

        std::ostream& toLua(std::ostream& str);

        dagbase::OutputStream& write(dagbase::OutputStream& str) const;
    private:
        SignalPathID _id;
        Port* _source{nullptr};
        Port* _dest{nullptr};
        Flags _flags{0};
        static SignalPathID _nextID;
    };
}
