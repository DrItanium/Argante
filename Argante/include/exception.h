/*

   Argante virtual OS 
   ------------------

   Exception numbers and symbols.

   Status: done, add own exceptions here

   Author:     Michal Zalewski <lcamtuf@ids.pl>
   Maintainer: (public domain)

*/

#define ERROR_STACK_OVER		0x1
#define ERROR_STACK_UNDER		0x2
#define ERROR_OUTSIDE_CODE		0x3
#define ERROR_OUTSIDE_REG		0x4
#define ERROR_BAD_PARAM			0x5
#define ERROR_BAD_INSTR			0x6
#define ERROR_OUTSIDE_MEM		0x7
#define ERROR_PROTFAULT			0x8
#define ERROR_TOOBIG			0x9
#define ERROR_NOMODULE			0xa
#define ERROR_BAD_SYS_PARAM		0xb
#define ERROR_ACL_PROBLEM		0xc
#define ERROR_NOPERM			0xd
#define ERROR_NOMEM			0xe
#define ERROR_DEADLOCK			0xf
#define ERROR_NOOBJECT			0x10
#define ERROR_FSERROR			0x11

#define ERROR_FS_BAD_PATH		0x12
#define ERROR_FS_OPEN_ERROR		0x13
#define ERROR_FS_BAD_OPEN_MODE		0x14
#define ERROR_FS_CREATE_ERROR		0x15
#define ERROR_FS_BAD_VFD		0x16
#define ERROR_FS_NOSEEK			0x17
#define ERROR_FS_EXISTS			0x18
#define ERROR_FS_NOFILE		        0x19
#define ERROR_FS_NODIRENT		0x1a
#define ERROR_RESULT_TOOLONG		0x1b

#define ERROR_NOUSTACK			0x20
#define ERROR_USTACK_UNDER		0x21
#define ERROR_USTACK_OVER		0x22

#define ERROR_BADID			0x1101 // HLL ;>
#define ERROR_BADCHAR			0x1001
#define ERROR_BADMODE			0x1201

#define ERROR_NETERROR                  0x60
#define ERROR_NET_BAD_SD                0x61
#define ERROR_NET_NO_FREE_SD            0x62
#define ERROR_NET_CONN_REFUSED          0x63
#define ERROR_NET_SOCK_NOT_CONN         0x64
#define ERROR_NET_EOF                   0x65
#define ERROR_NET_EPIPE                 0x66
#define ERROR_NET_PORT_OOR              0x67
#define ERROR_NET_SOCK                  0x68
#define ERROR_NET_BIND                  0x69
#define ERROR_NET_NONBLOCK              0x70
#define ERROR_NET_BAD_BLOG              0x71
#define ERROR_NET_SOCK_NON_LISTEN       0x72
#define ERROR_NET_UNREACH               0x73
#define ERROR_NET_TIMEO                 0x74

#define ERROR_MEM_FORMAT		0x80
#define ERROR_MEM_OFFSET		0x81
#define ERROR_NET_BAD_HOW               0x75

#define ERROR_MATH_RANGE		0x90
#define ERROR_MATH_DIV			0x91

#define ERROR_HLL_NULLPTR		10001 // Pointer is not initialized
#define ERROR_HLL_INDEX_UNDER           10002 // Table index too low
#define ERROR_HLL_INDEX_ABOVE           10003 // Table index too high
#define ERROR_HLL_PTR_BINDED            10004 // Pointer is already binded
#define ERROR_HLL_PTR_STATIC            10005 // This pointer cannot be freed

#define ERROR_IPC_NOMEM			1+0x400	// memory allocation failure
#define ERROR_IPC_BAD_FLAGS		2+0x400	// bad flags passed
#define ERROR_IPC_BAD_TARGET		3+0x400	// invalid target specified
#define ERROR_IPC_NO_TARGET		4+0x400	// (NETWORKING) no target found
#define ERROR_IPC_NOT_REGISTERED 	5+0x400	// not registered for ipc
#define ERROR_IPC_NO_RESOURCES		6+0x400	// no free stream or so...
#define ERROR_IPC_NO_REQUEST		7+0x400	// when nacking or acking, no such request
#define ERROR_IPC_REQUEST_TIMEOUTED	8+0x400	// ?? :))
#define ERROR_IPC_REQUEST_NACKED	9+0x400	// ?? :))
#define ERROR_IPC_STREAM_ID_INVALID	10+0x400	// trying to abuse unused stream
#define ERROR_IPC_STREAM_CLOSED		11+0x400	// writing to closed stream
#define ERROR_IPC_BLOCK_ID_INVALID	12+0x400	// guess ;)
#define ERROR_IPC_STREAM_DEADLOCK	13+0x400
#define ERROR_IPC_DEAD			14+0x400

#define ERROR_GFX_INIT			1001	// cannot initialize gfx mode
#define ERROR_GFX_BUSY			1002	// another program uses gfx module
#define ERROR_GFX_NOTINITED		1003	// videolib not inited yet
#define ERROR_GFX_TOOMUCH		1004	// your software wants too much, baby

#define ERROR_GGI_INVALID_VISUAL    2000 // No such visual
#define ERROR_GGI_OPEN              2001 // Could not open visual
#define ERROR_GGI_CONTROL           2002 // Generic control function failure
#define ERROR_GGI_OUTPUT            2003 // Generic output function failure
#define ERROR_GGI_INPUT             2004 // Generic input function failure

