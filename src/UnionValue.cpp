#include "config/config.h"

#include "UnionValue.h"

#include <osg/Vec3d>

namespace nbe
{
    UnionValue::UnionValue(std::int32_t i)
    {
        _value.type = TYPE_INT;
        _value.value.i = i;
    }
    
    UnionValue::UnionValue(double d)
    {
        _value.type = TYPE_DOUBLE;
        _value.value.d = d;
    }
    
    UnionValue::UnionValue(bool b)
    {
        _value.type = TYPE_BOOL;
        _value.value.b = b;
    }
    
    UnionValue::UnionValue(std::string* s)
    {
        _value.type = TYPE_STRING;
        _value.value.s = s;
    }
    
    UnionValue::UnionValue(osg::Vec3d* v)
    {
        _value.type = TYPE_VEC3D;
        _value.value.v = v;
    }
    
    UnionValue& UnionValue::operator=(std::int32_t i)
    {
        switch (_value.type)
        {
        case TYPE_STRING:
            delete _value.value.s;
            _value.value.s = nullptr;
            break;
        case TYPE_VEC3D:
            delete _value.value.v;
            _value.value.v = nullptr;
            break;
        default:
            break;
        }
        
        _value.type = TYPE_INT;
        _value.value.i = i;
        
        return *this;
    }
    
    UnionValue& UnionValue::operator=(double d)
    {
        switch (_value.type)
        {
        case TYPE_STRING:
            delete _value.value.s;
            _value.value.s = nullptr;
            break;
        case TYPE_VEC3D:
            delete _value.value.v;
            _value.value.v = nullptr;
            break;
        default:
            break;
        }
        
        _value.type = TYPE_DOUBLE;
        _value.value.d = d;
        
        return *this;
    }
    
    UnionValue& UnionValue::operator=(bool b)
    {
        switch (_value.type)
        {
        case TYPE_STRING:
            delete _value.value.s;
            _value.value.s = nullptr;
            break;
        case TYPE_VEC3D:
            delete _value.value.v;
            _value.value.v = nullptr;
            break;
        default:
            break;
        }
        
        _value.type = TYPE_BOOL;
        _value.value.b = b;
        
        return *this;
    }
    
    UnionValue& UnionValue::operator=(std::string* s)
    {
        switch (_value.type)
        {
        case TYPE_STRING:
            delete _value.value.s;
            _value.value.s = nullptr;
            break;
        case TYPE_VEC3D:
            delete _value.value.v;
            _value.value.v = nullptr;
            break;
        default:
            break;
        }
        
        _value.type = TYPE_STRING;
        _value.value.s = s;
        
        return *this;
    }
    
    UnionValue& UnionValue::operator=(osg::Vec3d* v)
    {
        switch (_value.type)
        {
        case TYPE_STRING:
            delete _value.value.s;
            _value.value.s = nullptr;
            break;
        case TYPE_VEC3D:
            delete _value.value.v;
            _value.value.v = nullptr;
            break;
        default:
            break;
        }
        
        _value.type = TYPE_VEC3D;
        _value.value.v = v;
        
        return *this;
    }
    
    UnionValue& UnionValue::operator ++()
    {
        switch ( _value.type)
        {
        case TYPE_INT:
            ++_value.value.i;
            break;
        case TYPE_DOUBLE:
            ++_value.value.d;
            break;
        default:
            break;
        }
        
        return *this;
    }
}
