/*
 * Argante-GTK frontend - button functions
 * (c) 2001 James Kehl <ecks@optusnet.com.au>
 * released under GPL, yada yada yada
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <string.h>
#include "notglib.h"

#ifdef GTK
#include <gtk/gtk.h>
#else
#include <curses.h>
#include <sys/wait.h>
#endif

#ifndef OPEN_MAX
#define OPEN_MAX 256
#endif

#include "toplevel.h"

#ifdef GTK
#define XTERM(a) "xterm", "xterm", "-e", a
#define CALLBACK_ARGS GtkWidget *widget, GdkEvent *event
#else
#define XTERM(a) a, a
#define CALLBACK_ARGS char *a
#endif

void gls_free(GList *g)
{
	while (g)
	{
		free(g->data);
		g=g->next;
	}
	g_free(g);
}

/*
 * All of these input_ routines are very cranky and
 * liable to explode if the input formatting changes.
 */ 

/*
 * XXX: Do we need to remove the list items and free them?
 */
static GList *input_modules()
{
	static gchar buf[1024];
	static gchar bufout[80];
	gchar *bp, *bpz, *bd, *bd2;
	GList *g=NULL;
	int slot;
	
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
			/* module file is enclosed in ''s */
			bd=strchr(bp, '#');
			if (bd) { 
				slot=atoi(bd+1);

			bd=strchr(bp, '\'');
			if (bd)
			{
				bd++;
				bd2=strchr(bd, '\'');
				if (bd2) bd2[0]=0;
				snprintf(bufout, sizeof(bufout)-1, "%d : %s", slot, bd);
				g=g_list_append(g, g_strdup(bufout));
			}
			}
			
			bp=bpz;
		}
	} 
	return g;
}

static GList *input_tasks()
{
	static gchar buf[1024];
	static gchar bufout[80];
	gchar *bp, *bpz, *bd;
	int i, cpu;
	GList *g=NULL;
	
	int s;

	bp=&buf[0];
	s=read(console_out_fd, buf, sizeof(buf)-1);

	if (s > 0) {
		buf[s]=0;
		bp=strchr(bp, '+');
		while (bp)
		{
			bpz=strchr(bp, '\n');
			if (bpz)
			{
				bpz[0]=0;
				bpz++;
				bpz=strchr(bpz, '+');
			}
			/* ! display - + VCPU #n running process NAME */
			bd=strchr(bp, '#');
			if (bd) {
				cpu=atoi(bd+1);
			
				for(i=0;i<3;i++) {
					if (bd) bd=strchr(bd+1, ' ');
				}
			
				if (bd)
				{
					snprintf(bufout, sizeof(bufout)-1, "%d :%s", cpu, bd);
					g=g_list_append(g, g_strdup(bufout));
				}
			}
			
			bp=bpz;
		}
	} 
	return g;
}

/*
 * I might say that the argante-core team could have made this
 * easier! ATM it's fairly procfs specific.
 */
static GList *input_consoles()
{
	static gchar buf[2048];
	gchar *bp, *bz;
	static char filename[PATH_MAX];
	static ino_t ino[OPEN_MAX];
	int fd;
	struct stat st;
	int i;
	ino_t inod;
	GList *file=NULL;
	
	bp=&buf[0];
	/*
	 * Step 1. Work out what the inodes are for the sockets
	 *
	 * Start from 3, as 0, 1 + 2 are always going to be useless.
	 * 
	 */
	bzero(&ino, sizeof(ino));
	for(i=3;i<OPEN_MAX;i++)
	{
		snprintf(filename, sizeof(filename)-1, "/proc/%d/fd/%d", pid, i);
		/* Don't bother with error messages, they're all
		 * going to be file not found */
		if (!stat(filename, &st)) {
			if (S_ISSOCK(st.st_mode))
				ino[i]=st.st_ino;
		}
	}
	
	/*
	 * Step 2. Find the filenames of these sockets
	 *
	 * Fingers crossed that your /proc/net/unix never goes above
	 * 2048bytes! Actually, this is easy to do! FIXME!
	 */
	fd=open("/proc/net/unix", O_RDONLY);
	if (fd < 0)
	{
		perror("open /proc/net/unix");
		return NULL;
	}

	i=read(fd, buf, sizeof(buf)-1);
	close(fd);
	
	if (i > 0)
	{
		buf[i]=0;
		/* Skip the first line, it's useless. */
		bp=strchr(buf, '\n');
		
		while (bp) {
			/* We finished the last line, now onwards! */
			bp++;
			
			/* Null-terminate this line */
			bz=strchr(bp, '\n');
			if (bz) { bz[0]=0; }
			
			/* Skip 5 spaces */
			for(i=0;i<5;i++) {
				if (bp) bp=strchr(bp+1, ' ');
			}
			if (bp) {
			/* Now bp points to "state" "inode" "filename" */
			inod=strtol(bp, &bp, 0);
			/* Only listening sockets - or we get multiples */
			if (inod == 1) { 
			inod=strtol(bp, &bp, 0);
			if (bp) {
				bp++;
				for(i=3;i<OPEN_MAX;i++)
				{
					if (inod==ino[i])
					{
						file=g_list_append(file, g_strdup(bp));
						break;
					}
				}
			}
			}
			}
			bp=bz;
		}
	}
	
	return file;
}

