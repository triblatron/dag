#include "config/config.h"

#include "Port.h"
#include "TypedPort.h"
#include "Node.h"
#include "OutputStream.h"
#include "InputStream.h"
#include "NodeLibrary.h"
#include "DebugPrinter.h"
#include "CloningFacility.h"

namespace nbe
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
                Port *newInput = oldInput->clone(facility, CopyOp{0}, nullptr);
                // Connect the output port to the new input port
                // Disconnect the old input port
                newDest->addDynamicPort(newInput);

                // Create a new output from this, without deep copying inputs and outputs.
                Port *newOutput = this->clone(facility, CopyOp{0}, nullptr);
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

    OutputStream &Port::write(OutputStream &str) const
    {
        str.write(_id);

        if (str.writeRef(_metaPort))
        {
            _metaPort->write(str);
        }

        if (str.writeRef(_parent))
        {
            std::string className = _parent->className();
            str.write(className);
            _parent->write(str);
        }

        str.write(_outgoingConnections.size());
        for (auto c: _outgoingConnections)
        {
            if (str.writeRef(c))
            {
                c->write(str);
            }
        }

        str.write(_incomingConnections.size());
        for (auto c : _incomingConnections)
        {
            if (str.writeRef(c))
            {
                c->write(str);
            }
        }

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

    void Port::debug(DebugPrinter &printer) const
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

    Port::Port(InputStream &str, NodeLibrary& nodeLib)
    :
    _metaPort(nullptr),
    _parent(nullptr)
    {
        str.addObj(this);
        str.read(&_id);
        Stream::ObjId metaPortId = 0;
        _metaPort = str.readRef<MetaPort>(&metaPortId);
        setOwnMetaPort(true);
        Stream::ObjId parentId = 0;
        _parent = str.readRef<Node>(&parentId, nodeLib);
        size_t numOutgoingConnections = 0;
        str.read(&numOutgoingConnections);
        for (auto i=0; i<numOutgoingConnections; ++i)
        {
            Port* port = str.readPort(nodeLib);
            if (port!=nullptr)
            {
                addOutgoingConnection(port);
            }
        }
        std::size_t numIncomingConnections = 0;
        str.read(&numIncomingConnections);
        for (auto i=0; i<numIncomingConnections; ++i)
        {
            Port* port = str.readPort(nodeLib);
            if (port != nullptr)
            {
                addIncomingConnection(port);
            }

        }
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

    Transfer* VariantPort::setDestination(nbe::Transfer *transfer)
    {
        if (auto typedTransfer = dynamic_cast<VariantPortTransfer*>(transfer); typedTransfer!=nullptr)
        {
            typedTransfer->setDest(this);

            return typedTransfer;
        }

        return nullptr;
    }

    VariantPort::VariantPort(InputStream &str, NodeLibrary &nodeLib)
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
                str.read(&value);
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

    ValuePort::ValuePort(InputStream &str, NodeLibrary &nodeLib)
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
                str.read(&value);
                _value = value;
            }
            case PortType::TYPE_BOOL:
            {
                bool value = false;
                str.read(&value);
                _value = value;
            }
        }
    }


}
