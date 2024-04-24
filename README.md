# nodebackend
Backend for Node based behaviours

Intended to be used by multiple projects so it will not restrict the types of behaviour that are created.

It will be factored out of an existing project to serve as its backend.

There will be the following concepts:
* Graph a graph of nodes and edges
* Node a unit of behaviour
* Port a connection point for wiring up Node inputs and outputs
* SignalPath a connection between Ports
* OytputStream to serialise a Graph
* InputStream to read previously serialised objects
There will be an EditorInterface that defines operations on these elements.
Typically, implementations will use the Command pattern and rollback using serialisation.
A Graph will provd3 the following operations:
* Create a Node
* Delete a Node 
* Create a SignalPath 
* Delete a SignalPath 
* Create a child Graph
* Delete a child Graph
* Serialise to a binary stream
* Construct from a binary stream
* Determine an evaluation order to satisfy dependencies between Nodes
* Persistence to a Lua based file format

A Port will support:
* Add an incoming connection
* Remove an incoming connection
* Add an outgoing connection
* Remove an outgoing connection
* Evaluation
* Serialisation to a binary stream
* Construction from a binary stream

A NodeLibrary will aupport:
* Instantiation of a Node
* Instantiation of a Port
* Loading of a plugin to support new node types

We will use the Prototype pattern to instantiate Nodes and Ports.  Thus includes during deserialisation when the prototype gives us access to the concrete class so we can call the constructor from stream without having to dynamic_cast<> to different types.  This is important for plugins where we do not know tbe type of the Prototype at compile time.

There is a Lua based persistent format that supports all components of a Graph including children to any level of nesting.

There will be a customisable value type based on std::variant

## Dependencies

* C++ 17
* Lua >= 5.4
* possibly a base library that provides   
  * the Lua interface
  * stream interface

