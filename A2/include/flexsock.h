#ifndef _HAVE_FLEXSOCK_H
#define _HAVE_FLEXSOCK_H

#ifdef __WIN32__
#include <winsock.h>
#else
#include <sys/types.h>
#include <netinet/in.h>
#endif

#define FXST_STDIO	0x1 /* e.g. agent has forked out a kernel... */
#define FXST_UNIX_B	0x2 /* Bind a Unix socket */
#define FXST_UNIX_C	0x3 /* Connect to a Unix socket */
#define FXST_NPIPE_B	0x4 /* WIN32 Named Pipes. Not recommended/supported/etc. */
#define FXST_NPIPE_C	0x5 
#define FXST_TCP_B	0x6
#define FXST_TCP_C	0x7 /* TCP reverse connects... security risk? */
#define FXST_SSL_B	0x8
#define FXST_SSL_C	0x9 /* Risk? */

#define FXS_PFLAG(a) (1 << (a))

/* WARNING! This is machine specific and must be translated later. */
struct flexsock_desc {
	uint16_t type;
	union {
		/* Nothing for stdin/stdout */
		char unixpath[256]; /* Pathname for unix/local socket */
		/* network byte order here! */
		struct {
			uint32_t ip;
			/* Port _may_ be ignored on bind for security reasons */
			uint16_t port;
		} tcp;
	} data;
};

typedef struct _flexsock* flexsock;
typedef struct _flexlisock* flexlisock;

int FXS_Init(void);
void FXS_Shutdown(void);

void FXS_SetProtocols(int new_protos);
void FXS_SetBindsAdvisory(int val);
int FXS_Desc2Ascii(const struct flexsock_desc *d, char *buf, size_t size);
int FXS_Ascii2Desc(struct flexsock_desc *d, const char *buf);

int FXS_IsConnectType(const struct flexsock_desc *d);

/* Def'n changed, A2 0.010 - we'll DuplicateHandle named pipes (when we get them).
 * Therefore you can CloseListener on anything safely. */
int FXS_IsSingleAcceptType(const struct flexsock_desc *d);

flexsock FXS_StdIO(void);

flexsock FXS_ConnectTo(const struct flexsock_desc *to);
/* Returns a listening socket. */
flexlisock FXS_BindTo(const struct flexsock_desc *to, struct flexsock_desc *revers);

/* Returns an ordinary flexsock. */
flexsock FXS_Accept(flexlisock bound);
flexsock FXS_AcceptPoll(flexlisock bound);

/* Poll must return -1 on error (or eof) and 0 on EAGAIN */
int FXS_ReadPoll(flexsock f, char *buf, size_t size);

int FXS_Read(flexsock f, char *buf, size_t size);
int FXS_Write(flexsock f, const char *buf, size_t size);

void FXS_Close(flexsock f);
void FXS_CloseListener(flexlisock f);

#endif
