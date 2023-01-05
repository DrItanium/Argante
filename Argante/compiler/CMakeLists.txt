
add_executable(agtc 
	agtc.c)

target_link_libraries(agtc PUBLIC common-includes compiler-generated)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/examples DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/benchmarks DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

