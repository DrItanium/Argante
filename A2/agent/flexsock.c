/*
 * A2 Virtual Machine - flexible sockets
 * Copyright (c) 2002	James Kehl <ecks@optusnet.com.au>
 * 
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published
 * by the Free Software Foundation; version 2 of the License, with the
 * added restriction that it may only be converted to the version 2 of the
 * GNU General Public License.
 * 
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */
#include "autocfg.h"
#include "compat/alloca.h"
#include "compat/strncmpi.h"

/* XXX: Unix section, fds non inheritable! */

#ifdef __WIN32__
#include <windows.h>
#include <winsock.h>
#else
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/fcntl.h>
#include <sys/signal.h>

/* Fix Solaris. */
#ifndef PF_LOCAL
#define PF_LOCAL PF_UNIX
#endif
#ifndef AF_LOCAL
#define AF_LOCAL AF_UNIX
#endif

#endif

#include "flexsock.h"

/* The idea is that we accept a universal format for connections
 * of such things as agents and consoles. Only agents and kernel
 * should be using the creation functions, but modules that implement
 * agent-VFDs should use FXS_Read/Write (e.g. consoles and GUIs).
 */

/* For each of the protocols, there is a flag enabling or disabling them.
 * I am tempted to make this flag settable by (trusted) agents, but that will
 * be optional at most. */

/* For BIND type protocols, there is an 'advisory' flag which makes us
 * ignore the specified port. This should be done for any not-entirely-trusted
 * command, such as Agent-VFDs. Otherwise, agents can open arbitrary ports,
 * create arbitrary files, and generally fsck your system till it falls
 * apart. Say, by creating the \\$lsass pipe or something.
 * 
 * This means that binding a socket will return a "reverse" descriptor for
 * connecting to the new creation.
 */

static int protocols_enabled=0; /* Stdio, unixb/c, tcpb/c? */
static int binds_advisory=0; /* Trust(honour port/filename) bind requests by default */

void FXS_SetProtocols(int new_protos) {
	protocols_enabled=new_protos;
}

inline int FXS_IsProtoEnabled(int type) {
	return (protocols_enabled & FXS_PFLAG(type));
}

void FXS_SetBindsAdvisory(int val) {
	binds_advisory=val;
}

inline int FXS_IsConnectType(const struct flexsock_desc *d) {
	switch(d->type) {
		case FXST_STDIO:
		case FXST_UNIX_C:
		case FXST_NPIPE_C:
		case FXST_TCP_C:
		case FXST_SSL_C:
			return 1;
		case FXST_UNIX_B:
		case FXST_NPIPE_B:
		case FXST_TCP_B:
		case FXST_SSL_B:
			return 0;
		default: /* What the flock is this? */
			return -1;
	}
}

static inline int bytesfordigits(int i) {
	/* The most digits you'll get in a short. */
	if (i > 9999) return 5;
	if (i > 999) return 4;
	if (i > 99) return 3;
	if (i > 9) return 2;
	return 1;
}

/* Expects network byte order. Returns bytes written.
 * Some systems don't provide decent ntoa's and co. (no names :)
 * XXX: this ought to go in compat/ */
static int FXS_tcp4toa(uint32_t addr, char *buf, size_t size) {
	uint32_t haddr;
	int a, b, c, d;
	int size_required;
	haddr=ntohl(addr);

	a=(haddr >> 24);
	b=(haddr >> 16) & 0xff;
	c=(haddr >> 8) & 0xff;
	d=haddr & 0xff;

	/* Dots and terminating nulls. */
	size_required=
		bytesfordigits(a) + 1 +
		bytesfordigits(b) + 1 +
		bytesfordigits(c) + 1 +
		bytesfordigits(d) + 1;
	if (size < size_required) return 0;
#ifndef __WIN32__
	return sprintf(buf, "%d.%d.%d.%d", a, b, c, d);
#else
	return wsprintf(buf, "%d.%d.%d.%d", a, b, c, d);
#endif
}

