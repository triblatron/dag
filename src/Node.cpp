#include "config/config.h"

#include "Node.h"
#include "NodeDescriptor.h"
#include "io/OutputStream.h"
#include "io/InputStream.h"
#include "util/DebugPrinter.h"
#include "NodeLibrary.h"
#include "KeyGenerator.h"
#include "CloningFacility.h"

#include <iostream>

namespace dag
{
    Node::Node(KeyGenerator& keyGen, std::string name, NodeCategory::Category category)
    :
    _id(keyGen.nextNodeID()),
    _name(std::move(name)),
    _category(category)
    {
        // Do nothing.
    }

    Node::Node(const Node& other, CloningFacility& facility, CopyOp copyOp, KeyGenerator* keyGen)
    :
    _id(other._id),
    _name(other._name),
    _category(other._category)
    {
        std::uint64_t otherId = 0;
        bool shouldClone = facility.putOrig(const_cast<Node*>(&other), &otherId);
        facility.addClone(otherId, this);
        if ((copyOp & CopyOp::GENERATE_UNIQUE_ID_BIT)!=0x0 && keyGen!=nullptr)
        {
            _id = keyGen->nextNodeID();
        }
    }

    //! Include ID, name and category.
    void Node::describe(NodeDescriptor& descriptor) const
    {
        descriptor.id = _id;
        descriptor.name = _name;
        descriptor.category = _category;
    }

    //! Reconnect from newSource to each input Port whose parent is in selection by adding new Ports.
    void Node::reconnectInputs(NodeSet const& selection, Node *newSource)
    {
        // for each output port do
        for (auto portIndex = 0; portIndex < totalPorts(); ++portIndex)
        {
            Port* p = dynamicPort(portIndex);
            if (p->dir() == PortDirection::DIR_IN)
            {
                p->reconnectFrom(selection, newSource);
            }

        }
    }

    //! Reconnect from each output Port whose parent is in selection to newDest by adding new Ports.
    void Node::reconnectOutputs(NodeSet const& selection, Node *newDest)
    {
        // for each output port do
        for (auto portIndex=0; portIndex<totalPorts(); ++portIndex)
        {
            Port* p = dynamicPort(portIndex);
            if (p->dir() == PortDirection::DIR_OUT)
            {
                p->reconnectTo(selection, newDest);
            }

        }

    }

    dagbase::OutputStream &Node::write(dagbase::OutputStream &str) const
    {
        str.writeHeader("Node");
        str.writeField("id");
        str.writeInt64(_id);
        str.writeField("name");
        str.writeString(_name, true);
        str.writeField("category");
        str.writeUInt32(_category);
        str.writeField("numDynamicPortDescriptors");
        str.writeUInt32(_dynamicPortDescriptors.size());
        for (auto const & d : _dynamicPortDescriptors)
        {
            str.writeField("id");
            str.write(d.id);
            str.writeField("name");
            str.writeString(d.name, false);
            str.writeField("direction");
            str.write(d.direction);
        }
        str.writeField("flags");
        str.writeUInt32(_flags);
        str.writeFooter();

        return str;
    }

    Node::Node(dagbase::InputStream &str, NodeLibrary& nodeLib)
    {
        Node::readFromStream(str, nodeLib);
    }

    dagbase::InputStream& Node::readFromStream(dagbase::InputStream& str, dag::NodeLibrary& nodeLib)
    {
        std::string className;
        str.readHeader(&className);
        str.addObj(this);
        std::string fieldName;
        str.readField(&fieldName);
        std::int64_t id{0};
        str.readInt64(&id);
        _id = id;
        str.readField(&fieldName);
        str.readString(&_name, true);
        str.readField(&fieldName);
        std::uint32_t category{0};
        str.readUInt32(&category);
        _category = static_cast<NodeCategory::Category>(category);
        str.readField(&fieldName);
        std::uint32_t numDescriptors = 0;
        str.readUInt32(&numDescriptors);
        for (auto i=0; i<numDescriptors; ++i)
        {
            PortDescriptor d;
            str.readField(&fieldName);
            std::uint32_t portId{0};
            str.readUInt32(&portId);
            d.id = portId;
            str.readField(&fieldName);
            str.readString(&d.name, false);
            str.readField(&fieldName);
            std::uint32_t direction;
            str.read(&direction);
            d.direction = static_cast<PortDirection::Direction>(direction);
            _dynamicPortDescriptors.push_back(d);
        }
        str.readField(&fieldName);
        std::uint32_t flags{0};
        str.readUInt32(&flags);
        _flags = static_cast<Node::NodeFlags>(flags);
        str.readFooter();
        return str;
    }

    bool Node::operator==(const Node &other) const
    {
        if (_id != other._id)
        {
            return false;
        }

        if (_name != other._name)
        {
            return false;
        }

        if (_category != other._category)
        {
            return false;
        }

        if (_dynamicPortDescriptors.size() != other._dynamicPortDescriptors.size())
        {
            return false;
        }

        if (!(_dynamicPortDescriptors==other._dynamicPortDescriptors))
        {
            return false;
        }

        if (_flags != other._flags)
        {
            return false;
        }

        return true;
    }

    void Node::debug(dagbase::DebugPrinter& printer) const
    {
        printer.println("id: " + std::to_string(_id));
        printer.println("name: " + _name);
        printer.println("category: " + std::to_string(_category));
        printer.println("numDynamicPortDescriptors: " + std::to_string(_dynamicPortDescriptors.size()));
        printer.println("flags: " + std::to_string(_flags));
    }

    std::ostream &Node::toLua(std::ostream &str)
    {
        str << "{ ";
        str << "id = " << _id << ", ";
        str << "class = \"" << className() << "\", ";
        str << "name = \"" << _name << "\", ";
        str << "category = \"" << NodeCategory::toString(_category) << "\", ";
        str << "numDynamicPortDescriptors = " << _dynamicPortDescriptors.size() << ", ";
        str << "flags = " << _flags << ", ";
        str << "ports = { ";
        for (size_t i=0; i<totalPorts(); ++i)
        {
            str << "{ ";
            dynamicPort(i)->toLua(str);
            str << " }, ";
        }
        str << " }";
        str << " }";

        return str;
    }
}