#ifdef GTK
/* Dirty hack - atoi of the list gives the number to kill */
#define LIST_GET_B \
	if (!GTK_CLIST(widget)->selection) return TRUE; \
	d=(int) GTK_CLIST(widget)->selection->data; \
	gtk_clist_get_text(GTK_CLIST(widget), d, 0, &b) 
#define LIST_END \
	gtk_widget_destroy(widget->parent->parent->parent); \
	return TRUE 
#else
#define LIST_GET_B b=a
#define LIST_END return 0 
#endif

static gint module_kill( CALLBACK_ARGS )
{
	gchar *b;
	int d;
	gchar buf[80];

	LIST_GET_B;
	snprintf(buf, sizeof(buf)-1, "<%d\n", atoi(b));
	IO_writestr(buf);
	LIST_END;
}

static gint task_kill( CALLBACK_ARGS )
{
	gchar *b;
	gchar buf[80];
	int d;
	
	LIST_GET_B;
	snprintf(buf, sizeof(buf)-1, "-%d\n", atoi(b));
	IO_writestr(buf);
	LIST_END;
}

static gint task_open_console( CALLBACK_ARGS )
{
	gchar *b;
	int d;
	static gchar buf[PATH_MAX];

	LIST_GET_B;
	
#ifndef GTK
	endwin();
#endif
	/* vfork would be better, but I don't trust
	 * that we can clean up without messing up */
	if (!(d=fork())) {
		/* Fork should kill our excess FD's */
	
		/* Ok, our qualified filename's in b.
		 * Let's get to argante's working directory so
		 * it can be used qualified */
		snprintf(buf, sizeof(buf)-1, "/proc/%d/cwd", pid);
		chdir(buf);
		/* Ok, now let the show begin! */
		execlp(XTERM("vcpucons"), b, NULL);
		perror("execlp failed");

		/* Should never happen */
		exit(1);
	}

#ifndef GTK
	waitpid(d, NULL, 0);
	/* Touchwin too? */
	refresh();
#endif

	LIST_END;
}

#ifdef GTK
/* Dirty hack - atoi of the list gives the number to kill */
#define FILE_GET_B \
	b=gtk_file_selection_get_filename(GTK_FILE_SELECTION(widget))
#define FILE_END \
	gtk_widget_destroy(widget); \
	return TRUE
#else
#define FILE_GET_B b=a
#define FILE_END return 0 
#endif

static gint module_load( CALLBACK_ARGS )
{
	gchar *b;
	gchar buf[80];
	
	FILE_GET_B;
	snprintf(buf, sizeof(buf)-1, ">%s\n", b);
	IO_writestr(buf);
	FILE_END;
}

static gint task_load( CALLBACK_ARGS )
{
	gchar *b;
	gchar buf[80];
	
	FILE_GET_B;
	snprintf(buf, sizeof(buf)-1, "$%s\n", b);
	IO_writestr(buf);
	FILE_END;
}

static void Mi_mod_load(gchar *string)
{
	FSB_create("Load Module", module_load);
}

static void Mi_task_load(gchar *string)
{
	FSB_create("Load Image", task_load);
}

static void Mi_s_detach( gchar *string )
{
#ifdef GTK
	gtk_main_quit();
#else
	keepgoing=0;
#endif
}

static void Mi_s_shutdown( gchar *string )
{
	const char *cmd1="~<-> Shutdown command sent\n";
	const char *cmd2=".\n";
	IO_writestr(cmd1);
	IO_writestr(cmd2);
}

static void Mi_s_kill( gchar *string )
{
	MW_add_line("<----> KILL signal sent");
	kill(pid, 9);
}

