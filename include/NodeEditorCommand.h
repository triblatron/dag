//
// Created by Tony Horrobin on 14/11/2026.
//

#pragma once

#include "config/Export.h"
#include "core/Command.h"

#include "util/enums.h"
#include "NodeEditorLive.h"

namespace dag
{
    class DAG_API NodeEditorCommand : public dagbase::Command
    {
    public:
        enum Command : std::uint32_t
        {
            COMMAND_UNKNOWN,
            COMMAND_NOP,
            COMMAND_CREATE_NODE,
            COMMAND_CONNECT,
            COMMAND_DISCONNECT,
            COMMAND_SELECT,
            COMMAND_CREATE_CHILD,
            COMMAND_SET_ACTIVE_GRAPH,
            COMMAND_DELETE_NODE,
            COMMAND_COPY_NODE,
            COMMAND_COMPARE_NODES,
            COMMAND_CREATE_TEMPLATE,
            COMMAND_BROWSE_DOWN,
            COMMAND_BROWSE_UP,
            COMMAND_SET_POSITION,
            COMMAND_SAVE,
            COMMAND_LOAD,
            COMMAND_SERIALISE,
            COMMAND_DESERIALISE,
            COMMAND_TOPO_SORT,
            COMMAND_ADD_PORT,
            COMMAND_DELETE_PORT,
            COMMAND_SET_PORT_VALUE,
            COMMAND_SET_NODE_ACTIVE,
            COMMAND_EVALUATE_GRAPH,
            COMMAND_LOAD_SCRIPTS,
            COMMAND_ATTACH_SCRIPT
        };

    public:
        explicit NodeEditorCommand(NodeEditorCommand* decoratee);

        void setNodeEditor(dag::NodeEditorLive* nodeEditor)
        {
            _nodeEditor = nodeEditor;
        }

        NodeEditorLive* nodeEditor()
        {
            return _nodeEditor;
        }

        void configure(dagbase::ConfigurationElement& config) override;

        void reset() override;

        dagbase::Status undo() override;

        dagbase::Status redo(std::pmr::vector<dagbase::Variant>* results) override;

        dagbase::Status makeItSo(const std::pmr::vector<dagbase::Variant>& args, std::pmr::vector<dagbase::Variant>* results) override;

        static const char* commandToString(Command value)
        {
            switch (value)
            {
                ENUM_NAME(COMMAND_UNKNOWN)
                ENUM_NAME(COMMAND_NOP)
                ENUM_NAME(COMMAND_CREATE_NODE)
                ENUM_NAME(COMMAND_CONNECT)
                ENUM_NAME(COMMAND_DISCONNECT)
                ENUM_NAME(COMMAND_SELECT)
                ENUM_NAME(COMMAND_CREATE_CHILD)
                ENUM_NAME(COMMAND_SET_ACTIVE_GRAPH)
                ENUM_NAME(COMMAND_DELETE_NODE)
                ENUM_NAME(COMMAND_COPY_NODE)
                ENUM_NAME(COMMAND_COMPARE_NODES)
                ENUM_NAME(COMMAND_CREATE_TEMPLATE)
                ENUM_NAME(COMMAND_BROWSE_DOWN)
                ENUM_NAME(COMMAND_BROWSE_UP)
                ENUM_NAME(COMMAND_SET_POSITION)
                ENUM_NAME(COMMAND_SAVE)
                ENUM_NAME(COMMAND_LOAD)
                ENUM_NAME(COMMAND_SERIALISE)
                ENUM_NAME(COMMAND_DESERIALISE)
                ENUM_NAME(COMMAND_TOPO_SORT)
                ENUM_NAME(COMMAND_ADD_PORT)
                ENUM_NAME(COMMAND_DELETE_PORT)
                ENUM_NAME(COMMAND_SET_PORT_VALUE)
                ENUM_NAME(COMMAND_SET_NODE_ACTIVE)
                ENUM_NAME(COMMAND_EVALUATE_GRAPH)
                ENUM_NAME(COMMAND_LOAD_SCRIPTS)
                ENUM_NAME(COMMAND_ATTACH_SCRIPT)
            }

            return "<error>";
        }

