//
// Created by tony on 07/03/24.
//

#pragma  once

#include "config/Export.h"

#include "Stream.h"

#include <map>
#include <string>

namespace nbe
{
    class NODEBACKEND_API OutputStream : public Stream
    {
    public:
        virtual OutputStream& writeBuf(const value_type* buf, std::size_t len) = 0;
        //virtual bool writeRef(void * ptr) = 0;

        template<typename T>
        bool writeRef(T* ptr)
        {
            if (ptr != nullptr)
            {
                if (auto it=_idLookup.find(ptr); it!=_idLookup.end())
                {
                    write(it->second);
                    return false;
                }
                else
                {
                    std::uint32_t id = _idLookup.size() + 1;
                    _idLookup.insert(PtrToIdMap::value_type(ptr, id));
                    write(id);
                    return true;
                }
            }
            else
            {
                write(ObjId(0));
                return false;
            }
        }
        template<typename T>
        OutputStream& write(T value)
        {
            return writeBuf(reinterpret_cast<const value_type*>(&value), sizeof(T));
        }

        OutputStream& write(std::string const& value);
    private:
        typedef std::map<void*, ObjId> PtrToIdMap;
        PtrToIdMap _idLookup;
    };
}
