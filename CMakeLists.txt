cmake_minimum_required(VERSION 3.22)
PROJECT(dag)
cmake_policy(SET CMP0114 NEW)

SET( CMAKE_CXX_STANDARD 17 )
SET( CMAKE_CXX_EXTENSIONS OFF )

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

#FIND_PACKAGE(GTest)
#FIND_PACKAGE(benchmark REQUIRED)
FIND_PACKAGE(Lua 5.4)

SET( ALL_PUBLIC_HEADERS include/Node.h include/Action.h include/Boundary.h include/Command.h include/CreateNode.h include/FileSystemTraverser.h include/Graph.h include/MemoryNodeLibrary.h include/MetaCoroutine.h include/MetaOperation.h include/MetaPort.h include/NodeCategory.h include/NodeDescriptor.h include/NodeEditorInterface.h include/NodeEditorLive.h include/NodeLibrary.h include/NodePlugin.h include/NodePluginScanner.h include/Nodes.h include/Port.h include/PortDescriptor.h include/SelectionInterface.h include/SelectionLive.h include/SignalPath.h include/SignalPathDef.h include/Transfer.h include/TypedPort.h include/Types.h include/TypeTraits.h include/Value.h
        include/DynamicLibrary.h
		include/MathNode.h
		include/NodeEditorFrontEnd.h
		include/NodeEditorFrontEndTUI.h
		include/NodeEditorBackEnd.h
		include/NodeEditorBackEndNodeEditorInterface.h
		include/Menu.h
		include/CloningFacility.h
		include/Fields.h
		include/KeyGenerator.h 
		include/MetaField.h
		include/UnionValue.h
)

SET( DEP_ROOT CACHE PATH "Dependency root" )

SET(HEADERS_GROUP "Header Files")

SOURCE_GROUP(
    ${HEADERS_GROUP}
    FILES ${ALL_PUBLIC_HEADERS}
)

SET(GENERATED_HEADERS_GROUP "Generated header files")
SOURCE_GROUP(
    ${GENERATED_HEADERS_GROUP}
    FILES ${GENERATED_HEADERS}
)

# Build the main inf library
SET( dag_SRC ${GENERATED_HEADERS} ${ALL_PUBLIC_HEADERS}
        src/Action.cpp
        src/Boundary.cpp
        src/Command.cpp
        src/CreateNode.cpp
        src/DynamicLibrary.cpp
        src/FileSystemTraverser.cpp
        src/Graph.cpp
        src/KeyGenerator.cpp
        src/MemoryNodeLibrary.cpp
        src/MetaCoroutine.cpp
        src/MetaField.cpp
        src/MetaOperation.cpp
        src/MetaPort.cpp
        src/Node.cpp
        src/NodeCategory.cpp
        src/NodeDescriptor.cpp
        src/NodeEditorLive.cpp
        src/NodeLibrary.cpp
        src/NodePlugin.cpp
        src/NodePluginScanner.cpp
        src/Nodes.cpp
        src/Port.cpp
        src/PortDescriptor.cpp
        src/SelectionLive.cpp
        src/SignalPath.cpp
		src/SignalPathDef.cpp
        src/Transfer.cpp
        src/Types.cpp
        src/UnionValue.cpp
 		src/MathsNode.cpp
		src/NodeEditorFrontEnd.cpp
		src/NodeEditorFrontEndTUI.cpp
		src/NodeEditorBackEndNodeEditorInterface.cpp
		src/Menu.cpp
		src/CloningFacility.cpp
)

set(CMAKE_XCODE_ATTRIBUTE_OTHER_CODE_SIGN_FLAGS "-o linker-signed")

ADD_LIBRARY( dag SHARED ${dag_SRC} )

TARGET_INCLUDE_DIRECTORIES( dag PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include ${PROJECT_BINARY_DIR}/include ${LUA_INCLUDE_DIR})
TARGET_LINK_DIRECTORIES( dag PUBLIC ${DEP_ROOT}/lib )
TARGET_LINK_LIBRARIES( dag PRIVATE dagbase GTest::gtest ${LUA_LIBRARIES})

set_target_properties( dag PROPERTIES DEFINE_SYMBOL DAG_LIBRARY )

REMOVE_DEFINITIONS( -DDAG_LIBRARY )


INCLUDE( CheckIncludeFile )
INCLUDE( CheckIncludeFileCXX )

#ADD_EXECUTABLE( demoSerialiseGraph src/demos/demoSerialiseGraph.cpp )
#target_link_libraries(demoSerialiseGraph inf)

