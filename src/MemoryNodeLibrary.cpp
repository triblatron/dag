#include "config/config.h"

#include "MemoryNodeLibrary.h"
#include "Node.h"
#include "TypedPort.h"

#include <stdexcept>

#include "io/InputStream.h"
#include "io/OutputStream.h"
#include "Nodes.h"
#include "Boundary.h"
#include "MathNode.h"
#include "CloningFacility.h"

#include <cstring>

namespace dag
{
    MemoryNodeLibrary::MemoryNodeLibrary()
	    :
    NodeLibrary()
    {
        _classes["Foo"] = new Foo(*this, "foo1", dag::NodeCategory::CAT_SINK);
        _classes["Bar"] = new Bar(*this, "bar1", dag::NodeCategory::CAT_SOURCE);
        _classes["FooTyped"] = new FooTyped(*this, "footyped1", dag::NodeCategory::CAT_SINK);
        _classes["BarTyped"] = new BarTyped(*this, "bartyped1", dag::NodeCategory::CAT_SOURCE);
        _classes["GroupTyped"] = new GroupTyped(*this, "grouptyped1", dag::NodeCategory::CAT_GROUP);
        _classes["Base"] = new Base(*this, "base1", dag::NodeCategory::CAT_SOURCE);
        _classes["Derived"] = new Derived(*this, "derived1", dag::NodeCategory::CAT_CONDITION);
        _classes["Final"] = new Final(*this, "final1", dag::NodeCategory::CAT_GROUP);
        _classes["Boundary"] = new Boundary(*this, "b1", dag::NodeCategory::CAT_GROUP);
        _classes["MathsNode"] = new MathsNode(*this, "maths1", dag::NodeCategory::CAT_ACTION);
    }

    MemoryNodeLibrary::~MemoryNodeLibrary()
	{
		for (auto it : _classes)
		{
			delete it.second;
		}
	}
	
    Node* MemoryNodeLibrary::instantiateNode(NodeID id, const std::string& className, const std::string& name)
    {
        CloningFacility facility;

	    if (auto const it = _classes.find(className); it != _classes.end() )
        {
	        const auto copy = it->second->clone(facility, CopyOp{0}, nullptr);
            copy->setId(id);
            copy->setName(name);

            return copy;
        }
        else
        {
            throw std::runtime_error("Unknown class \"" + className + "\"");
        }
        return nullptr;
    }

//    OutputStream &MemoryNodeLibrary::write(OutputStream &str) const
//    {
//        str.write(_classes.size());
//        for (auto p : _classes)
//        {
//            str.write(p.first);
//            if (str.writeRef(p.second))
//            {
//                p.second->write(str);
//            }
//        }
//        return str;
//    }

    Node *MemoryNodeLibrary::instantiateNode(dagbase::InputStream &str)
    {
        std::string className;
        str.readString(&className, false);
        if (auto it=_classes.find(className); it!=_classes.end())
        {
            return it->second->create(str, *this);
        }
        return nullptr;
    }

    Port *
    MemoryNodeLibrary::instantiatePort(const std::string &className, const std::string& name, PortType::Type type, PortDirection::Direction dir,
                                       Value value)
    {
        if (className == "ValuePort")
        {
            return new ValuePort(nextPortID(), name, type, dir, value);
        }
        else if (className == "TypedPort")
        {
            switch (type)
            {
                case PortType::TYPE_INT:
                    return new TypedPort<std::int64_t>(nextPortID(), name, type, dir, static_cast<std::int64_t>(value));
                case PortType::TYPE_DOUBLE:
                    return new TypedPort<double>(nextPortID(), name, type, dir, static_cast<double>(value));
                case PortType::TYPE_STRING:
                    return new TypedPort<std::string>(nextPortID(), name, type, dir, value.operator std::string());
                case PortType::TYPE_BOOL:
                    return new TypedPort<bool>(nextPortID(), name, type, dir, static_cast<bool>(value));
                default:
                    assert(false);
            }
        }

        return nullptr;
    }

    Port *MemoryNodeLibrary::instantiatePort(dagbase::InputStream &str)
    {
        std::string className;
        str.readString(&className, false);
        if (className == "ValuePort")
        {
            return new ValuePort(str, *this);
        }
        else if (className == "TypedPort<int64_t>")
        {
            return new TypedPort<std::int64_t>(str, *this);
        }
        else if (className == "TypedPort<double>")
        {
            return new TypedPort<double>(str, *this);
        }
        else if (className == "TypedPort<string>")
        {
            return new TypedPort<std::string>(str, *this);
        }
        else if (className == "TypedPort<bool>")
        {
            return new TypedPort<bool>(str, *this);
        }

        return nullptr;
//        std::string name;
//        str.read(&name);
//        PortType::Type type = PortType::TYPE_UNKNOWN;
//        str.read(&type);
//        PortDirection::Direction dir = PortDirection::DIR_UNKNOWN;
//        str.read(&dir);
//        Value value;
//        value.read(str);
//        return instantaatePort(className, name, type, dir, value);
    }

    dagbase::Class* MemoryNodeLibrary::instantiate(const char* baseClassName, dagbase::InputStream& str)
    {
        if (std::strcmp(baseClassName, "Node") == 0)
        {
            return instantiateNode(str);
        }
        else if (std::strcmp(baseClassName, "Port") == 0)
        {
            return instantiatePort(str);
        }

        return nullptr;
    }

    dagbase::OutputStream &MemoryNodeLibrary::write(dagbase::OutputStream& str,  Node *node) const
    {
        std::string className = node->className();
        str.writeString(className, false);
        node->write(str);
        
        return str;
    }

    void MemoryNodeLibrary::registerNode(Node *node)
    {
        if (node != nullptr)
        {
            std::string className = node->className();
            if (_classes.find(className)==_classes.end())
            {
                _classes.insert(PrototypeMap::value_type(className, node));
            }
        }
    }
/*
    MemoryNodeLibrary::MemoryNodeLibrary(InputStream &str)
    {
        size_t len = 0;
        str.read(&len);
        for (auto i=0; i<len; ++i)
        {
            std::string className;
            str.read(&className);
            auto n = str.readRef<Nod
        }
    }
*/

}