static int FXS_atotcp4(uint32_t *addr, const char *tbuf) {
	int digits[4], i;
	const char *buf;

	buf=tbuf;
	i=0; digits[0]=0;
	while(*buf) {
		if (*buf=='.') {
			i++;
			if (i > 3) return 0; /* Excessive .'s! */
			digits[i]=0;
		} else if (*buf >= '0' && *buf <= '9') {
			/* XXX: Ascii dependance */
			digits[i]*=10;
			digits[i]+=*buf-'0';
			if (digits[i] > 0xff) return 0;
		} else {
			if (i != 3) return 0;
			break;
		}
		buf++;
	}
	if (i < 3) return 0;
	*addr=htonl((digits[0] << 24) | (digits[1] << 16) |
		(digits[2] << 8) | (digits[3]));
	return (buf - tbuf);
}

int FXS_Desc2Ascii(const struct flexsock_desc *d, char *buf, size_t size) {
	int i;
	/* Buf not big enough for TCPCx? */
	if (size <= 4) return 1;
	switch(d->type) {
		case FXST_STDIO:
			memcpy(buf, "STDI", 4);
			buf+=4; size-=4;
			break;
		case FXST_UNIX_C:
			memcpy(buf, "UNXC", 4);
			buf+=4; size-=4;
			i=strlen(d->data.unixpath);
			if (size < i) return 1;
			memcpy(buf, d->data.unixpath, i);
			buf+=i; size-=i;
			break;
		case FXST_UNIX_B:
			memcpy(buf, "UNXB", 4);
			buf+=4; size-=4;
			i=strlen(d->data.unixpath);
			if (size < i) return 1;
			memcpy(buf, d->data.unixpath, i);
			buf+=i; size-=i;
			break;
		case FXST_TCP_C:
			memcpy(buf, "TCPC", 4);
			buf+=4; size-=4;
			if (!(i=FXS_tcp4toa(d->data.tcp.ip, buf, size))) return 1;
			buf+=i; size-=i;
			/* Nul and : */
			if (size < (bytesfordigits(ntohs(d->data.tcp.port)+2))) return 1;
#ifndef __WIN32__
			i=sprintf(buf, ":%d", ntohs(d->data.tcp.port));
#else
			i=wsprintf(buf, ":%d", ntohs(d->data.tcp.port));
#endif
			buf+=i; size-=i;
			break;
		case FXST_TCP_B:
			memcpy(buf, "TCPB", 4);
			buf+=4; size-=4;
			if (!(i=FXS_tcp4toa(d->data.tcp.ip, buf, size))) return 1;
			buf+=i; size-=i;
			/* Nul and : */
			if (size < (bytesfordigits(ntohs(d->data.tcp.port)+2))) return 1;
#ifndef __WIN32__
			i=sprintf(buf, ":%d", ntohs(d->data.tcp.port));
#else
			i=wsprintf(buf, ":%d", ntohs(d->data.tcp.port));
#endif
			buf+=i; size-=i;
			break;
		case FXST_NPIPE_C:
			memcpy(buf, "NPIC", 4);
			buf+=4; size-=4;
			break;
		case FXST_NPIPE_B:
			memcpy(buf, "NPIB", 4);
			buf+=4; size-=4;
			break;
		default: return 1;
	}
	/* Space for nul? */
	if (((signed) size) <= 0) { /* If it's negative, eek! */
		if (((signed) size) < 0) {
#ifndef __WIN32__
			fprintf(stderr, "FXS_Desc2Ascii: EVIL HACK ALERT!?\n");
#else
			MessageBox(NULL, "EVIL HACK ALERT?!", "FXS_Desc2Ascii",
				MB_ICONSTOP | MB_TASKMODAL);
#endif
			exit(1); /* die quick, don't ret! */
		}
		return 1;
	}
	*buf=0;
	return 0;
}

