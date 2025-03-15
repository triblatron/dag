#include "config/config.h"

#include "Port.h"
#include "TypedPort.h"
#include "Node.h"
#include "io/OutputStream.h"
#include "io/InputStream.h"
#include "NodeLibrary.h"
#include "util/DebugPrinter.h"
#include "CloningFacility.h"

namespace dag
{
    Port::Port(PortID id, Node *parent, MetaPort *metaPort, std::uint32_t flags)
            :
            _id(id),
            _metaPort(metaPort),
            _parent(parent),
            _flags(flags)
    {
        // Do nothing.
    }

    Port::~Port()
    {
        eachOutgoingConnection([this](Port *port)
                               {
                                   port->removeIncomingConnection(this);
                               });
        eachIncomingConnection([this](Port *port)
                               {
                                   port->removeOutgoingConnection(this);
                               });
        if (ownMetaPort())
        {
            delete _metaPort;
        }

//        if (_flags & OWN_INPUTS_BIT)
//        {
//            for (auto input : _incomingConnections)
//            {
//                delete input;
//            }
//        }
//
//        if (_flags & OWN_OUTPUTS_BIT)
//        {
//            for (auto output : _outgoingConnections)
//            {
//                delete output;
//            }
//        }
    }

    Port::Port(const Port &other, CloningFacility& facility, CopyOp copyOp, KeyGenerator* keyGen)
            :
            _id(other._id),
            _metaPort(new MetaPort(*other._metaPort)),
            _parent(other._parent),
            _flags(other._flags|OWN_META_PORT_BIT)
    {
        std::uint64_t otherId = 0;
        bool shouldClone = facility.putOrig(const_cast<Port*>(&other), &otherId);
        facility.addClone(otherId, this);
        if (copyOp & CopyOp::DEEP_COPY_INPUTS_BIT)
        {
            setFlag(OWN_INPUTS_BIT);
            for (auto it=other._incomingConnections.begin(); it!=other._incomingConnections.end(); ++it)
            {
                std::uint64_t connectionId = 0;
                Port* connection = nullptr;

                if (facility.putOrig(*it, &connectionId))
                {
                    connection = (*it)->clone(facility, copyOp, keyGen);
                }
                else
                {
                    connection = static_cast<Port*>(facility.getClone(connectionId));
                }
                _incomingConnections.push_back(connection);
            }
        }

        if (copyOp & CopyOp::DEEP_COPY_OUTPUTS_BIT)
        {
            setFlag(OWN_OUTPUTS_BIT);
            for (auto it=other._outgoingConnections.begin(); it!=other._outgoingConnections.end(); ++it)
            {
                std::uint64_t connectionId = 0;
                Port* connection = nullptr;

                if (facility.putOrig(*it, &connectionId))
                {
                    connection = (*it)->clone(facility, copyOp, keyGen);
                }
                else
                {
                    connection = static_cast<Port*>(facility.getClone(connectionId));
                }
                _outgoingConnections.push_back(connection);
            }
        }

        if (copyOp & CopyOp::DEEP_COPY_PARENT_BIT)
        {
            std::uint64_t parentId = 0;
            if (facility.putOrig(other._parent,&parentId))
            {
                _parent = other._parent->clone(facility,copyOp,keyGen);
            }
            else
            {
                _parent = static_cast<Node*>(facility.getClone(parentId));
            }
        }
    }

    //! Reconnect to nodes of our output connections that are in the selection by adding new Ports
    //! on the new destination Node.
    void Port::reconnectTo(NodeSet const &selection, Node *newDest)
    {
        //   if the destination input port has a parent of oldDest then
        CloningFacility facility;

        for (auto &oldInput: _outgoingConnections)
        {
            if (auto it = selection.find(oldInput->parent()); it == selection.end())
            {
                // Create a new input port in newDest, without deep copying inputs and outputs.
                Port *newInput = nullptr;
                std::uint64_t newInputId = 0;
                newInput = oldInput->clone(facility, CopyOp{0}, nullptr);
                // Connect the output port to the new input port
                // Disconnect the old input port
                newDest->addDynamicPort(newInput);

                // Create a new output from this, without deep copying inputs and outputs.
                Port *newOutput = nullptr;
                std::uint64_t newOutputId = 0;

                newOutput = this->clone(facility, CopyOp{0}, nullptr);
                newDest->addDynamicPort(newOutput);
                newOutput->_outgoingConnections.push_back(oldInput);
                auto itOld = oldInput->findIncomingConnection(*this);
                if (itOld != oldInput->_incomingConnections.end())
                {
                    (*itOld) = newOutput;
                }
                oldInput = newInput;

            }
        }

    }