static void Mi_mod_list( gchar *string )
{
	const char *cmd1="~<-> Mod list sent\n";
	const char *cmd2="#\n";
	GList *g;
	IO_writestr(cmd1);
	IO_writestr(cmd2);
	
	g=input_modules();
	LMB_create("Module List", "Remove", g, module_kill);
	gls_free(g);
}

static void Mi_task_list( gchar *string )
{
	const char *cmd1="~<-> Task list sent\n";
	const char *cmd2="!\n";
	GList *g;
	IO_writestr(cmd1);
	IO_writestr(cmd2);
	
	g=input_tasks();
	LMB_create("Task List", "Kill", g, task_kill);
	gls_free(g);
}

static void Mi_task_console( gchar *string )
{
	GList *g;
	g=input_consoles();
	LMB_create("Console List", "Open Console", g, task_open_console);
	gls_free(g);
}

static void Mi_conf_ruleset( gchar *string )
{
	const char *cmd1="~<-> HAC Ruleset updated\n";
	const char *cmd2="^\n";
	IO_writestr(cmd1);
	IO_writestr(cmd2);
}

static void Mi_conf_edit( gchar *string )
{
	static gchar buf[PATH_MAX];

#ifdef GTK
	/* vfork? */
	if (fork()) return;
#else
	/*
	 * Well, running two ncurses programs on the
	 * same terminal would be fun
	 */
	endwin();
	{
		int i;
		if ((i=fork())) {
			waitpid(i, NULL, 0);
			/* Touchwin too? */
			refresh();
			return;
		}
	}
#endif
	
	/* Fork should kill our excess FD's */

	/* Get to Argante conf dir */
	snprintf(buf, sizeof(buf)-1, "/proc/%d/cwd/conf", pid);
	chdir(buf);

	/* First, try gvim if we run under X -
	 * even if we are non-X version */
	if (getenv("DISPLAY"))
		execlp("gvim", "gvim", "access.hac", "fsconv.dat", "ipc.conf", NULL);
	/* If we're here, it didn't work. Try vim/vi. */
	
	/* BUG: when we do the xterm dance, we will always succeed even if
	 * we open an xterm with error message within */

	execlp(XTERM("vim"), "access.hac", "fsconv.dat", "ipc.conf", NULL);
	execlp(XTERM("vi"), "access.hac", "fsconv.dat", "ipc.conf", NULL);

	perror("execlp failed");
	fprintf(stderr, "Editing configuration files requires an editor.\n"
			"Perhaps you need to install one, or add your\n"
			"editor of choice to Mi_conf_edit in funcs.c.\n");

	/* XXX: Bad things happen when we get here in GTK. */
	exit(1);
}
	
static void Mi_raw_console(gchar *string)
{
	char buf[20];
	
#ifndef GTK
	/* Ever tried agtses with a "raw" terminal? */
	endwin();
#endif
	/*
	 * Uses PATH.
	 * Not that you'd run this SUID anyway.
	 */
	snprintf(buf, sizeof(buf)-1, "%d", pid);
	
	/* I can't believe I missed this!
	 * Dunno how xterm liked two extra fd's... */
	IO_stop();

	execlp(XTERM("agtses"), buf, NULL);
	perror("execlp failed");
}

/*
 * Sigh... arrays suck
 * 
 * (grin)
 */
menui _M_tasks[]=
{
	{ "List running tasks", Mi_task_list }, 
	{ "Load new task", Mi_task_load },
	{ "Kill running task", Mi_task_list },
	{ "Attach to task console", Mi_task_console },
	{ NULL, NULL }
};

menui _M_modules[]=
{
	{ "List loaded modules", Mi_mod_list },
	{ "Load new module", Mi_mod_load },
	{ "Remove module", Mi_mod_list },
	{ NULL, NULL }
};

menui _M_session[]=
{
	{ "Open Argante console (raw)", Mi_raw_console },
	{ "Shutdown Argante", Mi_s_shutdown },
	{ "Kill Argante", Mi_s_kill }, 
	{ "Detach", Mi_s_detach }, 
	{ NULL, NULL }
};

menui _M_conf[]=
{
	{ "Reload HAC ruleset", Mi_conf_ruleset },
	{ "Edit configuration files", Mi_conf_edit },
	{ NULL, NULL }
};

menui *M_tasks=_M_tasks;
menui *M_modules=_M_modules;
menui *M_session=_M_session;
menui *M_conf=_M_conf;


