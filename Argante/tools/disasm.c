/*

   Argante virtual OS
   ------------------

   Disassembler for .img files

   Status: unstable, to be debugged

   Author:     Maurycy Prodeus <z33d@eth-security.net>
   Maintainer: Maurycy Prodeus <z33d@eth-security.net>


                 Oczy niebieskie    
    Znow wstaje i po omacku dotykam sciany
    Przechodze poprzez pokoj na wpol rozebrany
    Gdy docieram do lustra, to juz polowa drogi
    Nie pozbede sie przez dzien dzisiejszy uczucia trwogi
    Patrzy na mnie twarz z lustra. Jej oczy niebieskie
    Nie daja mi zapomniec, ze ponoc jestem czlowiekiem
    Wiec odchodze od lustra i o tym nie pamietam
    Taki jestem od wczoraj. Czy to ubior ten sam?
    Oczy niebieskie mowia wprost:
    Wczoraj wyjatkowo aktywna noc

    Mrowie chodzi po glowie, swiatlo slonca razi
    Nie mam tyle sily, by sie patrzec odwazyc
    Brudne rece od wczoraj i wlosy. Dwa swiaty
    Dnia i Nocy sie lacza bez wieczornej toalety
    Klade sie i po omacku opuszczam swoja sciane
    Ta metoda jest znana, czas zaleczy rane
    A sen wyleczy mnie i usunie spod powiek
    Bol, co oczy niebieskie wpychaja mi w glowe
    Oczy niebieskie mowia wprost:
    Wczoraj wyjatkowo aktywna noc...

		- Kazik Staszewski
*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include "config.h"
#include "console.h"
#include "task.h"
#include "bcode.h"
#include "bformat.h"
#include "language.h"

struct sdes{
    char* name;
    int num;
};
struct sdes sys[]={
#include "autogen-debug.h"
};
#define SCNUM (sizeof(sys)/sizeof(struct sdes))
 
struct bformat header;
unsigned char *bytecode, *data;
FILE *in, *out;

/* NAG symbol support - jk */
#define STATE_DATA 1
#define STATE_CODE 2

typedef struct _syment syment; /* No puns please */
struct _syment {
	char *name; /* Dynamic len. Worth the effort IMHO */
	int addr;
	int size;
	char stortype;
	syment *next;
};

syment *syment_head=NULL;

#define misprint(c) (isprint(c) || c=='\r' || c=='\t' || c=='\e' || c=='\n' || c=='\b')

void read_syms(char *f)
{
	FILE *sym;
	char buf[80]; /* Symname is < 70 for agtc & nagt so we're safe */
	int n, ci, ct=0;
	syment *new;

	sym=fopen(f, "rb");
	if (!sym) 
	{
		perror("fopen of symfile failed");
		return;
	}

	while(1)
	{
		ct++;
		n=0;
		
		while (n < (sizeof(buf) - 1) && (ci=fgetc(sym)) > 0 && (buf[n]=ci))
			n++;
		buf[n]=0;

		if (n==0)
		{
			fprintf(stderr, "%d symbols read from table\n", ct);
			break;
		} else if (n >= (sizeof(buf) - 1))
		{
			fprintf(stderr, "Overlong symbol name: aborting symbol read!\n");
			break;
		}

		new=malloc(sizeof(syment));
		if (!new)
		{
			fprintf(stderr, "Malloc failure!\n");
			break;
		}
		new->name=strdup(buf);
		/* Fixme - truncation checks */
		fread(&new->addr, sizeof(new->addr), 1, sym);
		fread(&new->size, sizeof(new->size), 1, sym);
		fread(&new->stortype, sizeof(new->stortype), 1, sym);

		/* Link in */
		new->next=syment_head;
		syment_head=new;
	}
	fclose(sym);
}

int addrtosym(int addr, char stortype, char *ibuf)
{
	syment *s;

	s=syment_head;
	while (s && (s->addr != addr || s->stortype != stortype))
	{
	//	fprintf(stderr, "addr %d!=%d || type %d != %d (name %s)\n", s->addr, addr, s->stortype, stortype, s->name);
		s=s->next;
	}

	if (s)
	{
		sprintf(ibuf,":%s",s->name);
		return 0;
	}

	switch(stortype) {
		case STATE_CODE:
			sprintf(ibuf,":CODE_0x%x",addr);
			break;
		case STATE_DATA:
			sprintf(ibuf,":DATA_0x%x",addr);
			break;
		default:
			sprintf(ibuf,":unknown_type_0x%x",addr);
	}
	return 1;
}

