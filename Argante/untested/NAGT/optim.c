/*
 * DCA - Dead Code Analyser
 * 
 * Input:
 * Single relocatable Argante image.
 * 
 * Output:
 * Lotsa warnings :)
 * 
 * When relocatable images come around, we might be able to 
 * rename this the DCE - E for Eliminator.
 *
 * I'm also tempted to do Dead Data Analysis; but that involves lots of
 * knowledge about syscalls. Ick.
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include "config.h"
#include "bcode.h"
#include "bformat.h"

/*
struct sdes{
    char* name;
    int num;
};
struct sdes sys[]={
#include "autogen-debug.h"
};
#define SCNUM (sizeof(sys)/sizeof(struct sdes))
*/

struct bformat header;
unsigned char *bytecode, *data;

typedef struct _dpoint dpoint;
struct _dpoint {
	int loc;
	dpoint *next;
};

dpoint *dphead=NULL;

/* The way the DCA works:
 * 
 * 1) We start at INITIAL_IP.
 * 2) We work out where code flow MIGHT go from there.
 *    If it forks (IFABO, IFBEL, IFEQ, IFNEQ etc) we add the
 *    alternate location to a stack.
 * 3) If we come to a HALT (or a HLL RAISE, perhaps? might be caught) we pop
 *    a location off the stack. If we have no locations left we're done.
 *    Else we set this op to HALT.
 * 4) Go to 2.
 *
 * Then it's easy to spot dead code - it's anything other than HALT :)
 */    

int pop_off_dpoint()
{
	dpoint *d;
	int r;
	
	if (!dphead) return -1;
	
	d=dphead->next;
	r=dphead->loc;
	free(dphead);
	dphead=d;
	return r;
}

void push_on_dpoint(int loc)
{
	dpoint *d;
	
	d=malloc(sizeof(dphead));
	d->loc=loc;
	d->next=dphead;
	dphead=d;
}

/* returns next address */
int follow(int loc)
{
	char op;
	int a1;

	if (loc<0 || loc>header.bytesize)
	{
		a1=pop_off_dpoint();
		fprintf(stderr, "I just fell off the edge of the world!\nWas at %d, now going to %d\n", loc, a1);
		return a1;
	}

	op=bytecode[loc * 12];
	a1=bytecode[loc * 12 + 4];
	bytecode[loc * 12] = CMD_HALT; /* Prevent loops, erase everything :) */
	
	switch(op) {
	case CMD_RET:
	case CMD_HALT: /* hurray! no work! */
		return pop_off_dpoint();
	case CMD_JMP:
		return a1;
	case CMD_CALL:
		push_on_dpoint(a1);
		break;
	case CMD_ONFAIL: /* Almost forgot this! */
		push_on_dpoint(a1);
		break;
	case CMD_IFEQ:
	case CMD_IFNEQ:
	case CMD_IFABO:
	case CMD_IFBEL:
		push_on_dpoint(loc+2);
		break;
	}
	return loc + 1;
}

int main(int argc, char **argv)
{
	int i;
	int dbc;
	int ldba;
	FILE *in;

	printf("\nDead Code Analyser - (C) 2001 James Kehl <ecks@optusnet.com.au>\n");
	printf("based on Disassembler - (C) 2000 Maurycy Prodeus <z33d@eth-security.net>\n");
	printf("for %s ver %d.%03d (C) 2000 Michal Zalewski <lcamtuf@tpi.pl>\n\n",
              SYSNAME,SYS_MAJOR,SYS_MINOR);
	
	if (argc!=2){
	    printf("Usage: %s <filename.img>\n",argv[0]);
	    exit(-1);
	}
	in=fopen(argv[1],"r");
	if (!in){
	    perror("open");
	    exit(-1);
	}	
	
	if(fread(&header,sizeof(header), 1, in)!=1){
	    printf("%s is broken.\n",argv[1]);
	    exit(-1);
	}
	if (header.magic1 != BFMT_MAGIC1 || header.magic2 != BFMT_MAGIC2){
	    printf("%s: invalid header.\n",argv[1]);
	    exit(-1);
	}
	bytecode=(char*) malloc(header.bytesize*12);
	if (!bytecode && header.bytesize>0){
	    printf("Not enough memory!\n");
	    exit(-1);
	}
	if(fread(bytecode, header.bytesize*12, 1, in)!=1){
	    printf("I can't read %s.\n",argv[1]);
	    exit(-1);
	}
	data=(char*) malloc(header.datasize*4);
	if (!data && header.datasize>0){
	    printf("Not enough memory!\n");
	    exit(-1);
	}
	if (fread(data, header.datasize*4, 1, in)!=1 && header.datasize!=0){
	    printf("I can't read %s.\n",argv[1]);
	    exit(-1);
	}
	fclose(in);

	i=header.init_IP;
	while ((i=follow(i))>=0); /* Yer classic recursion loop, mate! */

	ldba=-1;
	dbc=0;
	
	/* Ok, check for leftovers */
	for(i=0;i<header.bytesize;i++)
	{
		if (bytecode[i * 12] != CMD_HALT)
		{
			dbc++;
			if (ldba < 0) ldba=i;
		} else {
			if (0 < ldba)
			{
				fprintf(stderr, "<-> bytecode packets %x-%x are unused\n", ldba, i);
				ldba=-1;
			}
		}
	}
	fprintf(stderr, "<+> Total %d unused bytecode packets\n", dbc);
	
	free(data);
	free(bytecode);
	return 0;
}
