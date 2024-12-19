//
// Created by tony on 07/03/24.
//

#pragma  once

#include "config/Export.h"

#include "Stream.h"

#include <map>
#include <string>

namespace dag
{
    //! \class OutputStream
    //! Supports writing primitive types, strings and pointers.
    //! \note Pointers are encoded as integer identifiers allocated in an LUT as they are encountered.
    class DAG_API OutputStream : public Stream
    {
    public:
        //! Write a buffer of bytes to the stream.
        virtual OutputStream& writeBuf(const value_type* buf, std::size_t len) = 0;

        //! Type-safe version of writeRef() that takes a typed pointer.
        //! \retval true if the pointer has not yet been encountered.
        //! \retval false otherwise.
        //! \note The pointer may be nullptr and the method returns false.
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
                    std::size_t id = _idLookup.size() + 1;
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

        //! Type-safe writing of a primitive value.
        //! \note Delegates to writeBuf().
        template<typename T>
        OutputStream& write(T value)
        {
            return writeBuf(reinterpret_cast<const value_type*>(&value), sizeof(T));
        }

        //! Write a string which requires special encoding because it is not a primitive type.
        OutputStream& write(std::string const& value);
    private:
        typedef std::map<void*, ObjId> PtrToIdMap;
        PtrToIdMap _idLookup;
    };
}
