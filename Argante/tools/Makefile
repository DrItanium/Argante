TOOLS = actest binedit disasm vcpucons agtexe

all: $(TOOLS) subdirs

subdirs:
	cd GMI; make
	cd ripc-daemon; make

clean:
	rm -f $(TOOLS)
	cd GMI; make clean
	cd ripc-daemon; make clean

