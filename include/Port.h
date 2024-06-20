#pragma once

#include "config/Export.h"

#include "Value.h"

#include <string>
#include <type_traits>
#include <utility>
#include <vector>
#include <algorithm>
#include <functional>
#include "MetaPort.h"
#include "Types.h"
#include "MetaPort.h"
#include "NodeLibrary.h"

namespace nbe
{
    class CloningFacility;
    class DebugPrinter;
    class InputStream;
    class KeyGenerator;
    class Node;
    class OutputStream;
	class Transfer;

    class NBE_API ValueVisitor
    {
    public:
        void setInt(std::int64_t value)
        {
            _value = value;
        }

        void setDouble(double value)
        {
            _value = value;
        }

        void setString(std::string value)
        {
            _value = std::move(value);
        }

        void setBool(bool value)
        {
            _value = value;
        }

        void setValue(const Value& value)
        {
            _value = (value);
        }

        template<typename T>
        void setValue(T value)
        {
            _value = value;
        }

        [[nodiscard]]Value value() const
        {
            return _value;
        }
    private:
        Value _value;
    };

    class SetValueVisitor
    {
    public:
        explicit SetValueVisitor(Value value)
        :
        _value(std::move(value))
        {
            // Do nothing.
        }

        [[nodiscard]]Value value() const
        {
            return _value;
        }
    private:
        Value _value;
    };

	class NBE_API Port
    {
    public:
        typedef std::vector<Port*> PortArray;

        enum
        {
            OWN_META_PORT_BIT   = (1<<0),
            OWN_INPUTS_BIT      = (1<<1),
            OWN_OUTPUTS_BIT     = (1<<2)
        };
    public:
        explicit Port(PortID id, Node* parent, MetaPort *metaPort, std::uint32_t flags=0x0);

        Port(const Port &port, CloningFacility& facility, CopyOp copyOp, KeyGenerator* keyGen);

        Port(Port &&port) = default;

        //! Construct from a stream and a node library.
        //! \note Requires the NodeLibrary to read the parent.
        Port(InputStream& str, NodeLibrary& nodeLib);

        virtual ~Port();

        bool operator==(const Port& other) const;
        
        void setId(PortID id)
        {
            _id = id;
        }

        [[nodiscard]]PortID id() const
        {
            return _id;
        }

        [[nodiscard]] const std::string &name() const
        {
            return _metaPort->name;
        }

        [[nodiscard]] PortType::Type type() const
        {
            return _metaPort->type;
        }

        void setDir(PortDirection::Direction dir)
        {
            _metaPort->direction = dir;
        }

        [[nodiscard]] PortDirection::Direction dir() const
        {
            return _metaPort->direction;
        }

        [[nodiscard]] const PortArray& outgoingConnections() const
        {
            return _outgoingConnections;
        }

        [[nodiscard]] size_t numOutgoingConnections() const
        {
            return _outgoingConnections.size();
        }

        [[nodiscard]] const PortArray& incomingConnections() const
        {
            return _incomingConnections;
        }

        [[nodiscard]] size_t numIncomingConnections() const
        {
            return _incomingConnections.size() -
                   std::count_if(_incomingConnections.begin(), _incomingConnections.end(), [](Port *p) {
                       return p->_removed;
                   });
        }

        void unmarkAllConnections()
        {
            for (auto p : _incomingConnections)
            {
                p->_removed = false;
            }

            for (auto p : _outgoingConnections)
            {
                p->_removed = false;
            }
        }

        void markIncomingRemoved(Port *n)
        {
            for (auto p : _incomingConnections)
            {
                if (n==p)
                {
                    n->_removed = true;
                }
            }
        }

        void markOutgoingRemoved(Port* n)
        {
            for (auto p : _outgoingConnections)
            {
                if (n==p)
                {
                    n->_removed = true;
                }
            }
        }

        [[nodiscard]] bool hasNoDependencies() const
        {
            return  _incomingConnections.empty();
        }

		void addOutgoingConnection(Port* port)
		{
			if (port != nullptr)
			{
				_outgoingConnections.push_back(port);
			}
		}

