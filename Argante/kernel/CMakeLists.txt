
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
)

target_link_libraries(agrante common-includes)
