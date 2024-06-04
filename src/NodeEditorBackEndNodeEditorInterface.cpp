//
// Created by tony on 04/06/24.
//
#include "config/config.h"

#include "NodeEditorBackEndNodeEditorInterface.h"

namespace nbe
{
    NodeEditorBackEndNodeEditorInterface::NodeEditorBackEndNodeEditorInterface(nbe::NodeEditorInterface *nodeEditor,
                                                                               nbe::NodeEditorFrontEnd *frontEnd)
       :
       _nodeEditor(nodeEditor),
       _frontEnd(frontEnd)
    {
        // Do nothing.
    }
}