int FXS_Ascii2Desc(struct flexsock_desc *d, const char *buf) {
	int i;
	char *bt;
	if (!strncasecmp(buf, "STDI", 4)) {
		d->type=FXST_STDIO;
	} else if (!strncasecmp(buf, "UNXC", 4)) {
		d->type=FXST_UNIX_C;
		buf+=4;
		i=strlen(buf);
		if (!i) return 1;
		if (i >= sizeof(d->data.unixpath)) return 1;
		memcpy(d->data.unixpath, buf, i+1);
	} else if (!strncasecmp(buf, "UNXB", 4)) {
		d->type=FXST_UNIX_B;
		buf+=4;
		i=strlen(buf);
		/* Empty == unspecified path OK */
		if (i >= sizeof(d->data.unixpath)) return 1;
		memcpy(d->data.unixpath, buf, i+1);
	} else if (!strncasecmp(buf, "TCPC", 4)) {
		d->type=FXST_TCP_C;
		buf+=4;
		i=FXS_atotcp4(&d->data.tcp.ip, buf);
		if (!i) return 1;
		buf+=i;
		if (*buf != ':') return 1;
		buf++;
		d->data.tcp.port=htons((short) strtoul(buf, &bt, 0));
		if (bt && *bt) return 1;
	} else if (!strncasecmp(buf, "TCPB", 4)) {
		d->type=FXST_TCP_B;
		buf+=4;
		i=FXS_atotcp4(&d->data.tcp.ip, buf);
		if (!i) return 1;
		buf+=i;
		if (!*buf) {
			/* Port number is optional */
			d->data.tcp.port=0;
			return 0;
		}
		if (*buf != ':') return 1;
		buf++;
		d->data.tcp.port=htons((short) strtoul(buf, &bt, 0));
		if (bt && *bt) return 1;
	} else if (!strncasecmp(buf, "NPIC", 4)) {
		return 1; /* Win32 Named Pipes not supported */
	} else if (!strncasecmp(buf, "NPIB", 4)) {
		return 1; /* Win32 Named Pipes not supported */
	} else return 1; /* unknown protocol */
	return 0;
}

int FXS_IsSingleAcceptType(const struct flexsock_desc *to) {
	if (to->type == FXST_NPIPE_B) return 1;
	return 0;
}

/* Because of StdIO, we must keep separate pipes for input and output. */

#ifndef __WIN32__

struct _flexsock {
	int fd_in;
	int fd_out;
};

struct _flexlisock {
	int fd;
	/* Don't leave icky unix sockets around. */
	char *unlink_pathname;
};

int FXS_Init(void) {
	signal(SIGPIPE, SIG_IGN);
	return 0;
}

void FXS_Shutdown(void) { return; }

static inline int SetCLOExec(int fd) {
	int flags;
	/* Try and set FD_CLOEXEC */
	flags=fcntl(fd, F_GETFD, 0);
	if (flags < 0) return 1;
	flags|=FD_CLOEXEC;
	if (fcntl(fd, F_SETFD, flags) < 0)
		return 1;
	return 0;
}

flexsock FXS_StdIO(void) {
	flexsock f;
	f=malloc(sizeof(struct _flexsock));
	if (!f) return NULL;
	f->fd_in=dup(fileno(stdin));
	SetCLOExec(f->fd_in);
	f->fd_out=dup(fileno(stdout));
	SetCLOExec(f->fd_out);
	return f;
}

void FXS_Close(flexsock f) {
	close(f->fd_in);
	close(f->fd_out);
	free(f);
}

flexsock FXS_ConnectTo(const struct flexsock_desc *to) {
	int sockfd;
	flexsock s;
	if (!FXS_IsProtoEnabled(to->type)) return NULL;
	switch(to->type) {
		case FXST_STDIO:
			return FXS_StdIO();
		case FXST_UNIX_C: {
			struct sockaddr_un *soa;
			int len;
			len=strlen(to->data.unixpath);
			soa=alloca(sizeof(soa->sun_family) + len);
			soa->sun_family=AF_LOCAL;
			memcpy(&soa->sun_path, to->data.unixpath, len);
			len+=sizeof(soa->sun_family);
			
			sockfd=socket(PF_LOCAL, SOCK_STREAM, 0);
			if (sockfd < 0) return NULL;
			if (connect(sockfd, (struct sockaddr *) soa, len) < 0) {
				close(sockfd);
				return NULL;
			}

			break;
		}
		case FXST_TCP_C: {
			struct sockaddr_in soa;
			soa.sin_family=AF_INET;
			soa.sin_addr.s_addr=to->data.tcp.ip;
			soa.sin_port=to->data.tcp.port;
			
			sockfd=socket(PF_INET, SOCK_STREAM, 0);
			if (sockfd < 0) return NULL;
			if (connect(sockfd, (struct sockaddr *) &soa, sizeof(soa)) < 0) {
				close(sockfd);
				return NULL;
			}
			break;
		}
		default:
			return NULL;
	}
	s=malloc(sizeof(struct _flexsock));
	s->fd_in=sockfd;
	s->fd_out=sockfd;
	SetCLOExec(sockfd);
	return s;
}

