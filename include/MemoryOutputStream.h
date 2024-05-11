//
// Created by tony on 07/03/24.
//

#pragma once

#include "config/Export.h"

#include "OutputStream.h"

namespace nbe
{
    class ByteBuffer;

    class NBE_API MemoryOutputStream : public OutputStream
    {
    public:
        //! Construct from a byffer that is typically empty.
        explicit MemoryOutputStream(ByteBuffer* buf);

        OutputStream& writeBuf(const value_type* buf, std::size_t len) override;
    private:
        ByteBuffer* _buf{nullptr};
    };
}
