//
// Created by tony on 07/03/24.
//
#include "config/config.h"

#include "MemoryOutputStream.h"
#include "ByteBuffer.h"

namespace dag
{
    MemoryOutputStream::MemoryOutputStream(ByteBuffer *buf)
            :
            _buf(buf)
    {
        // Do nothing.
    }

    OutputStream& MemoryOutputStream::writeBuf(const dag::OutputStream::value_type *buf, std::size_t len)
    {
        if (_buf!=nullptr)
        {
            _buf->put(buf, len);
        }

        return *this;
    }
}
