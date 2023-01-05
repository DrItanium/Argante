add_executable(ahlt 
	ahlt.h
	ahlt.c)
target_link_libraries(ahlt common-includes)


add_custom_command(TARGET ahlt
	POST_BUILD 
	COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/acc ${CMAKE_BINARY_DIR}/hll
	COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/elim ${CMAKE_BINARY_DIR}/hll
	COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/gfx.* ${CMAKE_BINARY_DIR}/hll
)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/examples DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/include DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