CHECK_INCLUDE_FILE( windows.h HAVE_WINDOWS_H PATHS DOC "Whether we have the windows.h header, hopefully only on Windows!")
CHECK_INCLUDE_FILE( winsock2.h HAVE_WINSOCK2_H PATHS DOC "")
CHECK_INCLUDE_FILE( float.h HAVE_FLOAT_H  DOC "")
CHECK_INCLUDE_FILE( unistd.h HAVE_UNISTD_H  DOC "")
CHECK_INCLUDE_FILE( sys/types.h HAVE_SYS_TYPES_H  DOC "")
CHECK_INCLUDE_FILE( sys/socket.h HAVE_SYS_SOCKET_H  DOC "")
CHECK_INCLUDE_FILE( sys/time.h HAVE_SYS_TIME_H  DOC "")
CHECK_INCLUDE_FILE( sys/ioctl.h HAVE_SYS_IOCTL_H  DOC "")
CHECK_INCLUDE_FILE( sys/errno.h HAVE_SYS_ERRNO_H  DOC "")
CHECK_INCLUDE_FILE( sys/sysmp.h HAVE_SYS_SYSMP_H  DOC "")
CHECK_INCLUDE_FILE( sys/mman.h HAVE_SYS_MMAN_H  DOC "")
CHECK_INCLUDE_FILE( sys/stat.h HAVE_SYS_STAT_H  DOC "")
CHECK_INCLUDE_FILE( errno.h HAVE_ERRNO_H  DOC "")
CHECK_INCLUDE_FILE( netdb.h HAVE_NETDB_H  DOC "")
CHECK_INCLUDE_FILE( netinet/tcp.h HAVE_NETINET_TCP_H  DOC "")
CHECK_INCLUDE_FILE( netinet/in.h HAVE_NETINET_IN_H  DOC "")
CHECK_INCLUDE_FILE( arpa/inet.h HAVE_ARPA_INET_H  DOC "")
CHECK_INCLUDE_FILE( getopt.h HAVE_GETOPT_H  DOC "")
CHECK_INCLUDE_FILE( fcntl.h HAVE_FCNTL_H  DOC "")
CHECK_INCLUDE_FILE( stdint.h HAVE_STDINT_H  DOC "")
CHECK_INCLUDE_FILE( sys/timerfd.h HAVE_SYS_TIMERFD_H DOC "Whether we have the sys/timerfd header - only present on modern Linux distros, not on Control")
CHECK_INCLUDE_FILE( dirent.h HAVE_DIRENT_H DOC "Whether we have the dirent header, Linux and hopefully macOS X")
CHECK_INCLUDE_FILE( dlfcn.h HAVE_DLFCN_H DOC "Whether we have the dlfcn header for dynnamic libraries, Linux and hopefully macOS X")
CHECK_INCLUDE_FILE( TargetConditionals.h DOC "Whether we have the TargetConditionals header to detect which Apple platform we are on")
CHECK_INCLUDE_FILE_CXX( iostream HAVE_STD_IOSTREAM DOC "")
CHECK_INCLUDE_FILE_CXX( fstream HAVE_STD_FSTREAM DOC "")
CHECK_INCLUDE_FILE_CXX( iosfwd HAVE_STD_IOSFWD DOC "")
CHECK_INCLUDE_FILE_CXX( cassert HAVE_STD_CASSERT DOC "")
CHECK_INCLUDE_FILE_CXX( sstream HAVE_STD_SSTREAM DOC "")
CHECK_INCLUDE_FILE_CXX( stdexcept HAVE_STD_EXCEPT )

CONFIGURE_FILE( include/config/config.h.in ${PROJECT_BINARY_DIR}/include/config/config.h )
IF ( UNIX AND NOT APPLE )
CONFIGURE_FILE( etc/setup.sh.in ${PROJECT_BINARY_DIR}/etc/setup.sh )
ELSEIF ( APPLE )
CONFIGURE_FILE( etc/setup_macos.sh.in ${PROJECT_BINARY_DIR}/etc/setup.sh )
ELSEIF ( WIN32 )
CONFIGURE_FILE( etc/setup.bat.in ${PROJECT_BINARY_DIR}/etc/setup.bat )
ENDIF ( UNIX AND NOT APPLE )
CONFIGURE_FILE( etc/setup.bat.in ${PROJECT_BINARY_DIR}/etc/setup.bat )

SUBDIRS( src/tests/DagTest src/apps/DagBenchmark thirdparty/dagbase )

INSTALL( TARGETS dag
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

INSTALL( DIRECTORY etc DESTINATION . )
IF ( UNIX )
INSTALL( FILES ${PROJECT_BINARY_DIR}/etc/setup.sh DESTINATION . )
ELSEIF ( WIN32 )
INSTALL( FILES ${PROJECT_BINARY_DIR}/etc/setup.bat DESTINATION . )
ENDIF( UNIX )
add_library(NodePlugin SHARED include/NodePlugin.h
        src/NodePlugin.cpp)
set_target_properties( NodePlugin PROPERTIES DEFINE_SYMBOL NODEPLUGIN_LIBRARY )

TARGET_LINK_LIBRARIES(NodePlugin PRIVATE dag dagbase)

INSTALL( TARGETS NodePlugin LIBRARY DESTINATION lib/NodePlugins RUNTIME DESTINATION bin/NodePlugins )
