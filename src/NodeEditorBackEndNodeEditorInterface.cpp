//
// Created by tony on 04/06/24.
//
#include "config/config.h"

#include "NodeEditorBackEndNodeEditorInterface.h"

namespace dag
{
    NodeEditorBackEndNodeEditorInterface::NodeEditorBackEndNodeEditorInterface(dag::NodeEditorInterface *nodeEditor,
                                                                               dag::NodeEditorFrontEnd *frontEnd)
       :
       _nodeEditor(nodeEditor),
       _frontEnd(frontEnd)
    {
        // Do nothing.
    }
}
