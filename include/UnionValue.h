#include "config/Export.h"

#include <string>
#include <cstdint>
#include <variant>

namespace dag
{    
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
   
   template<typename T>
   T& get(UnionValue & v);
   
   template<>
   std::string& get(UnionValue& v)
   {
        if (v.index()==UnionValue::TYPE_STRING)
        {
            return *v._value.value.s;
        }
        else
        {
            throw std::bad_variant_access();
        }
   }
   
   template<>
   std::int32_t& get(UnionValue& v)
   {
       if (v.index()==UnionValue::TYPE_INT)
       {
           return v._value.value.i;
       }
       else
       {
           throw std::bad_variant_access();
       }
   }
}
