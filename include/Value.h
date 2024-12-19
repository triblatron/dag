#pragma once

#include "config/Export.h"

#include <cstdint>
#include <variant>
#include <string>
#include <vector>
#include <utility>

namespace dag
{
    //! A value type to be used throughout the software, uses std::variant.
    //! Provides useful conversions and constructors.
    class Value
    {
    public:
        //! Typedef to improve readability when referencing the type of the variant.
        typedef std::variant<std::int64_t, double, std::string, bool, void*, std::vector<Value >> ValueType;
        
    public:
        //! Default ctor, initialises variant member to its first choice, an int64_t.
        Value() = default;

        //! Increment operator for types that can be added.
        template<typename T>
        Value& operator+=(T const& op)
        {
            std::get<T>(_value) += op;
            
            return *this;
        }

        explicit Value(ValueType value)
	        :
        _value(std::move(value))
        {
	        // Do nothing.
        }
        //! A constraint for the supported types.
        //! Certain template methods and operators can only be instantiated for types that satisfy it.
        template<typename T>
        using EnableIfSupported = std::enable_if_t<
            std::is_convertible_v<T, std::string> || std::is_convertible_v<T, std::int64_t> || std::is_convertible_v<T, bool> || std::is_convertible_v<T, double> || std::is_convertible_v<T, void*> || std::is_convertible_v<T, std::vector<Value>>>;

        //! Perfect forwarding ctor for supported types.
        template<typename T,typename = EnableIfSupported<T>>
        explicit Value(T&& value)
            :
            _value(std::forward<T>(value))
        {
            // Do nothing.
        }

        //! Copy ctor.
        Value(const Value& other) = default;

        //! Move ctor.
        Value(Value&& value) = default;

        //! Assignment of another Value.
        Value& operator=(const Value& value)
        {
            if (this != &value)
            {
                _value = value._value;
            }

            return *this;
        }
        
        //! Perfect-forwarding assignment of supported types.
        template<typename T, typename = EnableIfSupported<T>>
        Value& operator=(T&& value)
        {
            _value = std::forward<T>(value);

            return *this;
        }

        //! Allow conversion to any type we can hold.
        //! Prevents throwing at runtime by causing a compile-time 
        //! error for unsupported types.
        template<typename T, typename = EnableIfSupported<T>>
        explicit operator T() const
        {
            return std::get<T>(_value);
        }

        bool operator==(const Value& other) const
        {
            return _value == other._value;
        }

        //! Convert to our variant, required for template ctor to compile.
        operator ValueType() const
        {
            return _value;
        }

        //! Add a value, converting to an array type if necessary.
        template<typename T>
        void push_back(T v)
        {
            // If we are currently a vector, append the value.
            if (_value.index()==6)
            {
                std::get<std::vector<Value>>(_value).push_back(Value(v));
            }
            else
            // If we are currently a scalar, convert to a vector with the original scalar and the new value.
            {
                Value original;
                
                switch (_value.index())
                {
                    case 0:
                    {
                        original = std::get<0>(_value);
                        
                        break;
                    }
                    case 1:
                    {
                        original = std::get<1>(_value);
                        
                        break;
                    }
                    case 2:
                    {
                        original = std::get<2>(_value);
                        
                        break;
                    }
                    case 3:
                    {
                        original = std::get<3>(_value);
                        
                        break;
                    }
                    case 4:
                    {
                        original = std::get<4>(_value);
                        
                        break;
                    }
                    case 5:
                    {
                        original = std::get<5>(_value);
                        
                        break;
                    }
                }
                _value = std::vector<Value> {original,Value(v)};
                
            }
        }
    private:
        ValueType _value;
    };
}
