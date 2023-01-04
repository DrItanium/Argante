#ifndef _AGTMAIN_TOP_LEVEL_H__
#define _AGTMAIN_TOP_LEVEL_H__
extern int console_in_fd;
extern int console_out_fd;
extern int pid;

typedef struct {
	char *name;
	void (*func)(char *);
} menui;

extern void IO_writestr(const char *z);
extern int IO_start(int process);
extern void IO_stop();

extern void FSB_create(char *title, void *callback);
extern void LMB_create(char *title, char *buttonlabel, GList *gl, void *callback );
extern void MW_create();

extern void MW_add_line(gchar *what);

extern menui *M_tasks;
extern menui *M_modules;
extern menui *M_session;
extern menui *M_conf;

#ifndef GTK
extern int keepgoing;
#endif

/* Lifted from glib.h */
#ifndef	FALSE
#define	FALSE	(0)
#endif

#ifndef	TRUE
#define	TRUE	(!FALSE)
#endif

#endif
