//
// Created by tony on 09/03/24.
//

#pragma once

#include "config/Export.h"

#include "Stream.h"
#include "NodeLibrary.h"
#include <vector>
#include <string>

namespace nbe
{
    class NBE_API InputStream : public Stream
    {
    public:
        virtual InputStream& readBuf(value_type* buf, std::size_t len) = 0;

        template<typename T>
        InputStream& read(T* value)
        {
            return readBuf(reinterpret_cast<value_type*>(value), sizeof(T));
        }

        //virtual void* readRef(ObjId* id) = 0;

        template<typename T>
        T* readRef(ObjId* id, NodeLibrary& nodeLib)
        {
            read(id);

            if (*id != 0)
            {
                _lastReadId = *id;
                if (_lastReadId-1<_ptrLookup.size())
                {
                    return static_cast<T*>(_ptrLookup[_lastReadId-1]);
                }
                else
                {
                    return nodeLib.instantiateNode(*this);
                }
            }
            else
            {
                return nullptr;
            }
        }

        Ref readRef(ObjId* id)
        {
            read(id);

            if (*id != 0)
            {
                _lastReadId = *id;
                if (_lastReadId-1<_ptrLookup.size())
                {
                    return (_ptrLookup[_lastReadId-1]);
                }
                else
                {
                    return nullptr;
                }
            }
            else
            {
                return nullptr;
            }
        }

        template<typename T>
        T* readRef(ObjId* id)
        {
            read(id);

            if (*id != 0)
            {
                _lastReadId = *id;
                if (_lastReadId-1<_ptrLookup.size())
                {
                    return static_cast<T*>(_ptrLookup[_lastReadId-1]);
                }
                else
                {
                    return new T(*this);
                }
            }
            else
            {
                return nullptr;
            }
        }

        Port* readPort(NodeLibrary& nodeLib)
        {
            ObjId id{0};
            Ref ref = readRef(&id);
            if (id != 0)
            {
                _lastReadId = id;
                if (_lastReadId-1<_ptrLookup.size())
                {
                    return static_cast<Port*>(ref);
                }
                else
                {
                    return nodeLib.instantiatePort(*this);
                }
            }
            else
            {
                return nullptr;
            }
        }

        void addObj(void* ref)
        {
            if (_lastReadId>_ptrLookup.size())
            {
                _ptrLookup.resize(_lastReadId);
            }
            _ptrLookup[_lastReadId-1] = ref;
        }

        InputStream& read(std::string* value);
    private:
        using PtrArray = std::vector<Ref>;
        PtrArray _ptrLookup;
        ObjId _lastReadId{0};
    };
}