    //! Reconnect from nodes of our incoming connections that are in the selection by  adding Ports on the new source
    //! \note newSource is typically a Boundary node used during an AddChild operation.
    void Port::reconnectFrom(NodeSet const &selection, Node *newSource)
    {
        CloningFacility facility;
        for (auto &oldOutput: _incomingConnections)
        {
            if (auto it = selection.find(oldOutput->parent()); it == selection.end())
            {
                Port *newOutput = oldOutput->clone(facility, CopyOp{0}, nullptr);
                newSource->addDynamicPort(newOutput);

                Port *newInput = this->clone(facility, CopyOp{0}, nullptr);
                newSource->addDynamicPort(newInput);
                newInput->_incomingConnections.push_back(oldOutput);
                if (auto itOld = oldOutput->findOutgoingConnection(*this); itOld != _outgoingConnections.end())
                {
                    (*itOld) = newInput;
                }
                oldOutput = newOutput;
            }
        }
    }

    dagbase::OutputStream &Port::write(dagbase::OutputStream &str) const
    {
        str.writeHeader("Port");
        str.writeField("id");
        str.writeInt64(_id);

        str.writeField("metaPort");
        if (str.writeRef(_metaPort))
        {
            _metaPort->write(str);
        }

        str.writeField("parent");
        if (str.writeRef(_parent))
        {
            std::string className = _parent->className();
            str.writeField("className");
            str.writeString(className, false);
            _parent->write(str);
        }
        str.writeField("numOutgoingConnections");
        str.writeUInt32(_outgoingConnections.size());
        str.writeField("outgoingConnections");
        for (auto c: _outgoingConnections)
        {
            if (str.writeRef(c))
            {
                c->write(str);
            }
        }

        str.writeField("numIncomingConnections");
        str.writeUInt32(_incomingConnections.size());
        str.writeField("incomingConnections");
        for (auto c : _incomingConnections)
        {
            if (str.writeRef(c))
            {
                c->write(str);
            }
        }
        str.writeFooter();
        return str;
    }

    std::ostream& Port::toLua(std::ostream &str)
    {
        str << "id = " << _id << ", ";
        _metaPort->toLua(str);
        str << "class = \"" << className() << "\", ";
        if (_parent!=nullptr)
        {
            str << "parent = \"" << _parent->name() << "\", ";
        }
        else
        {
            str << "parent = nil, ";
        }

        return str;
    }

    void Port::debug(dagbase::DebugPrinter& printer) const
    {
        printer.println("id: " + std::to_string(_id));
        printer.println("metaPort:");
        if (_metaPort!=nullptr)
        {
            printer.println("{");
            printer.indent();
            _metaPort->debug(printer);
            printer.outdent();
            printer.println("}");
        }
        if (_parent!=nullptr)
        {
            printer.println("parent:" + _parent->name());
        }
        else
        {
            printer.println("parent:\"<none>\"");
        }
        printer.println("remove: " + std::to_string(_removed));
        printer.println("value:");
        //printer.print(_value);
    }

    Port::Port(dagbase::InputStream &str, NodeLibrary& nodeLib)
    :
    _metaPort(nullptr),
    _parent(nullptr)
    {
        readFromStream(str, nodeLib);
    }

    dagbase::InputStream& Port::readFromStream(dagbase::InputStream& str, NodeLibrary& nodeLib)
    {
        std::string className;
        std::string fieldName;
        str.readHeader(&className);
        str.readField(&fieldName);
        str.addObj(this);
        std::int64_t id{0};
        str.readInt64(&id);
        _id = id;
        dagbase::Stream::ObjId metaPortId = 0;
        str.readField(&fieldName);
        _metaPort = str.readRef<MetaPort>(&metaPortId);
        setOwnMetaPort(true);
        //        dagbase::Stream::ObjId parentId = 0;
        str.readField(&fieldName);
        _parent = str.readRef<Node>("Node", nodeLib);
        std::uint32_t numOutgoingConnections = 0;
        str.readField(&fieldName);
        str.readUInt32(&numOutgoingConnections);
        str.readField(&fieldName);
        for (auto i=0; i<numOutgoingConnections; ++i)
        {
            Port* port = str.readRef<Port>("Port",nodeLib);
            if (port!=nullptr)
            {
                addOutgoingConnection(port);
            }
        }
        std::uint32_t numIncomingConnections = 0;
        str.readField(&fieldName);
        str.readUInt32(&numIncomingConnections);
        str.readField(&fieldName);
        for (auto i=0; i<numIncomingConnections; ++i)
        {
            Port* port = str.readRef<Port>("Port",nodeLib);
            if (port != nullptr)
            {
                addIncomingConnection(port);
            }

        }

        return str;
    }

