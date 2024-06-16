//
// Created by tony on 16/06/24.
//
#include "config/config.h"

#include "CloningFacility.h"

namespace nbe
{
    bool CloningFacility::putOrig(void* ptr, std::uint64_t *id)
    {
        if (ptr!= nullptr)
        {
            if (auto it=_idLookup.find(ptr); it!=_idLookup.end())
            {
                if (id!=nullptr)
                {
                    *id = it->second;
                }
                return false;
            }
            else
            {
                if (id!= nullptr)
                {
                    *id = _idLookup.size()+1;
                    _idLookup.insert(PointerToIdMap::value_type (ptr, *id));
                }
                return true;
            }
        }
        else
        {
            if (id != nullptr)
            {
                *id = 0;
            }

            return false;
        }
    }
}
