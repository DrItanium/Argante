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
        if (whatever==CMD_SYSCALL){
            for(l=0;l<SCNUM;l++){
              if(sys[l].num==a1){
                sprintf(str2,"$%s",sys[l].name);
                return 2;
              }
            }
        }
 	
	if (((a1<0)||(a1>=REGISTERS)) && t1!=TYPE_IMMEDIATE && t1!=TYPE_IMMPTR){
    	    sprintf(str,"Bad instruction.");
	    return 0;
          }
        tmp=(float*)&a1;
	switch(t1){
          case TYPE_IMMEDIATE: 
	  if (op[i].params==2 && t2==TYPE_FREG)
	    sprintf(str2,"%f",(float)*tmp);
	  else sprintf(str2,"0x%x",a1);break;
          case TYPE_IMMPTR: sprintf(str2,"*0x%x",a1);break;
          case TYPE_UREG: sprintf(str2,"u%d",a1);break;
          case TYPE_SREG: sprintf(str2,"s%d",a1);break;
          case TYPE_FREG: sprintf(str2,"f%d",a1);break;
          case TYPE_UPTR: sprintf(str2,"*u%d",a1);break;
          default: sprintf(str,"Bad instruction."); return 0;
	}
	// Ist das the end ?
	if (op[i].params==1)
          return 2;
	if (((a2<0)||(a2>=REGISTERS)) && t2!=TYPE_IMMEDIATE && t2!=TYPE_IMMPTR){
    	    sprintf(str,"Bad instruction.");
	    *str2='\0'; *str3='\0';
	    return 0;
          }
        tmp=(float*)&a2;  
	switch(t2){
          case TYPE_IMMEDIATE: 
	  if (t1==TYPE_FREG)
	    sprintf(str3,"%f",(float)*tmp);
	  else sprintf(str3,"0x%x",a2);break;
          case TYPE_IMMPTR: sprintf(str3,"*0x%x",a2);break;
          case TYPE_UREG: sprintf(str3,"u%d",a2);break;
          case TYPE_SREG: sprintf(str3,"s%d",a2);break;
          case TYPE_FREG: sprintf(str3,"f%d",a2);break;
          case TYPE_UPTR: sprintf(str3,"*u%d",a2);break;
          default: *str2='\0'; *str3='\0';sprintf(str,"Bad instruction.");return 0;
	}
	return 3;
}

int main(int argc, char **argv)
{
	char s[128], s2[128], s3[128];
	unsigned int addr,i,d,repeat=0;
	printf("\nMaurycy Prodeus (C) 2000 <z33d@eth-security.net>\n");
	printf("Disassembler of %s ver %d.%03d (C) 2000 Michal Zalewski <lcamtuf@tpi.pl>\n\n",
              SYSNAME,SYS_MAJOR,SYS_MINOR);
	if (argc<3){
	    printf("Usage: %s <filename.img> <filename.agt>\n",argv[0]);
	    printf("\nWhere .img is input and .agt is output\n");
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
	    addr=0;d=0;
	    fprintf(out,":DATA_0x%x\n\n\t",addr);
	    while(addr<header.datasize*4){
	    
	    i=addr;
	    repeat=0;
	    while (*(int*)(&data[addr])==*(int*)(&data[addr+4])){
	      addr+=4;
	      if (addr>=header.datasize*4)
	          break;
	      repeat++;
	    }
	    if (repeat>0){
		repeat++;
		fprintf(out,"0x%x repeat %d\n",*(int*)(&data[addr]), repeat);
		addr+=4;
		if (addr>=header.datasize*4)
		    break;
		fprintf(out,":DATA_0x%x\n\n\t",addr);
	        d=0;
	    }
	    i=addr;
	    while((isprint(data[i]) || data[i]=='\n' || data[i]=='\r'
	    || data[i]=='\b') && (i<header.datasize*4) && 
	    data[i]!='"' && data[i]!='\\'){
	      i++;
	    }
	    if (i-addr<4){
	      fprintf(out,"0x%x\n", *(int*)(&data[addr]));
	      addr+=4;
	      if (addr>=header.datasize*4)
	         break;
	      fprintf(out,":DATA_0x%x\n\n\t",addr);
	      d=0;
	      continue;
	    }
	    
	    while((isprint(data[addr]) || data[addr]=='\n' || data[addr]=='\r'
	    || data[addr]=='\b') && (addr<header.datasize*4) && 
	    data[addr]!='"' && data[addr]!='\\'){
		if (!d){
		  d=1;
		  fprintf(out,"\"");
		  i=0;
		}	    
		if (!iscntrl(data[addr]))
		  fprintf(out,"%c",data[addr]);
		else switch(data[addr]){
		    case '\n': fprintf(out,"\\n");break;
		    case '\r': fprintf(out,"\\r");break;
		    case '\b': fprintf(out,"\\b");break;
		}
	        i++;
		addr++;
		if (addr>=header.datasize*4){
		    if (d)
		      fprintf(out,"\"\n");
		    d=0;
		    break;
		}
	    } // end of while(isprint ...
	    if (i%4!=0)
	      addr+=4-i%4;
	    
	    if (addr>=header.datasize*4)
		break;
	    if (d){
		fprintf(out,"\"\n:DATA_0x%x\n\n\t",addr);
		d=0;
	    }
	    if ((!isprint(data[addr]) && data[addr]!='\n' && data[addr]!='\r'
	    && data[addr]!='\b') || data[addr]=='\\' || data[addr]=='"'){
		fprintf(out,"0x%x\n", *(int*)(&data[addr]));
		addr+=4;
		if (addr>=header.datasize*4)
		    break;
		fprintf(out,":DATA_0x%x\n\n\t",addr);
		d=0;
	    }
	    
	    }
	    printf("[DONE]\n");
	}
	
	printf("Disassembling code ... ");
	fprintf(out,"\n.CODE\n\n");
	for(addr=0;addr<header.bytesize;addr++){
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
