//
// Created by tony on 09/02/24.
//

#pragma  once

#include "config/Export.h"

#include "Types.h"

#include <vector>
#include <string>

namespace nbe
{
    class Node;
    class SelectionInterface;

    class NBE_API NodeEditorInterface
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
        typedef std::vector<Node*> NodeArray;
public:
        virtual ~NodeEditorInterface() = default;

        //! Select a set of Nodes with the given mode
        //! \param[in] mode The mode of the selection
        virtual Status select(SelectionMode mode, NodeSet& a) = 0;

        //! Select every node in the Graph
        virtual Status selectAll() = 0;

        //! Cancel the selection
        virtual Status selectNone() = 0;

        virtual size_t selectionCount() = 0;

        //! Create a Node from the library
        virtual Status createNode(std::string const& className, std::string const& name) = 0;

        //! Delete a Node from the Graph
        virtual Status deleteNode(NodeID id) = 0;

        //! Connect two ports
        virtual Status connect(PortID from, PortID to) = 0;

        //! Disconnect two ports
        virtual Status disconnect(SignalPathID id) = 0;

        //! Create a Group from the selection
        virtual Status createChild() = 0;

        //! Create a template from a Group
        virtual Status createTemplate(NodeID id) = 0;

        virtual Status deleteTemplate(TemplateID id) = 0;
    };
}