flexlisock FXS_BindTo(const struct flexsock_desc *to, struct flexsock_desc *revers) {
	int sockfd;
	char *unlink_path=NULL;
	flexlisock s;
	if (!FXS_IsProtoEnabled(to->type)) return NULL;
	switch(to->type) {
		case FXST_UNIX_B: {
			struct sockaddr_un *soa;
			int len;
			char *f;
			ALLOCA_STACK;
			revers->type=FXST_UNIX_C;
			if (binds_advisory || !to->data.unixpath[0]) {
				f=tempnam(NULL, "fxsun");
				len=strlen(f);
				if (len >= sizeof(revers->data.unixpath)) {
					free(f);
					fprintf(stderr, "<!> untrustworthy tempnam!\n");
					return NULL;
				}
				soa=alloca(sizeof(soa->sun_family) + len);
				memcpy(&soa->sun_path, f, len);
				memcpy(revers->data.unixpath, f, len+1);
				unlink_path=malloc(len + 1);
				if (!unlink_path) {
					free(f);
					return NULL;
				}
				memcpy(unlink_path, f, len + 1);
			} else {
				len=strlen(to->data.unixpath);
				soa=alloca(sizeof(soa->sun_family) + len);
				memcpy(&soa->sun_path, to->data.unixpath, len);
				memcpy(revers->data.unixpath, to->data.unixpath, len+1);
				unlink_path=malloc(len + 1);
				if (!unlink_path) return NULL;
				memcpy(unlink_path, to->data.unixpath, len+1);
			}
			soa->sun_family=AF_LOCAL;
			len+=sizeof(soa->sun_family);
			sockfd=socket(PF_LOCAL, SOCK_STREAM, 0);
			if (sockfd < 0) return NULL;
			if (bind(sockfd, (struct sockaddr *) soa, len) < 0) {
				close(sockfd);
				return NULL;
			}
			break;
		}
		case FXST_TCP_B: {
			struct sockaddr_in soa;
			soa.sin_family=AF_INET;
			soa.sin_addr.s_addr=to->data.tcp.ip;
			revers->type=FXST_TCP_C;
			revers->data.tcp.ip=to->data.tcp.ip;
			
			sockfd=socket(PF_INET, SOCK_STREAM, 0);
			if (sockfd < 0) return NULL;

			if (binds_advisory || !to->data.tcp.port) {
				/* Allocate some unused port number,
				 * don't bind to a specified one. */
				soa.sin_port=0;
			} else {
				soa.sin_port=to->data.tcp.port;
			}
			if (bind(sockfd, (struct sockaddr *) &soa, sizeof(soa)) < 0) {
				close(sockfd);
				return NULL;
			}
			/* Hmm, what did we end up with? */
			if (binds_advisory || !to->data.tcp.port) {
				int len=sizeof(soa);
				if (getsockname(sockfd, (struct sockaddr *) &soa, &len) < 0) {
					close(sockfd);
					return NULL;
				}
				revers->data.tcp.port=soa.sin_port;
			} else {
				revers->data.tcp.port=to->data.tcp.port;
			}
			break;
		}
		default: return NULL;
	}
	if (listen(sockfd, 8) < 0) {
		close(sockfd);
		return NULL;
	}
	s=malloc(sizeof(struct _flexlisock));
	s->fd=sockfd;
	SetCLOExec(sockfd);
	s->unlink_pathname=unlink_path;
	return s;
}

flexsock FXS_Accept(flexlisock f) {
	int fd;
	flexsock s;
	fd=accept(f->fd, NULL, NULL);
	if (fd < 0) return NULL;
	s=malloc(sizeof(struct _flexsock));
	s->fd_in=fd;
	s->fd_out=fd;
	SetCLOExec(fd);
	return s;
}

flexsock FXS_AcceptPoll(flexlisock f) {
	int fd;
	flexsock s;
	int flags;

	flags=fcntl(f->fd, F_GETFL, 0);
	if (flags < 0) return NULL;
	if (fcntl(f->fd, F_SETFL, flags | O_NONBLOCK) < 0) return NULL;
	/* Sandwich fillings. */
	fd=accept(f->fd, NULL, NULL);
	fcntl(f->fd, F_SETFL, flags); /* This better not fail! */

	if (fd < 0) return NULL;
	s=malloc(sizeof(struct _flexsock));
	s->fd_in=fd;
	s->fd_out=fd;
	SetCLOExec(fd);
	return s;
}