        static Command parseCommand(const char* str)
        {
            TEST_ENUM(COMMAND_UNKNOWN, str)
            TEST_ENUM(COMMAND_NOP, str)
            TEST_ENUM(COMMAND_CREATE_NODE, str)
            TEST_ENUM(COMMAND_CONNECT, str)
            TEST_ENUM(COMMAND_DISCONNECT, str)
            TEST_ENUM(COMMAND_SELECT, str)
            TEST_ENUM(COMMAND_CREATE_CHILD, str)
            TEST_ENUM(COMMAND_SET_ACTIVE_GRAPH, str)
            TEST_ENUM(COMMAND_DELETE_NODE, str)
            TEST_ENUM(COMMAND_COPY_NODE, str)
            TEST_ENUM(COMMAND_COMPARE_NODES, str)
            TEST_ENUM(COMMAND_CREATE_TEMPLATE, str)
            TEST_ENUM(COMMAND_BROWSE_DOWN, str)
            TEST_ENUM(COMMAND_BROWSE_UP, str)
            TEST_ENUM(COMMAND_SET_POSITION, str)
            TEST_ENUM(COMMAND_SAVE, str)
            TEST_ENUM(COMMAND_LOAD, str)
            TEST_ENUM(COMMAND_SERIALISE, str)
            TEST_ENUM(COMMAND_DESERIALISE, str)
            TEST_ENUM(COMMAND_TOPO_SORT, str)
            TEST_ENUM(COMMAND_ADD_PORT, str)
            TEST_ENUM(COMMAND_DELETE_PORT, str)
            TEST_ENUM(COMMAND_SET_PORT_VALUE, str)
            TEST_ENUM(COMMAND_SET_NODE_ACTIVE, str)
            TEST_ENUM(COMMAND_EVALUATE_GRAPH, str)
            TEST_ENUM(COMMAND_LOAD_SCRIPTS, str)
            TEST_ENUM(COMMAND_ATTACH_SCRIPT, str)

            return COMMAND_UNKNOWN;
        }

        void set(std::string_view name, dagbase::Variant value)
        {
            if (_decoratee)
                _decoratee->set(name, value);

            if (name == "nodeClass")
            {
                nodeClass = value.asString();
            }
            else if (name == "nodeName")
            {
                nodeName = value.asString();
            }
            else if (name == "fromPort")
            {
                fromPort = value.asInteger();
            }
            else if (name == "toPort")
            {
                toPort = value.asInteger();
            }
            else if (name == "signalPath")
            {
                signalPath = value.asInteger();
            }
            else if (name == "node")
            {
                nodeId = value.asInteger();
            }
            else if (name == "otherNode")
            {
                otherNodeId = value.asInteger();
            }
            else if (name == "selectionMode")
            {
                selectionMode = static_cast<dag::NodeEditorInterface::SelectionMode>(value.asUint32());
            }
            else if (name == "filename")
            {
                filename = value.asString();
            }
        }

        NodeEditorCommand* clone() const override
        {
            return new NodeEditorCommand(*this);
        }

        void setDecoratee(NodeEditorCommand* decoratee)
        {
            _decoratee = decoratee;
        }

        NodeEditorCommand* decoratee()
        {
            return _decoratee;
        }
    private:
        NodeEditorLive* _nodeEditor{nullptr};
        NodeEditorCommand* _decoratee{nullptr};
    public:
        Command cmd{ COMMAND_UNKNOWN };
        using NodeIDArray = std::vector<dagbase::NodeID>;
        NodeIDArray selection;
        std::string nodeClass;
        std::string nodeName;
        dagbase::PortID fromPort{ dagbase::PortID::INVALID_ID };
        dagbase::PortID toPort{ dagbase::PortID::INVALID_ID };
        dagbase::SignalPathID signalPath{ dagbase::SignalPathID::INVALID_ID };
        dagbase::NodeID nodeId;
        dagbase::NodeID otherNodeId;
        dag::NodeEditorInterface::SelectionMode selectionMode{ dag::NodeEditorInterface::SELECTION_UNKNOWN };
        dag::NodeEditorLive::GraphChildPath graphChildPath;
        std::string filename;
        std::vector<std::pair<dagbase::NodeID, dagbase::NodeID>> order;
        std::vector<dagbase::NodeID> cycle;
        float position[2]{};
        dagbase::ComparisonFlags cmpFlags{dagbase::CMP_NONE};
        dag::NodeEditorLive::SerialiseFormat serialiseFormat{dag::NodeEditorLive::SERIALISE_OBJECT_GRAPH};
        dagbase::PortID portId{dagbase::PortID::INVALID_ID};
        std::string portName;
        dagbase::PortDirection::Direction portDirection{dagbase::PortDirection::DIR_UNKNOWN};
        dagbase::Port::PortFlags portFlags{dagbase::Port::FLAGS_NONE};
        dagbase::Variant portValue;
        dagbase::Node::Active active{dagbase::Node::ACTIVE_ON};
        std::string funcName;
    };
}
