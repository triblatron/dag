#pragma once

#include "config/Export.h"

#include <string>
#include <cstdint>
#include <variant>

namespace dag
{    
    class UnionValue;

    template<typename T>
    T& get(UnionValue& v);
    
    class DAG_API UnionValue
    {
    public:
        enum Type
        {
            TYPE_INT,
            TYPE_DOUBLE,
            TYPE_BOOL,
            TYPE_STRING,
        };
    public:
        explicit UnionValue(std::int32_t i);
        explicit UnionValue(double d);
        explicit UnionValue(bool b);
        explicit UnionValue(std::string* s);

        UnionValue& operator=(std::int32_t i);
        UnionValue& operator=(double d);
        UnionValue& operator=(bool b);
        UnionValue& operator=(std::string* s);

        UnionValue& operator++();
        
        Type index() const
        {
            return _value.type;
        }
    private:
        struct Value
        {
            union
            {
                std::int32_t i;
                double d;
                bool b;
                std::string* s;
            } value;
            Type type;
        };
        Value _value;
        template<typename T> friend T& get(UnionValue& v);
    };
    
    template<>
    std::string DAG_API & get(UnionValue& v);
    
    template<>
    std::int32_t DAG_API & get(UnionValue& v);
}
