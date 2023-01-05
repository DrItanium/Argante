
add_executable(argante 
	bcode.c
	console.c
	manager.c
	module.c
	task.c
	acman.c
	debugger.c
	cmd.c
	main-boot.c
)

target_link_libraries(argante common-includes m dl)
add_custom_command(TARGET argante 
	POST_BUILD 
	COMMAND cp argante ${CMAKE_BINARY_DIR})
