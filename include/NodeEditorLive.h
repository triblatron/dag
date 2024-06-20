//
// Created by tony on 10/02/24.
//

#pragma once

#include "config/Export.h"

#include "NodeEditorInterface.h"

#include <vector>

namespace nbe
{
    class Graph;
    class MemoryNodeLibrary;
    class SelectionLive;
    class Transfer;

    class NBE_API NodeEditorLive : public NodeEditorInterface
    {
    public:
    public:
        NodeEditorLive();

        ~NodeEditorLive() override;

        //! Select a set of Nodes with the given mode
        //! \param[in] mode The mode of the selection
        Status select(SelectionMode mode, NodeSet& a) override;

        //! Select every node in the Graph
        Status selectAll() override;

        //! Cancel the selection
        Status selectNone() override;

        size_t selectionCount() override;

        //! Create a Node from the library
        Status createNode(std::string const& className, std::string const& name) override;

        //! Delete a Node from the Graph
        Status deleteNode(NodeID id) override;

        //! Connect two ports
        Status connect(PortID from, PortID to) override;

        //! Disconnect two ports
        Status disconnect(SignalPathID id) override;

        //! Create a Group from the selection
        Status createChild() override;

        //! Create a template from a Group
        Status createTemplate(NodeID id) override;

        Status deleteTemplate(TemplateID id) override;
    private:
        MemoryNodeLibrary *_nodeLib{nullptr};
        Graph* _graph{nullptr};
        SelectionLive* _selection{nullptr};
        typedef std::vector<Transfer*> TransferArray;
        TransferArray _transfers;
    };
}