		void removeOutgoingConnection(Port const * port)
		{
			if (auto const it=std::find(_outgoingConnections.begin(), _outgoingConnections.end(),port); it!=_outgoingConnections.end())
			{
				_outgoingConnections.erase(it);
			}
		}

		void addIncomingConnection(Port* port)
		{
			if (port!=nullptr)
			{
				_incomingConnections.push_back(port);
			}
		}

		void removeIncomingConnection(Port const* port)
		{
			if (auto const it = std::find(_incomingConnections.begin(), _incomingConnections.end(), port); it != _incomingConnections.end())
			{
				_incomingConnections.erase(it);
			}
		}

		[[nodiscard]]auto findOutgoingConnection(const Port& port)
		{
			return std::find(_outgoingConnections.begin(), _outgoingConnections.end(), &port);
		}

		auto findIncomingConnection(const Port& port)
		{
			return std::find(_incomingConnections.begin(), _incomingConnections.end(), &port);
		}

		[[nodiscard]]bool isCompatibleWith(const Port& other) const
		{
			return ((type() == other.type()));// || (type() == TYPE_INT && other.type() == TYPE_DOUBLE) || (type() == TYPE_BOOL && other.type() == TYPE_INT) || (type() == TYPE_BOOL && other.type() == TYPE_DOUBLE));
		}

		template<typename F>
		void eachOutgoingConnection(F f)
		{
			for (auto & _outgoingConnection : _outgoingConnections)
			{
				std::invoke(f, _outgoingConnection);
			}
		}

		template<typename F>
		void eachIncomingConnection(F f)
		{
			for (auto & _incomingConnection : _incomingConnections)
			{
				std::invoke(f, _incomingConnection);
			}
		}

        [[nodiscard]]MetaPort* metaPort() const
        {
            return _metaPort;
        }

        void setParent(Node* parent)
        {
            _parent = parent;
        }

        [[nodiscard]]Node* parent()
        {
            return _parent;
        }

        virtual Transfer* connectTo(Port& dest) = 0;

        void reconnectTo(NodeSet const& selection, Node* newDest);

        void reconnectFrom(NodeSet const& selection, Node* newSource);

        void disconnect(Port& dest)
        {
            removeOutgoingConnection(&dest);
            dest.removeIncomingConnection(this);
        }

        virtual Transfer* setDestination(Transfer* transfer) = 0;

        virtual void accept(ValueVisitor& visitor) = 0;

        virtual void accept(SetValueVisitor& visitor) = 0;

        bool isConnectedTo(Port* other) const
        {
            return (std::find(_incomingConnections.begin(), _incomingConnections.end(), other) != _incomingConnections.end() ||
                   std::find(_outgoingConnections.begin(), _outgoingConnections.end(), other) != _outgoingConnections.end());
        }

        virtual Port* clone(CloningFacility& facility, CopyOp copyOp, KeyGenerator* keyGen) = 0;

        virtual OutputStream& write(OutputStream& str) const;

        [[nodiscard]]virtual bool equals(const Port& other) const
        {
            return false;
        }

        virtual void debug(DebugPrinter& printer) const;

        virtual std::ostream& toLua(std::ostream& str);

        [[nodiscard]]virtual const char* className() const = 0;
    protected:
		PortArray _outgoingConnections;
		PortArray _incomingConnections;

        void setFlag(std::uint32_t mask)
        {
            _flags |= mask;
        }

        void setOwnMetaPort(bool own)
        {
            if (own)
            {
                _flags |= OWN_META_PORT_BIT;
            }
            else
            {
                _flags &= ~OWN_META_PORT_BIT;
            }
        }

        [[nodiscard]]bool ownMetaPort() const
        {
            return (_flags & OWN_META_PORT_BIT) != 0x0;
        }
	private:
        PortID _id{ 0 };
        MetaPort* _metaPort{nullptr};
        Node* _parent{nullptr};
        bool _removed{false};
//        Value _value{0.0};
        std::uint32_t _flags{0x0};
	};

