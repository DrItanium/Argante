#ifdef HAVE_USLEEP
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#else
/* no, nanosleep's no better. */
extern int usleep(int millis);
#endif
