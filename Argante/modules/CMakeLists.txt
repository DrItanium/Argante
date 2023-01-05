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

add_custom_command(TARGET fs 
	POST_BUILD 
	COMMAND cp libfs.so fs.so)
add_custom_command(TARGET display 
	POST_BUILD 
	COMMAND cp libdisplay.so display.so)
add_custom_command(TARGET ipc 
	POST_BUILD 
	COMMAND cp libipc.so ipc.so)
add_custom_command(TARGET network 
	POST_BUILD 
	COMMAND cp libnetwork.so network.so)
add_custom_command(TARGET packet 
	POST_BUILD 
	COMMAND cp libpacket.so packet.so)

add_custom_command(TARGET advmem 
	POST_BUILD 
	COMMAND cp libadvmem.so advmem.so)

add_custom_command(TARGET access 
	POST_BUILD 
	COMMAND cp libaccess.so access.so)

add_custom_command(TARGET locallib 
	POST_BUILD 
	COMMAND cp liblocallib.so locallib.so)
add_custom_command(TARGET math 
	POST_BUILD 
	COMMAND cp libmath.so math.so)
