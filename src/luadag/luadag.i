%module luadag

%{
#include "core/Node.h"
#include "core/Port.h"
#include "core/PortDirection.h"
#include "core/Status.h"
#include "util/Name.h"
#include "Scripting.h"
%}

%define DAGBASE_API
%enddef

%define DAG_API
%enddef

%define DAGUI_API
%enddef

%define DAGBASE_REGISTER_ENUM
%enddef

//%define INF_ID_DECLARE
//%enddef
%ignore dagbase::Class;
%ignore dagbase::Editable;
%ignore dagbase::Node::writeToStream;
%ignore dagbase::Node::clone;
%ignore dagbase::Node::equals;
%ignore dagbase::Node::setParent;
%ignore dagbase::Node::parent;
%ignore dagbase::Node::setId;
%ignore dagbase::Node::id;
%ignore dagbase::Node::setCategory;
%ignore dagbase::Node::category;
%ignore dagbase::Node::indexOfPort;
%ignore dagbase::Node::reconnectInputs;
%ignore dagbase::Node::reconnectOutputs;
%ignore dagbase::Node::find;
%ignore dagbase::Node::writeFlat;
%ignore dagbase::Node::readFlat;
%ignore dagbase::Node::removePort;
%ignore dagbase::Node::addValue;
%ignore dagbase::Node::value;

%ignore dagbase::Port::Port;
%ignore dagbase::Port::setId;
%ignore dagbase::Port::id;
%ignore dagbase::Port::initialValue() const;
%ignore dagbase::Port::type;
%ignore dagbase::Port::setDir;
%ignore dagbase::Port::dir;
%ignore dagbase::Port::addOutgoingConnection;
%ignore dagbase::Port::addIncomingConnection;
%ignore dagbase::Port::clone;
%ignore dagbase::Port::writeToStream;
%ignore dagbase::Port::writeFlat;
%ignore dagbase::Port::readFlat;
%ignore dagbase::Port::equals;
%ignore dagbase::Port::setInitialValue;
%ignore dagbase::Port::initialValue;
%ignore dagbase::Port::value;
%ignore dagbase::Port::setValue;
%ignore dagbase::Port::setValueId;
%ignore dagbase::Port::valueId;
%ignore dagbase::Port::find;

%ignore dagbase::Name::find;

%ignore dagbase::Status::find;
%ignore operator<<(std::ostream&, const Status&);
%rename(toUInt32) dagbase::Name::operator std::uint32_t;
%include "typemaps.i"
%include "stdint.i"
%typemap(out) uint32_t {
 lua_pushinteger(L, (lua_Integer)$1);
 SWIG_arg++;
}


%include "core/Node.h"
%include "core/Port.h"
%include "core/PortDirection.h"
%include "core/Status.h"
%include "util/Name.h"
%include "Scripting.h"

%template(NodeID) dagbase::Name<dagbase::Node>;
%template(PortID) dagbase::Name<dagbase::Port>;
