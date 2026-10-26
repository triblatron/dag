//
// Created by Tony Horrobin on 16/10/2026.
//

#pragma once

#include "config/Export.h"

#include "NodeEditorLive.h"
#include "core/ScriptProtocol.h"

namespace dag
{
    class NodeEditorLive;
    class DAG_API NodeEditorScriptProtocol : public dagbase::ScriptProtocol
    {
    public:
        NodeEditorScriptProtocol(dag::NodeEditorLive* editor);

        void request(dagbase::Node* self, std::uint32_t opCode, const std::pmr::vector<dagbase::Variant> &args, std::vector<dagbase::Variant> *results) override;
    private:
        dag::NodeEditorLive* _nodeEditor{nullptr};
    };
}
