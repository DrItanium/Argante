/*
 * Argante frontend - curses routines
 * (c) 2001 James Kehl <ecks@optusnet.com.au>
 * released under GPL, yada yada yada
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "notglib.h"
#include <curses.h>

#include <dirent.h>
#include <sys/stat.h>

#include "toplevel.h"

/*
 * MB_ - menu-button functions
 */

menui *curmenu;

static void MB_slk_submenu(menui *submenu)
{
	int i=1;
	menui *z;
	z=curmenu=submenu;
	while (i < 8 && z->name)
	{
		slk_set(i++, z->name, 0);
		z++;
	}
	for (;i<8;i++)
		slk_set(i, NULL, 0);

	slk_noutrefresh();
}

static void MB_slk_main()
{
	int i=1;
	/* Null = main menu */
	curmenu=NULL;
	/* Use slk for the menus */
	slk_set(i++, "Tasks", 0);
	slk_set(i++, "Modules", 0);
	slk_set(i++, "Session", 0);
	slk_set(i++, "Config", 0);

	/* Blank the rest */
	for (;i<8;i++)
		slk_set(i, NULL, 0);
	slk_noutrefresh();
}

static void MB_slk_key(int fkey)
{
	if (!curmenu)
	{
		/* Main menu then */
		switch (fkey)
		{
			case 1: MB_slk_submenu(M_tasks); break;
			case 2: MB_slk_submenu(M_modules); break;
			case 3: MB_slk_submenu(M_session); break;
			case 4: MB_slk_submenu(M_conf); break;
		}
		return;
	} else {
		menui *z;
		/* ok, we're in a submenu. damn */
		z=curmenu;
		fkey--;
		while (z->name && fkey)
		{
			z++; fkey--;
		}
		/* did the moron push a non-assigned key?! */
		if (fkey) return;
		/* no. so we have to call the func */
		if (z->func) z->func(NULL);
	}
}

/*
 * MW_ - main window functions
 */
#define BUFLEN 300
static char *linebuf[BUFLEN];
static int bufdirty=1;
int bufoffs=0;

void MW_add_line(gchar *what)
{
	int i;
	/* Erase the end */
	free(linebuf[0]);
	/* Move the buffer back one line */
	for(i=1;i<BUFLEN;i++)
		linebuf[i-1]=linebuf[i];
	/* Now we have space, insert this one */
	linebuf[BUFLEN - 1]=strdup(what);
	bufdirty=1;
	bufoffs=0;
}

static void MW_scroll(int x)
{
	bufoffs+=x;
	if (bufoffs < 0) bufoffs=0;
	if (bufoffs > (BUFLEN - LINES)) bufoffs=BUFLEN - LINES;
	printf("%d", bufoffs);
	bufdirty=1;
}

static WINDOW *MW_window;

void MW_create()
{
	bzero(linebuf, sizeof(linebuf));
	slk_init(1);
	MW_window=initscr();
	halfdelay(1);
	noecho();
	nonl();
	intrflush(stdscr, TRUE);
	MB_slk_main();
	keypad(stdscr, TRUE);

	return;
}

void MW_update()
{
	int key;

	key=getch();
	switch(key)
	{
		case KEY_F(1): MB_slk_key(1); break;
		case KEY_F(2): MB_slk_key(2); break;
		case KEY_F(3): MB_slk_key(3); break;
		case KEY_F(4): MB_slk_key(4); break;
		case KEY_F(5): MB_slk_key(5); break;
		case KEY_F(6): MB_slk_key(6); break;
		case KEY_F(7): MB_slk_key(7); break;
		case KEY_F(8): MB_slk_key(8); break;
		/* Escape */
		case 27: MB_slk_main(); break;
		/* Scroll buffer keys */
		case KEY_UP: MW_scroll(1); break;
		case KEY_DOWN: MW_scroll(-1); break;
		case KEY_PPAGE: MW_scroll(LINES); break;
		case KEY_NPAGE: MW_scroll(-LINES); break;
	}
	
	if (bufdirty)
	{
		int i, y=0;
		clear();
		for(i=BUFLEN - LINES - bufoffs;i<BUFLEN - bufoffs;i++)
		{
			mvaddstr(y, 0, linebuf[i]);
			y++;
		}
		bufdirty=0;
	}
	refresh();
}

/*
 * FSB_ - file selection box
 */

extern void gls_free(GList *g);

