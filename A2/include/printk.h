#ifndef _HAVE_PRINTK
#define _HAVE_PRINTK

/* 8 levels... approximately */
#define PRINTK_DEBUG 0
#define PRINTK_INFO 2
#define PRINTK_WARN 4
#define PRINTK_ERR 5
#define PRINTK_CRIT 7

extern int printk2(int level, const char *format, ...);
/* The obsolete version. */
extern int printk(const char *format, ...);

#endif
