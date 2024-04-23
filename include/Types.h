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

    class NODEBACKEND_API PortDirection
    {
    public:
        enum Direction
        {
            DIR_UNKNOWN,
            DIR_IN,
            DIR_OUT,
            DIR_INTERNAL
        };

        static Direction parseFromString(const char* str);
        static const char* toString(Direction dir);
    };

    class NODEBACKEND_API PortType
    {
    public:
        enum Type
        {
            TYPE_INT,
            TYPE_DOUBLE,
            TYPE_STRING,
            TYPE_BOOL,
            TYPE_VEC3D,
            TYPE_OPAQUE,
            TYPE_VECTOR,
            TYPE_UNKNOWN
        };

        static Type parseFromString(const char*);
        static const char* toString(Type type);
    };

//! Macro to declare a ID type
//! This ensures type safety because IDs will have different types.
//! They therefore cannot be mixed.
#define INF_ID_DECLARE(Name) \
    class Name              \
    {                        \
    public:                  \
        Name()               \
        :                    \
        id(-1)               \
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

    struct Status
    {
        enum StatusCode
        {
            STATUS_OK,
            STATUS_FILE_NOT_FOUND,
            STATUS_OBJECT_NOT_FOUND,
            STATUS_INVALID_PORT,
            STATUS_INVALID_SELECTION
        };

        StatusCode status{STATUS_OK};
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
        :
        status(STATUS_OK)
        {
			resultType = RESULT_NONE;
		}
    };

    typedef std::unordered_set<Node*> NodeSet;
}
