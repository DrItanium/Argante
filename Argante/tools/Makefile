TOOLS = actest binedit disasm vcpucons agtexe

all: $(TOOLS) subdirs

subdirs:
	@$(MAKE) -C GMI         TOP=$(TOP)
	@$(MAKE) -C ripc-daemon TOP=$(TOP)

clean:
	rm -f $(TOOLS)
	@$(MAKE) -C GMI         TOP=$(TOP) clean
	@$(MAKE) -C ripc-daemon TOP=$(TOP) clean

