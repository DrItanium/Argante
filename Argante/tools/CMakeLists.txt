add_executable(actest
	actest.c)
target_link_libraries(actest common-includes)

add_executable(binedit
	binedit.c)
target_link_libraries(binedit common-includes)


add_executable(disasm
	disasm.c)

target_link_libraries(disasm common-includes)


add_executable(vcpucons
	vcpucons.c)

add_executable(agtexe
	agtexe.c)

add_executable(agtman
	GMI/agtmain.c
	GMI/cursrtns.c
	GMI/funcs.c
	GMI/iortns.c
	GMI/notglib.c
	GMI/toplevel.h
)
target_compile_definitions(agtman PRIVATE CURSES)

target_link_libraries(agtman curses tinfo)
# @todo add support for the GTK GUI GMI interface
add_executable(ripcd
	ripc-daemon/ripcd.c)

target_compile_definitions(ripcd PRIVATE CFGFILE="r1")

add_executable(ripc-cli
	ripc-daemon/cli.c)

if(WITH_SSL)
	target_compile_definitions(ripcd PRIVATE USE_SSL)
	target_link_libraries(ripcd ssl crypto) 
	target_link_libraries(ripc-cli ssl crypto)
else()
	message(WARNING "building remote IPC connectivity daemon without SSL support! It won't affect rIPC functionality, but makes all remote data transfers dependent on security of the underlying communication layer. So, if you have secure or tunneled link between rIPC endpoints (ssh tunnels, VPN networks), it does not matter. Otherwise, it *might* cause problems.  Also, non-SSL daemons could not connect to SSL-enabled rIPC HUBs.")
	message(WARNING "The above is the original message from the makefiles.  OpenSSL support is deactivated because the implementation uses the deprecated and broken SSLv3 implementation. A more secure implementation will need to be used in its place")
endif()