    bool Port::operator==(const Port &other) const
    {
        if ((_metaPort != nullptr && other._metaPort == nullptr) || (_metaPort == nullptr && other._metaPort != nullptr))
        {
            return false;
        }
        if (_metaPort != nullptr && !(*_metaPort == *other._metaPort))
        {
            return false;
        }

        if (_parent!=nullptr && other._parent!=nullptr && _parent->id() != other._parent->id())
        {
            return false;
        }

        if (_outgoingConnections.size() != other._outgoingConnections.size())
        {
            return false;
        }

        for (auto it=_outgoingConnections.begin(); it!=_outgoingConnections.end(); ++it)
        {
            auto it2 = other._outgoingConnections.begin() + std::distance(_outgoingConnections.begin(), it);

            if ((*it)->id() != (*it2)->id())
            {
                return false;
            }
        }

        if (_incomingConnections.size() != other._incomingConnections.size())
        {
            return false;
        }

        for (auto it=_incomingConnections.begin(); it!=_incomingConnections.end(); ++it)
        {
            auto it2 = other._incomingConnections.begin() + std::distance(_incomingConnections.begin(), it);

            if ((*it)->id() != (*it2)->id())
            {
                return false;
            }
        }

        return true;
    }

/*
    TypedPortBase::TypedPortBase(Node* parent, MetaPort* metaPort)
		:
		Port(parent, metaPort)
	{
		// Do nothing.
	}
*/
    Transfer* VariantPort::connectTo(Port& dest)
    {
        if (dir() == PortDirection::DIR_OUT && dest.dir() == PortDirection::DIR_IN && isCompatibleWith(dest))
        {
            auto transfer = new VariantPortTransfer(this);

            dest.setDestination(transfer);

            addOutgoingConnection(&dest);
            dest.addIncomingConnection(this);

            return transfer;
        }
        
        return nullptr;
    }

    Transfer* VariantPort::setDestination(dag::Transfer *transfer)
    {
        if (auto typedTransfer = dynamic_cast<VariantPortTransfer*>(transfer); typedTransfer!=nullptr)
        {
            typedTransfer->setDest(this);

            return typedTransfer;
        }

        return nullptr;
    }

    VariantPort::VariantPort(dagbase::InputStream &str, NodeLibrary &nodeLib)
    :
    Port(str, nodeLib)
    {
        switch (type())
        {
            case PortType::TYPE_INT:
            {
                std::int64_t value = 0;
                str.read(&value);
                _value = value;
            }
            case PortType::TYPE_DOUBLE:
            {
                double value = 0.0;
                str.read(&value);
                _value = value;
            }
            case PortType::TYPE_STRING:
            {
                std::string value;
                str.readString(&value, false);
                _value = value;
            }
            case PortType::TYPE_BOOL:
            {
                bool value = false;
                str.read(&value);
                _value = value;
            }
            default:
                assert(false);
        }
    }

    Transfer* ValuePort::connectTo(Port& dest)
    {
        if (dir() == PortDirection::DIR_OUT && dest.dir() == PortDirection::DIR_IN && isCompatibleWith(dest))
        {
            auto transfer = new PortTransfer(this);
            dest.setDestination(transfer);

            addOutgoingConnection(&dest);
            dest.addIncomingConnection(this);

            return transfer;
        }

        return nullptr;
    }

    Transfer* ValuePort::setDestination(Transfer* transfer)
    {
        if (auto typedTransfer = dynamic_cast<PortTransfer*>(transfer); typedTransfer != nullptr)
        {
            typedTransfer->setDest(this);
        }

        return transfer;
    }

    ValuePort::ValuePort(dagbase::InputStream &str, NodeLibrary &nodeLib)
    :
    Port(str, nodeLib)
    {
        switch (type())
        {
            case PortType::TYPE_INT:
            {
                std::int64_t value = 0;
                str.read(&value);
                _value = value;
            }
            case PortType::TYPE_DOUBLE:
            {
                double value = 0.0;
                str.read(&value);
                _value = value;
            }
            case PortType::TYPE_STRING:
            {
                std::string value;
                str.readString(&value, false);
                _value = value;
            }
            case PortType::TYPE_BOOL:
            {
                bool value = false;
                str.read(&value);
                _value = value;
            }
            case PortType::TYPE_VEC3D: {
                
                break;
            }
            case PortType::TYPE_OPAQUE: {
                
                break;
            }
            case PortType::TYPE_VECTOR: {
                
                break;
            }
            case PortType::TYPE_UNKNOWN: {
                
                break;
            }
        }
    }


}
