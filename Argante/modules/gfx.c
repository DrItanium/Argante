/*

   Argante virtual OS
   ------------------

   Graph routines 
   
   Status: done, but just for amusement
   
   Author:     Lukasz Jachowicz <honey@linux.net.pl>
   Maintainer: Lukasz Jachowicz <honey@linux.net.pl>
   
*/

#include <vga.h>
#include <unistd.h>
#include <sys/types.h>

#include "config.h"
#include "task.h"
#include "bcode.h"
#include "module.h"
#include "memory.h"
#include "console.h"
#include "syscall.h"
#include "acman.h"

int lib_inited;
int *int_pointer; //temp variable
vga_modeinfo *modeinfo;
int gfx_memory;	// memory needed for one video page in current mode
#define no_of_modes 32  // number of gfx_modes in table

char gfx_modes[no_of_modes][25] = {
    "gfx/output/text",
    "gfx/output/320x200/16",
    "gfx/output/640x200/16",
    "gfx/output/640x350/16",
    "gfx/output/640x480/16",
    "gfx/output/320x200/256",
    "gfx/output/320x240/256",
    "gfx/output/320x400/256",
    "gfx/output/360x480/256",
    "gfx/output/640x480/2",
    "gfx/output/640x480/256",
    "gfx/output/800x600/256",
    "gfx/output/1024x768/256",
    "gfx/output/1280x1024/256",
    "gfx/output/320x200/32K",
    "gfx/output/320x200/64K",
    "gfx/output/320x200/16M",
    "gfx/output/640x480/32K",
    "gfx/output/640x480/64K",
    "gfx/output/640x480/16M",
    "gfx/output/800x600/32K",
    "gfx/output/800x600/64K",
    "gfx/output/800x600/16M",
    "gfx/output/1024x768/32K",
    "gfx/output/1024x768/64K",
    "gfx/output/1024x768/16M",
    "gfx/output/1280x1024/32K",
    "gfx/output/1280x1024/64K",
    "gfx/output/1280x1024/16M",
    "gfx/output/800x600/16",
    "gfx/output/1024x768/16",
    "gfx/output/1280x1024/16"
}; // of modes

void syscall_load(int* x) {
  if (geteuid() == 0) {
  *(x++)=SYSCALL_GFX_SETMODE;
  *(x++)=SYSCALL_GFX_CHECKMODE;
  *(x++)=SYSCALL_GFX_CLEARSCREEN;
  *(x++)=SYSCALL_GFX_MEMCOPY;
  *(x++)=SYSCALL_GFX_VC;
  *(x++)=SYSCALL_GFX_SETPALETTESNGL;
  *(x++)=SYSCALL_GFX_SETPALETTE;
  *(x++)=SYSCALL_GFX_GETCHAR;
  *(x++)=SYSCALL_GFX_SETCLUT8;
  *x=SYSCALL_ENDLIST;
  lib_inited=-1;

    printk(">> WARNING: gfx module is an experimental code.\n");
    printk(">> It is not stable, some console problems might occour.\n");
    printk(">> Please do not load this module if you do not need it.\n");

  printk("GFX: Argante's gfx module loaded...\n");
  }
  else printk("GFX: Argante's gfx module cannot be loaded - you aren't root...\n");
} // of syscall_load

void syscall_task_cleanup(int c) {
	if (lib_inited==c) {
	    if (vga_getcurrentmode()!=0) {
	      vga_setmode(TEXT);
	      lib_inited=-1;
	      printk("GFX: Be happy, gfx module cleaned up the mess made by your software...\n");
	    } // of if mode!=3
	}
} // of syscall_task_cleanup

void syscall_unload(void) {
	    if (vga_getcurrentmode()!=0) {
	      vga_setmode(TEXT);
	      lib_inited=-1;
	    } // of if mode!=3
} // of syscall_module_cleanup

