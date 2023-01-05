cmake_minimum_required(VERSION 3.10)
project(hitagimon VERSION 0.1)
enable_language(CXX C)
file(READ kernel/.argbuild BUILDNUMBER)
execute_process(COMMAND whoami OUTPUT_VARIABLE CURRENT_USER OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND uname -n OUTPUT_VARIABLE MACHINE_NAME OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND date "+%A, %d %B %Y, %H:%M" OUTPUT_VARIABLE CURRENT_DATE_TIME OUTPUT_STRIP_TRAILING_WHITESPACE)
set(IDSTR "${CURRENT_USER}@${MACHINE_NAME} on ${CURRENT_DATE_TIME}")
string(STRIP ${BUILDNUMBER} BUILDNUMBER)
string(STRIP ${IDSTR} IDSTR)
add_compile_definitions(BUILD=${BUILDNUMBER} IDSTR="${IDSTR}")
add_custom_command(
	OUTPUT include/autogen-debug.h 
	COMMAND include/autogen.sh
	DEPENDS include/autogen.sh modules/syscall.h
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
# target_include_directories(hitagicortex PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
add_custom_command(
	OUTPUT compiler/autogen.h
	COMMAND compiler/autogen.sh
	DEPENDS compiler/autogen.sh modules/syscall.h include/exception.h
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
# there are symlinks that need to be there so make sure that they are there
add_custom_command(
	OUTPUT include/language.h
	COMMAND ln -sf ${CMAKE_CURRENT_SOURCE_DIR}/compiler/language.h include/language.h > /dev/null
	DEPENDS compiler/language.h
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_custom_command(
	OUTPUT include/syscall.h
	COMMAND ln -sf ${CMAKE_CURRENT_SOURCE_DIR}/modules/syscall.h include/syscall.h > /dev/null
	DEPENDS modules/syscall.h
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_library(common-includes INTERFACE)
target_sources(common-includes INTERFACE
	FILE_SET HEADERS
	BASE_DIRS include
	FILES include/acman.h
		  include/autogen-debug.h
		  include/bcode.h
		  include/bformat.h
		  include/cmd.h
		  include/config.h
		  include/console.h
		  include/debugger.h
		  include/evaluate.h
		  include/exception.h
		  include/language.h
		  include/linker.h
		  include/manager.h
		  include/memory.h
		  include/module.h
		  include/syscall.h
		  include/task.h)
add_library(compiler-generated INTERFACE)
target_sources(compiler-generated INTERFACE
	FILE_SET HEADERS
	BASE_DIRS compiler
	FILES compiler/autogen.h
)


add_subdirectory(compiler)
add_subdirectory(kernel)
add_subdirectory(modules)
add_subdirectory(hll)
# @todo add support for generating the IDSTR value



add_executable(actest
	tools/actest.c)
target_link_libraries(actest common-includes)

add_executable(binedit
	tools/binedit.c)
target_link_libraries(binedit common-includes)


add_executable(disasm
	tools/disasm.c)

target_link_libraries(disasm common-includes)


add_executable(vcpucons
	tools/vcpucons.c)

add_executable(agtexe
	tools/agtexe.c)

add_executable(agtman
	tools/GMI/agtmain.c
	tools/GMI/cursrtns.c
	tools/GMI/funcs.c
	tools/GMI/iortns.c
	tools/GMI/notglib.c
	tools/GMI/toplevel.h
)
target_compile_definitions(agtman PRIVATE CURSES)

target_link_libraries(agtman curses tinfo)
# @todo add support for the GTK GUI GMI interface
add_executable(ripcd
	tools/ripc-daemon/ripcd.c)

target_compile_definitions(ripcd PRIVATE CFGFILE="r1")

add_executable(ripc-cli
	tools/ripc-daemon/cli.c)

if(WITH_SSL)
	target_compile_definitions(ripcd PRIVATE USE_SSL)
	target_link_libraries(ripcd ssl crypto) 
	target_link_libraries(ripc-cli ssl crypto)
else()
	message(WARNING "building remote IPC connectivity daemon without SSL support! It won't affect rIPC functionality, but makes all remote data transfers dependent on security of the underlying communication layer. So, if you have secure or tunneled link between rIPC endpoints (ssh tunnels, VPN networks), it does not matter. Otherwise, it *might* cause problems.  Also, non-SSL daemons could not connect to SSL-enabled rIPC HUBs.")
	message(WARNING "The above is the original message from the makefiles.  OpenSSL support is deactivated because the implementation uses the deprecated and broken SSLv3 implementation. A more secure implementation will need to be used in its place")
endif()

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/conf DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/contrib DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Documentation DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Examples DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/fs DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/obsolete DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
