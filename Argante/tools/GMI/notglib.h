#ifdef GTK
/* No point */
#include <glib.h>
#else
/* Keep up appearances... */
#include <limits.h>

typedef struct _GList GList;
typedef char gchar;
typedef int gint;

struct _GList {
	char *data;
	GList *next;
	GList *prev;
};

extern void g_free(void *);
extern char *g_strdup(char *);
extern GList *g_list_append(GList *, void *);

#endif
