
add_executable(agtc 
	${CMAKE_SOURCE_DIR}/include/config.h
	${CMAKE_SOURCE_DIR}/include/exception.h
	${CMAKE_SOURCE_DIR}/include/syscall.h
	${CMAKE_SOURCE_DIR}/include/autogen-debug.h
	autogen.h
	agtc.c)

target_include_directories(agtc PUBLIC ${CMAKE_SOURCE_DIR}/include)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/examples DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/benchmarks DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

