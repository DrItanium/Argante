

add_executable(agtc 
	include/config.h
	include/exception.h
	modules/syscall.h
	compiler/autogen.h
	include/autogen-debug.h
	compiler/agtc.c)

target_include_directories(agtc PUBLIC ${CMAKE_SOURCE_DIR}/include)