ssize_t FXS_ReadPoll(flexsock f, char *buf, size_t size) {
	int flags, enb;
	ssize_t ret;
	flags=fcntl(f->fd_in, F_GETFL, 0);
	if (flags < 0) return -1;
	if (fcntl(f->fd_in, F_SETFL, flags | O_NONBLOCK) < 0) return -1;
	/* Sandwich fillings. */
	ret=read(f->fd_in, buf, size); enb=errno;
	fcntl(f->fd_in, F_SETFL, flags); /* This better not fail! */
	if (ret == 0) return -1;
	if (ret < 0) {
		if (enb != EAGAIN) return -1;
		return 0;
	}
	return ret;
}

ssize_t FXS_Read(flexsock f, char *buf, size_t size) {
	int i;
	while(1) {
		i=read(f->fd_in, buf, size);
		if (i > 0) return i;
		else if (i == 0 || errno != EINTR) /* incl. zero reads? */
			return -1;
	}
}

/* When we do a write, guarantee that it completes fully.  */
ssize_t FXS_Write(flexsock f, const char *buf, size_t size) {
	int i, j=0;
	while (j < size) {
		i=write(f->fd_out, &buf[j], size-j);
		if (i > 0) j+=i;
		else if (i == 0 || errno != EINTR) /* Signal */
			return -1;
	}
	return j;
}

void FXS_CloseListener(flexlisock f) {
	if (f->unlink_pathname) {
		/* XXX: I hope there's no race here... */
		unlink(f->unlink_pathname);
		free(f->unlink_pathname);
	}
	close(f->fd);
	free(f);
}

#else /* WIN32 */
struct _flexsock {
	enum { fst_stdi, fst_winsock } kind;
	union {
		struct {
			HANDLE in;
			HANDLE out;
		} stdi;
		SOCKET sock;
	} u;
};

struct _flexlisock {
	SOCKET sock;
};

/* Winsock version 1.1. Old and buggy...? */
#define FXS_MINWINSOCK 0x0101

int FXS_Init(void) {
	WSADATA dat;
	if (WSAStartup(FXS_MINWINSOCK, &dat)) return 1;
	return 0;
}

void FXS_Shutdown(void) {
	WSACleanup();
	return;
}

