//
// Created by tony on 10/02/24.
//

#pragma once

#include "config/Export.h"

#include "NodeEditorInterface.h"
#include "core/Variant.h"

#include <vector>
#include <functional>
#include <string_view>

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
        using GraphChildPath = std::vector<std::uint32_t>;

    public:
        NodeEditorLive();

        ~NodeEditorLive() override;

        dagbase::Status setActiveGraph(const GraphChildPath& path);

        dagbase::Graph* activeGraph()
        {
            return _activeGraph;
        }

        dagbase::Status load(const char* filename);

        void eachClass(std::function<bool(const std::string&, dagbase::Node&)> f);

        void eachNode(std::function<bool(dagbase::Node*)> f);

        void eachSignalPath(std::function<bool(dagbase::SignalPath*)> f);

        //! Select a set of Nodes with the given mode
        //! \param[in] mode The mode of the selection
        dagbase::Status select(SelectionMode mode, SelectionInterface::Cont& a) override;

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

        //! Create a GraphNode from the selection
        dagbase::Status createChild() override;

        //! Copy the selection
        dagbase::Status copyNodes();

        dagbase::Status compareNodes();

        //! Create a template from a Group
        dagbase::Status createTemplate(std::string className) override;

        dagbase::Status deleteTemplate(dagbase::TemplateID id) override;

        dagbase::Status browseDown();

        dagbase::Status browseUp();

        dagbase::Variant find(std::string_view path) const;

        void debug();
    private:
        MemoryNodeLibrary *_nodeLib{nullptr};
        dagbase::Graph* _graph{nullptr};
        dagbase::Graph* _activeGraph{nullptr};
        SelectionLive* _selection{nullptr};
        typedef std::vector<dagbase::Transfer*> TransferArray;
        TransferArray _transfers;
    };
}