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


add_executable(agtc 
	include/config.h
	include/exception.h
	modules/syscall.h
	compiler/autogen.h
	include/autogen-debug.h
	compiler/agtc.c)

target_include_directories(agtc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# @todo add support for generating the IDSTR value

add_executable(agrante 
	kernel/bcode.c
	kernel/console.c
	kernel/manager.c
	kernel/module.c
	kernel/task.c
	kernel/acman.c
	kernel/debugger.c
	kernel/cmd.c
	kernel/main-boot.c
	include/acman.h
	include/bcode.h
	include/cmd.h
	include/config.h
	include/console.h
	include/debugger.h
	include/evaluate.h
	include/language.h
	include/manager.h
	include/memory.h
	include/module.h
	include/task.h
)
target_include_directories(agrante PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(display  SHARED
	include/config.h
	include/task.h
	include/bcode.h
	include/module.h
	include/memory.h
	include/console.h
	include/syscall.h
	include/acman.h
	modules/display.c)

add_library(fs SHARED
	include/config.h
	include/task.h
	include/bcode.h
	include/module.h
	include/memory.h
	include/console.h
	include/syscall.h
	include/acman.h
	modules/fs.c)

add_library(ipc SHARED
	include/config.h
	include/task.h
	include/bcode.h
	include/module.h
	include/memory.h
	include/console.h
	include/syscall.h
	include/acman.h
	modules/ipc.c)

add_library(network SHARED
	include/config.h
	include/task.h
	include/bcode.h
	include/module.h
	include/memory.h
	include/console.h
	include/syscall.h
	include/exception.h
	include/acman.h
	modules/network.c)

add_library(packet SHARED
	include/config.h
	include/task.h
	include/bcode.h
	include/module.h
	include/memory.h
	include/console.h
	include/syscall.h
	include/acman.h
	include/exception.h
	modules/packet.c)
add_library(advmem SHARED
	include/config.h
	include/task.h
	include/bcode.h
	include/module.h
	include/memory.h
	include/console.h
	include/syscall.h
	include/acman.h
	include/exception.h
	modules/advmem.c)
add_library(access SHARED
	include/config.h
	include/task.h
	include/bcode.h
	include/module.h
	include/memory.h
	include/console.h
	include/syscall.h
	include/acman.h
	include/exception.h
	modules/access.c)
add_library(locallib SHARED
	include/config.h
	include/task.h
	include/bcode.h
	include/module.h
	include/memory.h
	include/console.h
	include/syscall.h
	include/acman.h
	include/exception.h
	modules/locallib.c)

add_library(math SHARED
	include/config.h
	include/task.h
	include/bcode.h
	include/module.h
	include/memory.h
	include/console.h
	include/syscall.h
	include/acman.h
	include/exception.h
	modules/math.c)

target_include_directories(display PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(fs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(ipc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(network PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(advmem PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(access PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(locallib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(math PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(packet PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(ahlt 
	include/config.h
	include/console.h
	include/bcode.h
	include/syscall.h
	include/bformat.h
	hll/ahlt.h
	hll/ahlt.c)

target_include_directories(ahlt PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(actest
	include/config.h
	include/console.h
	tools/actest.c)

target_include_directories(actest PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(binedit
	include/config.h
	include/bformat.h
	include/console.h
	tools/binedit.c)

target_include_directories(binedit PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(disasm
	include/config.h
	include/console.h
	include/bformat.h
	include/bcode.h
	include/task.h
	include/language.h
	tools/disasm.c)

target_include_directories(disasm PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)


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

