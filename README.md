# Dag

## Supported Platforms

* Windows 10,11
* Ubuntu Linux 22.04,24.04
* macOS 10.15 Intel
* macOS 15.1+ Apple Silicon
* Raspberry Pi OS 12 on Pi4, Pi5

## Dependencies

* C++ 17
* Lua >= 5.4
* possibly a base library that provides   
  * the Lua interface
  * stream interface

## Building

### MacOS 15.1 Sequoia Apple Silicon

* Install brew
* Install Apple clang(using gcc as the name for some reason) and cmake
```bash
brew install gcc cmake
```
* Clone the repo
```bash
git clone https://github.com/triblatron/nodebackend
```
* Configure and generate the build system
```bash
mkdir nodebackend_build && cd nodebackend_build

cmake -C ../nodebackend/Automation/Build/InitialCacheLinuxGitHub.txt -B . -S ../nodebackend
```
* Build
```bash
cmake --build . --target install -j <number_of_cores> --config Release
```
* Test
```bash
NodeBackendTest
```
* Note that the repo used to be called nodebackend and the code has not yet been updated to use dag

# Synopsis

Intended to be used by multiple projects so it will not restrict the types of behaviour that are created.

It will be factored out of an existing project to serve as its backend.

There will be the following concepts:
* Graph a graph of nodes and edges
* Node a unit of behaviour
* Port a connection point for wiring up Node inputs and outputs
* SignalPath a connection between Ports
* OutputStream to serialise a Graph
* InputStream to read previously serialised objects

There will be an EditorInterface that defines operations on these elements.
Typically, implementations will use the Command pattern and rollback using serialisation.

A Graph will provde the following operations:
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

## Constraints on port values
The Port has a MetaPort to describe its type.  One possible type for a Port is an enum.  This will typically be mapped to a combo box in the UI.  It can take on a range of values each with an associated string to describe it.  Examples would be units for a maths node or times of day in a scenario specification.

The Lua persistent format and serialisation will both need to carry the constraints, preferably only once.  This is easy to achieve for serialisation but it is unclear how it will map to Lua.  We would have to announce Node types once with their MetaPorts but that does not cover DynamicNodes or Boundary Nodes which have a variable number of Ports.  We will need to announce unique MetaPorts the first time they are encountered and reference them subsequently like in serialisation.

## Progress
* ~~Node base class~~
* ~~Port~~
* ~~Boundary node with variable number of ports~~
* ~~Graph~~
* ~~Solving of dependencies using Kaaaaaaaaahn's algorithm~~
* ~~Serialisation of graphs~~
* ~~Lua-based persistent format~~
* ~~Plugins to support nodes not known at compile time~~

## Release Schedule
When I deem it feature complete, there will be an 0.1 release that will eventually lead to a 1.0 milestone.
It looks like that will happen this year.
This is a side project along with [dagbase](https://github.com/triblatron/dagbase), [dagui](https://github.com/triblatron/dagui) and [Repton Infinity](https://github.com/triblatron/reptoninfinity)

