//
// Created by tony on 04/06/24.
//

#pragma once

#include "config/Export.h"

#include "NodeEditorBackEnd.h"

namespace dag
{
    class NodeEditorFrontEnd;
    class NodeEditorInterface;

    class DAG_API NodeEditorBackEndNodeEditorInterface : public NodeEditorBackEnd
    {
    public:
        NodeEditorBackEndNodeEditorInterface(NodeEditorInterface* nodeEditor, NodeEditorFrontEnd* frontEnd);
    private:
        NodeEditorInterface* _nodeEditor{nullptr};
        NodeEditorFrontEnd* _frontEnd{nullptr};
    };
}
