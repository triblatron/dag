//
// Created by tony on 16/06/24.
//

#pragma once

#include "config/Export.h"

#include <unordered_map>
#include <vector>
#include <cstdint>

namespace nbe
{
    class NBE_API CloningFacility
    {
    public:
        bool putOrig(void* ptr, std::uint64_t* id);

        void addClone(std::uint64_t id, void* ptr)
        {
            if (_cloned.size()<id)
            {
                _cloned.resize(id);
            }

            _cloned[id-1] = ptr;
        }

        void* getClone(std::uint64_t id)
        {
            if (id>0 && id<=_cloned.size())
            {
                return _cloned[id-1];
            }

            return nullptr;
        }

        std::size_t numClones() const
        {
            return _cloned.size();
        }
    private:
        typedef std::unordered_map<void*, std::int64_t> PointerToIdMap;
        PointerToIdMap _idLookup;
        typedef std::vector<void*> ClonedLookup;
        ClonedLookup _cloned;
    };
}