/*
 * A2 vconsole
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
#include "compat/usleep.h"

#include "flexsock.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
	int i;
	struct flexsock_desc fxsd;
	static char buf[512];
	flexsock remt;
	flexsock cons;
	
	if (argc != 2) {
		fprintf(stderr, 
"vconsole Copyright (C) 2002 James Kehl <ecks@optusnet.com.au>\n"
"vconsole comes with ABSOLUTELY NO WARRANTY; this is free software, and you are\n"
"welcome to redistribute it under certain conditions. See COPYING for details.\n\n"
"Usage: %s flexsock_descriptor\n"
" descriptor examples:\n"
" STDI                   - Become a bloated 'cat'.\n"
#ifndef __WIN32__
" UNXC/pathto/unixsock   - Connect to Unix socket.\n"
" UNXB[/pathto/unixsock] - Create Unix socket.\n"
#endif
" TCPC127.0.0.1:21       - Connect to localhost port 21.\n"
" TCPB127.0.0.1[:port]   - Open port on localhost.\n"
		, argv[0]);
		exit(1);
	}

	if (FXS_Init()) {
		fprintf(stderr, "Could not initialize flexsock library.\n");
      exit(1);
	}
	FXS_SetProtocols( /* As we're run by the user, let them do what they like. */
		FXS_PFLAG(FXST_STDIO)
		| FXS_PFLAG(FXST_UNIX_B) | FXS_PFLAG(FXST_UNIX_C)
		| FXS_PFLAG(FXST_TCP_B)  | FXS_PFLAG(FXST_TCP_C));

	if (FXS_Ascii2Desc(&fxsd, argv[1])) {
		fprintf(stderr, "Error parsing descriptor.\n");
		exit(1);
	}

	i=FXS_IsConnectType(&fxsd);
	if (i<0) {
		fprintf(stderr, "Internal error in flexsock.\n");
		exit(1);
	} else if (i==0) {
		flexlisock bound;
		struct flexsock_desc revers;
		/* Bind a flexsock. */
		bound=FXS_BindTo(&fxsd, &revers);
		if (!bound) {
			fprintf(stderr, "Couldn't bind.\n");
			exit(1);
		}
		if (FXS_Desc2Ascii(&revers, buf, sizeof(buf))) {
			fprintf(stderr, "Listening, but I can't say where.\n");
		} else {
			fprintf(stderr, "Listening. To connect, use descriptor %s.\n", buf);
		}
		remt=FXS_Accept(bound);
		FXS_CloseListener(bound);
	} else {
		/* Connect a flexsock. */
		remt=FXS_ConnectTo(&fxsd);
	}

	if (!remt) {
		if (FXS_Desc2Ascii(&fxsd, buf, sizeof(buf))) {
			fprintf(stderr, "Couldn't open flexsock.\n");
		} else {
			fprintf(stderr, "Could not open flexsock %s.\n", buf);
		}
		exit(1);
	}
	cons=FXS_StdIO();
	if (!cons) {
		fprintf(stderr, "Could not open flexsock for console.\n");
		exit(1);
	}

	fprintf(stderr, "Connected.\n");
	/* Poll must return -1 on error and 0 on EAGAIN */
	while(1) {
		if ((i=FXS_ReadPoll(cons, buf, sizeof(buf))) > 0) {
//			fprintf(stderr, "got %d cons bytes\n", i);
			if (FXS_Write(remt, buf, i) < 0) {
				fprintf(stderr, "Unexpected write error (%s), connection aborted.\n", "remt");
				return 0;
			}
			continue;
		}
		if (i < 0) break;
		if ((i=FXS_ReadPoll(remt, buf, sizeof(buf))) > 0) {
  //			fprintf(stderr, "got %d remt bytes\n", i);
			if (FXS_Write(cons, buf, i) < 0) {
				fprintf(stderr, "Unexpected write error (%s), connection aborted.\n", "cons");
				return 0;
			}
			continue;
		}
		if (i < 0) break;
		usleep(10000);
	}
	fprintf(stderr, "Disconnected.\n");

	FXS_Close(remt);
	FXS_Close(cons);
	FXS_Shutdown();
	return 0;
}
