//
// Created by tony on 07/03/24.
//
#include "config/config.h"

#include "OutputStream.h"

namespace dag
{
    OutputStream &OutputStream::write(const std::string & value)
    {
        write(value.length());
        writeBuf(reinterpret_cast<const value_type *>(value.data()), value.length());

        return *this;
    }
}
