
#ifndef HAVE_LOCALTIME_R
extern struct tm *localtime_r(const time_t *time, struct tm *buf);
/* To decently create semaphores we need single thread time... */
extern void init_localtime_r(void);
#endif