flexsock FXS_StdIO(void) {
	flexsock f;
	HANDLE h1, h2;
	h1=CreateFile("CONIN$", GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (h1 == INVALID_HANDLE_VALUE) return NULL;
	/* Limit events to those we can read with ReadFile,
		otherwise we may hang because of mouse events... */
	SetConsoleMode(h1, ENABLE_PROCESSED_INPUT);
		// ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT);
	FlushConsoleInputBuffer(h1);

	h2=CreateFile("CONOUT$", GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (!h2) {
		CloseHandle(h1);
		return NULL;
	}

	f=(void *) GlobalAlloc(GMEM_FIXED, sizeof(struct _flexsock));
	if (!f) {
		CloseHandle(h1);
		CloseHandle(h2);
		return NULL;
	}
	f->kind=fst_stdi;
	f->u.stdi.in=h1;
	f->u.stdi.out=h2;
	return f;
}

void FXS_Close(flexsock f) {
	if (f->kind == fst_winsock) {
		closesocket(f->u.sock);
	} else { /* stdi/handles type */
		CloseHandle(f->u.stdi.in);
		CloseHandle(f->u.stdi.out);
	}
	GlobalFree(f);
}

flexsock FXS_ConnectTo(const struct flexsock_desc *to) {
	SOCKET sockfd;
	flexsock s;
	if (!FXS_IsProtoEnabled(to->type)) return NULL;
	switch(to->type) {
		case FXST_STDIO:
			return FXS_StdIO();
		case FXST_TCP_C: {
			struct sockaddr_in soa;
			soa.sin_family=AF_INET;
			soa.sin_addr.s_addr=to->data.tcp.ip;
			soa.sin_port=to->data.tcp.port;

			sockfd=socket(PF_INET, SOCK_STREAM, 0);
			if (sockfd == INVALID_SOCKET) return NULL;
			if (connect(sockfd, (struct sockaddr *) &soa, sizeof(soa))) {
				closesocket(sockfd);
				return NULL;
			}
			break;
		}
		default:
			return NULL;
	}
	s=malloc(sizeof(struct _flexsock));
	s->kind=fst_winsock;
	s->u.sock=sockfd;
	return s;
}

flexlisock FXS_BindTo(const struct flexsock_desc *to, struct flexsock_desc *revers) {
	SOCKET sockfd;
	flexlisock s;
	if (!FXS_IsProtoEnabled(to->type)) return NULL;
	switch(to->type) {
		case FXST_TCP_B: {
			struct sockaddr_in soa;
			soa.sin_family=AF_INET;
			soa.sin_addr.s_addr=to->data.tcp.ip;
			revers->type=FXST_TCP_C;
			revers->data.tcp.ip=to->data.tcp.ip;

			sockfd=socket(PF_INET, SOCK_STREAM, 0);
			if (sockfd == INVALID_SOCKET) return NULL;

			if (binds_advisory || !to->data.tcp.port) {
				/* Allocate some unused port number,
				 * don't bind to a specified one. */
				soa.sin_port=0;
			} else {
				soa.sin_port=to->data.tcp.port;
			}
			if (bind(sockfd, (struct sockaddr *) &soa, sizeof(soa))) {
				closesocket(sockfd);
				return NULL;
			}
			/* Hmm, what did we end up with? */
			if (binds_advisory || !to->data.tcp.port) {
				int len=sizeof(soa);
				if (getsockname(sockfd, (struct sockaddr *) &soa, &len)) {
					closesocket(sockfd);
					return NULL;
				}
				revers->data.tcp.port=soa.sin_port;
			} else {
				revers->data.tcp.port=to->data.tcp.port;
			}
			break;
		}
		default: return NULL;
	}
	if (listen(sockfd, 8)) {
		closesocket(sockfd);
		return NULL;
	}
	s=(void *) GlobalAlloc(GMEM_FIXED, sizeof(struct _flexlisock));
	s->sock=sockfd;
	return s;
}

flexsock FXS_AcceptPoll(flexlisock f) {
	SOCKET fd;
	flexsock s;
	u_long nonblock;

	if (ioctlsocket(f->sock, FIONBIO, &nonblock)) return -1;
	/* Sandwich fillings. */
	fd=accept(f->sock, NULL, NULL);
	nonblock=FALSE;
	ioctlsocket(f->sock, FIONBIO, &nonblock);

	if (fd == INVALID_SOCKET) return NULL;
	s=(void *) GlobalAlloc(GMEM_FIXED, sizeof(struct _flexsock));
	s->kind=fst_winsock;
	s->u.sock=fd;
	return s;
}

flexsock FXS_Accept(flexlisock f) {
	SOCKET fd;
	flexsock s;
	fd=accept(f->sock, NULL, NULL);
	if (fd == INVALID_SOCKET) return NULL;
	s=(void *) GlobalAlloc(GMEM_FIXED, sizeof(struct _flexsock));
	s->kind=fst_winsock;
	s->u.sock=fd;
	return s;
}

/* Overlapped ops don't work on 9x'en apparently */
#if 0
static ssize_t FXS_AsyncRead(BOOL async, flexsock f, char *buf, size_t size) {
	size_t ret;
	if (!f->u.stdi.evtRead) {
		/* No current read op. */
		if (async) {
			LPOVERLAPPED ov;
			HANDLE ev;
			ev=CreateEvent(NULL, TRUE, FALSE, NULL);
			if (!ev) return -1;
			f->u.stdi.bufRead=GlobalAlloc(GMEM_FIXED, size);
			if (!f->u.stdi.bufRead) {
				CloseHandle(ev);
				return -1;
			}
			ov=GlobalAlloc(GMEM_FIXED, sizeof(OVERLAPPED));
			if (!ov) {
				GlobalFree(f->u.stdi.bufRead);
				f->u.stdi.bufRead=NULL;
				CloseHandle(ev);
				return -1;
			}
			/* Can't read files. Comms devices only. */
			ov->Offset=0;
			ov->OffsetHigh=0;
			ov->hEvent=ev;
			f->u.stdi.evtRead=ov;
			ReadFile(f->u.stdi.in, f->u.stdi.bufRead, size, &ret, &ov);
		} else {
			if (!ReadFile(f->u.stdi.in, buf, size, &ret, NULL))
				return -1;
			if (ret == 0) return -1;
			return ret;
		}
	}
	/* Async operation in progress. Check up on it. */
	if (GetOverlappedResult(f->u.stdi.in, f->u.stdi.evtRead,
		&ret, async)) {
	} else {
	}
}
#endif

ssize_t FXS_ReadPoll(flexsock f, char *buf, size_t size) {
	size_t ret;
	if (f->kind == fst_winsock) {
		u_long nonblock;
		int errb;
		nonblock=TRUE;
		if (ioctlsocket(f->u.sock, FIONBIO, &nonblock)) return -1;
		/* Sandwich fillings. */
		ret=recv(f->u.sock, buf, size, 0);
		errb=WSAGetLastError();
		nonblock=FALSE;
		ioctlsocket(f->u.sock, FIONBIO, &nonblock);
		if (ret == 0) return -1;
		else if (ret == SOCKET_ERROR) {
				if (errb != WSAEWOULDBLOCK) return -1;
				return 0;
		}
		return ret;
	} else if (f->kind == fst_stdi) {
		unsigned num, n, i, nEvents;
		INPUT_RECORD *ir;
		static unsigned lastEventDisplayed=0;
		ALLOCA_STACK; /* XXX: Key repeat ignored. XXX: Ctrl-Z ignored. */
		/* We are emulating line input, so look for the \r character. */
		if (!GetNumberOfConsoleInputEvents(f->u.stdi.in, &nEvents))
			return -1;
		ir=alloca(sizeof(INPUT_RECORD) * nEvents);
		if (!ir) return -1;
		if (!PeekConsoleInput(f->u.stdi.in, ir, nEvents, &num))
			return -1;
		for(i=0;i<num;i++) {
			if (ir[i].EventType == KEY_EVENT && ir[i].Event.KeyEvent.bKeyDown &&
				i >= lastEventDisplayed && ir[i].Event.KeyEvent.uChar.AsciiChar) {
				WriteConsole(f->u.stdi.out, &ir[i].Event.KeyEvent.uChar.UnicodeChar, 1, &n, NULL);
				if (ir[i].Event.KeyEvent.uChar.AsciiChar=='\r')
					break;
			}
		}
		if (i>=num) {
			lastEventDisplayed=num;
			return 0;
		}
		lastEventDisplayed=0;
		/* ReadFile doesn't like 1/3cooked, so do it ourself.*/
		if (!ReadConsoleInput(f->u.stdi.in, ir, nEvents, &num))
			return -1;
		n=0;
		for(i=0;i<num;i++) {
			if (ir[i].EventType == KEY_EVENT &&
				ir[i].Event.KeyEvent.bKeyDown &&
				ir[i].Event.KeyEvent.uChar.AsciiChar) {
				buf[n]=ir[i].Event.KeyEvent.uChar.AsciiChar;
				if (buf[n]=='\r') {
					n++;
					buf[n]='\n';
					WriteConsole(f->u.stdi.out, &buf[n], 1, &i, NULL);
					n++;
					return n;
				} else if (buf[n]=='\b') {
					n--;
				} else n++;
				/* Ensure if the next char is '\r' there's space for '\n'. */
				if (n + 2 >= size) return n;
			}
		}
		return -1;
	} else return -1;
}

ssize_t FXS_Read(flexsock f, char *buf, size_t size) {
	int i;
	if (f->kind==fst_winsock) {
		while(1) {
			i=recv(f->u.sock, buf, size, 0);
			if (i > 0) return i;
			else return -1;
		}
	} else if (f->kind==fst_stdi) {
		/* XXX: bleuch. */
		while ((i=FXS_ReadPoll(f, buf, size)) == 0) {
			Sleep(500);
		}
		return i;
	} else return -1;
}

/* When we do a write, guarantee that it completes fully.  */
ssize_t FXS_Write(flexsock f, const char *buf, size_t size) {
	int i, j=0;
	if (f->kind==fst_winsock) {
		while (j < size) {
			i=send(f->u.sock, &buf[j], size-j, 0);
			if (i > 0) j+=i;
			else if (i == SOCKET_ERROR) return -1;
			/* Hope i != 0 */
		}
		return j;
	} else if (f->kind==fst_stdi) {
		while(j < size) {
			if (!WriteFile(f->u.stdi.out, &buf[j], size-j,
				&i, NULL)) return -1;
			if (i==0) return -1; /* shouldn't happen */
			j+=i;
		}
		return j;
	} else return -1;
}

void FXS_CloseListener(flexlisock f) {
	closesocket(f->sock);
	GlobalFree(f);
}

#endif
