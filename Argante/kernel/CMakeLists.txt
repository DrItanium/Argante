
add_executable(agrante 
	bcode.c
	console.c
	manager.c
	module.c
	task.c
	acman.c
	debugger.c
	cmd.c
	main-boot.c
	${CMAKE_SOURCE_DIR}/include/acman.h
	${CMAKE_SOURCE_DIR}/include/bcode.h
	${CMAKE_SOURCE_DIR}/include/cmd.h
	${CMAKE_SOURCE_DIR}/include/config.h
	${CMAKE_SOURCE_DIR}/include/console.h
	${CMAKE_SOURCE_DIR}/include/debugger.h
	${CMAKE_SOURCE_DIR}/include/evaluate.h
	${CMAKE_SOURCE_DIR}/include/language.h
	${CMAKE_SOURCE_DIR}/include/manager.h
	${CMAKE_SOURCE_DIR}/include/memory.h
	${CMAKE_SOURCE_DIR}/include/module.h
	${CMAKE_SOURCE_DIR}/include/task.h
)
target_include_directories(agrante PUBLIC ${CMAKE_SOURCE_DIR}/include)