void syscall_handler(int c, int num) {
char *virtual_address;	// address of user's memory block

  switch (num) {

	case SYSCALL_GFX_VC:		// if u0 != 0 lock console switching
		VALIDATE(c,"none","gfx/console/vclock");
		check_status();
		if (cpu[c].uregs[0]!=0)	vga_unlockvc();
		else vga_lockvc();
		return;
		break;

	case SYSCALL_GFX_SETMODE:	// inits svgalib and sets the u0 mode
		VALIDATE(c,"none","gfx/init");
		check_status();

		if (lib_inited==-1) {
		    vga_runinbackground(0);
		    vga_disabledriverreport();

		    if (vga_init()!=0) { 
		       non_fatal(ERROR_GFX_INIT,"GFX: Cannot initialize gfx card",c);
		       failure=1;
		       lib_inited=-1;
		       return;
		       }

		    else {
		      printk("GFX: Library initialized successfully. \n");
		      lib_inited=c;
		      }
		}
		
		if (lib_inited!=c) {
		   if (lib_inited!=-1)
		      non_fatal(ERROR_GFX_BUSY,"GFX: Another program uses gfx module",c);
		      failure=1;
		      return;
		}
		
		if (lib_inited==c) {
		    if (cpu[c].uregs[0]>no_of_modes) {
		       non_fatal(ERROR_GFX_INIT,"GFX: This mode doesn't exists!",c);
		       failure=1;
		       return;
		    }
		    VALIDATE(c,gfx_modes[cpu[c].uregs[0]],"gfx/setmode");
		    check_status();
		    if (vga_hasmode(cpu[c].uregs[0])) {
		      cpu[c].uregs[1]=vga_setmode(cpu[c].uregs[0]);
		      modeinfo=vga_getmodeinfo(cpu[c].uregs[0]);
		      gfx_memory=modeinfo->memory;
		      if (gfx_memory==0) gfx_memory=256*1024; // I guess that 256 kB is safe enough
		    }
		    else {
		      non_fatal(ERROR_GFX_INIT,"GFX: I can't use this mode!",c);
		      failure=1;
	    	      return;
		    }
		}
		return;
		break;


	case SYSCALL_GFX_CHECKMODE:	// returns u0=0 if mode isn't supported 
					// by your hardware
		if (cpu[c].uregs[0]>no_of_modes) {
		  cpu[c].uregs[0]=0;		
		}
                else if (!is_permitted(c,gfx_modes[cpu[c].uregs[0]],"gfx/setmode"))
		  cpu[c].uregs[0]=0;
		else
		  cpu[c].uregs[0]=vga_hasmode(cpu[c].uregs[0]);
		return;
		break;
		

	case SYSCALL_GFX_CLEARSCREEN:	// clears the screen
	if (lib_inited!=c) {
	   non_fatal(ERROR_GFX_NOTINITED,"GFX: You haven't initialized your card yet",c);
	   failure=1;
	   return;
	}
	else
		vga_clear();
		return;
		break;

	
	case SYSCALL_GFX_MEMCOPY:	// copies u1 bytes from *u0 
					// to video buffer

	if (lib_inited!=c) {
	   non_fatal(ERROR_GFX_NOTINITED,"GFX: You haven't initialized your card yet",c);
	   failure=1;
	   return;
	}
	else {
		virtual_address=verify_access(c,cpu[c].uregs[0],(cpu[c].uregs[1]+3)/4,MEM_FLAG_READ);
		if (!virtual_address) {
		  non_fatal(ERROR_PROTFAULT,"GFX: Attempt to access protected memory",c);
		  failure=1;
		  return;
		}
		if (cpu[c].uregs[1] <= gfx_memory)
		    memcpy(graph_mem, virtual_address, cpu[c].uregs[1]);
		else {
		  non_fatal(ERROR_GFX_TOOMUCH,"GFX: You've tried to copy too much data",c);
		  failure=1;
		  return;
		} // of else
	} // of else
		return;
		break;
	
	case SYSCALL_GFX_SETPALETTESNGL:	// sets color u0 to (u1,u2,u3)
	if (lib_inited!=c) {
	   non_fatal(ERROR_GFX_NOTINITED,"GFX: You haven't initialized your card yet",c);
	   failure=1;
	   return;
	}
	else
		vga_setpalette(cpu[c].uregs[0],cpu[c].uregs[1],cpu[c].uregs[2],cpu[c].uregs[3]);
		return;
		break;
	
	case SYSCALL_GFX_SETCLUT8:	// sets 8 bits per color (instead of 4)
	if (lib_inited!=c) {
	   non_fatal(ERROR_GFX_NOTINITED,"GFX: You haven't initialized your card yet",c);
	   failure=1;
	   return;
	}
	else
		vga_ext_set(VGA_EXT_SET,VGA_CLUT8);
		return;
		break;

	case SYSCALL_GFX_GETCHAR:

	if (lib_inited!=c) {
	   non_fatal(ERROR_GFX_NOTINITED,"GFX: You haven't initialized your card yet",c);
	   failure=1;
	   return;
	}
 
	cpu[c].uregs[0]=vga_getkey();
	return;
	break;

	
	case SYSCALL_GFX_SETPALETTE:	// gets the palette table from *u0, 
					// the 1st color changed is u1 and
					// the number of colors in palette
					// table is u2
	if (lib_inited!=c) {
	   non_fatal(ERROR_GFX_NOTINITED,"GFX: You haven't initialized your card yet",c);
	   failure=1;
	   return;
	}

        virtual_address=verify_access(c,cpu[c].uregs[0],(cpu[c].uregs[2]+3)*3/4,MEM_FLAG_READ);
        if (!virtual_address) {
          non_fatal(ERROR_PROTFAULT,"GFX: Attempt to access protected memory.",c);
          failure=1;
          return;
        }

printk("Setting palette starting at %d, %d colors\n",cpu[c].uregs[1],cpu[c].uregs[2]);

	vga_setpalvec(cpu[c].uregs[1],cpu[c].uregs[2],(int*)virtual_address);
	return;
	break;
		
  } // of switch
} // of syscall_handler
