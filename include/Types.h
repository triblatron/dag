//
// Created by tony on 04/02/24.
//

#pragma once

#include "config/Export.h"

#include <cstdint>
#include <optional>
#include <unordered_set>

namespace nbe
{
    class Graph;
    class Node;
    class Port;

    //! The direction of a port.
    class NBE_API PortDirection
    {
    public:
        enum Direction
        {
            //! Initial or invalid port direction.
            DIR_UNKNOWN,
            //! Input port.
            DIR_IN,
            //! Output port.
            DIR_OUT,
            //! Internal port, cannot be connected.
            DIR_INTERNAL
        };

        //! Parse a string representation of a port direction.
        static Direction parseFromString(const char* str);

        //! Convert a port direction to a string.
        static const char* toString(Direction dir);
    };

    class NBE_API PortType
    {
    public:
        enum Type
        {
            //! A std::int64_t
            TYPE_INT,
            //! A double-precision floating point number.
            TYPE_DOUBLE,
            //! A std::string
            TYPE_STRING,
            //! A bool
            TYPE_BOOL,
            //! An osg::Vec3d
            TYPE_VEC3D,
            //! An opaque pointer (void*).
            TYPE_OPAQUE,
            //! A std::vector of values.
            TYPE_VECTOR,
            //! Initial or invalid type.
            TYPE_UNKNOWN
        };

        //! Parse a string representation of a port type.
        static Type parseFromString(const char* str);

        //! Convert a port type to a string.
        static const char* toString(Type type);
    };

//! Macro to declare a ID type
//! This ensures type safety because IDs will have different types.
//! They therefore cannot be mixed.
#define INF_ID_DECLARE(Name) \
    class Name              \
    {                        \
    public:                  \
        enum : int64_t       \
        {                    \
            INVALID_ID = -1  \
        };                   \
    public:                  \
        Name()               \
        :                    \
        id(INVALID_ID)       \
        {                    \
        }                    \
        \
        Name(std::int64_t id)\
        :                    \
        id(id)               \
        {                    \
        }                    \
                             \
        Name& operator=(const Name& other) \
        {                    \
            id = other.id;   \
                             \
            return *this;\
        }                    \
                             \
        Name& operator=(std::int64_t id_)   \
        {                    \
            this->id = id_;   \
                             \
            return *this;    \
        }                    \
                             \
        const Name operator++(int) \
        {                    \
            Name old = *this;\
            id++;            \
                             \
            return old;      \
        }                    \
                             \
        operator std::int64_t() const       \
        {                    \
            return id;\
        }                    \
                             \
        bool valid() const   \
        {                    \
            return id!=-1;   \
        }\
    private:             \
        std::int64_t id;     \
                             \
    }
    INF_ID_DECLARE(NodeID);
    INF_ID_DECLARE(PortID);
    INF_ID_DECLARE(TemplateID);
    INF_ID_DECLARE(SignalPathID);

    //! A return value from an RPC.
    struct Status
    {
        enum StatusCode
        {
            //! Success
            STATUS_OK,
            //! A file could not be located.
            STATUS_FILE_NOT_FOUND,
            //! An object could not be located.
            STATUS_OBJECT_NOT_FOUND,
            //! An invalid port was specified.
            STATUS_INVALID_PORT,
            //! An invalid selection was specified.
            STATUS_INVALID_SELECTION,
            //! An initial invalid status.
            STATUS_UNKNOWN
        };

        StatusCode status{STATUS_UNKNOWN};
        enum ResultType
        {
            RESULT_NONE,
            RESULT_NODE,
            RESULT_PORT,
            RESULT_GRAPH,
            RESULT_NODE_ID,
            RESULT_PORT_ID,
            RESULT_SIGNAL_PATH_ID
        };
        ResultType resultType{RESULT_NONE};
        union Result
        {
            Node* node{nullptr};
            Port* port;
            Graph* graph;
            NodeID nodeId;
            PortID portId;
            TemplateID templateId;
            SignalPathID signalPathId;
            
            Result()
            :
            node(nullptr)
            {
				// Do nothing.
			}
        };
        Result result;
        
        Status()
        {
            // Do nothing.
		}

        explicit Status(StatusCode code)
        :
        status(code)
        {
            // Do nothing.
        }
    };

    typedef std::unordered_set<Node*> NodeSet;

    enum CopyOp : std::uint32_t
    {
        DEEP_COPY_NODES_BIT     = 1<<0,
        DEEP_COPY_INPUTS_BIT    = 1<<1,
        DEEP_COPY_OUTPUTS_BIT   = 1<<2,
        GENERATE_UNIQUE_ID_BIT  = 1<<3,
        DEEP_COPY_PARENT_BIT    = 1<<4
    };
};