void FSB_create(char *title, void *callback)
{
	char buf[PATH_MAX];
	void (*cb)(char *);
	GList *dirs, *files;
	GList *dirsel, *filesel;
	GList *z;
	int pane=0, i, y;
	DIR *dir;
	struct dirent *t;
	struct stat st;

	cb=callback;
	dirsel=filesel=dirs=files=NULL;
	bufdirty=1;

	clear();
	
	attron(A_BOLD);
	mvaddstr(0, 10, title);
	attroff(A_BOLD);
	mvaddstr(22, 0, "Select : Enter");
	mvaddstr(22, 18, "Switch : Tab");
	mvaddstr(22, 36, "Cancel: Escape");

	while(1)
	{
		if (!dirs && !files)
		{
			dir=opendir(".");
			if (!dir) perror("opendir");
			while ((t=readdir(dir)))
			{
				i=stat(t->d_name, &st);
				if (!i) {
				if (S_ISDIR(st.st_mode))
				{
					dirs=g_list_append(dirs, g_strdup(t->d_name));
				}
				else if (S_ISREG(st.st_mode))
				{
					files=g_list_append(files, g_strdup(t->d_name));
				}
				}
			}
			closedir(dir);
			filesel=files; dirsel=dirs;
		}

		/* Display */
		/* Directories.
		 * Let's go back 5 lines and start */
		i=10; z=dirsel;
		while(i > 0 && z && z->prev) { i--; z=z->prev; }
		for(y=0;y<20 && z;y++)
		{
			if (z==dirsel) {
				if (pane == 0)
					attron(A_STANDOUT);
				else
					attron(A_BOLD);
			}
			for(i=0;i<28;i++) { mvaddch(y+2, i, ' '); }

			mvaddstr(y+2, 0, (char *) z->data);
			attroff(A_STANDOUT | A_BOLD);

			z=z->next;
		}
		/* Erase blanks */
		for(;y<20;y++)
			for(i=0;i<28;i++) { mvaddch(y+2, i, ' '); }
		/* Files. Same deal. */
		i=10; z=filesel;
		while(i > 0 && z && z->prev) { i--; z=z->prev; }
		for(y=0;y<20 && z;y++)
		{
			if (z==filesel) {
				if (pane == 1)
					attron(A_STANDOUT);
				else
					attron(A_BOLD);
			}
			for(i=30;i<58;i++) { mvaddch(y+2, i, ' '); }

			mvaddstr(y+2, 30, (char *) z->data);
			attroff(A_STANDOUT | A_BOLD);

			z=z->next;
		}
		/* Erase blanks */
		for(;y<20;y++)
			for(i=30;i<58;i++) { mvaddch(y+2, i, ' '); }

		/* Keys */
		nodelay(MW_window, FALSE);
		i=getch();
		halfdelay(1);

		switch(i)
		{
			case 27:
				gls_free(files);
				gls_free(dirs);
				return;
				break;
			case '\n':
			case '\r': 
				if (pane==1)
				{
					getcwd(buf, sizeof(buf));
					y=strlen(buf);
					/* strncat is useless! what moron thought that up!? */
					i=0;
					if(y < sizeof(buf)-2) { buf[y]='/'; y++; }
					while(y < sizeof(buf) - 1 && filesel->data[i])
					{
						buf[y]=filesel->data[i];
						i++;y++;
					}
					buf[y]=0;
					cb((char *) buf);
				}
				else
					chdir(dirsel->data);
				
				gls_free(files);
				gls_free(dirs);
				if (pane==1)
					return;
				else
					files=dirs=NULL;
				break;
			case KEY_DOWN: 
				if (pane==0) {
					if (dirsel && dirsel->next) dirsel=dirsel->next;
				} else {
					if (filesel && filesel->next) filesel=filesel->next;
				}
				break;
			case KEY_UP:
				if (pane==0) {
					if (dirsel && dirsel->prev) dirsel=dirsel->prev;
				} else {
					if (filesel && filesel->prev) filesel=filesel->prev;
				}
				break;
			case '\t':
				pane=(pane + 1) % 2;
				break;
		}

		

	}

	return;
}

/*
 * LMB_ - list message box
 * 
 */

void LMB_create(char *title, char *buttonlabel, GList *gl, void *callback)
{
	GList *m, *sel;
	int x=0, y;
	void (*cb)(char *);

	cb=callback;
	sel=gl;
	
	bufdirty=1;
	clear();

	attron(A_BOLD);
	mvaddstr(0, 10, title);
	attroff(A_BOLD);
	mvaddstr(20, 0, buttonlabel);
	mvaddstr(20, strlen(buttonlabel) + 2, ": Enter");
	mvaddstr(20, 36, "Cancel: Escape");

	while(1)
	{
		m=gl;
		x=2;
		while (m)
		{
			if (m==sel) attron(A_STANDOUT);
			/* Make a nice big pad */
			for(y=0;y<50;y++) { mvaddch(x, y, ' '); }

			mvaddstr(x, 0, (char *) m->data);
			attroff(A_STANDOUT);

			m=m->next;
			x++;
		}

		nodelay(MW_window, FALSE);
		x=getch();
		halfdelay(1);

		switch(x)
		{
			case 27: return; break;
			case '\n':
			case '\r': 
				 cb((char *) sel->data);
				 return;
				 break;
			case KEY_DOWN: if (sel->next) sel=sel->next; break;
			case KEY_UP: if (sel->prev) sel=sel->prev; break;
		}
	}
}


