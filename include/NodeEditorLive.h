//
// Created by tony on 10/02/24.
//

#pragma once

#include "config/Export.h"

#include "NodeEditorInterface.h"

#include <vector>
#include <functional>

namespace dagbase
{
    class SignalPath;
    class Transfer;
}

namespace dag
{
    class Graph;
    class MemoryNodeLibrary;
    class SelectionLive;

    class DAG_API NodeEditorLive : public NodeEditorInterface
    {
    public:
        NodeEditorLive();

        ~NodeEditorLive() override;

        dagbase::Graph* graph()
        {
            return _graph;
        }

        void eachClass(std::function<bool(dagbase::Node&)> f);

        void eachNode(std::function<bool(dagbase::Node*)> f);

        void eachSignalPath(std::function<bool(dagbase::SignalPath*)> f);

        //! Select a set of Nodes with the given mode
        //! \param[in] mode The mode of the selection
        dagbase::Status select(SelectionMode mode, dagbase::NodeSet& a) override;

        //! Select every node in the Graph
        dagbase::Status selectAll() override;

        //! Cancel the selection
        dagbase::Status selectNone() override;

        size_t selectionCount() override;

        //! Create a Node from the library
        dagbase::Status createNode(std::string const& className, std::string const& name) override;

        //! Delete a Node from the Graph
        dagbase::Status deleteNode(dagbase::NodeID id) override;

        //! Connect two ports
        dagbase::Status connect(dagbase::PortID from, dagbase::PortID to) override;

        //! Disconnect two ports
        dagbase::Status disconnect(dagbase::SignalPathID id) override;

        //! Create a Group from the selection
        dagbase::Status createChild() override;

        //! Create a template from a Group
        dagbase::Status createTemplate(dagbase::NodeID id) override;

        dagbase::Status deleteTemplate(dagbase::TemplateID id) override;

        void debug();
    private:
        MemoryNodeLibrary *_nodeLib{nullptr};
        dagbase::Graph* _graph{nullptr};
        SelectionLive* _selection{nullptr};
        typedef std::vector<dagbase::Transfer*> TransferArray;
        TransferArray _transfers;
    };
}