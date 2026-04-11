//
// Created by Tony Horrobin on 11/04/2026.
//

#pragma once

#include "config/Export.h"

#include "Port.h"
#include "io/InputStream.h"
#include "io/OutputStream.h"
#include "Transfer.h"

namespace dag
{
    template <typename T>
    class PrimitivePort : public Port
    {
    public:
        using Writer = dagbase::OutputStream & (dagbase::OutputStream::*)(T);
        using Reader = dagbase::InputStream& (dagbase::InputStream::*)(T*) const;
    public:
		static_assert(std::is_convertible_v<T, std::string> || std::is_integral_v<T> || std::is_convertible_v<T, bool> || std::is_floating_point_v<T>);
        PrimitivePort(PortID id, std::string name, PortType::Type type, PortDirection::Direction dir, T value, Node* parent = nullptr, std::uint32_t flags=0x0)
        :
        Port(id, parent, new MetaPort(std::move(name), type, dir), flags|Port::OWN_META_PORT_BIT),
        _value(value)
        {
            setOwnMetaPort(true);
        }

		PrimitivePort(PortID id, Node* parent, MetaPort* metaPort, T value, std::uint32_t flags=0x0)
			:
			Port(id, parent, metaPort, flags),
			_value(value)
		{
			// Do nothing.
		}

        PrimitivePort(const PrimitivePort& other, CloningFacility& facility, CopyOp copyOp, KeyGenerator* keyGen)
        :
        Port(other, facility, copyOp, keyGen)
        {
            _value = other._value;
        }

        explicit PrimitivePort(dagbase::InputStream& str, NodeLibrary& nodeLib, dagbase::Lua& lua)
        {
        	std::string className;
        	std::string fieldName;
        	str.readHeader(&className);
        	Port::readFromStream(str, nodeLib, lua);
        	str.readField(&fieldName);
        	dagbase::Variant configValue(_value);
            str.read(lua, &configValue);
        	if (configValue.has_value())
        	{
        		_value = std::get<T>(configValue.value().value());
        	}
        	str.readFooter();
        }

        dagbase::OutputStream& write(dagbase::OutputStream& str) const override
        {
            std::string className;

            switch(type())
            {
                case PortType::TYPE_INT64:
                    className = "PrimitivePort<int64_t>";
                    break;
                case PortType::TYPE_DOUBLE:
                    className = "PrimitivePort<double>";
                    break;
                case PortType::TYPE_STRING:
                    className = "PrimitivePort<string>";
                    break;
                case PortType::TYPE_BOOL:
                    className = "PrimitivePort<bool>";
                    break;
                default:
                    assert(false);
                    break;
            }
        	str.writeField("className");
        	str.writeString(className, true);
        	str.writeHeader(className);
            Port::write(str);
        	str.writeField("value");
        	str.write(dagbase::ConfigurationElement::ValueType(_value));
        	str.writeFooter();

            return str;
        }

        PrimitivePort* clone(CloningFacility& facility, CopyOp copyOp, KeyGenerator* keyGen) override
        {
            return new PrimitivePort(*this, facility, copyOp, keyGen);
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

            const PrimitivePort<T> & typed = dynamic_cast<const PrimitivePort<T>&>(other);

            if (_value != typed._value)
            {
                return false;
            }

            return true;
        }

        void debug(dagbase::DebugPrinter& printer) const override;

        [[nodiscard]]const char* className() const override
        {
            return classNames[type()];
        }

        std::ostream& toLua(std::ostream& str) override;
    private:
		T _value;
        static const char* classNames[];
	};

}