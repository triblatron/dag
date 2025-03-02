ADD_EXECUTABLE( DagBenchmark DagBenchmark.cpp )
#TARGET_INCLUDE_DIRECTORIES( DagBenchmark PUBLIC ${CMAKE_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include ${LUA_INCLUDE_DIR} ${ANTLR_Reptol_OUTPUT_DIR})

SET(BENCHMARK_LIBS ${benchmark_LIBRARY})
#MESSAGE(${benchmark_LIBRARY})
IF (WIN32)
	SET(BENCHMARK_LIBS ${BENCHMARK_LIBS} Shlwapi)
	target_compile_definitions(DagBenchmark PUBLIC BENCHMARK_STATIC_DEFINE)
ENDIF(WIN32)
target_link_libraries(DagBenchmark PRIVATE benchmark::benchmark dag dagbase)

INSTALL( TARGETS DagBenchmark
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
