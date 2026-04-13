//
// Created by tony on 09/02/24.
//

#pragma  once

#include "config/Export.h"

#include "core/Types.h"

#include <vector>
#include <string>

namespace dagbase
{
    class Node;
}

namespace dag
{
    class SelectionInterface;

    class DAG_API NodeEditorInterface
    {
    public:
        enum SelectionMode
        {
            //! Replace the current selection
            SELECTION_SET,
            //! Add to the current selection
            SELECTION_ADD,
            //! Subtract from the current selection
            SELECTION_SUBTRACT,
            //! Flip between selected and deselected.
            SELECTION_TOGGLE,
        };
        typedef std::vector<dagbase::Node*> NodeArray;
public:
        virtual ~NodeEditorInterface() = default;

        //! Select a set of Nodes with the given mode
        //! \param[in] mode The mode of the selection
        virtual dagbase::Status select(SelectionMode mode, dagbase::NodeSet& a) = 0;

        //! Select every node in the Graph
        virtual dagbase::Status selectAll() = 0;

        //! Cancel the selection
        virtual dagbase::Status selectNone() = 0;

        virtual size_t selectionCount() = 0;

        //! Create a Node from the library
        virtual dagbase::Status createNode(std::string const& className, std::string const& name) = 0;

        //! Delete a Node from the Graph
        virtual dagbase::Status deleteNode(dagbase::NodeID id) = 0;

        //! Connect two ports
        virtual dagbase::Status connect(dagbase::PortID from, dagbase::PortID to) = 0;

        //! Disconnect two ports
        virtual dagbase::Status disconnect(dagbase::SignalPathID id) = 0;

        //! Create a Group from the selection
        virtual dagbase::Status createChild() = 0;

        //! Create a template from a Group
        virtual dagbase::Status createTemplate(dagbase::NodeID id) = 0;

        virtual dagbase::Status deleteTemplate(dagbase::TemplateID id) = 0;
    };
}
