/*
 * Argante-GTK frontend - GTK routines
 * (c) 2001 James Kehl <ecks@optusnet.com.au>
 * released under GPL, yada yada yada
 */

#include <stdio.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include "toplevel.h"

/*
 * MB_ - menu-button functions
 */

/* Click handler */
static gint MB_press( GtkWidget *widget, GdkEvent *event )
{
	
	if (event->type == GDK_BUTTON_PRESS) {
		GdkEventButton *bevent = (GdkEventButton *) event;
		gtk_menu_popup (GTK_MENU (widget), NULL, NULL, NULL, NULL,
				bevent->button, bevent->time);
		/* Tell calling code that we have handled this event; the buck
		 * * stops here. */
		return TRUE;
	}
	
	/* Tell calling code that we have not handled this event; pass it on. */
	return FALSE;
}

/* Create a menu-button and menu */ 
static void MB_create(char *name, GtkWidget *bbox, menui *mi) {
	GtkWidget *button;
	GtkWidget *menu_items;
	GtkWidget *menu;
	menui *m;

	m=mi;

	button = gtk_button_new_with_label (name);
	gtk_container_add (GTK_CONTAINER (bbox), button);
	
	menu = gtk_menu_new ();
	gtk_signal_connect_object (GTK_OBJECT (button), "event",
			GTK_SIGNAL_FUNC (MB_press), GTK_OBJECT (menu));

	while (m->name)
	{
		menu_items = gtk_menu_item_new_with_label (m->name);
		gtk_menu_append (GTK_MENU (menu), menu_items);
		gtk_signal_connect_object (GTK_OBJECT (menu_items), "activate",
				GTK_SIGNAL_FUNC (m->func), NULL);
		gtk_widget_show (menu_items);
		m++;
	}
}

/*
 * MW_ - main window functions
 */

static GtkWidget *MW_console_list;
static GdkFont *MW_fixed_font=NULL;

void MW_add_line(gchar *what)
{
	gtk_clist_append(GTK_CLIST(MW_console_list), &what);
}

/*
 * Create our boxful of buttons.
 * Why, oh why, don't I just use a menu!?
 */
static GtkWidget *MW_create_bbox()
{
	GtkWidget *frame;
	GtkWidget *bbox;
	
	frame = gtk_frame_new ("Commands");
	
	bbox = gtk_vbutton_box_new ();
	
	gtk_container_set_border_width (GTK_CONTAINER (bbox), 5);
	gtk_container_add (GTK_CONTAINER (frame), bbox);
	
	/* Set the appearance of the Button Box */
	gtk_button_box_set_layout (GTK_BUTTON_BOX (bbox), GTK_BUTTONBOX_START);
	gtk_button_box_set_spacing (GTK_BUTTON_BOX (bbox), 5);
	gtk_button_box_set_child_size (GTK_BUTTON_BOX (bbox), 85, 5);

	MB_create("Tasks", bbox, M_tasks);
	MB_create("Modules", bbox, M_modules);
	MB_create("Session", bbox, M_session);
	MB_create("Configure", bbox, M_conf);
	
	return(frame);
}

