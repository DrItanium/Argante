#include <vga.h>

main() { 
	vga_setmode(1); 
	vga_ext_set(1,1); 
	vga_hasmode(1); 
	vga_setpalette(1,1,1,1);
	vga_setpalvec(1,1,1); 
	vga_getch(); 
	vga_getgraphmem(); 
	graph_mem=1;
	vga_lockvc(); 
	vga_unlockvc(); 
	vga_getkey(); 
}
