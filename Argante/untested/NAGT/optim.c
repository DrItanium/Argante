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
unsigned char *bcode_tmp;

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

	op=bcode_tmp[loc * 12];
	a1=*((int *) &bytecode[loc * 12 + 4]);
	bcode_tmp[loc * 12] = CMD_HALT; /* Prevent loops, erase everything :) */
	
	switch(op) {
	case CMD_RET:
	case CMD_HALT: /* hurray! no work! */
		return pop_off_dpoint();
	case CMD_JMP:
		return a1;
	case CMD_CALL:
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

/* A shabby code relocator/shortener.
 * This is for writing out images.
 *
 * At first I thought this was easy... then I remembered
 * we might call somewhere further on in the code...
 */
typedef struct _achain achain;
struct _achain {
	int target;
	int loc;
	achain *next, *prev;
};

achain *achead=NULL;

void fixup(int inloc, int outloc)
{
	achain *a;
	char op;
	int *a1;

	if (inloc<0 || inloc>header.bytesize)
	{
		fprintf(stderr, "Fell off the edge of the world during fixup!\n");
		exit(-1);
	}
	if (outloc<0 || outloc>header.bytesize)
	{
		fprintf(stderr, "Jumped off the edge of the world during fixup!\n");
		exit(-1);
	}

	/* Sort through chain looking for addresses we're at/have passed */
	a=achead;
	while (a)
	{
		if (a->target < inloc)
		{
			fprintf(stderr, "<!> Jump target lost (this is a DCA bug). Op %x, target %x, now at %x.\n", a->loc, a->target, inloc);
			exit(-1);
		}
		if (a->target == inloc)
		{
			/* Ok, target found, adjust it */
			*((int *)&bytecode[a->loc * 12 + 4])=outloc;
//			fprintf(stderr, "<.> Repairing %x -> %x\n", a->loc, outloc); 
			/* Throw it away now */
			if (a->prev)
				a->prev->next=a->next;
			else
				achead=a->next;
			
			if (a->next)
				a->next->prev=a->prev;

			free(a);
		}

		a=a->next;
	}
	
	/* Don't put memcpy to the test */
	if (inloc != outloc) memcpy(&bytecode[outloc * 12], &bytecode[inloc * 12], 12);
	
	op=bytecode[inloc * 12];
	/* We have a rather large scratch space in bcode_tmp.
	 * Seeing as we've already tested the inloc contents,
	 * store the outloc over them */
	a1=(int *) &bcode_tmp[inloc * 12];
	*a1=outloc;
	/* Get a pointer to a1 */
	a1=(int *) &bytecode[outloc * 12 + 4];

	switch(op) {
	case CMD_JMP:
	case CMD_CALL:
	case CMD_ONFAIL:
		/* If we can, correct the address at a1 */
		if (*a1 <= inloc)
		{
			/* There has to be a better way than this, but I'm too dumb */
			*a1=*((int *)&bcode_tmp[*a1 * 12]);
		}
		else /* or if not save it for later */
		{
		//	fprintf(stderr, "<.> Saving %x -> %x\n", inloc, *a1); 
			a=malloc(sizeof(achain));
			a->loc=outloc;
			a->target=*a1;
			a->next=achead;
			a->prev=NULL;
			achead=a;
		}
		break;
	}
}

int main(int argc, char **argv)
{
	int i;
	int dbc, ubc; /* DeadByteCount, UsedByteCount */
	int ldba; /* LastDeadByteAddress */
	FILE *in, *out;

	printf("\nDead Code Analyser - (C) 2001 James Kehl <ecks@optusnet.com.au>\n");
	printf("based on Disassembler - (C) 2000 Maurycy Prodeus <z33d@eth-security.net>\n");
	printf("for %s ver %d.%03d (C) 2000 Michal Zalewski <lcamtuf@tpi.pl>\n\n",
              SYSNAME,SYS_MAJOR,SYS_MINOR);
	
	if (argc!=2 && argc!=3){
	    printf("Usage: %s <input.img> [output.img]\n",argv[0]);
	    exit(-1);
	}

	in=fopen(argv[1],"rb");
	if (!in){
	    perror("open input file");
	    exit(-1);
	}	
	if (argc==3)
	{
		out=fopen(argv[2],"wb");
		if (!out){
		    perror("open output file");
		    exit(-1);
		}
	} else out=NULL;
	
	/* Simple tests to verify image... Endian Fixme! */
	if(fread(&header,sizeof(header), 1, in)!=1){
	    printf("%s is broken.\n",argv[1]);
	    exit(-1);
	}
	if (header.magic1 != BFMT_MAGIC1 || header.magic2 != BFMT_MAGIC2){
	    printf("%s: invalid header.\n",argv[1]);
	    exit(-1);
	}

	/* Load 2 copies of bytecode. */
	bytecode=malloc(header.bytesize*12);
	bcode_tmp=malloc(header.bytesize*12);
	
	if ((!bytecode || !bcode_tmp) && header.bytesize>0){
	    printf("Not enough memory!\n");
	    exit(-1);
	}
	if(fread(bytecode, header.bytesize*12, 1, in)!=1){
	    printf("I can't read %s.\n",argv[1]);
	    exit(-1);
	}

	memcpy(bcode_tmp, bytecode, header.bytesize * 12);

	/* Load data */
	data=(char*) malloc(header.datasize*4);
	if (!data && header.datasize>0){
	    printf("Not enough memory!\n");
	    exit(-1);
	}
	if (fread(data, header.datasize*4, 1, in)!=1 && header.datasize!=0){
	    printf("I can't read %s.\n",argv[1]);
	    exit(-1);
	}
	/* We've finished with the input image */
	fclose(in);

	i=header.init_IP;
	while ((i=follow(i))>=0); /* Yer classic recursion loop, mate! */

	ldba=-1;
	dbc=ubc=0;
	
	/* Ok, check for leftovers */
	for(i=0;i<header.bytesize;i++)
	{
		if (bcode_tmp[i * 12] != CMD_HALT)
		{
			dbc++;
			if (ldba < 0) ldba=i;
		} else {
			ubc++;
			if (0 < ldba)
			{
				fprintf(stderr, "<-> bytecode packets %x-%x are unused\n", ldba, i);
				ldba=-1;
			}
		}
	}
	/* In case I stuff up */
	if (ubc != header.bytesize - dbc)
		fprintf(stderr, "<!> Mismatch! Expected used bytes: %d\n", header.bytesize - dbc);

	fprintf(stderr, "<+> Bytecode packets: %d unused, %d used\n", dbc, ubc);
	
	/* Now translate code if we have an output file */
	if (out)
	{
		i=ubc;
		/* Now UBC and DBC become indexes. Sorry */
		ubc=dbc=0;
		
		for(dbc=0;dbc<header.bytesize;dbc++)
		{
			if (bcode_tmp[dbc * 12] == CMD_HALT)
			{
				fixup(dbc, ubc);
				ubc++;
			}
		}
	
		/* Output stage */
		header.bytesize=i;
		
		if(fwrite(&header, sizeof(header), 1, out)!=1){
			perror("fwrite");
			exit(-1);
		}
		if(fwrite(bytecode, header.bytesize * 12, 1, out)!=1){
			perror("fwrite");
			exit(-1);
		}
		if(fwrite(data, header.datasize * 4, 1, out)!=1){
			perror("fwrite");
			exit(-1);
		}
		fclose(out);
	}
	free(data);
	free(bytecode);
	free(bcode_tmp);
	return 0;
}
