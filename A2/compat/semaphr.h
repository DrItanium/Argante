#ifndef _HAVE_SEMAPHR
#define _HAVE_SEMAPHR
#ifdef __WIN32__
typedef long sem_t;
extern void sem_wait(sem_t *);
extern void sem_post(sem_t *);
extern void sem_init(sem_t *, int, int val);
#else
#ifndef NO_THREADS
#include <semaphore.h>
#else
typedef long sem_t;
extern void sem_wait(sem_t *);
extern void sem_post(sem_t *);
extern void sem_init(sem_t *, int, int val);
#endif
#endif
#endif
