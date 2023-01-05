
add_executable(agtc 
	${CMAKE_SOURCE_DIR}/compiler/autogen.h
	agtc.c)

target_link_libraries(agtc common-includes)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/examples DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/benchmarks DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

