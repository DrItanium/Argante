/*

   Argante virtual OS
   ------------------

   Debugging subsystem.

   Status: done, but ugly ;>

   Author:     Maurycy Prodeus <z33d@eth-security.net>
   Maintainer: Maurycy Prodeus <z33d@eth-security.net>

   I did some minor bugfixes here -- lcamtuf

*/

#define I_AM_DE_BURGER 0xbeef
#define mniam

#include <unistd.h>
#include <stdlib.h>
#include "config.h"
#include "task.h"
#include "bcode.h"
#include "console.h"
#include "memory.h"
#include "debugger.h"
#include "language.h"

struct sdes{
    char* name;
    int num;
};
struct sdes sys[]={
#include "autogen-debug.h"
};
#define SCNUM (sizeof(sys)/sizeof(struct sdes))

// don't ask ...
struct debug_desc debug[MAX_VCPUS];

int disasm(int c, char*, char*, char*,int);
// Main debug function
int main_debug(int c)
{
	int i;
	char opc[128], s2[128], s3[128];
	
	if (c<0 || c>=MAX_VCPUS) {
	  printk("Bad parameter.\n");
	  return 0;
	}
	// if it's dead and somebody tried to continue
	if (debug[c].traceflag & DEBUG_DEAD){
	    printk("-> DEBUG: Unhandled exception or kill, process is dead.\n");
	    destroy_task(c);
	    return 1;
	}
	// if step, only one instruction
	if (debug[c].traceflag & DEBUG_STEP){
	  debug[c].traceflag&=~DEBUG_STEP;
	  return 0;
	}
	// if breakpoint
	for(i=1;i<=debug[c].b_size;i++)
	  if (debug[c].breakpoints[i-1]==cpu[c].IP){
	     printk("=> DEBUG: %d# Breakpoint at 0x%x\n",i-1,cpu[c].IP);
	     printk("=> DEBUG: %d vCPU is stopped.\n",c);
	     cpu[c].state|=VCPU_STATE_STOPPED;
	     debug[c].traceflag=DEBUG_DEBUG;
	     switch(disasm(c,(char*)&opc,(char*)&s2,(char*)&s3,-1)){
	       case 0: printk("ASM> 0x%x: %s\n",cpu[c].IP,opc); break;
	       case 1: printk("ASM> 0x%x: %s  %s\n",cpu[c].IP,opc,s2);break;
	       case 2: printk("ASM> 0x%x: %s  %s, %s\n",cpu[c].IP,opc,s2,s3);break;
	     }
	     return 1;
	  }
	// if syscall
	if (debug[c].traceflag & DEBUG_SYSCALL){
	    if (cpu[c].bytecode[cpu[c].IP*12]==CMD_SYSCALL){
		printk("=> DEBUG: %d vCPU is stopped.\n",c);
		cpu[c].state|=VCPU_STATE_STOPPED;
		debug[c].traceflag=DEBUG_DEBUG;
	        switch(disasm(c,(char*)&opc,(char*)&s2,(char*)&s3,-1)){
	          case 0: printk("ASM> 0x%x: %s\n",cpu[c].IP,opc); break;
	          case 1: printk("ASM> 0x%x: %s  %s\n",cpu[c].IP,opc,s2);break;
	          case 2: printk("ASM> 0x%x: %s  %s, %s\n",cpu[c].IP,opc,s2,s3);break;
	        }
		return 1;
	    } else return 0;
	}
	// if ret
	if (debug[c].traceflag & DEBUG_RET){
	    if (cpu[c].bytecode[cpu[c].IP*12]==CMD_RET){
		printk("=> DEBUG: %d vCPU is stopped.\n",c);
		cpu[c].state|=VCPU_STATE_STOPPED;
		debug[c].traceflag=DEBUG_DEBUG;
	        switch(disasm(c,(char*)&opc,(char*)&s2,(char*)&s3,-1)){
	          case 0: printk("ASM> 0x%x: %s\n",cpu[c].IP,opc); break;
	          case 1: printk("ASM> 0x%x: %s  %s\n",cpu[c].IP,opc,s2);break;
	          case 2: printk("ASM> 0x%x: %s  %s, %s\n",cpu[c].IP,opc,s2,s3);break;
	        }
		return 1;
	    } else return 0;
	}
	
	// Me thinks someone had to continue
	if (debug[c].traceflag & DEBUG_CONTINUE)
	  return 0;
	cpu[c].state|=VCPU_STATE_STOPPED;
	debug[c].traceflag=DEBUG_DEBUG;

	switch(disasm(c,(char*)&opc,(char*)&s2,(char*)&s3,-1)){
	  case 0: printk("ASM> 0x%x: %s\n",cpu[c].IP,opc); break;
	  case 1: printk("ASM> 0x%x: %s  %s\n",cpu[c].IP,opc,s2);break;
	  case 2: printk("ASM> 0x%x: %s  %s, %s\n",cpu[c].IP,opc,s2,s3);break;
	}
	return 1;
}
void add_breakpoint(int c, unsigned int IP)
{
	int i;
	if (c<0 || c>=MAX_VCPUS) {
	    printk("Bad parameter.\n");
	    return;
	}
	if (!(cpu[c].flags & VCPU_FLAG_USED)) {
	  printk("This VCPU is idle.\n");
	  return;
	}

	if (!cpu[c].flags & VCPU_FLAG_DEBUG){
	    printk("Sorry, this vCPU hasn't debug flag.\n");
	    return; 
	}
	if (IP>=cpu[c].bytecode_size){
	    printk("This address points beyond code.\n");
	    return;
	}
	// strange ...
	for (i=1;i<=debug[c].b_size;i++)
	  if(debug[c].breakpoints[i-1]==IP)
	      return;
	if (debug[c].b_size>=MAXBREAKPOINTS){
	    printk("Breakpoint table is full :(\n");
	    return;
	}
	debug[c].breakpoints[debug[c].b_size]=IP;
	debug[c].b_size++;
}
void del_breakpoint(int c, int which)
{
	// poor but it works ...
	int i;
	if (c<0 || c>=MAX_VCPUS) {
	    printk("Bad parameter.\n");
	    return;
	}
	if (!(cpu[c].flags & VCPU_FLAG_USED)) {
          printk("This VCPU is idle.\n");
	  return;
	}

	if (!cpu[c].flags & VCPU_FLAG_DEBUG){
	    printk("Sorry, this vCPU hasn't debug flag.\n");
	    return; 
	}
	if (which<0 || which>=debug[c].b_size){
	    printk("Bad parameter.\n");
	    return;
	}
	for(i=which+1;i<debug[c].b_size;i++)
	    debug[c].breakpoints[i-1]=debug[c].breakpoints[i];
	debug[c].b_size--;
}
void show_breakpoints(int c)
{
  
	int i;
	if (c<0 || c>=MAX_VCPUS) {
	    printk("Bad parameter.\n");
	    return;
        }
	if (!(cpu[c].flags & VCPU_FLAG_USED)) {
	  printk("This VCPU is idle.\n");
	  return;
	}

	printk("List of breakpoints on %d vCPU:\n",c);
	for(i=1;i<=debug[c].b_size;i++)
	  printk("+ %d# Breakpoint at 0x%x\n",i-1,debug[c].breakpoints[i-1]);
}
// Show registers of chosen cpu
void show_registers(int c)
{
	int i;
	
	if (c<0 || c>=MAX_VCPUS) {
	  printk("Bad parameter.\n");
	  return;
	}

	if (!(cpu[c].flags & VCPU_FLAG_USED)) {
          printk("This VCPU is idle.\n");
          return;
	}

	printk("Registers of %d [%s] :\n",c,cpu[c].name);
	printk("IP: 0x%x  %d\n",cpu[c].IP,cpu[c].IP);
	for (i=0;i<REGISTERS;i++) {
	  printk("u%d: 0x%x  %d      ",i,cpu[c].uregs[i],cpu[c].uregs[i]);
	  if(i%2==1)
	    printk("\n");
	}
	for (i=0;i<REGISTERS;i++) {
	  printk("s%d: 0x%x  %d      ",i,cpu[c].sregs[i],cpu[c].sregs[i]);
	  if(i%2==1)
	    printk("\n");
	}
	for (i=0;i<REGISTERS;i++) {
	  printk("f%d: 0x%x  %f      ",i,(int)cpu[c].fregs[i],cpu[c].fregs[i]);
	  if(i%2==1)
	    printk("\n");
	}
}
// unset stopped state -> in debug mode it will execute one instruction
void step(int c)
{
	if (c<0 || c>=MAX_VCPUS) {
	  printk("Bad parameter.\n");
	  return;
	}
	if (!(cpu[c].flags & VCPU_FLAG_USED)) {
	  printk("This VCPU is idle.\n");
	  return;
	}

	if (cpu[c].state & VCPU_STATE_STOPPED){
	  printk("Step.\n");
	  cpu[c].state&=~VCPU_STATE_STOPPED;
	  debug[c].traceflag|=DEBUG_STEP;
	} else printk("Sorry, this function requires debug mode\n");
}    
void continue_process(int c)
{
	if(c<0 || c>=MAX_VCPUS) {
	  printk("Bad parameter.\n");
	  return;
	}
	if (!(cpu[c].flags & VCPU_FLAG_USED)) {
	  printk("This VCPU is idle.\n");
	  return;
	}

	if (cpu[c].state & VCPU_STATE_STOPPED){
	  printk("Continue on %d vCPU.\n",c);
	  debug[c].traceflag|=DEBUG_CONTINUE;
	  debug[c].traceflag|=DEBUG_STEP; // because
	  cpu[c].state&=~VCPU_STATE_STOPPED;
	} else printk("Sorry, %c vCPU isn't stopped.\n",c);
}
void continue_process_syscall(int c)
{
	if(c<0 || c>=MAX_VCPUS) {
	  printk("Bad parameter.\n");
	  return;
	}
	if (!(cpu[c].flags & VCPU_FLAG_USED)) {
          printk("This VCPU is idle.\n");
	  return;
	}

	if (cpu[c].state & VCPU_STATE_STOPPED){
	  printk("Continue on %d vCPU to next syscall.\n",c);
	  debug[c].traceflag|=DEBUG_SYSCALL;
	  debug[c].traceflag|=DEBUG_STEP; // because
	  cpu[c].state&=~VCPU_STATE_STOPPED;
	} else printk("Sorry, %c vCPU isn't stopped.\n",c);
}
void continue_ret(int c)
{
	if(c<0 || c>=MAX_VCPUS){
	  printk("Bad parameter.\n");
	  return;
	}
	if (!(cpu[c].flags & VCPU_FLAG_USED)) {
          printk("This VCPU is idle.\n");
	  return;
	}

	if (cpu[c].state & VCPU_STATE_STOPPED){
	  printk("Continue on %d vCPU to next ret.\n",c);
	  debug[c].traceflag|=DEBUG_RET;
	  debug[c].traceflag|=DEBUG_STEP; // because
	  cpu[c].state&=~VCPU_STATE_STOPPED;
	} else printk("Sorry, %c vCPU isn't stopped.\n",c);
}
void show_stack(int c)
{
	int i;
	
	if(c<0 || c>=MAX_VCPUS){
	  printk("Bad parameter.\n");
	  return;
	}
	if (!(cpu[c].flags & VCPU_FLAG_USED)) {
          printk("This VCPU is idle.\n");
	  return;
	}
	if (!(cpu[c].state & VCPU_STATE_STOPPED)){
	  printk("Sorry, %c vCPU isn't stopped.\n",c);
	  return;
	}	
	
	printk("-> Stack trace on %d vCPU:\n",c);
	for(i=0;i<cpu[c].stack_ptr;i++)
	  if (i<cpu[c].dssiz) printk("# 0x%x %s\n",(*cpu[c].stack)[i],
	    ((*cpu[c].stack)[i]==cpu[c].first_except_ip)?"(first exception) ":"");
	printk("# Current IP: 0x%x\n",cpu[c].IP);
}
void show_memory(int c, unsigned int size, unsigned int addr, char type)
{
	int x,i;
	unsigned int s, a, *p1;
	unsigned char *p;
	float p2;
	
	if(c<0 || c>=MAX_VCPUS) {
	  printk("Bad parameter.\n");
	  return;
	}

	if (!(cpu[c].flags & VCPU_FLAG_USED)) {
	  printk("This VCPU is idle.\n");
	  return;
	}

	if (!cpu[c].flags & VCPU_FLAG_DEBUG){
	  printk("Sorry this process hasn't debug mode.\n");
	  return;
	}
	// most code ripped from kernel/memory.c :)
	// but lcamtuf fucked it up ;>
        // z33d too -- lcamtuf ;>
	x=addr/(2+MAX_ALLOC_MEMBLK);
	if (x>=cpu[c].dmsiz){
	    non_fatal(ERROR_OUTSIDE_MEM,"Attempt to access excessive memblk ID",c);
	    return;
	}
	  if ((*cpu[c].mem)[x].real_memory)
	    if (((*cpu[c].mem)[x].map_addr<=addr) &&
	    ((*cpu[c].mem)[x].map_addr+((*cpu[c].mem)[x].size)>addr+((size+3)/4)-1)){
	      if(!((*cpu[c].mem)[x].flags & MEM_FLAG_READ)){
	        printk("Attempt to read protected memory.\n");
	        return;
	      }
	      p=(unsigned char*)(*cpu[c].mem)[x].real_memory+4*addr-4*(*cpu[c].mem)[x].map_addr;
	      s=size;
	      a=addr;
	      if (type=='s'){
	      while (s>=16){
	        printk("0x%x: \"",a);
		for(i=1;i<=16;i++)
		  printk("%c",*p++);
		printk("\"\n");
	        s-=16;
	        a+=16;
	      }
	      printk("0x%x: \"",a);
	      for (i=s;i>0;i--)
	        printk("%c",*p++);
	      printk("\"\n");
	      return;
	      } else // end of type 's'
	      if (type=='b'){
	      while (s>=8){
	        printk("0x%x: ",a);
		for(i=1;i<=8;i++)
		  printk("0x%.2x ",(unsigned int)*p++);
		printk("\n");
	        s-=8;
	        a+=8;
	      }
	      printk("0x%x: ",a);
	      for (i=s;i>0;i--)
	        printk("0x%.2x ",(unsigned int)*p++);
	      printk("\n");
	      return;
	      } else // end of type 'b'
	      if (type=='x'){
	      p1=(unsigned int*)p;
	      // p1++;
	      while (s>=16){
	        printk("0x%x: ",a);
		for(i=1;i<=4;i++)
		  printk("0x%.8x ",*p1++);
		printk("\n");
	        s-=16;
	        a+=16;
	      }
	      printk("0x%x: ",a);
	      for (i=s;i>0;i-=4)
	        printk("0x%.8x ",*p1++);
	      printk("\n");
	      return;
	      } else // end of type 'x'
	      if (type=='f'){
	      p1=(unsigned int*)p;
	      // p1++;
	      while (s>=16){
	        printk("0x%x: ",a);
		for(i=1;i<=4;i++){
		  memcpy(&p2,p1,4);
		  printk("%.8f ",p2);
		  p1++;
		}
		printk("\n");
	        s-=16;
	        a+=16;
	      }
	      printk("0x%x: ",a);
	      for (i=s;i>0;i-=4){
	        memcpy(&p2,p1,4);
	        printk("%.8f ",p2);
	        p1++;
	      }
	      printk("\n");
	      return; // end of type 'f'
	      }
	    }
	printk("Attempt to access non-allocated memory.\n");
} 
// ripped from kernel/bcode.c ? ;)
// 3 chars arguments because max is 3 in our assembler
// it'll return: str->[opcode], str2->[1st arg], str3->[2nd arg]
// and number of returned strings
int disasm(int c, char *str, char *str2, char *str3,int otheraddr)
{
        unsigned int reality;
        int* MISIO;
        int a1, a2;
        int t1, t2;
        int whatever;
        float *tmp;
	int i,l;

	if (c<0 || c>=MAX_VCPUS) {
          sprintf(str,"Incorrect VCPU.");
          return 0;
	}

        if (otheraddr<0) otheraddr=cpu[c].IP;

        if (otheraddr<0 || otheraddr>=cpu[c].bytecode_size) {
	  sprintf(str,"Attempt to access non existing address.");
          return 0;
	}
    
	reality=otheraddr*12;
	t1=cpu[c].bytecode[reality+1];
	t2=cpu[c].bytecode[reality+2];
	// space
	MISIO=(int*)&cpu[c].bytecode[reality+4];
	a1=*MISIO;
	a2=*(MISIO+1);
	whatever=cpu[c].bytecode[reality];
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
          return 0;
	// put symbol
	if (whatever==CMD_SYSCALL){
	    for(l=0;l<SCNUM;l++){
	      if(sys[l].num==a1){
	        sprintf(str2,"$%s",sys[l].name);
		return 1;
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
          return 1;
	if (((a2<0)||(a2>=REGISTERS)) && t2!=TYPE_IMMEDIATE && t2!=TYPE_IMMPTR){
    	    sprintf(str,"Bad instruction.");
	    return 0;
        }
	tmp=(float*)&a2;
	switch(t2){
          case TYPE_IMMEDIATE: 
	  if (t1==TYPE_FREG)
	    sprintf(str3,"%f",(float)*tmp);
	  else sprintf(str3,"0x%x",a2);break;
          case TYPE_IMMPTR: sprintf(str3,"*0x%x",a2);break;
          if ((a2<0)||(a2>=REGISTERS)){
    	    sprintf(str,"Bad instruction.");
	    *str2='\0'; *str3='\0';
	    return 0;
          }
          case TYPE_UREG: sprintf(str3,"u%d",a2);break;
          case TYPE_SREG: sprintf(str3,"s%d",a2);break;
          case TYPE_FREG: sprintf(str3,"f%d",a2);break;
          case TYPE_UPTR: sprintf(str3,"*u%d",a2);break;
          default: *str2='\0'; *str3='\0';sprintf(str,"Bad instruction.");return 0;
	}
	return 2;
}


// lcamtuf

void dump_instructions(int c,int IP,int cnt)
{
  char j[100],d[100],t[100];
  int x,cnt2;

  if (c<0 || c>=MAX_VCPUS) {
      printk("Bad VCPU number.\n");
      return;
  }


  if (!(cpu[c].flags & VCPU_FLAG_USED)) {
      printk("This VCPU is idle.\n");
      return;
  }


  for (x=IP;x<IP+cnt;x++) { 
    cnt2=disasm(c,j,d,t,x);
    printk("[%d] 0x%x: %s",c,x,j);
    if (cnt2>0) printk(" %s",d);
    if (cnt2>1) printk(",%s",t);
    printk("\n");
  }
}
  


