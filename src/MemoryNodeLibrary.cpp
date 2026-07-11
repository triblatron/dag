#include "config/config.h"

#include "MemoryNodeLibrary.h"
#include "core/Node.h"
#include "core/TypedPort.h"
#include "core/CloningFacility.h"

#include <stdexcept>

#include "io/InputStream.h"
#include "io/OutputStream.h"
#include "Nodes.h"
#include "Boundary.h"
#include "MathNode.h"

#include <cstring>

#include "core/GraphNode.h"

namespace dag
{
    MemoryNodeLibrary::MemoryNodeLibrary()
	    :
    NodeLibrary()
    {
        _classes.emplace("FooTyped", new FooTyped(*this, "footyped1", dagbase::NodeCategory::CAT_SINK));
        _classes.emplace("BarTyped", new BarTyped(*this, "bartyped1", dagbase::NodeCategory::CAT_SOURCE));
        _classes.emplace("GroupTyped", new GroupTyped(*this, "grouptyped1", dagbase::NodeCategory::CAT_GROUP));
        _classes.emplace("Base", new Base(*this, "base1", dagbase::NodeCategory::CAT_SOURCE));
        _classes.emplace("Derived", new Derived(*this, "derived1", dagbase::NodeCategory::CAT_CONDITION));
        _classes.emplace("Final", new Final(*this, "final1", dagbase::NodeCategory::CAT_GROUP));
        _classes.emplace("Boundary", new Boundary(*this, "b1", dagbase::NodeCategory::CAT_GROUP));
        _classes.emplace("MathsNode", new MathsNode(*this, "maths1", dagbase::NodeCategory::CAT_ACTION));
        _classes.emplace("GraphNode", new dagbase::GraphNode(*this, "graph1", dagbase::NodeCategory::CAT_GROUP));
    }

    MemoryNodeLibrary::~MemoryNodeLibrary()
	{
		for (auto it : _classes)
		{
			delete it.second;
		}
	}
	
    void MemoryNodeLibrary::eachNode(std::function<bool(dagbase::Node&)> f)
    {
        for (auto c : _classes)
        {
            if (!f || !f(*c.second))
                break;
        }
    }

    dagbase::Node* dag::MemoryNodeLibrary::instantiateNode(dagbase::KeyGenerator& keyGen, const std::string& className, const std::string& name)
    {
        dagbase::CloningFacility facility;

	    if (auto const it = _classes.m.find(className); it != _classes.end() )
        {
	        const auto copy = it->second->clone(facility, dagbase::GENERATE_UNIQUE_ID_BIT, &keyGen);
            copy->setName(name);

            return copy;
        }

        throw std::runtime_error("Unknown class \"" + className + "\"");
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

    dagbase::Node *MemoryNodeLibrary::instantiateNode(dagbase::InputStream &str, dagbase::Lua &lua)
    {
        std::string className;
        std::string fieldName;
        str.readField(&fieldName);
        str.readString(&className, true);
        if (auto it=_classes.m.find(className); it!=_classes.end())
        {
            return it->second->create(str, *this, lua);
        }
        return nullptr;
    }

    dagbase::Port *
    MemoryNodeLibrary::instantiatePort(const std::string &className, const std::string& name, dagbase::PortType::Type type, dagbase::PortDirection::Direction dir,
                                       dagbase::Value value)
    {
        if (className == "TypedPort")
        {
            switch (type)
            {
                case dagbase::PortType::TYPE_INT64:
                    return new dagbase::TypedPort<std::int64_t>(nextPortID(), name, type, dir, static_cast<std::int64_t>(value));
                case dagbase::PortType::TYPE_DOUBLE:
                    return new dagbase::TypedPort<double>(nextPortID(), name, type, dir, static_cast<double>(value));
                case dagbase::PortType::TYPE_STRING:
                    return new dagbase::TypedPort<std::string>(nextPortID(), name, type, dir, value.operator std::string());
                case dagbase::PortType::TYPE_BOOL:
                    return new dagbase::TypedPort<bool>(nextPortID(), name, type, dir, static_cast<bool>(value));
                default:
                    assert(false);
            }
        }

        return nullptr;
    }

    dagbase::Port *MemoryNodeLibrary::instantiatePort(dagbase::InputStream &str, dagbase::Lua &lua)
    {
        std::string className;
        std::string fieldName;
        str.readField(&fieldName);
        str.readString(&className, true);

        if (className == "TypedPort<int64_t>")
        {
            return new dagbase::TypedPort<std::int64_t>(str, *this, lua);
        }
        else if (className == "TypedPort<double>")
        {
            return new dagbase::TypedPort<double>(str, *this, lua);
        }
        else if (className == "TypedPort<string>")
        {
            return new dagbase::TypedPort<std::string>(str, *this, lua);
        }
        else if (className == "TypedPort<bool>")
        {
            return new dagbase::TypedPort<bool>(str, *this, lua);
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

    dagbase::Class* MemoryNodeLibrary::instantiate(const char* baseClassName, dagbase::InputStream& str, dagbase::Lua& lua)
    {
        if (std::strcmp(baseClassName, "Node") == 0)
        {
            return instantiateNode(str, lua);
        }
        else if (std::strcmp(baseClassName, "Port") == 0)
        {
            return instantiatePort(str, lua);
        }

        return nullptr;
    }

    dagbase::Variant MemoryNodeLibrary::find(std::string_view path) const
    {
        dagbase::Variant retval;

        retval = dagbase::findInternal(path, "classes", _classes);
        if (retval.has_value())
            return retval;

        return {};
    }

    dagbase::OutputStream &MemoryNodeLibrary::write(dagbase::OutputStream& str, dagbase::Node *node, dagbase::Lua &lua)
    {
        std::string className = node->className();
        str.writeString(className, false);
        node->writeToStream(str, *this, lua);
        
        return str;
    }

    void MemoryNodeLibrary::registerNode(dagbase::Node *node)
    {
        if (node != nullptr)
        {
            std::string className = node->className();
            if (_classes.m.find(className)==_classes.end())
            {
                _classes.emplace(className, node);
            }
        }
    }

    void MemoryNodeLibrary::registerTemplate(std::string className, dagbase::Node *source)
    {
        if (source && !className.empty())
        {
            if (_classes.m.find(className)==_classes.end())
            {
                dagbase::CloningFacility facility;
                auto templ = source->clone(facility, dagbase::CopyOp::GENERATE_UNIQUE_ID_BIT, this);
                if (templ)
                    _classes.emplace(className, templ);
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