void do_args(int a, int t, int ta, char *out)
{
	float *tmp;
	static char buf[80];
	tmp=(float *) &a;
	
	switch(t){
          case TYPE_IMMEDIATE: 
		if (ta==TYPE_FREG)
			sprintf(out,"%f",*tmp);
		else
			sprintf(out,"0x%x",a);
		break;
          case TYPE_IMMPTR:
		if (addrtosym(a, STATE_DATA, buf))
		;
	//		sprintf(out,"*0x%x",a);
	//	else
			sprintf(out,"*%s", buf);
		break;
          case TYPE_UREG: sprintf(out,"u%d",a);break;
          case TYPE_SREG: sprintf(out,"s%d",a);break;
          case TYPE_FREG: sprintf(out,"f%d",a);break;
          case TYPE_UPTR: sprintf(out,"*u%d",a);break;
          default: sprintf(out,"BAD");
	}
}

int disasm(char *str, char *str2, char *str3,int otheraddr)
{
        unsigned int reality;
        int* MISIO;
        int a1, a2;
        int t1, t2;
        int whatever;
        int i,l;
	float *tmp;

        if (otheraddr<0 || otheraddr>header.bytesize) {
	  sprintf(str,"It's impossible. Your cpu corrupts.");
          return 0;
	}
    
	reality=otheraddr*12;
	t1=bytecode[reality+1];
	t2=bytecode[reality+2];
	// space
	MISIO=(int*)&bytecode[reality+4];
	a1=*MISIO;
	a2=*(MISIO+1);
	whatever=bytecode[reality];
	i=0;
	*str='\0'; *str2='\0'; *str3='\0';
	while((whatever!=op[i].bcode)&&(i<OPS)){
          i++;
	}
	if (whatever!=op[i].bcode){
          sprintf(str,"Bad instruction.");
          return 0;
	}
	sprintf(str,"%s",op[i].name);
	// Ist das the end ?
	if (op[i].params==0)
          return 1;
        // put symbol
	switch(whatever)
	{
		case CMD_SYSCALL:
			for(l=0;l<SCNUM;l++)
			{
				if(sys[l].num==a1)
				{
					sprintf(str2,"$%s",sys[l].name);
					return 2;
				}
			}
		case CMD_JMP:
		case CMD_CALL:
		case CMD_LOOP:
			if (addrtosym(a1, STATE_CODE, str2))
				sprintf(str2,"0x%x",a1);

			return 2;
        }
 	
	if (((a1<0)||(a1>=REGISTERS)) && t1!=TYPE_IMMEDIATE && t1!=TYPE_IMMPTR){
    	    sprintf(str,"Bad instruction.");
	    return 0;
          }

	if (op[i].params==1)
	{
		do_args(a1, t1, 0, str2);
		return 2;
	}
	
	do_args(a1, t1, t2, str2);

	if (((a2<0)||(a2>=REGISTERS)) && t2!=TYPE_IMMEDIATE && t2!=TYPE_IMMPTR){
    	    sprintf(str,"Bad instruction.");
	    *str2='\0'; *str3='\0';
	    return 0;
        }

	do_args(a2, t2, t1, str3);
	return 3;
}

