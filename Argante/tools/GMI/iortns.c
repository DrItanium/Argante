/*
 * Argante-GTK frontend - IO routines.
 * (c) 2001 James Kehl <ecks@optusnet.com.au>
 * released under GPL, yada yada yada
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <fcntl.h>
#include <string.h>

#include "notglib.h"
#ifdef GTK
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#endif

#include <errno.h>

#include "toplevel.h"


#ifdef GTK
static gint list_update_tag;
#endif

int console_in_fd;
int console_out_fd;

/*
 * This code will break lines if there's more than 1024
 * bytes to handle. IT SHOULDN'T, but it's no real issue.
 * 
 * Note: this sort of thing happens in funcs.c also,
 * in input_modules and input_tasks.
 */

#ifdef GTK
static gint IO_update(gpointer data, gint source, GdkInputCondition condition)
#else
int IO_update()
#endif
{
	gchar buf[1024];
	gchar *bp, *bpz;
	
	int s;

	bp=&buf[0];
	s=read(console_out_fd, buf, sizeof(buf)-1);

	if (s > 0) {
		buf[s]=0;
		while (bp)
		{
			bpz=strchr(bp, '\n');
			if (bpz)
			{
				bpz[0]=0;
				bpz++;
			}
			MW_add_line(bp);
			bp=bpz;
		}
		return TRUE;
	}
	/* This doesn't happen with read-from-file,
	 * unless you enter the wrong pid...
	 * 
	 * network connections should support this.
	 */
	else if (s < 0) {
		strncpy(buf, "<!!!!> Gtk-Argante: Connection lost", sizeof(buf) - 1);
		MW_add_line(bp);
		/* They LIED when they said returning false deregisters! */
		IO_stop();
		return FALSE;
	}
	return TRUE;
}

/* To guarantee that Argante-VM sees our command
 * and has a fair chance to act on it.
 *
 * Most of the problems should have been fixed by
 * opening the console O_APPEND, but there is still the
 * occasional empty dialog.
 */
void IO_writestr(const char *z)
{
	fsync(console_in_fd);
	write(console_in_fd, z, strlen(z));
	fsync(console_in_fd);
	usleep(100);
}

int IO_start(int process)
{
	char buf[PATH_MAX];
	
	/* TODO: agtses error checks */
	
	snprintf(buf, sizeof(buf), "/proc/%d/fd/0", process);
	console_in_fd=open(buf, O_WRONLY | O_APPEND);
	
	if (console_in_fd < 0) {
		perror("error opening in");
		return errno || 1; /* If it fails, make sure we let on */
	}
	
	snprintf(buf, sizeof(buf), "/proc/%d/fd/1", process);
	console_out_fd=open(buf, O_RDONLY);
	
	if (console_out_fd < 0) {
		perror("error opening out");
		return errno || 1;
	}

#ifdef GTK
	/*
	 * In my GTK/GDK (supposedly 1.2.8) inputs don't work too well.
	 * If you feel lucky, try switching from timeouts
	 */
	list_update_tag=gtk_timeout_add(100, (GtkFunction) IO_update, &console_out_fd);
	/* list_update_tag=gdk_input_add(0, GDK_INPUT_READ, list_update, &console_out_fd); */
#endif

	return 0;
}

void IO_stop()
{

#ifdef GTK
	gtk_timeout_remove(list_update_tag);
#endif
	close(console_in_fd);
	close(console_out_fd);
	console_in_fd=console_out_fd=-1;
}
