//
// Created by Tony Horrobin on 16/10/2026.
//

#include "config/config.h"

#include "NodeEditorScriptProtocol.h"
#include "Scripting.h"
#include "lua_runtime.h"

namespace dag
{
    NodeEditorScriptProtocol::NodeEditorScriptProtocol(dag::NodeEditorLive *editor)
        :
    _nodeEditor(editor)
    {
        // Do nothing.
    }

    void NodeEditorScriptProtocol::request(dagbase::Node* self, std::uint32_t opCode, const std::pmr::vector<dagbase::Variant> &args,
                                           std::vector<dagbase::Variant> *results)
    {
        switch (opCode)
        {
            case ACTION_ADD_PORT:
            {
                if (args.size()==4)
                {
                    auto name = args[0].asString();
                    auto dir = dagbase::PortDirection::parseFromString(args[1].asString().c_str());
                    auto flags = dagbase::Port::parsePortFlags(args[2].asString());
                    auto value = args[3].asDouble();
                    if (_nodeEditor)
                    {
                        auto status = _nodeEditor->addPort(self->id(), name, dir, flags, dagbase::Value(value));
                        if (status.status == dagbase::Status::STATUS_OK && status.resultType == dagbase::Status::RESULT_PORT_ID)
                        {
                            auto port = _nodeEditor->activeGraph()->port(std::get<dagbase::PortID>(status.result.value()));
                            if (results)
                            {
                                results->emplace_back(port);
                            }
                        }
                    }
                }
                break;
            }
        }

    }
}
