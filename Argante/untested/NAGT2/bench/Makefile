all: control integer

control: control.img
	../A2Kernel/a2 $^

control.img: control.agt
	../nagt $^ $@

integer: integer.img
	../A2Kernel/a2 $^

integer.img: integer.agt
	../nagt $^ $@

memory: memory.img
	../A2Kernel/a2 $^

memory.img: memory.agt
	../nagt $^ $@

clean:
	rm -f *.img

