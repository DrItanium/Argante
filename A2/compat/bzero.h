#ifdef HAVE_BZERO
#include <string.h>
#else
#ifdef HAVE_MEMSET
#ifdef HAVE_MEMORY_H
#include <memory.h>
#else
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#endif
#define bzero(targ, sz) memset(targ, 0, sz)
#else
extern void bzero(void *block, size_t size);
#endif
#endif

