add_library(display SHARED
	display.c)

add_library(fs SHARED
	fs.c)

add_library(ipc SHARED
	ipc.c)

add_library(network SHARED
	network.c)

add_library(packet SHARED
	packet.c)
add_library(advmem SHARED
	advmem.c)
add_library(access SHARED
	access.c)
add_library(locallib SHARED
	locallib.c)

add_library(math SHARED
	math.c)

target_link_libraries(math common-includes)
target_link_libraries(locallib common-includes)
target_link_libraries(packet common-includes)
target_link_libraries(access common-includes)
target_link_libraries(advmem common-includes)
target_link_libraries(network common-includes)
target_link_libraries(ipc common-includes)
target_link_libraries(fs common-includes)
target_link_libraries(display common-includes)
