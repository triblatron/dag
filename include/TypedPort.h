#pragma once

#include "config/Export.h"

#include "NodeLibrary.h"
#include "io/InputStream.h"
#include "io/OutputStream.h"
#include "Transfer.h"
#include "TypeTraits.h"
#include "Types.h"
#include <atomic>
#include <cassert>
#include <iostream>

namespace dag
{
/*	class DAG_API TypedPortBase : public Port
	{
	public:
		explicit TypedPortBase(Node* parent, MetaPort* metaPort);


	};
*/
    class DebugPrinter;

	template<typename T>
	class TypedPort : public Port
	{
	public:
		static_assert(std::is_convertible_v<T, std::string> || std::is_convertible_v<T, std::int64_t> || std::is_convertible_v<T, bool> || std::is_convertible_v<T, double>);
        TypedPort(PortID id, std::string name, PortType::Type type, PortDirection::Direction dir, T value, Node* parent = nullptr, std::uint32_t flags=0x0)
        :
        Port(id, parent, new MetaPort(std::move(name), type, dir), flags|Port::OWN_META_PORT_BIT),
        _value(value)
        {
            setOwnMetaPort(true);
        }

		TypedPort(PortID id, Node* parent, MetaPort* metaPort, T value, std::uint32_t flags=0x0)
			:
			Port(id, parent, metaPort, flags),
			_value(value)
		{
			// Do nothing.
		}

        TypedPort(const TypedPort& other, CloningFacility& facility, CopyOp copyOp, KeyGenerator* keyGen)
        :
        Port(other, facility, copyOp, keyGen)
        {
            _value = other._value;
        }

        explicit TypedPort(dagbase::InputStream& str, NodeLibrary& nodeLib)
        :
        Port(str, nodeLib)
        {
            str.read(&_value);
        }

        dagbase::OutputStream& write(dagbase::OutputStream& str) const override
        {
            std::string className;

            switch(type())
            {
                case PortType::TYPE_INT:
                    className = "TypedPort<int64_t>";
                    break;
                case PortType::TYPE_DOUBLE:
                    className = "TypedPort<double>";
                    break;
                case PortType::TYPE_STRING:
                    className = "TypedPort<string>";
                    break;
                case PortType::TYPE_BOOL:
                    className = "TypedPort<bool>";
                    break;
                default:
                    assert(false);
                    break;
            }
            str.write(className);
            Port::write(str);
            str.write(_value);

            return str;
        }

        TypedPort* clone(CloningFacility& facility, CopyOp copyOp, KeyGenerator* keyGen) override
        {
            return new TypedPort(*this, facility, copyOp, keyGen);
        }

		void setValue(T value)
		{
			_value = value;
		}

		T value() const
		{
			return _value;
		}

        Transfer* connectTo(Port& dest) override
        {
			if (dir() == PortDirection::DIR_OUT && dest.dir() == PortDirection::DIR_IN && isCompatibleWith(dest))
			{
				auto transfer = new TypedTransfer(&_value);
				dest.setDestination(transfer);

				addOutgoingConnection(&dest);
				dest.addIncomingConnection(this);

				return transfer;
			}

			return nullptr;
        }

		Transfer* setDestination(Transfer* transfer) override
		{
			auto typedTransfer = dynamic_cast<TypedTransfer<T>*>(transfer);

			if (typedTransfer != nullptr)
			{
				typedTransfer->setDest(&_value);
			}

			return transfer;
		}

        void accept(ValueVisitor& visitor) override
        {
            visitor.setValue(_value);
        }

        void accept(SetValueVisitor& visitor) override
        {
            _value = visitor.value().operator T();
        }

        [[nodiscard]]bool equals(const Port& other) const override
        {
            if (!Port::operator==(other))
            {
                return false;
            }

            const TypedPort<T> & typed = dynamic_cast<const TypedPort<T>&>(other);

            if (_value != typed._value)
            {
                return false;
            }

            return true;
        }

        void debug(DebugPrinter& printer) const override;

        [[nodiscard]]const char* className() const override
        {
            return classNames[type()];
        }

        std::ostream& toLua(std::ostream& str) override;
    private:
		T _value;
        static const char* classNames[];
	};

    template<typename T>
    std::ostream &TypedPort<T>::toLua(std::ostream &str)
    {
        Port::toLua(str);
        str << std::boolalpha << "value = " << _value;

        return str;
    }

    template<typename T>
    const char* TypedPort<T>::classNames[]=
            {
                    "TypedPort<int64_t>",
                    "TypedPort<double>",
                    "TypedPort<string>",
                    "TypedPort<bool>",
                    "TypedPort<Vec3d>",
                    "TypedPort<void*>",
                    "TypedPort<std::vector<Value>>",
                    "TypedPort<unknown>"
            };

    template<typename T>
    void TypedPort<T>::debug(DebugPrinter &printer) const
    {
        Port::debug(printer);
    }
}
