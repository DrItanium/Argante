/*
 * Argante-GTK frontend - main routine
 * (c) 2001 James Kehl <ecks@optusnet.com.au>
 * released under GPL, yada yada yada
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <fcntl.h>
#include "notglib.h"

#ifdef GTK
#include <gtk/gtk.h>
#else
#include <curses.h>
#endif

#include "toplevel.h"

int pid;
#ifndef GTK
/* Well, how else would we know when to quit?! */
int keepgoing=1;
/* Call the IO routines ourselves, if GTK won't oblige */
extern int IO_update();
extern void MW_update();
#endif

int main(int argc, char *argv[])
{
#ifdef GTK
	/* Initialize GTK */
	gtk_init( &argc, &argv );
#endif

	/* Attempt to pick up Argante PID from argv[1] */
	if (argc != 2)
	{
		fprintf(stderr, "Usage: %s <process id from agtback>\n", argv[0]);
		exit(1);
	}
	pid=atoi(argv[1]);
	
	if (IO_start(pid)) exit(1);
	MW_create();

#ifdef GTK
	/* Enter the event loop */
	gtk_main ();
#else
	while (keepgoing)
	{
		if (console_out_fd > 0) IO_update();
		MW_update();
	}
	endwin();
#endif
	
	return(0);
}

