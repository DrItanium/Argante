#ifndef _HAVE_PRINTK
#define _HAVE_PRINTK

/* 8 levels... approximately */
#define PRINTK_DEBUG 0
#define PRINTK_INFO 2
#define PRINTK_WARN 4
#define PRINTK_ERR 5
#define PRINTK_CRIT 7

/* printk is the obsolete version. */
#ifdef __GNUC__
extern int printk2(int level, const char *format, ...)
	__attribute__ ((format (printf, 2, 3)));
extern int printk(const char *format, ...)
	__attribute__ ((format (printf, 1, 2)));
#else
extern int printk2(int level, const char *format, ...);
extern int printk(const char *format, ...);
#endif

#endif
