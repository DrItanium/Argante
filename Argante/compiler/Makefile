
all: autogen.h agtc 

autogen.h: ../include/exception.h ../modules/syscall.h
	cd ..; ./include/autogen.sh; ./compiler/autogen.sh

clean:
	rm -f agtc autogen.h ../include/autogen-debug.h