int main(int argc, char **argv)
{
	char s[128], s2[128], s3[128];
	unsigned int addr,i,d,repeat=0;
	int rdl;
	printf("\nMaurycy Prodeus (C) 2000 <z33d@eth-security.net>\n");
	printf("Disassembler of %s ver %d.%03d (C) 2000 Michal Zalewski <lcamtuf@tpi.pl>\n\n",
              SYSNAME,SYS_MAJOR,SYS_MINOR);
	if (argc<3){
	    printf("Usage: %s <filename.img> <filename.agt> [filename.sym]\n",argv[0]);
	    printf("\nWhere .img is input and .agt is output\n");
	    printf("(and .sym is symbol table if one's available)\n");
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

	if (argc >= 4)
		read_syms(argv[3]);
	out=fopen(argv[2],"w");
	if (!out){
	    printf("I can't open output file!\n");
	    exit(-1);
	}
	fprintf(out,"!SIGNATURE\t\"%s\"\n",header.signature);
	fprintf(out,"!PRIORITY\t0x%x\n",header.priority);
	fprintf(out,"!IPCREG\t0x%x\n",header.ipc_reg);
	fprintf(out,"!INITDOMAIN\t0x%x\n",header.current_domain);
	fprintf(out,"!INITUID\t0x%x\n",header.domain_uid);
	for (i=0;i<MAX_EXEC_DOMAINS;i++){
	    if (header.domains[i]==0)
		break;
	    fprintf(out,"!DOMAINS\t%d\n",header.domains[i]);
	}
	// data, it's only temporary solution
	if (header.datasize>0){
	    printf("Parsing data ... ");
	    fprintf(out,"\n.DATA\n\n");
	    addr=0;
	    
	    rdl=header.datasize * 4; /* Real Data Length - a plague on both words! */
	    /* Ick. Did something crawl in here and die? */
	    while (addr < rdl)
	    {
		    d=0;
		    addrtosym(addr / 4, STATE_DATA, s);
		    fprintf(out,"%s\n\n\t",s);
		   
		    /* Repeat test */
		    repeat=1;
		    while (addr < rdl - 4 && *(int*)(&data[addr])==*(int*)(&data[addr+4])){
			    addr+=4;
			    repeat++;
		    }
		    
		    if (repeat > 1)
		    {
			    fprintf(out,"0x%x repeat %d\n",*(int*)(&data[addr]), repeat);
			    continue;
		    }

		    /* String test */
		    while(addr < rdl && misprint(data[addr]))
		    {
			    if (!d)
			    {
				    fprintf(out, "\"");
				    d=1;
			    }
			    
			    switch(data[addr])
			    {
				    case '\n':fprintf(out,"\\n");break;
				    case '\r':fprintf(out,"\\r");break;
				    case '\e':fprintf(out,"\\e");break;
				    case '\b':fprintf(out,"\\b");break;
				    case '\t':fprintf(out,"\\t");break;
				    case '\"':fprintf(out,"\\\"");break;
				    default: fputc(data[addr], out);
			    }
			    addr++;
		    }
		    
		    if (d) /* A string! */
		    {
			    d=0;
			    if (addr % 4) /* Hmm... string not on dword boundary */
			    {
				    /* If there's any data, we should print it.
				     * No compiler yet understands \x00 escapes but
				     * this will change and it's better than nothing.
				     */
				    i=addr;
				    while (i%4)
				    {
					    d+=data[i];
					    i++;
				    }

				    if (d)
				    { /* Yuk! there IS data! */
					    while(addr % 4)
					    {
						    fprintf(out, "\\x%x", data[addr]);
						    addr++;
					    }
				    } else addr=i;
			    }

			    fprintf(out, "\"\n");
			    continue;
		    }
		    /* Hm. Plain old int, I guess. */
		    fprintf(out, "0x%x\n", data[addr]);
		    addr+=4;
	    }
	    printf("[DONE]\n");
	}
	
	printf("Disassembling code ... ");
	fprintf(out,"\n.CODE\n\n");
	for(addr=0;addr<header.bytesize;addr++){
		if (!addrtosym(addr, STATE_CODE, s))
				fprintf(out, "%s\n", s);
	    switch(disasm(s,s2,s3,addr)){
		case 0:	printf("Error.\n0x%x: %s\n", addr, s); exit(-1); 
		case 1: fprintf(out,"\t%s\n",s); break;
		case 2: fprintf(out,"\t%s   %s\n",s,s2); break;
		case 3: fprintf(out,"\t%s   %s,%s\n",s,s2,s3); break;
		default: printf("Intel ssie ...\n"); exit(-1);
	    }
	}
	
	fprintf(out,"\n.END\n");
	fclose(out);
	printf("[DONE]\n");
	free(data);
	free(bytecode);
	return 0;
}