void MW_create()
{
	static GtkWidget* window = NULL;
	GtkWidget *hbox;
	GtkWidget *twindow;
	GtkStyle *style;
	static gchar buf[100];

	snprintf(buf, sizeof(buf) - 1, "Argante-GTK (attached to %d)", pid);
	
	/* main window stuff */
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window), buf);
        gtk_widget_set_usize( GTK_WIDGET ( window ) , 600 , 400 );
	
	gtk_signal_connect (GTK_OBJECT (window), "destroy",
			GTK_SIGNAL_FUNC(gtk_main_quit),
			NULL);
	
	gtk_container_set_border_width (GTK_CONTAINER (window), 5);
	
	/* horizontal box, for list and buttonframe */
	hbox = gtk_hbox_new (FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
	gtk_container_add (GTK_CONTAINER (window), hbox);
	
	gtk_box_pack_start (GTK_BOX (hbox), MW_create_bbox(), FALSE, TRUE, 5);

	/*
	 * Load fixed font.
	 * Avoid doing it more than once, though we don't (yet)
	 * have multiple windows
	 */
	if (!MW_fixed_font) 
		MW_fixed_font = gdk_font_load ("-misc-fixed-bold-r-*-*-*-100-*-*-*-*-*-*");
	
	/* Scrolled window for log-list */
	twindow=gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (twindow),
			GTK_POLICY_AUTOMATIC,
			GTK_POLICY_AUTOMATIC);

	gtk_container_add (GTK_CONTAINER (hbox), twindow);

	/* List for log */
	MW_console_list=gtk_clist_new(1);

	/* Make it fixed font */
	style = gtk_style_copy (gtk_widget_get_style (MW_console_list));
	gdk_font_unref (style->font);
	style->font = MW_fixed_font;
	gdk_font_ref (style->font);
	gtk_widget_set_style (MW_console_list, style);
	
	gtk_container_add (GTK_CONTAINER (twindow), MW_console_list);
//	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (twindow), tlist);
	
	gtk_widget_show_all (window);
}

/*
 * FSB_ - file selection box
 */

void FSB_create(char *title, void *callback)
{
	GtkWidget *filew;
	/* Create a new file selection widget */
	filew = gtk_file_selection_new (title);

	/* Connect the ok_button to file_ok_sel function */
	gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION (filew)->ok_button),
			"clicked", (GtkSignalFunc) callback, GTK_OBJECT(filew) );
	
	/* Connect the cancel_button to destroy the widget */
	gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION (filew)->cancel_button),
			"clicked", (GtkSignalFunc) gtk_widget_destroy,
			GTK_OBJECT (filew));
	
	gtk_widget_show(filew);

	/* Modal */
	gtk_grab_add(filew);
}

/*
 * LMB_ - list message box
 * 
 */

void LMB_create(char *title, char *buttonlabel, GList *gl, void *callback )
{
	GtkWidget *dia;
	GtkWidget *twindow, *tlist;
	GtkWidget *button;
	gchar *x;
	GList *g=gl;
	
	dia=gtk_dialog_new();

	gtk_window_set_title (GTK_WINDOW (dia), title);
	
	twindow=gtk_scrolled_window_new(NULL, NULL);

	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (twindow),
			GTK_POLICY_AUTOMATIC,
			GTK_POLICY_AUTOMATIC);

        gtk_widget_set_usize( GTK_WIDGET ( GTK_DIALOG(dia)->vbox ) , 300 , 200 );
        gtk_widget_set_usize( GTK_WIDGET ( GTK_DIALOG(dia)->action_area ) , 300 , 30 );
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dia)->vbox), twindow);

	tlist=gtk_clist_new(1);

	gtk_container_add (GTK_CONTAINER(twindow), tlist);
	
	while (g)
	{
		/* Why can't you take & of a pointer cast?
		 * This is just to shut up the warning*/
		x=g->data;

		gtk_clist_append(GTK_CLIST(tlist), &x);
		g=g->next;
	}

	gtk_container_set_border_width (GTK_CONTAINER (GTK_DIALOG(dia)->action_area), 5);
//	gtk_button_box_set_layout (GTK_CONTAINER(GTK_DIALOG(dia)->action_area), GTK_BUTTONBOX_SPREAD);

	button = gtk_button_new_with_label (buttonlabel);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dia)->action_area), button);

	gtk_signal_connect_object(GTK_OBJECT(button), "clicked",
			(GtkSignalFunc) callback, GTK_OBJECT(tlist));

	button = gtk_button_new_with_label ("Close");
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dia)->action_area), button);

	gtk_signal_connect_object (GTK_OBJECT (button),	"clicked",
			(GtkSignalFunc) gtk_widget_destroy, GTK_OBJECT (dia));
	
	gtk_widget_show_all (dia);
	
	/* Modal */
	gtk_grab_add(dia);
	
	return;
}


