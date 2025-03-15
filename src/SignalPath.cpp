//
// Created by tony on 18/02/24.
//
#include "config/config.h"

#include "SignalPath.h"
#include "io/InputStream.h"
#include "TypedPort.h"
#include "NodeLibrary.h"
#include "io/OutputStream.h"
#include "Node.h"

namespace dag
{
    SignalPathID SignalPath::_nextID = 0;

    dagbase::OutputStream &SignalPath::write(dagbase::OutputStream &str) const
    {
        str.writeHeader("SignalPath");
        str.writeField("id");
        str.writeInt64(_id);
        str.writeField("source");
        if (str.writeRef(_source))
        {
            _source->write(str);
        }
        str.writeField("dest");
        if (str.writeRef(_dest))
        {
            _dest->write(str);
        }
        str.writeField("flags");
        str.writeUInt32(_flags);
        str.writeFooter();

        return str;
    }


    SignalPath::SignalPath(dagbase::InputStream &str, NodeLibrary& nodeLib)
    :
    _source(nullptr),
    _dest(nullptr),
    _flags(0x0)
    {
        std::string className;
        std::string fieldName;
        str.readHeader(&className);
        str.readField(&fieldName);
        std::int64_t id{0};
        str.read(&id);
        _id = id;
        str.readField(&fieldName);
        _source = str.readRef<Port>("Port", nodeLib);
        str.readField(&fieldName);
        _dest = str.readRef<Port>("Port", nodeLib);
        str.readField(&fieldName);
        str.readUInt32(&_flags);
        str.readFooter();
    }

    std::ostream &SignalPath::toLua(std::ostream &str)
    {
        str << "{ ";
        str << "sourceNode = \"" << _source->parent()->name() << "\", ";
        str << "sourcePort = " << _source->parent()->indexOfPort(_source) << ", ";
        str << "destNode = \"" << _dest->parent()->name() << "\", ";
        str << "destPort = " << _dest->parent()->indexOfPort(_dest);
        str << " }";

        return str;
    }
}
