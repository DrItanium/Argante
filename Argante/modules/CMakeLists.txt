add_library(display SHARED
	${CMAKE_SOURCE_DIR}/include/config.h
	${CMAKE_SOURCE_DIR}/include/task.h
	${CMAKE_SOURCE_DIR}/include/bcode.h
	${CMAKE_SOURCE_DIR}/include/module.h
	${CMAKE_SOURCE_DIR}/include/memory.h
	${CMAKE_SOURCE_DIR}/include/console.h
	${CMAKE_SOURCE_DIR}/include/syscall.h
	${CMAKE_SOURCE_DIR}/include/acman.h
	display.c)

add_library(fs SHARED
	${CMAKE_SOURCE_DIR}/include/config.h
	${CMAKE_SOURCE_DIR}/include/task.h
	${CMAKE_SOURCE_DIR}/include/bcode.h
	${CMAKE_SOURCE_DIR}/include/module.h
	${CMAKE_SOURCE_DIR}/include/memory.h
	${CMAKE_SOURCE_DIR}/include/console.h
	${CMAKE_SOURCE_DIR}/include/syscall.h
	${CMAKE_SOURCE_DIR}/include/acman.h
	fs.c)

add_library(ipc SHARED
	${CMAKE_SOURCE_DIR}/include/config.h
	${CMAKE_SOURCE_DIR}/include/task.h
	${CMAKE_SOURCE_DIR}/include/bcode.h
	${CMAKE_SOURCE_DIR}/include/module.h
	${CMAKE_SOURCE_DIR}/include/memory.h
	${CMAKE_SOURCE_DIR}/include/console.h
	${CMAKE_SOURCE_DIR}/include/syscall.h
	${CMAKE_SOURCE_DIR}/include/acman.h
	ipc.c)

add_library(network SHARED
	${CMAKE_SOURCE_DIR}/include/config.h
	${CMAKE_SOURCE_DIR}/include/task.h
	${CMAKE_SOURCE_DIR}/include/bcode.h
	${CMAKE_SOURCE_DIR}/include/module.h
	${CMAKE_SOURCE_DIR}/include/memory.h
	${CMAKE_SOURCE_DIR}/include/console.h
	${CMAKE_SOURCE_DIR}/include/syscall.h
	${CMAKE_SOURCE_DIR}/include/exception.h
	${CMAKE_SOURCE_DIR}/include/acman.h
	network.c)

add_library(packet SHARED
	${CMAKE_SOURCE_DIR}/include/config.h
	${CMAKE_SOURCE_DIR}/include/task.h
	${CMAKE_SOURCE_DIR}/include/bcode.h
	${CMAKE_SOURCE_DIR}/include/module.h
	${CMAKE_SOURCE_DIR}/include/memory.h
	${CMAKE_SOURCE_DIR}/include/console.h
	${CMAKE_SOURCE_DIR}/include/syscall.h
	${CMAKE_SOURCE_DIR}/include/acman.h
	${CMAKE_SOURCE_DIR}/include/exception.h
	packet.c)
add_library(advmem SHARED
	${CMAKE_SOURCE_DIR}/include/config.h
	${CMAKE_SOURCE_DIR}/include/task.h
	${CMAKE_SOURCE_DIR}/include/bcode.h
	${CMAKE_SOURCE_DIR}/include/module.h
	${CMAKE_SOURCE_DIR}/include/memory.h
	${CMAKE_SOURCE_DIR}/include/console.h
	${CMAKE_SOURCE_DIR}/include/syscall.h
	${CMAKE_SOURCE_DIR}/include/acman.h
	${CMAKE_SOURCE_DIR}/include/exception.h
	advmem.c)
add_library(access SHARED
	${CMAKE_SOURCE_DIR}/include/config.h
	${CMAKE_SOURCE_DIR}/include/task.h
	${CMAKE_SOURCE_DIR}/include/bcode.h
	${CMAKE_SOURCE_DIR}/include/module.h
	${CMAKE_SOURCE_DIR}/include/memory.h
	${CMAKE_SOURCE_DIR}/include/console.h
	${CMAKE_SOURCE_DIR}/include/syscall.h
	${CMAKE_SOURCE_DIR}/include/acman.h
	${CMAKE_SOURCE_DIR}/include/exception.h
	access.c)
add_library(locallib SHARED
	${CMAKE_SOURCE_DIR}/include/config.h
	${CMAKE_SOURCE_DIR}/include/task.h
	${CMAKE_SOURCE_DIR}/include/bcode.h
	${CMAKE_SOURCE_DIR}/include/module.h
	${CMAKE_SOURCE_DIR}/include/memory.h
	${CMAKE_SOURCE_DIR}/include/console.h
	${CMAKE_SOURCE_DIR}/include/syscall.h
	${CMAKE_SOURCE_DIR}/include/acman.h
	${CMAKE_SOURCE_DIR}/include/exception.h
	locallib.c)

add_library(math SHARED
	${CMAKE_SOURCE_DIR}/include/config.h
	${CMAKE_SOURCE_DIR}/include/task.h
	${CMAKE_SOURCE_DIR}/include/bcode.h
	${CMAKE_SOURCE_DIR}/include/module.h
	${CMAKE_SOURCE_DIR}/include/memory.h
	${CMAKE_SOURCE_DIR}/include/console.h
	${CMAKE_SOURCE_DIR}/include/syscall.h
	${CMAKE_SOURCE_DIR}/include/acman.h
	${CMAKE_SOURCE_DIR}/include/exception.h
	math.c)

target_include_directories(display PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(fs PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(ipc PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(network PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(advmem PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(access PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(locallib PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(math PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(packet PUBLIC ${CMAKE_SOURCE_DIR}/include)
