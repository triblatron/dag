//
// Created by tony on 29/03/24.
//

#pragma once

#include "config/Export.h"


#ifdef __cplusplus
extern "C" {
#endif
namespace nbe
{
    class KeyGenerator;
    class NodeLibrary;
}
    void NODEPLUGIN_API init(nbe::KeyGenerator& keyGen, nbe::NodeLibrary& nodeLib);
#ifdef __cplusplus
}
#endif
