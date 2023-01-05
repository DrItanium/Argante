cmake_minimum_required(VERSION 3.10)
project(argante VERSION 0.1)
enable_language(CXX C)
add_link_options(-rdynamic)
file(READ kernel/.argbuild BUILDNUMBER)
execute_process(COMMAND whoami OUTPUT_VARIABLE CURRENT_USER OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND uname -n OUTPUT_VARIABLE MACHINE_NAME OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND date "+%A, %d %B %Y, %H:%M" OUTPUT_VARIABLE CURRENT_DATE_TIME OUTPUT_STRIP_TRAILING_WHITESPACE)
set(IDSTR "${CURRENT_USER}@${MACHINE_NAME} on ${CURRENT_DATE_TIME}")
string(STRIP ${BUILDNUMBER} BUILDNUMBER)
string(STRIP ${IDSTR} IDSTR)
add_compile_definitions(BUILD=${BUILDNUMBER} IDSTR="${IDSTR}")
add_custom_target(include-autogen-debug
	#OUTPUT include/autogen-debug.h 
	COMMAND include/autogen.sh
	DEPENDS include/autogen.sh modules/syscall.h
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
# target_include_directories(hitagicortex PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
add_custom_target(compiler-autogen
	#OUTPUT compiler/autogen.h
	COMMAND compiler/autogen.sh
	DEPENDS compiler/autogen.sh modules/syscall.h include/exception.h
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
# there are symlinks that need to be there so make sure that they are there
add_custom_target(include-language
	#OUTPUT include/language.h
	COMMAND ln -sf ${CMAKE_SOURCE_DIR}/compiler/language.h include/language.h > /dev/null
	DEPENDS compiler/language.h
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_custom_target(include-syscall
	#OUTPUT include/syscall.h
	COMMAND ln -sf ${CMAKE_SOURCE_DIR}/modules/syscall.h include/syscall.h > /dev/null
	DEPENDS modules/syscall.h
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

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
add_dependencies(common-includes 
	include-language 
	include-autogen-debug
	include-syscall)

add_library(compiler-generated INTERFACE)
target_sources(compiler-generated INTERFACE
	FILE_SET HEADERS
	BASE_DIRS compiler
	FILES compiler/autogen.h
)

add_dependencies(compiler-generated compiler-autogen)

add_subdirectory(compiler)
add_subdirectory(kernel)
add_subdirectory(modules)
add_subdirectory(hll)
add_subdirectory(tools)
# @todo add support for generating the IDSTR value





file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/conf DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/contrib DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Documentation DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Examples DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/fs DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/obsolete DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
