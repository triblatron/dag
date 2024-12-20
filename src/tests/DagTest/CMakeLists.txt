SET( DagTest_SRC DagTest.cpp )
ADD_EXECUTABLE( DagTest ${DagTest_SRC} )
TARGET_LINK_LIBRARIES( DagTest PRIVATE dagbase;dag;GTest::gtest;GTest::gtest_main;${LUA_LIBRARIES})
INSTALL(TARGETS DagTest RUNTIME DESTINATION bin)