	class NBE_API ValuePort final : public Port
	{
	public:
        ValuePort(PortID id, std::string name, PortType::Type type, PortDirection::Direction direction, Value value, Node* parent = nullptr)
        :
        Port(id, parent, new MetaPort(std::move(name),type, direction)),
        _value(std::move(value))
        {
            // Do nothing.
        }

		ValuePort(PortID id, MetaPort* metaPort, Value value, Node* parent = nullptr)
			:
			Port(id, parent, metaPort),
			_value(std::move(value))
		{
			// Do nothing.
		}

        explicit ValuePort(InputStream& str, NodeLibrary& nodeLib);

        ValuePort(const ValuePort& other, CloningFacility& facility, CopyOp copyOp, KeyGenerator* keyGen)
        :
        Port(other, facility, copyOp, keyGen)
        {
            _value = other._value;
        }

        bool operator==(const ValuePort& other) const
        {
            return Port::operator==(other) && _value == other._value;
        }

		void setValue(const Value& value)
		{
			_value = value;
		}

		[[nodiscard]]const Value& value() const
		{
			return _value;
		}

        Transfer* connectTo(Port& dest) override;

        Transfer* setDestination(Transfer* transfer) override;

        void accept(ValueVisitor& visitor) override
        {
            visitor.setValue(_value);
        }

        void accept(SetValueVisitor& visitor) override
        {
            _value = visitor.value();
        }

        ValuePort* clone(CloningFacility& facility, CopyOp copyOp, KeyGenerator* keyGen) override
        {
            return new ValuePort(*this, facility, copyOp, keyGen);
        }

        [[nodiscard]]const char* className() const override
        {
            return "ValuePort";
        }
    private:
		Value _value;
	};

	class NBE_API VariantPort : public Port
	{
	public:
		typedef Value::ValueType ValueType;

		static_assert(!std::is_reference_v<Value::ValueType>, "Cannot use a reference for the value of a Port");

        VariantPort(PortID id, std::string name, PortType::Type type, PortDirection::Direction direction, ValueType value, Node* parent = nullptr)
        :
        Port(id, parent, new MetaPort(std::move(name), type, direction)),
        _value(std::move(value))
        {
            setOwnMetaPort(true);
        }

		VariantPort(PortID id, MetaPort* metaPort, ValueType value, Node* parent = nullptr)
			:
		Port(id, parent, metaPort),
		_value(std::move(value))
		{
			// Do nothing.
		}

        VariantPort(const VariantPort& other, CloningFacility& facility, CopyOp copyOp, KeyGenerator* keyGen)
        :
        Port(other, facility, copyOp, keyGen)
        {
            _value = other._value;
        }

        explicit VariantPort(InputStream& str, NodeLibrary& nodeLib);

        VariantPort* clone(CloningFacility& facility, CopyOp copyOp, KeyGenerator* keyGen) override
        {
            return new VariantPort(*this, facility, copyOp, keyGen);
        }

		void setValue(const ValueType & value)
		{
			_value = value;
		}

		[[nodiscard]] const ValueType& value() const
		{
			return _value;
		}
        
        Transfer* connectTo(Port& dest) override;

        Transfer* setDestination(Transfer* transfer) override;

        void accept(ValueVisitor& visitor) override
        {
            switch (_value.index())
            {
                case 0:
                    visitor.setInt(std::get<0>(_value));
                    break;
                case 1:
                    visitor.setDouble(std::get<1>(_value));
                    break;
                case 2:
                    visitor.setString(std::get<2>(_value));
                    break;
                case 3:
                    visitor.setBool(std::get<3>(_value));
                    break;
            }
        }

        void accept(SetValueVisitor& visitor) override
        {
            switch (_value.index())
            {
                case 0:
                    _value = std::int64_t(visitor.value());
                    break;
                case 1:
                    _value = double(visitor.value());
                    break;
                case 2:
                    _value =visitor.value().operator nbe::Value::ValueType();
                    break;
                case 3:
                    _value = bool(visitor.value());
                    break;
            }
        }

        [[nodiscard]]const char* className() const override
        {
            return "VariantPort";
        }
    private:
		Value::ValueType _value;
	};
}
