/*

   Argante virtual OS
   ------------------

   Functions for binary code interpreter.

   Status: optimalization nearly done!

   Author:     Mariusz Woloszyn (Kil3r) <kil3r@dione.ids.pl>
   Maintainer: Mariusz Woloszyn (Kil3r) <kil3r@dione.ids.pl>

*/

#include <sys/time.h>
#include "config.h"
#include "task.h"
#include "bcode.h"
#include "console.h"
#include "memory.h"
#include "bcode.h"
#include "module.h"
#include "debugger.h"
#include "evaluate.h"
#include "cmd.h"

/* bytecode_p = &curr_cpu_p->bytecode[curr_cpu_p->IP*12]. */
extern char *bytecode_p;
#define CHECK_EXCEPT	if (got_nonfatal_round) return;

#define EVAL_A2_T2	t2=bytecode_p[2]; \
			a2=*((int *) (&bytecode_p[8]))

#define EVAL_T2		t2=bytecode_p[2]

#define	EVAL_A1		a1=*((int *) (&bytecode_p[4]))

#define	A1		*((int *) (&bytecode_p[4]))
	
#define	A2		*((int *) (&bytecode_p[8]))

/*
#define EVAL_A2_T2	t2=curr_cpu_p->bytecode[curr_cpu_p->IP*12+2]; \
			a2=*(((int*)&curr_cpu_p->bytecode[curr_cpu_p->IP*12+4])+1)

#define EVAL_T2		t2=curr_cpu_p->bytecode[curr_cpu_p->IP*12+2]

#define	EVAL_A1		a1=*((int*)&curr_cpu_p->bytecode[curr_cpu_p->IP*12+4])

#define	A1		*((int*)&curr_cpu_p->bytecode[curr_cpu_p->IP*12+4])

#define A2		*(((int*)&curr_cpu_p->bytecode[curr_cpu_p->IP*12+4])+1)
*/
extern int change;
extern int got_nonfatal_round;

extern int curr_cpu;
extern struct vcpu_struct *curr_cpu_p;

void cmd_nop () {
	return;
}

// cmd_mov_ureg

void cmd_mov_ureg_immediate () {

	UREG(A1)=A2;
}

void cmd_mov_ureg_ureg () {

	UREG(A1)=UREG(A2);
}

void cmd_mov_ureg_sreg () {

	UREG(A1)=SREG(A2);
}

void cmd_mov_ureg_freg () {

	UREG(A1)=FFREG(A2);
}

void cmd_mov_ureg_immptr () {
  int a2;
  	
	IMMPTRVAL(a2,A2);
	UREG(A1)=a2;
}

void cmd_mov_ureg_uptr () {
  int a2;
	
	UPTRVAL(a2,A2);
	UREG(A1)=a2;
}


// cmd_mov_sreg

void cmd_mov_sreg_immediate () {

	SREG(A1)=A2;
}

void cmd_mov_sreg_ureg () {

	SREG(A1)=UREG(A2);
}

void cmd_mov_sreg_sreg () {

	SREG(A1)=SREG(A2);
}

void cmd_mov_sreg_freg () {

	SREG(A1)=FFREG(A2);
}

void cmd_mov_sreg_immptr () {
  int a2;
  	
	IMMPTRVAL(a2,A2);
	SREG(A1)=a2;
}

void cmd_mov_sreg_uptr () {
  int a2;
	
	UPTRVAL(a2,A2);
	SREG(A1)=a2;
}


// cmd_mov_immptr

void cmd_mov_immptr_immediate () {
	
	set_mem_value(curr_cpu,A1,A2);
}

void cmd_mov_immptr_ureg () {

	set_mem_value(curr_cpu,A1,UREG(A2));
}

void cmd_mov_immptr_sreg () {
	
	set_mem_value(curr_cpu,A1,SREG(A2));
}

void cmd_mov_immptr_freg () {

	set_mem_value(curr_cpu,A1,FREG(A2));
}

void cmd_mov_immptr_immptr () {
  int a2;

  	IMMPTRVAL(a2,A2);
	set_mem_value(curr_cpu,A1,a2);
}

void cmd_mov_immptr_uptr () {
  int a2;
  	
	UPTRVAL(a2,A2);
	set_mem_value(curr_cpu,A1,a2);
}


// cmd_mov_uptr

void cmd_mov_uptr_immediate () {
	
	set_mem_value(curr_cpu,UREG(A1),A2);
}

void cmd_mov_uptr_ureg () {

	set_mem_value(curr_cpu,UREG(A1),UREG(A2));
}

void cmd_mov_uptr_sreg () {

	set_mem_value(curr_cpu,UREG(A1),SREG(A2));
}

void cmd_mov_uptr_freg () {

	set_mem_value(curr_cpu,UREG(A1),FREG(A2));
}

void cmd_mov_uptr_immptr () {
  int a2;

  	IMMPTRVAL(a2,A2);
	set_mem_value(curr_cpu,UREG(A1),a2);
}

void cmd_mov_uptr_uptr () {
  int a2;
  	
	UPTRVAL(a2,A2);
	set_mem_value(curr_cpu,UREG(A1),a2);
}


// cmd_mov_freg

void cmd_mov_freg_immediate () {
	FREG(A1)=*((int*)&A2);
}

void cmd_mov_freg_ureg () {
	FFREG(A1)=UREG(A2);
}

void cmd_mov_freg_sreg () {
	FFREG(A1)=SREG(A2);
}

void cmd_mov_freg_freg () {
	FFREG(A1)=FFREG(A2);
}

void cmd_mov_freg_immptr () {
  int a2;

	IMMPTRVAL(a2,A2);
	FFREG(A1)=*((float *)&a2);
}

void cmd_mov_freg_uptr () {
  int a2;

	UPTRVAL(a2,A2);
	FFREG(A1)=*((float *)&a2);
}


// cmd_add_ureg

void cmd_add_ureg_immediate () {

	UREG(A1)+=A2;
}

void cmd_add_ureg_ureg () {

	UREG(A1)+=UREG(A2);
}

void cmd_add_ureg_sreg () {

	UREG(A1)+=SREG(A2);
}

void cmd_add_ureg_freg () {

	UREG(A1)+=FFREG(A2);
}

void cmd_add_ureg_immptr () {
  int a2;

  	IMMPTRVAL(a2,A2);
	UREG(A1)+=a2;
}

void cmd_add_ureg_uptr () {
  int a2;

  	UPTRVAL(a2,A2);
	UREG(A1)+=a2;
}


// cmd_add_sreg

void cmd_add_sreg_immediate () {

	SREG(A1)+=A2;
}

void cmd_add_sreg_ureg () {

	SREG(A1)+=UREG(A2);
}

void cmd_add_sreg_sreg () {

	SREG(A1)+=SREG(A2);
}

void cmd_add_sreg_freg () {

	SREG(A1)+=FFREG(A2);
}

void cmd_add_sreg_immptr () {
  int a2;

	IMMPTRVAL(a2,A2);
	SREG(A1)+=a2;
}

void cmd_add_sreg_uptr () {
  int a2;

	UPTRVAL(a2,A2);
	SREG(A1)+=a2;
}


// cmd_add_immptr

void cmd_add_immptr_immediate () {
  int a1;
	
	EVAL_A1;
	set_mem_value(curr_cpu,a1,A2+get_mem_value(curr_cpu,a1));
}

void cmd_add_immptr_ureg () {
  int a1;

  	EVAL_A1;
	set_mem_value(curr_cpu,a1,UREG(A2)+get_mem_value(curr_cpu,a1));
}

void cmd_add_immptr_sreg () {
  int a1;

  	EVAL_A1;
	set_mem_value(curr_cpu,a1,SREG(A2)+get_mem_value(curr_cpu,a1));
}

void cmd_add_immptr_freg () {
  int a1;
  float f;

  	EVAL_A1;
	*((int *)&f)=get_mem_value(curr_cpu,a1);
	f+=FFREG(A2);
	set_mem_value(curr_cpu,a1,*((int *)&f));
}

void cmd_add_immptr_immptr () {
  int a1, a2;

  	IMMPTRVAL(a2,A2);
	EVAL_A1;
	set_mem_value(curr_cpu,a1,a2+get_mem_value(curr_cpu,a1));
}

void cmd_add_immptr_uptr () {
  int a1, a2;

  	UPTRVAL(a2,A2);
	EVAL_A1;
	set_mem_value(curr_cpu,a1,a2+get_mem_value(curr_cpu,a1));
}


// cmd_add_uptr

void cmd_add_uptr_immediate () {
  int a1;

	UREGVAL(a1,A1);
	set_mem_value(curr_cpu,a1,A2+get_mem_value(curr_cpu,a1));
}

void cmd_add_uptr_ureg () {
  int a1;

  	UREGVAL(a1,A1);
	set_mem_value(curr_cpu,a1,UREG(A2)+get_mem_value(curr_cpu,a1));
}

void cmd_add_uptr_sreg () {
  int a1;

  	UREGVAL(a1,A1);
	set_mem_value(curr_cpu,a1,SREG(A2)+get_mem_value(curr_cpu,a1));
}

void cmd_add_uptr_freg () {
  int a1;
  float f;
  
  	UREGVAL(a1,A1);
	*((int *)&f)=get_mem_value(curr_cpu,a1);
	f=FFREG(A2)+f;
	set_mem_value(curr_cpu,a1,*((int *)&f));
}

void cmd_add_uptr_immptr () {
  int a1, a2;

  	UREGVAL(a1,A1);
	IMMPTRVAL(a2,A2);
	set_mem_value(curr_cpu,a1,a2+get_mem_value(curr_cpu,a1));
}

void cmd_add_uptr_uptr () {
  int a1, a2;

  	UREGVAL(a1,A1);
	UPTRVAL(a2,A2);
	set_mem_value(curr_cpu,a1,a2+get_mem_value(curr_cpu,a1));
}


// cmd_add_freg

void cmd_add_freg_immediate () {

	FFREG(A1)+=*((float*)&A2);
}

void cmd_add_freg_ureg () {

	FFREG(A1)+=UREG(A2);
}

void cmd_add_freg_sreg () {

	FFREG(A1)+=SREG(A2);
}


void cmd_add_freg_freg () {

	FFREG(A1)+=FFREG(A2);
}

void cmd_add_freg_immptr () {
  int a2;

	IMMPTRVAL(a2,A2);
	FFREG(A1)+=*((float *)&a2);
}

void cmd_add_freg_uptr () {
  int a2;

	UPTRVAL(a2,A2);
	FFREG(A1)+=*((float *)&a2);
}


// cmd_sub_ureg

void cmd_sub_ureg_immediate () {

	UREG(A1)-=A2;
}

void cmd_sub_ureg_ureg () {

	UREG(A1)-=UREG(A2);
}

void cmd_sub_ureg_sreg () {

	UREG(A1)-=SREG(A2);
}

void cmd_sub_ureg_freg () {

	UREG(A1)-=FFREG(A2);
}

void cmd_sub_ureg_immptr () {
  int a2;

  	IMMPTRVAL(a2,A2);
	UREG(A1)-=a2;
}

void cmd_sub_ureg_uptr () {
  int a2;
	
	UPTRVAL(a2,A2);
	UREG(A1)-=a2;
}


// cmd_sub_sreg

void cmd_sub_sreg_immediate () {

	SREG(A1)-=A2;
}

void cmd_sub_sreg_ureg () {

	SREG(A1)-=UREG(A2);
}

void cmd_sub_sreg_sreg () {

	SREG(A1)-=SREG(A2);
}

void cmd_sub_sreg_freg () {

	SREG(A1)-=FFREG(A2);
}

void cmd_sub_sreg_immptr () {
  int a2;

  	IMMPTRVAL(a2,A2);
	SREG(A1)-=a2;
}

void cmd_sub_sreg_uptr () {
  int a2;
	
	UPTRVAL(a2,A2);
	SREG(A1)-=a2;
}


// cmd_sub_immptr

void cmd_sub_immptr_immediate () {
  int a1;
	
	EVAL_A1;
	set_mem_value(curr_cpu,a1,get_mem_value(curr_cpu,a1)-A2);
}

void cmd_sub_immptr_ureg () {
  int a1;

  	EVAL_A1;
	set_mem_value(curr_cpu,a1,get_mem_value(curr_cpu,a1)-UREG(A2));
}

void cmd_sub_immptr_sreg () {
  int a1;

  	EVAL_A1;
	set_mem_value(curr_cpu,a1,get_mem_value(curr_cpu,a1)-SREG(A2));
}

void cmd_sub_immptr_freg () {
  int a1;
  float f;

  	EVAL_A1;
	*((int *)&f)=get_mem_value(curr_cpu,a1);
	f-=FFREG(A2);
	set_mem_value(curr_cpu,a1,*((int *)&f));
}

void cmd_sub_immptr_immptr () {
  int a1, a2;

  	IMMPTRVAL(a2,A2);
	EVAL_A1;
	set_mem_value(curr_cpu,a1,get_mem_value(curr_cpu,a1)-a2);
}

void cmd_sub_immptr_uptr () {
  int a1, a2;

  	UPTRVAL(a2,A2);
	EVAL_A1;
	set_mem_value(curr_cpu,a1,get_mem_value(curr_cpu,a1)-a2);
}


// cmd_sub_uptr

void cmd_sub_uptr_immediate () {
  int a1;

	UREGVAL(a1,A1);
	set_mem_value(curr_cpu,a1,get_mem_value(curr_cpu,a1)-A2);
}

void cmd_sub_uptr_ureg () {
  int a1;

	UREGVAL(a1,A1);
	set_mem_value(curr_cpu,a1,get_mem_value(curr_cpu,a1)-UREG(A2));
}

void cmd_sub_uptr_sreg () {
  int a1;

  	UREGVAL(a1,A1);
	set_mem_value(curr_cpu,a1,get_mem_value(curr_cpu,a1)-SREG(A2));
}

void cmd_sub_uptr_freg () {
  int a1;
  float f;

	UREGVAL(a1,A1);
	*((int *)&f)=get_mem_value(curr_cpu,a1);
	f-=FFREG(A2);
	set_mem_value(curr_cpu,a1,*((int *)&f));
}

void cmd_sub_uptr_immptr () {
  int a1,a2;

  	IMMPTRVAL(a2,A2);
	UREGVAL(a1,A1);
	set_mem_value(curr_cpu,a1,get_mem_value(curr_cpu,a1)-a2);
}

void cmd_sub_uptr_uptr () {
  int a1,a2;
  
  	UPTRVAL(a2,A2);
	UREGVAL(a1,A1);
	set_mem_value(curr_cpu,a1,get_mem_value(curr_cpu,a1)-a2);
}


// cmd_sub_freg

void cmd_sub_freg () {
  int work2;
  int a2;
  int t2;

	EVAL_A2_T2;
         if (t2 == TYPE_IMMEDIATE) { work2=a2; } else
         if (t2 == TYPE_UREG) { UREGVAL(work2,a2) } else
         if (t2 == TYPE_SREG) { SREGVAL(work2,a2) } else
         if (t2 == TYPE_FREG) { FREGVAL(work2,a2) } else
         if (t2 == TYPE_IMMPTR) { IMMPTRVAL(work2,a2) } else
         if (t2 == TYPE_UPTR)  { UPTRVAL(work2,a2) } else {
           non_fatal(ERROR_BAD_PARAM,"SUB: Bad parameter #2",curr_cpu);
           return;
         }

	if ((t2 == TYPE_FREG) || (t2==TYPE_IMMEDIATE) || (t2==TYPE_IMMPTR)) {
		FREG(A1)-=*((float *)&work2);
	} else {
		FREG(A1)-=work2;
	}
}


// cmd_mul

void cmd_mul_ureg () {
  int work2;
  int a2;
  int t2;

    EVAL_A2_T2;
         if (t2 == TYPE_IMMEDIATE) { work2=a2; } else
         if (t2 == TYPE_UREG) { UREGVAL(work2,a2) } else
         if (t2 == TYPE_SREG) { SREGVAL(work2,a2) } else
         if (t2 == TYPE_FREG) { FREGVAL(work2,a2) } else
         if (t2 == TYPE_IMMPTR) { IMMPTRVAL(work2,a2) } else
         if (t2 == TYPE_UPTR)  { UPTRVAL(work2,a2) } else {
           non_fatal(ERROR_BAD_PARAM,"MUL: Bad parameter #2",curr_cpu);
           return;
         }

         UREG(A1)*=work2;
}


void cmd_mul_sreg () {
  int work2;
  int a2;
  int t2;

	EVAL_A2_T2;
         if (t2 == TYPE_IMMEDIATE) { work2=a2; } else
         if (t2 == TYPE_UREG) { UREGVAL(work2,a2) } else
         if (t2 == TYPE_SREG) { SREGVAL(work2,a2) } else
         if (t2 == TYPE_FREG) { FREGVAL(work2,a2) } else
         if (t2 == TYPE_IMMPTR) { IMMPTRVAL(work2,a2) } else
         if (t2 == TYPE_UPTR)  { UPTRVAL(work2,a2) } else {
           non_fatal(ERROR_BAD_PARAM,"MUL: Bad parameter #2",curr_cpu);
           return;
         }

        SREG(A1)*=work2;
}


void cmd_mul_immptr () {
  int work2;
  int a1,a2;
  int t2;


	EVAL_A2_T2;
         if (t2 == TYPE_IMMEDIATE) { work2=a2; } else
         if (t2 == TYPE_UREG) { UREGVAL(work2,a2) } else
         if (t2 == TYPE_SREG) { SREGVAL(work2,a2) } else
         if (t2 == TYPE_FREG) { FREGVAL(work2,a2) } else
         if (t2 == TYPE_IMMPTR) { IMMPTRVAL(work2,a2) } else
         if (t2 == TYPE_UPTR)  { UPTRVAL(work2,a2) } else {
           non_fatal(ERROR_BAD_PARAM,"MUL: Bad parameter #2",curr_cpu);
	   return;
         }

	 EVAL_A1;
	 set_mem_value(curr_cpu,a1,work2*get_mem_value(curr_cpu,a1));
}


void cmd_mul_uptr () {
  int work2;
  int a1,a2;
  int t2;

	EVAL_A2_T2;
         if (t2 == TYPE_IMMEDIATE) { work2=a2; } else
         if (t2 == TYPE_UREG) { UREGVAL(work2,a2) } else
         if (t2 == TYPE_SREG) { SREGVAL(work2,a2) } else
         if (t2 == TYPE_FREG) { FREGVAL(work2,a2) } else
         if (t2 == TYPE_IMMPTR) { IMMPTRVAL(work2,a2) } else
         if (t2 == TYPE_UPTR)  { UPTRVAL(work2,a2) } else {
           non_fatal(ERROR_BAD_PARAM,"MUL: Bad parameter #2",curr_cpu);
           return;
         }

	 EVAL_A1;
	 set_mem_value(curr_cpu,UREG(a1),work2*get_mem_value(curr_cpu,UREG(a1)));
}


void cmd_mul_freg () {
  int work2;
  int a2;
  int t2;


	EVAL_A2_T2;
         if (t2 == TYPE_IMMEDIATE) { work2=a2; } else
         if (t2 == TYPE_UREG) { UREGVAL(work2,a2) } else
         if (t2 == TYPE_SREG) { SREGVAL(work2,a2) } else
         if (t2 == TYPE_FREG) { FREGVAL(work2,a2) } else
         if (t2 == TYPE_IMMPTR) { IMMPTRVAL(work2,a2) } else
         if (t2 == TYPE_UPTR)  { UPTRVAL(work2,a2) } else {
           non_fatal(ERROR_BAD_PARAM,"MUL: Bad parameter #2",curr_cpu);
           return;
         }

	if ((t2 == TYPE_FREG) || (t2==TYPE_IMMEDIATE) || (t2==TYPE_IMMPTR)) {
		FFREG(A1)*=*((float *)&work2);
	} else {
		FFREG(A1)*=(float)work2;
	}
}


// cmd_xor

void cmd_xor_ureg () {
  int work2;
  int a2;
  int t2;

    EVAL_A2_T2;
         if (t2 == TYPE_IMMEDIATE) { work2=a2; } else
         if (t2 == TYPE_UREG) { UREGVAL(work2,a2) } else
         if (t2 == TYPE_SREG) { SREGVAL(work2,a2) } else
         if (t2 == TYPE_IMMPTR) { IMMPTRVAL(work2,a2) } else
         if (t2 == TYPE_UPTR)  { UPTRVAL(work2,a2) } else {
           non_fatal(ERROR_BAD_PARAM,"XOR: Bad parameter #2",curr_cpu);
           return;
         }

	UREG(A1)^=work2;
}


void cmd_xor_sreg () {
  int work2;
  int a2;
  int t2;

	EVAL_A2_T2;
         if (t2 == TYPE_IMMEDIATE) { work2=a2; } else
         if (t2 == TYPE_UREG) { UREGVAL(work2,a2) } else
         if (t2 == TYPE_SREG) { SREGVAL(work2,a2) } else
         if (t2 == TYPE_IMMPTR) { IMMPTRVAL(work2,a2) } else
         if (t2 == TYPE_UPTR)  { UPTRVAL(work2,a2) } else {
           non_fatal(ERROR_BAD_PARAM,"XOR: Bad parameter #2",curr_cpu);
           return;
         }

	SREG(A1)^=work2; 
}


void cmd_xor_immptr () {
  int work2;
  int a1,a2;
  int t2;

	EVAL_A2_T2;
         if (t2 == TYPE_IMMEDIATE) { work2=a2; } else
         if (t2 == TYPE_UREG) { UREGVAL(work2,a2) } else
         if (t2 == TYPE_SREG) { SREGVAL(work2,a2) } else
         if (t2 == TYPE_IMMPTR) { IMMPTRVAL(work2,a2) } else
         if (t2 == TYPE_UPTR)  { UPTRVAL(work2,a2) } else {
           non_fatal(ERROR_BAD_PARAM,"XOR: Bad parameter #2",curr_cpu);
	   return;
         }

	 EVAL_A1;
	 set_mem_value(curr_cpu,a1,get_mem_value(curr_cpu,a1) ^ work2);
}


void cmd_xor_uptr () {
  int work2;
  int a1,a2;
  int t2;

	EVAL_A2_T2;
         if (t2 == TYPE_IMMEDIATE) { work2=a2; } else
         if (t2 == TYPE_UREG) { UREGVAL(work2,a2) } else
         if (t2 == TYPE_SREG) { SREGVAL(work2,a2) } else
         if (t2 == TYPE_IMMPTR) { IMMPTRVAL(work2,a2) } else
         if (t2 == TYPE_UPTR)  { UPTRVAL(work2,a2) } else {
           non_fatal(ERROR_BAD_PARAM,"XOR: Bad parameter #2",curr_cpu);
           return;
         }

	 EVAL_A1;
	 set_mem_value(curr_cpu,UREG(a1),get_mem_value(curr_cpu,UREG(a1)) ^ work2);
}


// cmd_or

void cmd_or_ureg () {
  int work2;
  int a2;
  int t2;

    EVAL_A2_T2;
         if (t2 == TYPE_IMMEDIATE) { work2=a2; } else
         if (t2 == TYPE_UREG) { UREGVAL(work2,a2) } else
         if (t2 == TYPE_SREG) { SREGVAL(work2,a2) } else
         if (t2 == TYPE_IMMPTR) { IMMPTRVAL(work2,a2) } else
         if (t2 == TYPE_UPTR)  { UPTRVAL(work2,a2) } else {
           non_fatal(ERROR_BAD_PARAM,"OR: Bad parameter #2",curr_cpu);
           return;
         }

	UREG(A1)|=work2;
}


void cmd_or_sreg () {
  int work2;
  int a2;
  int t2;

	EVAL_A2_T2;
         if (t2 == TYPE_IMMEDIATE) { work2=a2; } else
         if (t2 == TYPE_UREG) { UREGVAL(work2,a2) } else
         if (t2 == TYPE_SREG) { SREGVAL(work2,a2) } else
         if (t2 == TYPE_IMMPTR) { IMMPTRVAL(work2,a2) } else
         if (t2 == TYPE_UPTR)  { UPTRVAL(work2,a2) } else {
           non_fatal(ERROR_BAD_PARAM,"OR: Bad parameter #2",curr_cpu);
           return;
         }

	SREG(A1)|=work2;
}


void cmd_or_immptr () {
  int work2;
  int a1,a2;
  int t2;


	EVAL_A2_T2;
         if (t2 == TYPE_IMMEDIATE) { work2=a2; } else
         if (t2 == TYPE_UREG) { UREGVAL(work2,a2) } else
         if (t2 == TYPE_SREG) { SREGVAL(work2,a2) } else
         if (t2 == TYPE_IMMPTR) { IMMPTRVAL(work2,a2) } else
         if (t2 == TYPE_UPTR)  { UPTRVAL(work2,a2) } else {
           non_fatal(ERROR_BAD_PARAM,"OR: Bad parameter #2",curr_cpu);
	   return;
         }

	 EVAL_A1;
	 set_mem_value(curr_cpu,a1,get_mem_value(curr_cpu,a1) | work2);
}


void cmd_or_uptr () {
  int work2;
  int a1,a2;
  int t2;

	EVAL_A2_T2;
         if (t2 == TYPE_IMMEDIATE) { work2=a2; } else
         if (t2 == TYPE_UREG) { UREGVAL(work2,a2) } else
         if (t2 == TYPE_SREG) { SREGVAL(work2,a2) } else
         if (t2 == TYPE_IMMPTR) { IMMPTRVAL(work2,a2) } else
         if (t2 == TYPE_UPTR)  { UPTRVAL(work2,a2) } else {
           non_fatal(ERROR_BAD_PARAM,"OR: Bad parameter #2",curr_cpu);
           return;
         }

	 EVAL_A1;
	 set_mem_value(curr_cpu,UREG(a1),get_mem_value(curr_cpu,UREG(a1)) | work2);
}


// cmd_and

void cmd_and_ureg () {
  int work2;
  int a2;
  int t2;

	EVAL_A2_T2;
         if (t2 == TYPE_IMMEDIATE) { work2=a2; } else
         if (t2 == TYPE_UREG) { UREGVAL(work2,a2) } else
         if (t2 == TYPE_SREG) { SREGVAL(work2,a2) } else
         if (t2 == TYPE_IMMPTR) { IMMPTRVAL(work2,a2) } else
         if (t2 == TYPE_UPTR)  { UPTRVAL(work2,a2) } else {
           non_fatal(ERROR_BAD_PARAM,"AND: Bad parameter #2",curr_cpu);
           return;
         }

	UREG(A1)&=work2;
}


void cmd_and_sreg () {
  int work2;
  int a2;
  int t2;

	EVAL_A2_T2;
         if (t2 == TYPE_IMMEDIATE) { work2=a2; } else
         if (t2 == TYPE_UREG) { UREGVAL(work2,a2) } else
         if (t2 == TYPE_SREG) { SREGVAL(work2,a2) } else
         if (t2 == TYPE_IMMPTR) { IMMPTRVAL(work2,a2) } else
         if (t2 == TYPE_UPTR)  { UPTRVAL(work2,a2) } else {
           non_fatal(ERROR_BAD_PARAM,"AND: Bad parameter #2",curr_cpu);
           return;
         }

	SREG(A1)&=work2;
}


void cmd_and_immptr () {
  int work2;
  int a1,a2;
  int t2;


	EVAL_A2_T2;
         if (t2 == TYPE_IMMEDIATE) { work2=a2; } else
         if (t2 == TYPE_UREG) { UREGVAL(work2,a2) } else
         if (t2 == TYPE_SREG) { SREGVAL(work2,a2) } else
         if (t2 == TYPE_IMMPTR) { IMMPTRVAL(work2,a2) } else
         if (t2 == TYPE_UPTR)  { UPTRVAL(work2,a2) } else {
           non_fatal(ERROR_BAD_PARAM,"AND: Bad parameter #2",curr_cpu);
	   return;
         }

	 EVAL_A1;
	 set_mem_value(curr_cpu,a1,get_mem_value(curr_cpu,a1) & work2);
}


void cmd_and_uptr () {
  int work2;
  int a1,a2;
  int t2;

	EVAL_A2_T2;
         if (t2 == TYPE_IMMEDIATE) { work2=a2; } else
         if (t2 == TYPE_UREG) { UREGVAL(work2,a2) } else
         if (t2 == TYPE_SREG) { SREGVAL(work2,a2) } else
         if (t2 == TYPE_IMMPTR) { IMMPTRVAL(work2,a2) } else
         if (t2 == TYPE_UPTR)  { UPTRVAL(work2,a2) } else {
           non_fatal(ERROR_BAD_PARAM,"AND: Bad parameter #2",curr_cpu);
           return;
         }

	 EVAL_A1;
	 set_mem_value(curr_cpu,UREG(a1),get_mem_value(curr_cpu,UREG(a1)) & work2);
}


// cmd_div

void cmd_div_ureg () {
  int work2;
  int a2;
  int t2;

	EVAL_A2_T2;
         if (t2 == TYPE_IMMEDIATE) { work2=a2; } else
         if (t2 == TYPE_UREG) { UREGVAL(work2,a2) } else
         if (t2 == TYPE_SREG) { SREGVAL(work2,a2) } else
         if (t2 == TYPE_FREG) { FREGVAL(work2,a2) } else
         if (t2 == TYPE_IMMPTR) { IMMPTRVAL(work2,a2) } else
         if (t2 == TYPE_UPTR)  { UPTRVAL(work2,a2) } else {
           non_fatal(ERROR_BAD_PARAM,"DIV: Bad parameter #2",curr_cpu);
           return;
         }

	 if (!work2) {
		 non_fatal(ERROR_BAD_PARAM,"DIV: Division by zero",curr_cpu);
		 return;
	 }
	UREG(A1)/=work2;
}


void cmd_div_sreg () {
  int work2;
  int a2;
  int t2;

	EVAL_A2_T2;
         if (t2 == TYPE_IMMEDIATE) { work2=a2; } else
         if (t2 == TYPE_UREG) { UREGVAL(work2,a2) } else
         if (t2 == TYPE_SREG) { SREGVAL(work2,a2) } else
         if (t2 == TYPE_FREG) { FREGVAL(work2,a2) } else
         if (t2 == TYPE_IMMPTR) { IMMPTRVAL(work2,a2) } else
         if (t2 == TYPE_UPTR)  { UPTRVAL(work2,a2) } else {
           non_fatal(ERROR_BAD_PARAM,"DIV: Bad parameter #2",curr_cpu);
           return;
         }

	 if (!work2) {
		 non_fatal(ERROR_BAD_PARAM,"DIV: Division by zero",curr_cpu);
		 return;
	 }
	SREG(A1)/=work2;
}


void cmd_div_immptr () {
  int work2;
  int a1,a2;
  int t2;


	EVAL_A2_T2;
         if (t2 == TYPE_IMMEDIATE) { work2=a2; } else
         if (t2 == TYPE_UREG) { UREGVAL(work2,a2) } else
         if (t2 == TYPE_SREG) { SREGVAL(work2,a2) } else
         if (t2 == TYPE_FREG) { FREGVAL(work2,a2) } else
         if (t2 == TYPE_IMMPTR) { IMMPTRVAL(work2,a2) } else
         if (t2 == TYPE_UPTR)  { UPTRVAL(work2,a2) } else {
           non_fatal(ERROR_BAD_PARAM,"DIV: Bad parameter #2",curr_cpu);
	   return;
         }

	 if (!work2) {
		 non_fatal(ERROR_BAD_PARAM,"DIV: Division by zero",curr_cpu);
		 return;
	 }
	 EVAL_A1;
	 set_mem_value(curr_cpu,a1,get_mem_value(curr_cpu,a1)/work2);
}


void cmd_div_uptr () {
  int work2;
  int a1,a2;
  int t2;

	EVAL_A2_T2;
         if (t2 == TYPE_IMMEDIATE) { work2=a2; } else
         if (t2 == TYPE_UREG) { UREGVAL(work2,a2) } else
         if (t2 == TYPE_SREG) { SREGVAL(work2,a2) } else
         if (t2 == TYPE_FREG) { FREGVAL(work2,a2) } else
         if (t2 == TYPE_IMMPTR) { IMMPTRVAL(work2,a2) } else
         if (t2 == TYPE_UPTR)  { UPTRVAL(work2,a2) } else {
           non_fatal(ERROR_BAD_PARAM,"DIV: Bad parameter #2",curr_cpu);
           return;
         }

	 if (!work2) {
		 non_fatal(ERROR_BAD_PARAM,"DIV: Division by zero",curr_cpu);
		 return;
	 }
	 EVAL_A1;
	 set_mem_value(curr_cpu,UREG(a1),get_mem_value(curr_cpu,UREG(a1))/work2);
}


void cmd_div_freg () {
  int work2;
  int a2;
  int t2;

	EVAL_A2_T2;
         if (t2 == TYPE_IMMEDIATE) { work2=a2; } else
         if (t2 == TYPE_UREG) { UREGVAL(work2,a2) } else
         if (t2 == TYPE_SREG) { SREGVAL(work2,a2) } else
         if (t2 == TYPE_FREG) { FREGVAL(work2,a2) } else
         if (t2 == TYPE_IMMPTR) { IMMPTRVAL(work2,a2) } else
         if (t2 == TYPE_UPTR)  { UPTRVAL(work2,a2) } else {
           non_fatal(ERROR_BAD_PARAM,"DIV: Bad parameter #2",curr_cpu);
           return;
         }

	 if (!work2) {
		 non_fatal(ERROR_BAD_PARAM,"DIV: Division by zero",curr_cpu);
		 return;
	 }

	if ((t2 == TYPE_FREG) || (t2==TYPE_IMMEDIATE) || (t2==TYPE_IMMPTR)) {
		FFREG(A1)/=*((float *)&work2);
	} else {
		FFREG(A1)/=(float)work2;
	}
}


// cmd_mod

void cmd_mod_ureg () {
  int work2;
  int a2;
  int t2;

    EVAL_A2_T2;
         if (t2 == TYPE_IMMEDIATE) { work2=a2; } else
         if (t2 == TYPE_UREG) { UREGVAL(work2,a2) } else
         if (t2 == TYPE_SREG) { SREGVAL(work2,a2) } else
         if (t2 == TYPE_FREG) { FREGVAL(work2,a2) } else
         if (t2 == TYPE_IMMPTR) { IMMPTRVAL(work2,a2) } else
         if (t2 == TYPE_UPTR)  { UPTRVAL(work2,a2) } else {
           non_fatal(ERROR_BAD_PARAM,"MOD: Bad parameter #2",curr_cpu);
           return;
         }

	 if (!work2) {
		 non_fatal(ERROR_BAD_PARAM,"MOD: Division by zero",curr_cpu);
		 return;
	 }
	UREG(A1)%=work2;
}


void cmd_mod_sreg () {
  int work2;
  int a2;
  int t2;

	EVAL_A2_T2;
         if (t2 == TYPE_IMMEDIATE) { work2=a2; } else
         if (t2 == TYPE_UREG) { UREGVAL(work2,a2) } else
         if (t2 == TYPE_SREG) { SREGVAL(work2,a2) } else
         if (t2 == TYPE_FREG) { FREGVAL(work2,a2) } else
         if (t2 == TYPE_IMMPTR) { IMMPTRVAL(work2,a2) } else
         if (t2 == TYPE_UPTR)  { UPTRVAL(work2,a2) } else {
           non_fatal(ERROR_BAD_PARAM,"MOD: Bad parameter #2",curr_cpu);
           return;
         }

	 if (!work2) {
		 non_fatal(ERROR_BAD_PARAM,"MOD: Division by zero",curr_cpu);
		 return;
	 }
	 SREG(A1)%=work2;
}


void cmd_mod_immptr () {
  int work2;
  int a1,a2;
  int t2;


	EVAL_A2_T2;
         if (t2 == TYPE_IMMEDIATE) { work2=a2; } else
         if (t2 == TYPE_UREG) { UREGVAL(work2,a2) } else
         if (t2 == TYPE_SREG) { SREGVAL(work2,a2) } else
         if (t2 == TYPE_FREG) { FREGVAL(work2,a2) } else
         if (t2 == TYPE_IMMPTR) { IMMPTRVAL(work2,a2) } else
         if (t2 == TYPE_UPTR)  { UPTRVAL(work2,a2) } else {
           non_fatal(ERROR_BAD_PARAM,"MOD: Bad parameter #2",curr_cpu);
	   return;
         }

	 if (!work2) {
		 non_fatal(ERROR_BAD_PARAM,"MOD: Division by zero",curr_cpu);
		 return;
	 }
	 EVAL_A1;
	 set_mem_value(curr_cpu,a1,get_mem_value(curr_cpu,a1)%work2);
}


void cmd_mod_uptr () {
  int work2;
  int a1,a2;
  int t2;

	EVAL_A2_T2;
         if (t2 == TYPE_IMMEDIATE) { work2=a2; } else
         if (t2 == TYPE_UREG) { UREGVAL(work2,a2) } else
         if (t2 == TYPE_SREG) { SREGVAL(work2,a2) } else
         if (t2 == TYPE_FREG) { FREGVAL(work2,a2) } else
         if (t2 == TYPE_IMMPTR) { IMMPTRVAL(work2,a2) } else
         if (t2 == TYPE_UPTR)  { UPTRVAL(work2,a2) } else {
           non_fatal(ERROR_BAD_PARAM,"MOD: Bad parameter #2",curr_cpu);
           return;
         }

	 if (!work2) {
		 non_fatal(ERROR_BAD_PARAM,"MOD: Division by zero",curr_cpu);
		 return;
	 }
	 EVAL_A1;
	 set_mem_value(curr_cpu,UREG(a1),get_mem_value(curr_cpu,UREG(a1))%work2);
}

// cmd_jmp

void cmd_jmp_immediate () {

	curr_cpu_p->IP=A1;
	change=0;
}

void cmd_jmp_ureg () {

	curr_cpu_p->IP=UREG(A1);
	change=0;
}

void cmd_jmp_immptr () {
  int work;

	IMMPTRVAL(work,A1);
	curr_cpu_p->IP=work;
	change=0;
}

void cmd_jmp_uptr () {
  int work;

	UPTRVAL(work,A1);
	curr_cpu_p->IP=work; 
	change=0;
}

// cmd_call

void cmd_call_immediate () {

	push_on_stack();
	CHECK_EXCEPT;
	curr_cpu_p->IP=A1;
	change=0;
}

void cmd_call_ureg () {

	push_on_stack();
	CHECK_EXCEPT;
	curr_cpu_p->IP=UREG(A1);
	change=0;
}

void cmd_call_immptr () {
  int work;
	
	IMMPTRVAL(work,A1);
	push_on_stack();
	CHECK_EXCEPT;
	curr_cpu_p->IP=work;
	change=0;
}

void cmd_call_uptr () {
  int work;

	UPTRVAL(work,A1);
	push_on_stack();
	CHECK_EXCEPT;
	curr_cpu_p->IP=work; 
	change=0;
}

// cmd_loop

void cmd_loop_immediate () {

	if (curr_cpu_p->sregs[0]>0) {
		curr_cpu_p->sregs[0]--;
		curr_cpu_p->IP=A1;
                change=0;
	}
}

void cmd_loop_ureg () {

		if (curr_cpu_p->sregs[0]>0) {
			curr_cpu_p->sregs[0]--;
			curr_cpu_p->IP=UREG(A1);
                        change=0;
		} 
}

void cmd_loop_immptr () {
  int work;
	
	if (curr_cpu_p->sregs[0]>0) {
		IMMPTRVAL(work,A1);
		curr_cpu_p->sregs[0]--;
		curr_cpu_p->IP=work;
                change=0;
	}
}

void cmd_loop_uptr () {
  int work;

		if (curr_cpu_p->sregs[0]>0) {
			UPTRVAL(work,A1);
			curr_cpu_p->sregs[0]--;
			curr_cpu_p->IP=work;
                        change=0;
		}
}

// cmd_onfail

void cmd_onfail_immediate () {

	(*curr_cpu_p->ex_st)[curr_cpu_p->stack_ptr]=A1;
}

void cmd_onfail_ureg () {

		(*curr_cpu_p->ex_st)[curr_cpu_p->stack_ptr]=UREG(A1);
}

void cmd_onfail_immptr () {
  int work;
  	
	IMMPTRVAL(work,A1);
	(*curr_cpu_p->ex_st)[curr_cpu_p->stack_ptr]=work;

}

void cmd_onfail_uptr () {
  int work;
	
		UPTRVAL(work,A1);
		(*curr_cpu_p->ex_st)[curr_cpu_p->stack_ptr]=work;
}

// cmd_syscall

void cmd_syscall_immediate () {

	do_syscall(curr_cpu,A1);

}

void cmd_syscall_ureg () {

		do_syscall(curr_cpu,UREG(A1));
}

void cmd_syscall_immptr () {
  int work;
  	
	IMMPTRVAL(work,A1);
	do_syscall(curr_cpu,work);
}

void cmd_syscall_uptr () {
  int work;

		UPTRVAL(work,A1);
		do_syscall(curr_cpu,work);
}

// cmd_ret

void cmd_ret_immediate () {
  int a1,work;

  	EVAL_A1;

	if (!a1) a1=1;
	if (a1>MAX_STACK) {
		non_fatal(ERROR_BAD_PARAM,"RET: Bad parameter",curr_cpu);
		return;
	}
	// Lemme guess - it's buggy!
	curr_cpu_p->in_handler-=a1;
	if (!curr_cpu_p->in_handler) curr_cpu_p->uregs[0]=curr_cpu_p->u0_saved;
	if (curr_cpu_p->in_handler<0) curr_cpu_p->in_handler=0;

	for (work=0;work<a1;work++) {
		curr_cpu_p->IP=pop_from_stack()+1;
		change=0;

		if (curr_cpu_p->handling_failure && (curr_cpu_p->IP-1==curr_cpu_p->first_except_ip)) {
				curr_cpu_p->handling_failure=0;
		}
	}
}


void cmd_ret_ureg () {
  int a1,work;

	a1=UREG(A1);

	if (!a1) a1=1;
	if (a1>MAX_STACK) {
		non_fatal(ERROR_BAD_PARAM,"RET: Bad parameter",curr_cpu);
		return;
	}
	// Lemme guess - it's buggy!
	curr_cpu_p->in_handler-=a1;
	if (!curr_cpu_p->in_handler) curr_cpu_p->uregs[0]=curr_cpu_p->u0_saved;
	if (curr_cpu_p->in_handler<0) curr_cpu_p->in_handler=0;

	for (work=0;work<a1;work++) {
		curr_cpu_p->IP=pop_from_stack(curr_cpu)+1;
		change=0;

		if (curr_cpu_p->handling_failure && (curr_cpu_p->IP-1==curr_cpu_p->first_except_ip)) {
				curr_cpu_p->handling_failure=0;
		}
	}
}

void cmd_ret_immptr () {
  int a1,work;

	IMMPTRVAL(a1,A1);

	if (!a1) a1=1;
	if (a1>MAX_STACK) {
		non_fatal(ERROR_BAD_PARAM,"RET: Bad parameter",curr_cpu);
		return;
	}
	// Lemme guess - it's buggy!
	curr_cpu_p->in_handler-=a1;
	if (!curr_cpu_p->in_handler) curr_cpu_p->uregs[0]=curr_cpu_p->u0_saved;
	if (curr_cpu_p->in_handler<0) curr_cpu_p->in_handler=0;

	for (work=0;work<a1;work++) {
		curr_cpu_p->IP=pop_from_stack()+1;
		change=0;

		if (curr_cpu_p->handling_failure && (curr_cpu_p->IP-1==curr_cpu_p->first_except_ip)) {
				curr_cpu_p->handling_failure=0;
		}
	}
}

void cmd_ret_uptr () {
  int a1,work;

	UPTRVAL(a1,A1);

	if (!a1) a1=1;
	if (a1>MAX_STACK) {
		non_fatal(ERROR_BAD_PARAM,"RET: Bad parameter",curr_cpu);
		return;
	}
	// Lemme guess - it's buggy!

	curr_cpu_p->in_handler-=a1;
	if (!curr_cpu_p->in_handler) curr_cpu_p->uregs[0]=curr_cpu_p->u0_saved;
	if (curr_cpu_p->in_handler<0) curr_cpu_p->in_handler=0;

	for (work=0;work<a1;work++) {
		curr_cpu_p->IP=pop_from_stack()+1;
		change=0;

		if (curr_cpu_p->handling_failure && (curr_cpu_p->IP-1==curr_cpu_p->first_except_ip)) {
				curr_cpu_p->handling_failure=0;
		}
	}
}

// cmd_raise

void cmd_raise_immediate () {

	non_fatal(A1,"RAISE: User-raised exception",curr_cpu);
}

void cmd_raise_ureg () {

	non_fatal(UREG(A1),"RAISE: User-raised exception",curr_cpu);
}

void cmd_raise_immptr () {
  int work;

  	IMMPTRVAL(work,A1);
	non_fatal(work,"RAISE: User-raised exception",curr_cpu);
}

void cmd_raise_uptr () {
  int work;

  	UPTRVAL(work,A1);
	non_fatal(work,"RAISE: User-raised exception",curr_cpu);
}

// cmd_ifeq

void cmd_ifeq_immediate () {
  int work;
  int t2;

    EVAL_T2;
         if (t2 == TYPE_IMMEDIATE) { 
		 if ((A2)!=(A1)) change=2; 
		 return;
	 } else if (t2 == TYPE_UREG) {
		 if ((UREG(A2))!=(A1)) change=2;
		 return;
	 } else if (t2 == TYPE_SREG) {
		 if ((SREG(A2))!=(A1)) change=2;
		 return;
	 } else if (t2 == TYPE_FREG) {
		 if ((FREG(A2))!=(A1)) change=2;
	 } else if (t2 == TYPE_IMMPTR) {
		 IMMPTRVAL(work,A2);
		 if (work!=A1) change=2;
		 return;
	 } else if (t2 == TYPE_UPTR) {
		 UPTRVAL(work,A2)
		 if (work!=A1) change=2;
	 } else \
           non_fatal(ERROR_BAD_PARAM,"IFEQ: Bad parameter #2",curr_cpu);
           return;
}

void cmd_ifeq_ureg () {
  int work;
  int t2;

    EVAL_T2;
    	if (t2 == TYPE_IMMEDIATE) {
		if ((A2)!=(UREG(A1))) change=2;
		return;
	 } else if (t2 == TYPE_UREG) {
		 if ((UREG(A2))!=(UREG(A1))) change=2;
		 return;
	 } else if (t2 == TYPE_SREG) {
		 if ((SREG(A2))!=(UREG(A1))) change=2;
		 return;
	 } else if (t2 == TYPE_FREG) {
		 if ((FREG(A2))!=(UREG(A1))) change=2;
	 } else if (t2 == TYPE_IMMPTR) {
		 IMMPTRVAL(work,A2);
		 if (work!=UREG(A1)) change=2;
		 return;
	 } else if (t2 == TYPE_UPTR) {
		 UPTRVAL(work,A2)
		 if (work!=UREG(A1)) change=2;
	 } else \
           non_fatal(ERROR_BAD_PARAM,"IFEQ: Bad parameter #2",curr_cpu);
           return;
}

void cmd_ifeq_sreg () {
  int work;
  int t2;

    EVAL_T2;
    	if (t2 == TYPE_IMMEDIATE) {
		if ((A2)!=(SREG(A1))) change=2;
		return;
	 } else if (t2 == TYPE_UREG) {
		 if ((UREG(A2))!=(SREG(A1))) change=2;
		 return;
	 } else if (t2 == TYPE_SREG) {
		 if ((SREG(A2))!=(SREG(A1))) change=2;
		 return;
	 } else if (t2 == TYPE_FREG) {
		 if ((FREG(A2))!=(SREG(A1))) change=2;
	 } else if (t2 == TYPE_IMMPTR) {
		 IMMPTRVAL(work,A2);
		 if (work!=SREG(A1)) change=2;
		 return;
	 } else if (t2 == TYPE_UPTR) {
		 UPTRVAL(work,A2)
		 if (work!=SREG(A1)) change=2;
	 } else \
           non_fatal(ERROR_BAD_PARAM,"IFEQ: Bad parameter #2",curr_cpu);
           return;
}

void cmd_ifeq_freg () {
  int work;
  int t2;

    EVAL_T2;
    	if (t2 == TYPE_IMMEDIATE) {
		if ((A2)!=(FREG(A1))) change=2;
		return;
	 } else if (t2 == TYPE_UREG) {
		 if ((UREG(A2))!=(FREG(A1))) change=2;
		 return;
	 } else if (t2 == TYPE_SREG) {
		 if ((SREG(A2))!=(FREG(A1))) change=2;
		 return;
	 } else if (t2 == TYPE_FREG) {
		 if ((FREG(A2))!=(FREG(A1))) change=2;
	 } else if (t2 == TYPE_IMMPTR) {
		 IMMPTRVAL(work,A2);
		 if (work!=FREG(A1)) change=2;
		 return;
	 } else if (t2 == TYPE_UPTR) {
		 UPTRVAL(work,A2)
		 if (work!=FREG(A1)) change=2;
	 } else \
           non_fatal(ERROR_BAD_PARAM,"IFEQ: Bad parameter #2",curr_cpu);
           return;
}


void cmd_ifeq_immptr () {
  int work1,work2;
  int t2;

    EVAL_T2;
    	if (t2 == TYPE_IMMEDIATE) {
		IMMPTRVAL(work1,A1);
		if ((A2)!=work1) change=2;
		return;
	 } else if (t2 == TYPE_UREG) {
		 IMMPTRVAL(work1,A1);
		 if ((UREG(A2))!=work1) change=2;
		 return;
	 } else if (t2 == TYPE_SREG) {
		 IMMPTRVAL(work1,A1);
		 if ((SREG(A2))!=work1) change=2;
		 return;
	 } else if (t2 == TYPE_FREG) {
		 IMMPTRVAL(work1,A1);
		 if ((FREG(A2))!=work1) change=2;
	 } else if (t2 == TYPE_IMMPTR) {
		 IMMPTRVAL(work1,A1);
		 IMMPTRVAL(work2,A2);
		 if (work2!=work1) change=2;
		 return;
	 } else if (t2 == TYPE_UPTR) {
		 IMMPTRVAL(work1,A1);
		 UPTRVAL(work2,A2)
		 if (work2!=work1) change=2;
	 } else \
           non_fatal(ERROR_BAD_PARAM,"IFEQ: Bad parameter #2",curr_cpu);
           return;
}

void cmd_ifeq_uptr () {
  int work1,work2;
  int t2;

    EVAL_T2;
    	if (t2 == TYPE_IMMEDIATE) {
		UPTRVAL(work1,A1);
		if ((A2)!=work1) change=2;
		return;
	 } else if (t2 == TYPE_UREG) {
		 UPTRVAL(work1,A1);
		 if ((UREG(A2))!=work1) change=2;
		 return;
	 } else if (t2 == TYPE_SREG) {
		 UPTRVAL(work1,A1);
		 if ((SREG(A2))!=work1) change=2;
		 return;
	 } else if (t2 == TYPE_FREG) {
		 UPTRVAL(work1,A1);
		 if ((FREG(A2))!=work1) change=2;
	 } else if (t2 == TYPE_IMMPTR) {
		 UPTRVAL(work1,A1);
		 IMMPTRVAL(work2,A2);
		 if (work2!=work1) change=2;
		 return;
	 } else if (t2 == TYPE_UPTR) {
		 UPTRVAL(work1,A1);
		 UPTRVAL(work2,A2)
		 if (work2!=work1) change=2;
	 } else \
           non_fatal(ERROR_BAD_PARAM,"IFEQ: Bad parameter #2",curr_cpu);
           return;
}

// cmd_ifneq

void cmd_ifneq_immediate () {
  int work;
  int t2;

    EVAL_T2;
         if (t2 == TYPE_IMMEDIATE) { 
		 if ((A2)==(A1)) change=2; 
		 return;
	 } else if (t2 == TYPE_UREG) {
		 if ((UREG(A2))==(A1)) change=2;
		 return;
	 } else if (t2 == TYPE_SREG) {
		 if ((SREG(A2))==(A1)) change=2;
		 return;
	 } else if (t2 == TYPE_FREG) {
		 if ((FREG(A2))==(A1)) change=2;
	 } else if (t2 == TYPE_IMMPTR) {
		 IMMPTRVAL(work,A2);
		 if (work==A1) change=2;
		 return;
	 } else if (t2 == TYPE_UPTR) {
		 UPTRVAL(work,A2)
		 if (work==A1) change=2;
	 } else \
           non_fatal(ERROR_BAD_PARAM,"IFNEQ: Bad parameter #2",curr_cpu);
           return;
}

void cmd_ifneq_ureg () {
  int work;
  int t2;

    EVAL_T2;
    	if (t2 == TYPE_IMMEDIATE) {
		if ((A2)==(UREG(A1))) change=2;
		return;
	 } else if (t2 == TYPE_UREG) {
		 if ((UREG(A2))==(UREG(A1))) change=2;
		 return;
	 } else if (t2 == TYPE_SREG) {
		 if ((SREG(A2))==(UREG(A1))) change=2;
		 return;
	 } else if (t2 == TYPE_FREG) {
		 if ((FREG(A2))==(UREG(A1))) change=2;
	 } else if (t2 == TYPE_IMMPTR) {
		 IMMPTRVAL(work,A2);
		 if (work==UREG(A1)) change=2;
		 return;
	 } else if (t2 == TYPE_UPTR) {
		 UPTRVAL(work,A2)
		 if (work==UREG(A1)) change=2;
	 } else \
           non_fatal(ERROR_BAD_PARAM,"IFNEQ: Bad parameter #2",curr_cpu);
           return;
}

void cmd_ifneq_sreg () {
  int work;
  int t2;

    EVAL_T2;
    	if (t2 == TYPE_IMMEDIATE) {
		if ((A2)==(SREG(A1))) change=2;
		return;
	 } else if (t2 == TYPE_UREG) {
		 if ((UREG(A2))==(SREG(A1))) change=2;
		 return;
	 } else if (t2 == TYPE_SREG) {
		 if ((SREG(A2))==(SREG(A1))) change=2;
		 return;
	 } else if (t2 == TYPE_FREG) {
		 if ((FREG(A2))==(SREG(A1))) change=2;
	 } else if (t2 == TYPE_IMMPTR) {
		 IMMPTRVAL(work,A2);
		 if (work==SREG(A1)) change=2;
		 return;
	 } else if (t2 == TYPE_UPTR) {
		 UPTRVAL(work,A2)
		 if (work==SREG(A1)) change=2;
	 } else \
           non_fatal(ERROR_BAD_PARAM,"IFNEQ: Bad parameter #2",curr_cpu);
           return;
}

void cmd_ifneq_freg () {
  int work;
  int t2;

    EVAL_T2;
    	if (t2 == TYPE_IMMEDIATE) {
		if ((A2)==(FREG(A1))) change=2;
		return;
	 } else if (t2 == TYPE_UREG) {
		 if ((UREG(A2))==(FREG(A1))) change=2;
		 return;
	 } else if (t2 == TYPE_SREG) {
		 if ((SREG(A2))==(FREG(A1))) change=2;
		 return;
	 } else if (t2 == TYPE_FREG) {
		 if ((FREG(A2))==(FREG(A1))) change=2;
	 } else if (t2 == TYPE_IMMPTR) {
		 IMMPTRVAL(work,A2);
		 if (work==FREG(A1)) change=2;
		 return;
	 } else if (t2 == TYPE_UPTR) {
		 UPTRVAL(work,A2)
		 if (work==FREG(A1)) change=2;
	 } else \
           non_fatal(ERROR_BAD_PARAM,"IFNEQ: Bad parameter #2",curr_cpu);
           return;
}


void cmd_ifneq_immptr () {
  int work1,work2;
  int t2;

    EVAL_T2;
    	if (t2 == TYPE_IMMEDIATE) {
		IMMPTRVAL(work1,A1);
		if ((A2)==work1) change=2;
		return;
	 } else if (t2 == TYPE_UREG) {
		 IMMPTRVAL(work1,A1);
		 if ((UREG(A2))==work1) change=2;
		 return;
	 } else if (t2 == TYPE_SREG) {
		 IMMPTRVAL(work1,A1);
		 if ((SREG(A2))==work1) change=2;
		 return;
	 } else if (t2 == TYPE_FREG) {
		 IMMPTRVAL(work1,A1);
		 if ((FREG(A2))==work1) change=2;
	 } else if (t2 == TYPE_IMMPTR) {
		 IMMPTRVAL(work1,A1);
		 IMMPTRVAL(work2,A2);
		 if (work2==work1) change=2;
		 return;
	 } else if (t2 == TYPE_UPTR) {
		 IMMPTRVAL(work1,A1);
		 UPTRVAL(work2,A2)
		 if (work2==work1) change=2;
	 } else \
           non_fatal(ERROR_BAD_PARAM,"IFNEQ: Bad parameter #2",curr_cpu);
           return;
}

void cmd_ifneq_uptr () {
  int work1,work2;
  int t2;

    EVAL_T2;
    	if (t2 == TYPE_IMMEDIATE) {
		UPTRVAL(work1,A1);
		if ((A2)==work1) change=2;
		return;
	 } else if (t2 == TYPE_UREG) {
		 UPTRVAL(work1,A1);
		 if ((UREG(A2))==work1) change=2;
		 return;
	 } else if (t2 == TYPE_SREG) {
		 UPTRVAL(work1,A1);
		 if ((SREG(A2))==work1) change=2;
		 return;
	 } else if (t2 == TYPE_FREG) {
		 UPTRVAL(work1,A1);
		 if ((FREG(A2))==work1) change=2;
	 } else if (t2 == TYPE_IMMPTR) {
		 UPTRVAL(work1,A1);
		 IMMPTRVAL(work2,A2);
		 if (work2==work1) change=2;
		 return;
	 } else if (t2 == TYPE_UPTR) {
		 UPTRVAL(work1,A1);
		 UPTRVAL(work2,A2)
		 if (work2==work1) change=2;
	 } else \
           non_fatal(ERROR_BAD_PARAM,"IFNEQ: Bad parameter #2",curr_cpu);
           return;
}

// cmd_ifabo

void cmd_ifabo_immediate () {
  int work;
  int t2;

    EVAL_T2;
         if (t2 == TYPE_IMMEDIATE) { 
		 if ((A2)>=(A1)) change=2; 
		 return;
	 } else if (t2 == TYPE_UREG) {
		 if ((UREG(A2))>=(A1)) change=2;
		 return;
	 } else if (t2 == TYPE_SREG) {
		 if ((SREG(A2))>=(A1)) change=2;
		 return;
	 } else if (t2 == TYPE_FREG) {
		 if ((FREG(A2))>=(A1)) change=2;
	 } else if (t2 == TYPE_IMMPTR) {
		 IMMPTRVAL(work,A2);
		 if (work>=A1) change=2;
		 return;
	 } else if (t2 == TYPE_UPTR) {
		 UPTRVAL(work,A2)
		 if (work>=A1) change=2;
	 } else \
           non_fatal(ERROR_BAD_PARAM,"IFABO: Bad parameter #2",curr_cpu);
           return;
}

void cmd_ifabo_ureg () {
  int work;
  int t2;

    EVAL_T2;
    	if (t2 == TYPE_IMMEDIATE) {
		if ((A2)>=(UREG(A1))) change=2;
		return;
	 } else if (t2 == TYPE_UREG) {
		 if ((UREG(A2))>=(UREG(A1))) change=2;
		 return;
	 } else if (t2 == TYPE_SREG) {
		 if ((SREG(A2))>=(UREG(A1))) change=2;
		 return;
	 } else if (t2 == TYPE_FREG) {
		 if ((FREG(A2))>=(UREG(A1))) change=2;
	 } else if (t2 == TYPE_IMMPTR) {
		 IMMPTRVAL(work,A2);
		 if (work>=UREG(A1)) change=2;
		 return;
	 } else if (t2 == TYPE_UPTR) {
		 UPTRVAL(work,A2)
		 if (work>=UREG(A1)) change=2;
	 } else \
           non_fatal(ERROR_BAD_PARAM,"IFABO: Bad parameter #2",curr_cpu);
           return;
}

void cmd_ifabo_sreg () {
  int work;
  int t2;

    EVAL_T2;
    	if (t2 == TYPE_IMMEDIATE) {
		if ((A2)>=(SREG(A1))) change=2;
		return;
	 } else if (t2 == TYPE_UREG) {
		 if ((UREG(A2))>=(SREG(A1))) change=2;
		 return;
	 } else if (t2 == TYPE_SREG) {
		 if ((SREG(A2))>=(SREG(A1))) change=2;
		 return;
	 } else if (t2 == TYPE_FREG) {
		 if ((FREG(A2))>=(SREG(A1))) change=2;
	 } else if (t2 == TYPE_IMMPTR) {
		 IMMPTRVAL(work,A2);
		 if (work>=SREG(A1)) change=2;
		 return;
	 } else if (t2 == TYPE_UPTR) {
		 UPTRVAL(work,A2)
		 if (work>=SREG(A1)) change=2;
	 } else \
           non_fatal(ERROR_BAD_PARAM,"IFABO: Bad parameter #2",curr_cpu);
           return;
}

void cmd_ifabo_freg () {
  int work;
  int t2;

    EVAL_T2;
    	if (t2 == TYPE_IMMEDIATE) {
		if ((A2)>=(FREG(A1))) change=2;
		return;
	 } else if (t2 == TYPE_UREG) {
		 if ((UREG(A2))>=(FREG(A1))) change=2;
		 return;
	 } else if (t2 == TYPE_SREG) {
		 if ((SREG(A2))>=(FREG(A1))) change=2;
		 return;
	 } else if (t2 == TYPE_FREG) {
		 if ((FREG(A2))>=(FREG(A1))) change=2;
	 } else if (t2 == TYPE_IMMPTR) {
		 IMMPTRVAL(work,A2);
		 if (work>=FREG(A1)) change=2;
		 return;
	 } else if (t2 == TYPE_UPTR) {
		 UPTRVAL(work,A2)
		 if (work>=FREG(A1)) change=2;
	 } else \
           non_fatal(ERROR_BAD_PARAM,"IFABO: Bad parameter #2",curr_cpu);
           return;
}


void cmd_ifabo_immptr () {
  int work1,work2;
  int t2;

    EVAL_T2;
    	if (t2 == TYPE_IMMEDIATE) {
		IMMPTRVAL(work1,A1);
		if ((A2)>=work1) change=2;
		return;
	 } else if (t2 == TYPE_UREG) {
		 IMMPTRVAL(work1,A1);
		 if ((UREG(A2))>=work1) change=2;
		 return;
	 } else if (t2 == TYPE_SREG) {
		 IMMPTRVAL(work1,A1);
		 if ((SREG(A2))>=work1) change=2;
		 return;
	 } else if (t2 == TYPE_FREG) {
		 IMMPTRVAL(work1,A1);
		 if ((FREG(A2))>=work1) change=2;
	 } else if (t2 == TYPE_IMMPTR) {
		 IMMPTRVAL(work1,A1);
		 IMMPTRVAL(work2,A2);
		 if (work2>=work1) change=2;
		 return;
	 } else if (t2 == TYPE_UPTR) {
		 IMMPTRVAL(work1,A1);
		 UPTRVAL(work2,A2)
		 if (work2>=work1) change=2;
	 } else \
           non_fatal(ERROR_BAD_PARAM,"IFABO: Bad parameter #2",curr_cpu);
           return;
}

void cmd_ifabo_uptr () {
  int work1,work2;
  int t2;

    EVAL_T2;
    	if (t2 == TYPE_IMMEDIATE) {
		UPTRVAL(work1,A1);
		if ((A2)>=work1) change=2;
		return;
	 } else if (t2 == TYPE_UREG) {
		 UPTRVAL(work1,A1);
		 if ((UREG(A2))>=work1) change=2;
		 return;
	 } else if (t2 == TYPE_SREG) {
		 UPTRVAL(work1,A1);
		 if ((SREG(A2))>=work1) change=2;
		 return;
	 } else if (t2 == TYPE_FREG) {
		 UPTRVAL(work1,A1);
		 if ((FREG(A2))>=work1) change=2;
	 } else if (t2 == TYPE_IMMPTR) {
		 UPTRVAL(work1,A1);
		 IMMPTRVAL(work2,A2);
		 if (work2>=work1) change=2;
		 return;
	 } else if (t2 == TYPE_UPTR) {
		 UPTRVAL(work1,A1);
		 UPTRVAL(work2,A2)
		 if (work2>=work1) change=2;
	 } else \
           non_fatal(ERROR_BAD_PARAM,"IFABO: Bad parameter #2",curr_cpu);
           return;
}

// cmd_ifbel

void cmd_ifbel_immediate () {
  int work;
  int t2;

    EVAL_T2;
         if (t2 == TYPE_IMMEDIATE) { 
		 if ((A2)<=(A1)) change=2; 
		 return;
	 } else if (t2 == TYPE_UREG) {
		 if ((UREG(A2))<=(A1)) change=2;
		 return;
	 } else if (t2 == TYPE_SREG) {
		 if ((SREG(A2))<=(A1)) change=2;
		 return;
	 } else if (t2 == TYPE_FREG) {
		 if ((FREG(A2))<=(A1)) change=2;
	 } else if (t2 == TYPE_IMMPTR) {
		 IMMPTRVAL(work,A2);
		 if (work<=A1) change=2;
		 return;
	 } else if (t2 == TYPE_UPTR) {
		 UPTRVAL(work,A2)
		 if (work<=A1) change=2;
	 } else \
           non_fatal(ERROR_BAD_PARAM,"IFBEL: Bad parameter #2",curr_cpu);
           return;
}

void cmd_ifbel_ureg () {
  int work;
  int t2;

    EVAL_T2;
    	if (t2 == TYPE_IMMEDIATE) {
		if ((A2)<=(UREG(A1))) change=2;
		return;
	 } else if (t2 == TYPE_UREG) {
		 if ((UREG(A2))<=(UREG(A1))) change=2;
		 return;
	 } else if (t2 == TYPE_SREG) {
		 if ((SREG(A2))<=(UREG(A1))) change=2;
		 return;
	 } else if (t2 == TYPE_FREG) {
		 if ((FREG(A2))<=(UREG(A1))) change=2;
	 } else if (t2 == TYPE_IMMPTR) {
		 IMMPTRVAL(work,A2);
		 if (work<=UREG(A1)) change=2;
		 return;
	 } else if (t2 == TYPE_UPTR) {
		 UPTRVAL(work,A2)
		 if (work<=UREG(A1)) change=2;
	 } else \
           non_fatal(ERROR_BAD_PARAM,"IFBEL: Bad parameter #2",curr_cpu);
           return;
}

void cmd_ifbel_sreg () {
  int work;
  int t2;

    EVAL_T2;
    	if (t2 == TYPE_IMMEDIATE) {
		if ((A2)<=(SREG(A1))) change=2;
		return;
	 } else if (t2 == TYPE_UREG) {
		 if ((UREG(A2))<=(SREG(A1))) change=2;
		 return;
	 } else if (t2 == TYPE_SREG) {
		 if ((SREG(A2))<=(SREG(A1))) change=2;
		 return;
	 } else if (t2 == TYPE_FREG) {
		 if ((FREG(A2))<=(SREG(A1))) change=2;
	 } else if (t2 == TYPE_IMMPTR) {
		 IMMPTRVAL(work,A2);
		 if (work<=SREG(A1)) change=2;
		 return;
	 } else if (t2 == TYPE_UPTR) {
		 UPTRVAL(work,A2)
		 if (work<=SREG(A1)) change=2;
	 } else \
           non_fatal(ERROR_BAD_PARAM,"IFBEL: Bad parameter #2",curr_cpu);
           return;
}

void cmd_ifbel_freg () {
  int work;
  int t2;

    EVAL_T2;
    	if (t2 == TYPE_IMMEDIATE) {
		if ((A2)<=(FREG(A1))) change=2;
		return;
	 } else if (t2 == TYPE_UREG) {
		 if ((UREG(A2))<=(FREG(A1))) change=2;
		 return;
	 } else if (t2 == TYPE_SREG) {
		 if ((SREG(A2))<=(FREG(A1))) change=2;
		 return;
	 } else if (t2 == TYPE_FREG) {
		 if ((FREG(A2))<=(FREG(A1))) change=2;
	 } else if (t2 == TYPE_IMMPTR) {
		 IMMPTRVAL(work,A2);
		 if (work<=FREG(A1)) change=2;
		 return;
	 } else if (t2 == TYPE_UPTR) {
		 UPTRVAL(work,A2)
		 if (work<=FREG(A1)) change=2;
	 } else \
           non_fatal(ERROR_BAD_PARAM,"IFBEL: Bad parameter #2",curr_cpu);
           return;
}


void cmd_ifbel_immptr () {
  int work1,work2;
  int t2;

    EVAL_T2;
    	if (t2 == TYPE_IMMEDIATE) {
		IMMPTRVAL(work1,A1);
		if ((A2)<=work1) change=2;
		return;
	 } else if (t2 == TYPE_UREG) {
		 IMMPTRVAL(work1,A1);
		 if ((UREG(A2))<=work1) change=2;
		 return;
	 } else if (t2 == TYPE_SREG) {
		 IMMPTRVAL(work1,A1);
		 if ((SREG(A2))<=work1) change=2;
		 return;
	 } else if (t2 == TYPE_FREG) {
		 IMMPTRVAL(work1,A1);
		 if ((FREG(A2))<=work1) change=2;
	 } else if (t2 == TYPE_IMMPTR) {
		 IMMPTRVAL(work1,A1);
		 IMMPTRVAL(work2,A2);
		 if (work2<=work1) change=2;
		 return;
	 } else if (t2 == TYPE_UPTR) {
		 IMMPTRVAL(work1,A1);
		 UPTRVAL(work2,A2)
		 if (work2<=work1) change=2;
	 } else \
           non_fatal(ERROR_BAD_PARAM,"IFBEL: Bad parameter #2",curr_cpu);
           return;
}

void cmd_ifbel_uptr () {
  int work1,work2;
  int t2;

    EVAL_T2;
    	if (t2 == TYPE_IMMEDIATE) {
		UPTRVAL(work1,A1);
		if ((A2)<=work1) change=2;
		return;
	 } else if (t2 == TYPE_UREG) {
		 UPTRVAL(work1,A1);
		 if ((UREG(A2))<=work1) change=2;
		 return;
	 } else if (t2 == TYPE_SREG) {
		 UPTRVAL(work1,A1);
		 if ((SREG(A2))<=work1) change=2;
		 return;
	 } else if (t2 == TYPE_FREG) {
		 UPTRVAL(work1,A1);
		 if ((FREG(A2))<=work1) change=2;
	 } else if (t2 == TYPE_IMMPTR) {
		 UPTRVAL(work1,A1);
		 IMMPTRVAL(work2,A2);
		 if (work2<=work1) change=2;
		 return;
	 } else if (t2 == TYPE_UPTR) {
		 UPTRVAL(work1,A1);
		 UPTRVAL(work2,A2)
		 if (work2<=work1) change=2;
	 } else \
           non_fatal(ERROR_BAD_PARAM,"IFBEL: Bad parameter #2",curr_cpu);
           return;
}

// cmd_halt

void cmd_halt () {

	printk("=> Task #%d commited suicide (HALT command).\n",curr_cpu);
	destroy_task_respawn(curr_cpu);
	killed_proc--; exit_proc++;
}

// cmd_nofail

void cmd_nofail () {

	(*curr_cpu_p->ex_st)[curr_cpu_p->stack_ptr]=0;
}

// cmd_not

void cmd_not_ureg () {

	UREG(A1)^=0xffffffff;
}

void cmd_not_sreg () {

	SREG(A1)^=0xffffffff;
}

void cmd_not_immptr () {
  int a1;

	EVAL_A1;
	set_mem_value(curr_cpu,a1,get_mem_value(curr_cpu,a1)^0xffffffff);
}

void cmd_not_uptr () {
  int a1;

  	EVAL_A1;
	set_mem_value(curr_cpu,UREG(a1),get_mem_value(curr_cpu,UREG(a1)));
}

// cmd_sleepfor

void cmd_sleepfor_immediate () {

	curr_cpu_p->state |= VCPU_STATE_SLEEPFOR;
	curr_cpu_p->cnt_down = A1;
}

void cmd_sleepfor_ureg () {

	curr_cpu_p->state |= VCPU_STATE_SLEEPFOR;
	curr_cpu_p->cnt_down = UREG(A1);
}

void cmd_sleepfor_sreg () {

	curr_cpu_p->state |= VCPU_STATE_SLEEPFOR;
	curr_cpu_p->cnt_down = SREG(A1);
}

void cmd_sleepfor_freg () {

	curr_cpu_p->state |= VCPU_STATE_SLEEPFOR;
	curr_cpu_p->cnt_down = FREG(A1);
}


void cmd_sleepfor_immptr () {
  int work;

  	IMMPTRVAL(work,A1);
	curr_cpu_p->state |= VCPU_STATE_SLEEPFOR;
	curr_cpu_p->cnt_down = work;
}

void cmd_sleepfor_uptr () {
  int work;

  	UPTRVAL(work,A1);
	curr_cpu_p->state |= VCPU_STATE_SLEEPFOR;
	curr_cpu_p->cnt_down = work;
}

// cmd_waittill

void cmd_waittill_immediate () {
  struct timezone tz; struct timeval tv;

	gettimeofday(&tv,&tz);
	curr_cpu_p->state |= VCPU_STATE_SLEEPUNTIL;
	curr_cpu_p->wake_on=tv.tv_sec*1000000+tv.tv_usec+A1;
}

void cmd_waittill_ureg () {
  struct timezone tz; struct timeval tv;

	gettimeofday(&tv,&tz);
	curr_cpu_p->state |= VCPU_STATE_SLEEPUNTIL;
	curr_cpu_p->wake_on=tv.tv_sec*1000000+tv.tv_usec+UREG(A1);
}

void cmd_waittill_sreg () {
  struct timezone tz; struct timeval tv;

	gettimeofday(&tv,&tz);
	curr_cpu_p->state |= VCPU_STATE_SLEEPUNTIL;
	curr_cpu_p->wake_on=tv.tv_sec*1000000+tv.tv_usec+SREG(A1);
}

void cmd_waittill_freg () {
  struct timezone tz; struct timeval tv;

	gettimeofday(&tv,&tz);
	curr_cpu_p->state |= VCPU_STATE_SLEEPUNTIL;
	curr_cpu_p->wake_on=tv.tv_sec*1000000+tv.tv_usec+FREG(A1);
}


void cmd_waittill_immptr () {
  struct timezone tz; struct timeval tv;
  int work;

  	IMMPTRVAL(work,A1);
	gettimeofday(&tv,&tz);
	curr_cpu_p->state |= VCPU_STATE_SLEEPUNTIL;
	curr_cpu_p->wake_on=tv.tv_sec*1000000+tv.tv_usec+work;
}

void cmd_waittill_uptr () {
  struct timezone tz; struct timeval tv;
  int work;

  	UPTRVAL(work,A1);
	gettimeofday(&tv,&tz);
	curr_cpu_p->state |= VCPU_STATE_SLEEPUNTIL;
	curr_cpu_p->wake_on=tv.tv_sec*1000000+tv.tv_usec+work;
}

// cmd_ldb

void cmd_ldb_ureg () {
  int t2;

	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) {
		UREG(A1)=(curr_cpu_p->bytecode[curr_cpu_p->IP*12+8+(SREG(0)&3)]);
		return;
	} else 
	if (t2 == TYPE_UREG) {
		UREG(A1)=*(((char*)&(UREG(A2)))+(SREG(0)&3));
		return;
	} else
	if (t2 == TYPE_SREG) {
		UREG(A1)=*(((char*)&(SREG(A2)))+(SREG(0)&3));
	} else 
	if (t2 == TYPE_IMMPTR) {
		int work;
		IMMPTRVAL(work, (A2+(SREG(0)>>2)));
		UREG(A1)=*(((char*)&work)+(SREG(0)&3));
	} else
	if (t2 == TYPE_UPTR) {
		int work;
		// BLACK MAGIC!!! :)))
		IMMPTRVAL(work, ((UREG(A2))+(SREG(0)>>2)));
		UREG(A1)=*(((char*)&work)+(SREG(0)&3));
	} else \
	non_fatal(ERROR_BAD_PARAM,"LDB: Bad parameter #2",curr_cpu);
}

void cmd_ldb_sreg () {
  int t2;

	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) {
		SREG(A1)=(curr_cpu_p->bytecode[curr_cpu_p->IP*12+8+(SREG(0)&3)]);
		return;
	} else 
	if (t2 == TYPE_UREG) {
		SREG(A1)=*(((char*)&(UREG(A2)))+(SREG(0)&3));
		return;
	} else
	if (t2 == TYPE_SREG) {
		SREG(A1)=*(((char*)&(SREG(A2)))+(SREG(0)&3));
	} else 
	if (t2 == TYPE_IMMPTR) {
		int work;
		IMMPTRVAL(work, (A2+(SREG(0)>>2)));
		SREG(A1)=*(((char*)&work)+(SREG(0)&3));
	} else
	if (t2 == TYPE_UPTR) {
		int work;
		// BLACK MAGIC!!! :)))
		IMMPTRVAL(work, ((UREG(A2))+(SREG(0)>>2)));
		SREG(A1)=*(((char*)&work)+(SREG(0)&3));
	} else \
	non_fatal(ERROR_BAD_PARAM,"LDB: Bad parameter #2",curr_cpu);
}

void cmd_ldb_freg () {
  int t2;

	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) {
		FREG(A1)=(curr_cpu_p->bytecode[curr_cpu_p->IP*12+8+(SREG(0)&3)]);
		return;
	} else 
	if (t2 == TYPE_UREG) {
		FREG(A1)=*(((char*)&(UREG(A2)))+(SREG(0)&3));
		return;
	} else
	if (t2 == TYPE_SREG) {
		FREG(A1)=*(((char*)&(SREG(A2)))+(SREG(0)&3));
	} else 
	if (t2 == TYPE_IMMPTR) {
		int work;
		IMMPTRVAL(work, (A2+(SREG(0)>>2)));
		FREG(A1)=*(((char*)&work)+(SREG(0)&3));
	} else
	if (t2 == TYPE_UPTR) {
		int work;
		// BLACK MAGIC!!! :)))
		IMMPTRVAL(work, ((UREG(A2))+(SREG(0)>>2)));
		FREG(A1)=*(((char*)&work)+(SREG(0)&3));
	} else \
	non_fatal(ERROR_BAD_PARAM,"LDB: Bad parameter #2",curr_cpu);
}

void cmd_ldb_immptr () {
  int t2;

	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) {
		set_mem_value(curr_cpu,A1,(curr_cpu_p->bytecode[curr_cpu_p->IP*12+8+(SREG(0)&3)]));
		return;
	} else 
	if (t2 == TYPE_UREG) {
		set_mem_value(curr_cpu,A1,*(((char*)&(UREG(A2)))+(SREG(0)&3)));
		return;
	} else
	if (t2 == TYPE_SREG) {
		set_mem_value(curr_cpu,A1,*(((char*)&(SREG(A2)))+(SREG(0)&3)));
	} else 
	if (t2 == TYPE_IMMPTR) {
		int work;
		IMMPTRVAL(work, (A2+(SREG(0)>>2)));
		set_mem_value(curr_cpu,A1,*(((char*)&work)+(SREG(0)&3)));
	} else
	if (t2 == TYPE_UPTR) {
		int work;
		// BLACK MAGIC!!! :)))
		IMMPTRVAL(work, ((UREG(A2))+(SREG(0)>>2)));
		set_mem_value(curr_cpu,A1,*(((char*)&work)+(SREG(0)&3)));
	} else \
	non_fatal(ERROR_BAD_PARAM,"LDB: Bad parameter #2",curr_cpu);
}

void cmd_ldb_uptr () {
  int t2;

	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) {
		set_mem_value(curr_cpu,UREG(A1),(curr_cpu_p->bytecode[curr_cpu_p->IP*12+8+(SREG(0)&3)]));
		return;
	} else 
	if (t2 == TYPE_UREG) {
		set_mem_value(curr_cpu,UREG(A1),*(((char*)&(UREG(A2)))+(SREG(0)&3)));
		return;
	} else
	if (t2 == TYPE_SREG) {
		set_mem_value(curr_cpu,UREG(A1),*(((char*)&(SREG(A2)))+(SREG(0)&3)));
	} else 
	if (t2 == TYPE_IMMPTR) {
		int work;
		IMMPTRVAL(work, (A2+(SREG(0)>>2)));
		set_mem_value(curr_cpu,UREG(A1),*(((char*)&work)+(SREG(0)&3)));
	} else
	if (t2 == TYPE_UPTR) {
		int work;
		// BLACK MAGIC!!! :)))
		IMMPTRVAL(work, ((UREG(A2))+(SREG(0)>>2)));
		set_mem_value(curr_cpu,UREG(A1),*(((char*)&work)+(SREG(0)&3)));
	} else \
	non_fatal(ERROR_BAD_PARAM,"LDB: Bad parameter #2",curr_cpu);
}

// cmd_stob_ureg

void cmd_stob_ureg () {
  int t2;

	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) {
		*(((char*)&(UREG(A1)))+(SREG(0)&3))=(char)A2;
	} else
	if (t2 == TYPE_UREG) {
		*(((char*)&(UREG(A1)))+(SREG(0)&3))=(char)UREG(A2);
	} else
	if (t2 == TYPE_SREG) {
		*(((char*)&(UREG(A1)))+(SREG(0)&3))=(char)SREG(A2);
	} else
	if (t2 == TYPE_IMMPTR) {
		int work;
		IMMPTRVAL(work, A2);
		*(((char*)&(UREG(A1)))+(SREG(0)&3))=(char)work;
	} else
	if (t2 == TYPE_UPTR) {
		int work;
		UPTRVAL(work, A2);
		*(((char*)&(UREG(A1)))+(SREG(0)&3))=(char)work;
	} else \
		non_fatal(ERROR_BAD_PARAM,"STOB: Bad parameter #2",curr_cpu);
}

void cmd_stob_sreg () {
  int t2;

	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) {
		*(((char*)&(SREG(A1)))+(SREG(0)&3))=(char)A2;
	} else
	if (t2 == TYPE_UREG) {
		*(((char*)&(SREG(A1)))+(SREG(0)&3))=(char)UREG(A2);
	} else
	if (t2 == TYPE_SREG) {
		*(((char*)&(SREG(A1)))+(SREG(0)&3))=(char)SREG(A2);
	} else
	if (t2 == TYPE_IMMPTR) {
		int work;
		IMMPTRVAL(work, A2);
		*(((char*)&(SREG(A1)))+(SREG(0)&3))=(char)work;
	} else
	if (t2 == TYPE_UPTR) {
		int work;
		UPTRVAL(work, A2);
		*(((char*)&(SREG(A1)))+(SREG(0)&3))=(char)work;
	} else \
		non_fatal(ERROR_BAD_PARAM,"STOB: Bad parameter #2",curr_cpu);
}

void cmd_stob_immptr () {
  int a1,t2,work;

	EVAL_T2;
	EVAL_A1;
	IMMPTRVAL(work,(a1+(SREG(0)>>2)));
	if (t2 == TYPE_IMMEDIATE) {
		*(((char *)&work)+(SREG(0)&3))=(char)A2;
		set_mem_value(curr_cpu,(a1+(SREG(0)>>2)),work);
	} else
	if (t2 == TYPE_UREG) {
		*(((char *)&work)+(SREG(0)&3))=(char)UREG(A2);
		set_mem_value(curr_cpu,(a1+(SREG(0)>>2)),work);
	} else
	if (t2 == TYPE_SREG) {
		*(((char *)&work)+(SREG(0)&3))=(char)SREG(A2);
		set_mem_value(curr_cpu,(a1+(SREG(0)>>2)),work);
	} else
	if (t2 == TYPE_IMMPTR) {
		int work2;
		IMMPTRVAL(work2,A2);
		*(((char *)&work)+(SREG(0)&3))=(char)work2;
		set_mem_value(curr_cpu,(a1+(SREG(0)>>2)),work);
	} else
	if (t2 == TYPE_UPTR) {
		int work2;
		UPTRVAL(work2,A2);
		*(((char *)&work)+(SREG(0)&3))=(char)work2;
		set_mem_value(curr_cpu,(a1+(SREG(0)>>2)),work);
	} else \
		non_fatal(ERROR_BAD_PARAM,"STOB: Bad parameter #2",curr_cpu);
}

void cmd_stob_uptr () {
  int a1,t2,work;

  	EVAL_T2;
	EVAL_A1;
	// BLACK MAGIC!!! :)))
	IMMPTRVAL(work,((UREG(a1))+(SREG(0)>>2)));
	if (t2 == TYPE_IMMEDIATE) {
		*(((char *)&work)+(SREG(0)&3))=(char)A2;
		set_mem_value(curr_cpu,(UREG(a1)+(SREG(0)>>2)),work);
	} else
	if (t2 == TYPE_UREG) {
		*(((char *)&work)+(SREG(0)&3))=(char)UREG(A2);
		set_mem_value(curr_cpu,(UREG(a1)+(SREG(0)>>2)),work);
	} else
	if (t2 == TYPE_SREG) {
		*(((char *)&work)+(SREG(0)&3))=(char)SREG(A2);
		set_mem_value(curr_cpu,(UREG(a1)+(SREG(0)>>2)),work);
	} else
	if (t2 == TYPE_IMMPTR) {
		int work2;
		IMMPTRVAL(work2,A2);
		*(((char *)&work)+(SREG(0)&3))=(char)work2;
		set_mem_value(curr_cpu,(UREG(a1)+(SREG(0)>>2)),work);
	} else
	if (t2 == TYPE_UPTR) {
		int work2;
		UPTRVAL(work2,A2);
		*(((char *)&work)+(SREG(0)&3))=(char)work2;
		set_mem_value(curr_cpu,(UREG(a1)+(SREG(0)>>2)),work);
	} else \
		non_fatal(ERROR_BAD_PARAM,"STOB: Bad parameter #2",curr_cpu);
}

// cmd_alloc

void cmd_alloc_immediate () {
  int t2;

	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) {
		int x=mem_alloc(curr_cpu,A1,A2);
		if (x>=0) {
			UREG(1)=(*curr_cpu_p->mem)[x].map_addr;
			UREG(0)=x;
		}
	} else
	if (t2 == TYPE_UREG) {
		int x=mem_alloc(curr_cpu,A1,UREG(A2));
		if (x>=0) {
			UREG(1)=(*curr_cpu_p->mem)[x].map_addr;
			UREG(0)=x;
		}
	} else
	if (t2 == TYPE_SREG) {
		int x=mem_alloc(curr_cpu,A1,SREG(A2));
		if (x>=0) {
			UREG(1)=(*curr_cpu_p->mem)[x].map_addr;
			UREG(0)=x;
		}
	} else
	if (t2 == TYPE_IMMPTR) {
		int work;
		int x;
		IMMPTRVAL(work,A2);
		x=mem_alloc(curr_cpu,A1,work);
		if (x>=0) {
			UREG(1)=(*curr_cpu_p->mem)[x].map_addr;
			UREG(0)=x;
		}
	} else 
	if (t2 == TYPE_UPTR) {
		int work;
		int x;
		UPTRVAL(work,A2);
		x=mem_alloc(curr_cpu,A1,work);
		if (x>=0) {
			UREG(1)=(*curr_cpu_p->mem)[x].map_addr;
			UREG(0)=x;
		}
	} else \
	non_fatal(ERROR_BAD_PARAM,"ALLOC: Bad parameter #2",curr_cpu);
}

void cmd_alloc_ureg () {
  int t2;

	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) {
		int x=mem_alloc(curr_cpu,UREG(A1),A2);
		if (x>=0) {
			UREG(1)=(*curr_cpu_p->mem)[x].map_addr;
			UREG(0)=x;
		}
	} else
	if (t2 == TYPE_UREG) {
		int x=mem_alloc(curr_cpu,UREG(A1),UREG(A2));
		if (x>=0) {
			UREG(1)=(*curr_cpu_p->mem)[x].map_addr;
			UREG(0)=x;
		}
	} else
	if (t2 == TYPE_SREG) {
		int x=mem_alloc(curr_cpu,UREG(A1),SREG(A2));
		if (x>=0) {
			UREG(1)=(*curr_cpu_p->mem)[x].map_addr;
			UREG(0)=x;
		}
	} else
	if (t2 == TYPE_IMMPTR) {
		int work;
		int x;
		IMMPTRVAL(work,A2);
		x=mem_alloc(curr_cpu,UREG(A1),work);
		if (x>=0) {
			UREG(1)=(*curr_cpu_p->mem)[x].map_addr;
			UREG(0)=x;
		}
	} else 
	if (t2 == TYPE_UPTR) {
		int work;
		int x;
		UPTRVAL(work,A2);
		x=mem_alloc(curr_cpu,UREG(A1),work);
		if (x>=0) {
			UREG(1)=(*curr_cpu_p->mem)[x].map_addr;
			UREG(0)=x;
		}
	} else \
	non_fatal(ERROR_BAD_PARAM,"ALLOC: Bad parameter #2",curr_cpu);
}

void cmd_alloc_sreg () {
  int t2;

	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) {
		int x=mem_alloc(curr_cpu,SREG(A1),A2);
		if (x>=0) {
			UREG(1)=(*curr_cpu_p->mem)[x].map_addr;
			UREG(0)=x;
		}
	} else
	if (t2 == TYPE_UREG) {
		int x=mem_alloc(curr_cpu,SREG(A1),UREG(A2));
		if (x>=0) {
			UREG(1)=(*curr_cpu_p->mem)[x].map_addr;
			UREG(0)=x;
		}
	} else
	if (t2 == TYPE_SREG) {
		int x=mem_alloc(curr_cpu,SREG(A1),SREG(A2));
		if (x>=0) {
			UREG(1)=(*curr_cpu_p->mem)[x].map_addr;
			UREG(0)=x;
		}
	} else
	if (t2 == TYPE_IMMPTR) {
		int work;
		int x;
		IMMPTRVAL(work,A2);
		x=mem_alloc(curr_cpu,SREG(A1),work);
		if (x>=0) {
			UREG(1)=(*curr_cpu_p->mem)[x].map_addr;
			UREG(0)=x;
		}
	} else 
	if (t2 == TYPE_UPTR) {
		int work;
		int x;
		UPTRVAL(work,A2);
		x=mem_alloc(curr_cpu,SREG(A1),work);
		if (x>=0) {
			UREG(1)=(*curr_cpu_p->mem)[x].map_addr;
			UREG(0)=x;
		}
	} else \
	non_fatal(ERROR_BAD_PARAM,"ALLOC: Bad parameter #2",curr_cpu);
}

void cmd_alloc_immptr () {
  int t2;
  int work;

  	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) {
		int x;
		IMMPTRVAL(work,A1);
		x=mem_alloc(curr_cpu,work,A2);
		if (x>=0) {
			UREG(1)=(*curr_cpu_p->mem)[x].map_addr;
			UREG(0)=x;
		}
	} else 
	if (t2 == TYPE_UREG) {
		int x;
		IMMPTRVAL(work,A1);
		x=mem_alloc(curr_cpu,work,UREG(A2));
		if (x>=0) {
			UREG(1)=(*curr_cpu_p->mem)[x].map_addr;
			UREG(0)=x;
		}
	} else 
	if (t2 == TYPE_SREG) {
		int x;
		IMMPTRVAL(work,A1);
		x=mem_alloc(curr_cpu,work,SREG(A2));
		if (x>=0) {
			UREG(1)=(*curr_cpu_p->mem)[x].map_addr;
			UREG(0)=x;
		}
	} else 
	if (t2 == TYPE_IMMPTR) {
		int x;
		int work2;
		IMMPTRVAL(work,A1);
		IMMPTRVAL(work2,A2);
		x=mem_alloc(curr_cpu,work,work2);
		if (x>=0) {
			UREG(1)=(*curr_cpu_p->mem)[x].map_addr;
			UREG(0)=x;
		}
	} else 
	if (t2 == TYPE_UPTR) {
		int x;
		int work2;
		IMMPTRVAL(work,A1);
		UPTRVAL(work2,A2);
		x=mem_alloc(curr_cpu,work,work2);
		if (x>=0) {
			UREG(1)=(*curr_cpu_p->mem)[x].map_addr;
			UREG(0)=x;
		}
	} else \
	non_fatal(ERROR_BAD_PARAM,"ALLOC: Bad parameter #2",curr_cpu);
}

void cmd_alloc_uptr () {
  int t2;
  int work;

  	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) {
		int x;
		UPTRVAL(work,A1);
		x=mem_alloc(curr_cpu,work,A2);
		if (x>=0) {
			UREG(1)=(*curr_cpu_p->mem)[x].map_addr;
			UREG(0)=x;
		}
	} else 
	if (t2 == TYPE_UREG) {
		int x;
		UPTRVAL(work,A1);
		x=mem_alloc(curr_cpu,work,UREG(A2));
		if (x>=0) {
			UREG(1)=(*curr_cpu_p->mem)[x].map_addr;
			UREG(0)=x;
		}
	} else 
	if (t2 == TYPE_SREG) {
		int x;
		UPTRVAL(work,A1);
		x=mem_alloc(curr_cpu,work,SREG(A2));
		if (x>=0) {
			UREG(1)=(*curr_cpu_p->mem)[x].map_addr;
			UREG(0)=x;
		}
	} else 
	if (t2 == TYPE_IMMPTR) {
		int x;
		int work2;
		UPTRVAL(work,A1);
		IMMPTRVAL(work2,A2);
		x=mem_alloc(curr_cpu,work,work2);
		if (x>=0) {
			UREG(1)=(*curr_cpu_p->mem)[x].map_addr;
			UREG(0)=x;
		}
	} else 
	if (t2 == TYPE_UPTR) {
		int x;
		int work2;
		UPTRVAL(work,A1);
		UPTRVAL(work2,A2);
		x=mem_alloc(curr_cpu,work,work2);
		if (x>=0) {
			UREG(1)=(*curr_cpu_p->mem)[x].map_addr;
			UREG(0)=x;
		}
	} else \
	non_fatal(ERROR_BAD_PARAM,"ALLOC: Bad parameter #2",curr_cpu);
}

// cmd_realloc

void cmd_realloc_immediate () {
  int t2;

	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) mem_realloc(curr_cpu,A1,A2); else
	if (t2 == TYPE_UREG) mem_realloc(curr_cpu,A1,UREG(A2)); else
	if (t2 == TYPE_SREG) mem_realloc(curr_cpu,A1,SREG(A2)); else
	if (t2 == TYPE_IMMPTR) {
		int work;
		IMMPTRVAL(work,A2);
		mem_realloc(curr_cpu,A1,work);
	} else
	if (t2 == TYPE_UPTR) {
		int work;
		UPTRVAL(work,A2);
		mem_realloc(curr_cpu,A1,work);
	} else \
	non_fatal(ERROR_BAD_PARAM,"REALLOC: Bad parameter #2",curr_cpu);
}

void cmd_realloc_ureg () {
  int t2;

	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) mem_realloc(curr_cpu,UREG(A1),A2); else
	if (t2 == TYPE_UREG) mem_realloc(curr_cpu,UREG(A1),UREG(A2)); else
	if (t2 == TYPE_SREG) mem_realloc(curr_cpu,UREG(A1),SREG(A2)); else
	if (t2 == TYPE_IMMPTR) {
		int work;
		IMMPTRVAL(work,A2);
		mem_realloc(curr_cpu,UREG(A1),work);
	} else
	if (t2 == TYPE_UPTR) {
		int work;
		UPTRVAL(work,A2);
		mem_realloc(curr_cpu,UREG(A1),work);
	} else \
	non_fatal(ERROR_BAD_PARAM,"REALLOC: Bad parameter #2",curr_cpu);
}

void cmd_realloc_sreg () {
  int t2;

	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) mem_realloc(curr_cpu,SREG(A1),A2); else
	if (t2 == TYPE_UREG) mem_realloc(curr_cpu,SREG(A1),UREG(A2)); else
	if (t2 == TYPE_SREG) mem_realloc(curr_cpu,SREG(A1),SREG(A2)); else
	if (t2 == TYPE_IMMPTR) {
		int work;
		IMMPTRVAL(work,A2);
		mem_realloc(curr_cpu,SREG(A1),work);
	} else
	if (t2 == TYPE_UPTR) {
		int work;
		UPTRVAL(work,A2);
		mem_realloc(curr_cpu,SREG(A1),work);
	} else \
	non_fatal(ERROR_BAD_PARAM,"REALLOC: Bad parameter #2",curr_cpu);
}

void cmd_realloc_immptr () {
  int work, t2;

  	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) {
		IMMPTRVAL(work,A1);
		mem_realloc(curr_cpu,work,A2);
	} else
	if (t2 == TYPE_UREG) {
		IMMPTRVAL(work,A1);
		mem_realloc(curr_cpu,work,UREG(A2));
	} else
	if (t2 == TYPE_SREG) {
		IMMPTRVAL(work,A1);
		mem_realloc(curr_cpu,work,SREG(A2));
	} else
	if (t2 == TYPE_IMMPTR) {
		int work2;
		IMMPTRVAL(work,A1);
		IMMPTRVAL(work2,A2);
		mem_realloc(curr_cpu,work,work2);
	} else
	if (t2 == TYPE_UPTR) {
		int work2;
		IMMPTRVAL(work,A1);
		UPTRVAL(work2,A2);
		mem_realloc(curr_cpu,work,work2);
	} else \
	non_fatal(ERROR_BAD_PARAM,"REALLOC: Bad parameter #2",curr_cpu);
}

void cmd_realloc_uptr () {
  int work, t2;

  	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) {
		UPTRVAL(work,A1);
		mem_realloc(curr_cpu,work,A2);
	} else
	if (t2 == TYPE_UREG) {
		UPTRVAL(work,A1);
		mem_realloc(curr_cpu,work,UREG(A2));
	} else
	if (t2 == TYPE_SREG) {
		UPTRVAL(work,A1);
		mem_realloc(curr_cpu,work,SREG(A2));
	} else
	if (t2 == TYPE_IMMPTR) {
		int work2;
		UPTRVAL(work,A1);
		IMMPTRVAL(work2,A2);
		mem_realloc(curr_cpu,work,work2);
	} else
	if (t2 == TYPE_UPTR) {
		int work2;
		UPTRVAL(work,A1);
		UPTRVAL(work2,A2);
		mem_realloc(curr_cpu,work,work2);
	} else \
	non_fatal(ERROR_BAD_PARAM,"REALLOC: Bad parameter #2",curr_cpu);
}

// cmd_dealloc

void cmd_dealloc_immediate () {

	mem_dealloc(curr_cpu,A1);
}

void cmd_dealloc_ureg () {

	mem_dealloc(curr_cpu,UREG(A1));
}

void cmd_dealloc_sreg () {

	mem_dealloc(curr_cpu,SREG(A1));
}

void cmd_dealloc_immptr () {
  int work;

  	IMMPTRVAL(work,A1);
	mem_dealloc(curr_cpu,work);
}

void cmd_dealloc_uptr () {
  int work;

  	UPTRVAL(work,A1);
	mem_dealloc(curr_cpu,work);
}

// cmd_cmpcnt

void cmd_cmpcnt_immediate () {
  int t2;

  	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) {
		int *addr1, *addr2;
		addr1=verify_access(curr_cpu,A1,SREG(0),MEM_FLAG_READ);
		addr2=verify_access(curr_cpu,A2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Cmp1 memory unsuitable",curr_cpu);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Cmp2 memory unsuitable",curr_cpu);
			return;
		}
		SREG(0)=memcmp(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_UREG) {
		int *addr1, *addr2;
		addr1=verify_access(curr_cpu,A1,SREG(0),MEM_FLAG_READ);
		addr2=verify_access(curr_cpu,UREG(A2),SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Cmp1 memory unsuitable",curr_cpu);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Cmp2 memory unsuitable",curr_cpu);
			return;
		}
		SREG(0)=memcmp(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_SREG) {
		int *addr1, *addr2;
		addr1=verify_access(curr_cpu,A1,SREG(0),MEM_FLAG_READ);
		addr2=verify_access(curr_cpu,SREG(A2),SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Cmp1 memory unsuitable",curr_cpu);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Cmp2 memory unsuitable",curr_cpu);
			return;
		}
		SREG(0)=memcmp(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_IMMEDIATE) {
		int a2;
		int *addr1, *addr2;
		IMMPTRVAL(a2,A2);
		addr1=verify_access(curr_cpu,A1,SREG(0),MEM_FLAG_READ);
		addr2=verify_access(curr_cpu,a2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Cmp1 memory unsuitable",curr_cpu);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Cmp2 memory unsuitable",curr_cpu);
			return;
		}
		SREG(0)=memcmp(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_UPTR) {
		int a2;
		int *addr1, *addr2;
		UPTRVAL(a2,A2);
		addr1=verify_access(curr_cpu,A1,SREG(0),MEM_FLAG_READ);
		addr2=verify_access(curr_cpu,a2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Cmp1 memory unsuitable",curr_cpu);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Cmp2 memory unsuitable",curr_cpu);
			return;
		}
		SREG(0)=memcmp(addr1,addr2,SREG(0)*4);
		return;
	} else
	non_fatal(ERROR_BAD_PARAM,"CP: Bad parameter 2",curr_cpu);
}

void cmd_cmpcnt_ureg () {
  int t2;

  	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) {
		int *addr1, *addr2;
		addr1=verify_access(curr_cpu,UREG(A1),SREG(0),MEM_FLAG_READ);
		addr2=verify_access(curr_cpu,A2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Cmp1 memory unsuitable",curr_cpu);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Cmp2 memory unsuitable",curr_cpu);
			return;
		}
		SREG(0)=memcmp(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_UREG) {
		int *addr1, *addr2;
		addr1=verify_access(curr_cpu,UREG(A1),SREG(0),MEM_FLAG_READ);
		addr2=verify_access(curr_cpu,UREG(A2),SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Cmp1 memory unsuitable",curr_cpu);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Cmp2 memory unsuitable",curr_cpu);
			return;
		}
		SREG(0)=memcmp(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_SREG) {
		int *addr1, *addr2;
		addr1=verify_access(curr_cpu,UREG(A1),SREG(0),MEM_FLAG_READ);
		addr2=verify_access(curr_cpu,SREG(A2),SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Cmp1 memory unsuitable",curr_cpu);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Cmp2 memory unsuitable",curr_cpu);
			return;
		}
		SREG(0)=memcmp(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_IMMEDIATE) {
		int a2;
		int *addr1, *addr2;
		IMMPTRVAL(a2,A2);
		addr1=verify_access(curr_cpu,UREG(A1),SREG(0),MEM_FLAG_READ);
		addr2=verify_access(curr_cpu,a2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Cmp1 memory unsuitable",curr_cpu);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Cmp2 memory unsuitable",curr_cpu);
			return;
		}
		SREG(0)=memcmp(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_UPTR) {
		int a2;
		int *addr1, *addr2;
		UPTRVAL(a2,A2);
		addr1=verify_access(curr_cpu,UREG(A1),SREG(0),MEM_FLAG_READ);
		addr2=verify_access(curr_cpu,a2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Cmp1 memory unsuitable",curr_cpu);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Cmp2 memory unsuitable",curr_cpu);
			return;
		}
		SREG(0)=memcmp(addr1,addr2,SREG(0)*4);
		return;
	} else
	non_fatal(ERROR_BAD_PARAM,"CP: Bad parameter 2",curr_cpu);
}

void cmd_cmpcnt_sreg () {
  int t2;

  	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) {
		int *addr1, *addr2;
		addr1=verify_access(curr_cpu,SREG(A1),SREG(0),MEM_FLAG_READ);
		addr2=verify_access(curr_cpu,A2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Cmp1 memory unsuitable",curr_cpu);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Cmp2 memory unsuitable",curr_cpu);
			return;
		}
		SREG(0)=memcmp(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_UREG) {
		int *addr1, *addr2;
		addr1=verify_access(curr_cpu,SREG(A1),SREG(0),MEM_FLAG_READ);
		addr2=verify_access(curr_cpu,UREG(A2),SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Cmp1 memory unsuitable",curr_cpu);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Cmp2 memory unsuitable",curr_cpu);
			return;
		}
		SREG(0)=memcmp(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_SREG) {
		int *addr1, *addr2;
		addr1=verify_access(curr_cpu,SREG(A1),SREG(0),MEM_FLAG_READ);
		addr2=verify_access(curr_cpu,SREG(A2),SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Cmp1 memory unsuitable",curr_cpu);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Cmp2 memory unsuitable",curr_cpu);
			return;
		}
		SREG(0)=memcmp(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_IMMEDIATE) {
		int a2;
		int *addr1, *addr2;
		IMMPTRVAL(a2,A2);
		addr1=verify_access(curr_cpu,SREG(A1),SREG(0),MEM_FLAG_READ);
		addr2=verify_access(curr_cpu,a2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Cmp1 memory unsuitable",curr_cpu);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Cmp2 memory unsuitable",curr_cpu);
			return;
		}
		SREG(0)=memcmp(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_UPTR) {
		int a2;
		int *addr1, *addr2;
		UPTRVAL(a2,A2);
		addr1=verify_access(curr_cpu,SREG(A1),SREG(0),MEM_FLAG_READ);
		addr2=verify_access(curr_cpu,a2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Cmp1 memory unsuitable",curr_cpu);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Cmp2 memory unsuitable",curr_cpu);
			return;
		}
		SREG(0)=memcmp(addr1,addr2,SREG(0)*4);
		return;
	} else
	non_fatal(ERROR_BAD_PARAM,"CP: Bad parameter 2",curr_cpu);
}

void cmd_cmpcnt_immptr () {
  int t2;

  	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) {
		int a1;
		int *addr1, *addr2;
		IMMPTRVAL(a1,A1);
		addr1=verify_access(curr_cpu,a1,SREG(0),MEM_FLAG_READ);
		addr2=verify_access(curr_cpu,A2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Cmp1 memory unsuitable",curr_cpu);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Cmp2 memory unsuitable",curr_cpu);
			return;
		}
		SREG(0)=memcmp(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_UREG) {
		int a1;
		int *addr1, *addr2;
		IMMPTRVAL(a1,A1);
		addr1=verify_access(curr_cpu,a1,SREG(0),MEM_FLAG_READ);
		addr2=verify_access(curr_cpu,UREG(A2),SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Cmp1 memory unsuitable",curr_cpu);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Cmp2 memory unsuitable",curr_cpu);
			return;
		}
		SREG(0)=memcmp(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_SREG) {
		int a1;
		int *addr1, *addr2;
		IMMPTRVAL(a1,A1);
		addr1=verify_access(curr_cpu,a1,SREG(0),MEM_FLAG_READ);
		addr2=verify_access(curr_cpu,SREG(A2),SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Cmp1 memory unsuitable",curr_cpu);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Cmp2 memory unsuitable",curr_cpu);
			return;
		}
		SREG(0)=memcmp(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_IMMEDIATE) {
		int a1,a2;
		int *addr1, *addr2;
		IMMPTRVAL(a1,A1);
		IMMPTRVAL(a2,A2);
		addr1=verify_access(curr_cpu,a1,SREG(0),MEM_FLAG_READ);
		addr2=verify_access(curr_cpu,a2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Cmp1 memory unsuitable",curr_cpu);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Cmp2 memory unsuitable",curr_cpu);
			return;
		}
		SREG(0)=memcmp(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_UPTR) {
		int a1,a2;
		int *addr1, *addr2;
		IMMPTRVAL(a1,A1);
		UPTRVAL(a2,A2);
		addr1=verify_access(curr_cpu,a1,SREG(0),MEM_FLAG_READ);
		addr2=verify_access(curr_cpu,a2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Cmp1 memory unsuitable",curr_cpu);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Cmp2 memory unsuitable",curr_cpu);
			return;
		}
		SREG(0)=memcmp(addr1,addr2,SREG(0)*4);
		return;
	} else
	non_fatal(ERROR_BAD_PARAM,"CP: Bad parameter 2",curr_cpu);
}

void cmd_cmpcnt_uptr () {
  int t2;

  	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) {
		int a1;
		int *addr1, *addr2;
		UPTRVAL(a1,A1);
		addr1=verify_access(curr_cpu,a1,SREG(0),MEM_FLAG_READ);
		addr2=verify_access(curr_cpu,A2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Cmp1 memory unsuitable",curr_cpu);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Cmp2 memory unsuitable",curr_cpu);
			return;
		}
		SREG(0)=memcmp(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_UREG) {
		int a1;
		int *addr1, *addr2;
		UPTRVAL(a1,A1);
		addr1=verify_access(curr_cpu,a1,SREG(0),MEM_FLAG_READ);
		addr2=verify_access(curr_cpu,UREG(A2),SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Cmp1 memory unsuitable",curr_cpu);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Cmp2 memory unsuitable",curr_cpu);
			return;
		}
		SREG(0)=memcmp(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_SREG) {
		int a1;
		int *addr1, *addr2;
		UPTRVAL(a1,A1);
		addr1=verify_access(curr_cpu,a1,SREG(0),MEM_FLAG_READ);
		addr2=verify_access(curr_cpu,SREG(A2),SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Cmp1 memory unsuitable",curr_cpu);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Cmp2 memory unsuitable",curr_cpu);
			return;
		}
		SREG(0)=memcmp(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_IMMEDIATE) {
		int a1,a2;
		int *addr1, *addr2;
		UPTRVAL(a1,A1);
		IMMPTRVAL(a2,A2);
		addr1=verify_access(curr_cpu,a1,SREG(0),MEM_FLAG_READ);
		addr2=verify_access(curr_cpu,a2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Cmp1 memory unsuitable",curr_cpu);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Cmp2 memory unsuitable",curr_cpu);
			return;
		}
		SREG(0)=memcmp(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_UPTR) {
		int a1,a2;
		int *addr1, *addr2;
		UPTRVAL(a1,A1);
		UPTRVAL(a2,A2);
		addr1=verify_access(curr_cpu,a1,SREG(0),MEM_FLAG_READ);
		addr2=verify_access(curr_cpu,a2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Cmp1 memory unsuitable",curr_cpu);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Cmp2 memory unsuitable",curr_cpu);
			return;
		}
		SREG(0)=memcmp(addr1,addr2,SREG(0)*4);
		return;
	} else
	non_fatal(ERROR_BAD_PARAM,"CP: Bad parameter 2",curr_cpu);
}


// cmd_cpcnt

void cmd_cpcnt_immediate () {
  int t2;

  	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) {
		int *addr1, *addr2;
		addr1=verify_access(curr_cpu,A1,SREG(0),MEM_FLAG_WRITE);
		addr2=verify_access(curr_cpu,A2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Destination memory unsuitable",curr_cpu);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Source memory unsuitable",curr_cpu);
			return;
		}
		memcpy(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_UREG) {
		int *addr1, *addr2;
		addr1=verify_access(curr_cpu,A1,SREG(0),MEM_FLAG_WRITE);
		addr2=verify_access(curr_cpu,UREG(A2),SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Destination memory unsuitable",curr_cpu);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Source memory unsuitable",curr_cpu);
			return;
		}
		memcpy(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_SREG) {
		int *addr1, *addr2;
		addr1=verify_access(curr_cpu,A1,SREG(0),MEM_FLAG_WRITE);
		addr2=verify_access(curr_cpu,SREG(A2),SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Destination memory unsuitable",curr_cpu);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Source memory unsuitable",curr_cpu);
			return;
		}
		memcpy(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_IMMEDIATE) {
		int a2;
		int *addr1, *addr2;
		IMMPTRVAL(a2,A2);
		addr1=verify_access(curr_cpu,A1,SREG(0),MEM_FLAG_WRITE);
		addr2=verify_access(curr_cpu,a2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Destination memory unsuitable",curr_cpu);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Source memory unsuitable",curr_cpu);
			return;
		}
		memcpy(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_UPTR) {
		int a2;
		int *addr1, *addr2;
		UPTRVAL(a2,A2);
		addr1=verify_access(curr_cpu,A1,SREG(0),MEM_FLAG_WRITE);
		addr2=verify_access(curr_cpu,a2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Destination memory unsuitable",curr_cpu);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Source memory unsuitable",curr_cpu);
			return;
		}
		memcpy(addr1,addr2,SREG(0)*4);
		return;
	} else
	non_fatal(ERROR_BAD_PARAM,"CP: Bad parameter 2",curr_cpu);
}

void cmd_cpcnt_ureg () {
  int t2;

  	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) {
		int *addr1, *addr2;
		addr1=verify_access(curr_cpu,UREG(A1),SREG(0),MEM_FLAG_WRITE);
		addr2=verify_access(curr_cpu,A2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Destination memory unsuitable",curr_cpu);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Source memory unsuitable",curr_cpu);
			return;
		}
		memcpy(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_UREG) {
		int *addr1, *addr2;
		addr1=verify_access(curr_cpu,UREG(A1),SREG(0),MEM_FLAG_WRITE);
		addr2=verify_access(curr_cpu,UREG(A2),SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Destination memory unsuitable",curr_cpu);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Source memory unsuitable",curr_cpu);
			return;
		}
		memcpy(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_SREG) {
		int *addr1, *addr2;
		addr1=verify_access(curr_cpu,UREG(A1),SREG(0),MEM_FLAG_WRITE);
		addr2=verify_access(curr_cpu,SREG(A2),SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Destination memory unsuitable",curr_cpu);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Source memory unsuitable",curr_cpu);
			return;
		}
		memcpy(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_IMMEDIATE) {
		int a2;
		int *addr1, *addr2;
		IMMPTRVAL(a2,A2);
		addr1=verify_access(curr_cpu,UREG(A1),SREG(0),MEM_FLAG_WRITE);
		addr2=verify_access(curr_cpu,a2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Destination memory unsuitable",curr_cpu);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Source memory unsuitable",curr_cpu);
			return;
		}
		memcpy(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_UPTR) {
		int a2;
		int *addr1, *addr2;
		UPTRVAL(a2,A2);
		addr1=verify_access(curr_cpu,UREG(A1),SREG(0),MEM_FLAG_WRITE);
		addr2=verify_access(curr_cpu,a2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Destination memory unsuitable",curr_cpu);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Source memory unsuitable",curr_cpu);
			return;
		}
		memcpy(addr1,addr2,SREG(0)*4);
		return;
	} else
	non_fatal(ERROR_BAD_PARAM,"CP: Bad parameter 2",curr_cpu);
}

void cmd_cpcnt_sreg () {
  int t2;

  	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) {
		int *addr1, *addr2;
		addr1=verify_access(curr_cpu,SREG(A1),SREG(0),MEM_FLAG_WRITE);
		addr2=verify_access(curr_cpu,A2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Destination memory unsuitable",curr_cpu);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Source memory unsuitable",curr_cpu);
			return;
		}
		memcpy(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_UREG) {
		int *addr1, *addr2;
		addr1=verify_access(curr_cpu,SREG(A1),SREG(0),MEM_FLAG_WRITE);
		addr2=verify_access(curr_cpu,UREG(A2),SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Destination memory unsuitable",curr_cpu);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Source memory unsuitable",curr_cpu);
			return;
		}
		memcpy(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_SREG) {
		int *addr1, *addr2;
		addr1=verify_access(curr_cpu,SREG(A1),SREG(0),MEM_FLAG_WRITE);
		addr2=verify_access(curr_cpu,SREG(A2),SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Destination memory unsuitable",curr_cpu);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Source memory unsuitable",curr_cpu);
			return;
		}
		memcpy(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_IMMEDIATE) {
		int a2;
		int *addr1, *addr2;
		IMMPTRVAL(a2,A2);
		addr1=verify_access(curr_cpu,SREG(A1),SREG(0),MEM_FLAG_WRITE);
		addr2=verify_access(curr_cpu,a2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Destination memory unsuitable",curr_cpu);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Source memory unsuitable",curr_cpu);
			return;
		}
		memcpy(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_UPTR) {
		int a2;
		int *addr1, *addr2;
		UPTRVAL(a2,A2);
		addr1=verify_access(curr_cpu,SREG(A1),SREG(0),MEM_FLAG_WRITE);
		addr2=verify_access(curr_cpu,a2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Destination memory unsuitable",curr_cpu);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Source memory unsuitable",curr_cpu);
			return;
		}
		memcpy(addr1,addr2,SREG(0)*4);
		return;
	} else
	non_fatal(ERROR_BAD_PARAM,"CP: Bad parameter 2",curr_cpu);
}

void cmd_cpcnt_immptr () {
  int t2;

  	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) {
		int a1;
		int *addr1, *addr2;
		IMMPTRVAL(a1,A1);
		addr1=verify_access(curr_cpu,a1,SREG(0),MEM_FLAG_WRITE);
		addr2=verify_access(curr_cpu,A2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Destination memory unsuitable",curr_cpu);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Source memory unsuitable",curr_cpu);
			return;
		}
		memcpy(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_UREG) {
		int a1;
		int *addr1, *addr2;
		IMMPTRVAL(a1,A1);
		addr1=verify_access(curr_cpu,a1,SREG(0),MEM_FLAG_WRITE);
		addr2=verify_access(curr_cpu,UREG(A2),SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Destination memory unsuitable",curr_cpu);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Source memory unsuitable",curr_cpu);
			return;
		}
		memcpy(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_SREG) {
		int a1;
		int *addr1, *addr2;
		IMMPTRVAL(a1,A1);
		addr1=verify_access(curr_cpu,a1,SREG(0),MEM_FLAG_WRITE);
		addr2=verify_access(curr_cpu,SREG(A2),SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Destination memory unsuitable",curr_cpu);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Source memory unsuitable",curr_cpu);
			return;
		}
		memcpy(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_IMMEDIATE) {
		int a1,a2;
		int *addr1, *addr2;
		IMMPTRVAL(a1,A1);
		IMMPTRVAL(a2,A2);
		addr1=verify_access(curr_cpu,a1,SREG(0),MEM_FLAG_WRITE);
		addr2=verify_access(curr_cpu,a2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Destination memory unsuitable",curr_cpu);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Source memory unsuitable",curr_cpu);
			return;
		}
		memcpy(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_UPTR) {
		int a1,a2;
		int *addr1, *addr2;
		IMMPTRVAL(a1,A1);
		UPTRVAL(a2,A2);
		addr1=verify_access(curr_cpu,a1,SREG(0),MEM_FLAG_WRITE);
		addr2=verify_access(curr_cpu,a2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Destination memory unsuitable",curr_cpu);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Source memory unsuitable",curr_cpu);
			return;
		}
		memcpy(addr1,addr2,SREG(0)*4);
		return;
	} else
	non_fatal(ERROR_BAD_PARAM,"CP: Bad parameter 2",curr_cpu);
}

void cmd_cpcnt_uptr () {
  int t2;

  	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) {
		int a1;
		int *addr1, *addr2;
		UPTRVAL(a1,A1);
		addr1=verify_access(curr_cpu,a1,SREG(0),MEM_FLAG_WRITE);
		addr2=verify_access(curr_cpu,A2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Destination memory unsuitable",curr_cpu);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Source memory unsuitable",curr_cpu);
			return;
		}
		memcpy(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_UREG) {
		int a1;
		int *addr1, *addr2;
		UPTRVAL(a1,A1);
		addr1=verify_access(curr_cpu,a1,SREG(0),MEM_FLAG_WRITE);
		addr2=verify_access(curr_cpu,UREG(A2),SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Destination memory unsuitable",curr_cpu);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Source memory unsuitable",curr_cpu);
			return;
		}
		memcpy(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_SREG) {
		int a1;
		int *addr1, *addr2;
		UPTRVAL(a1,A1);
		addr1=verify_access(curr_cpu,a1,SREG(0),MEM_FLAG_WRITE);
		addr2=verify_access(curr_cpu,SREG(A2),SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Destination memory unsuitable",curr_cpu);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Source memory unsuitable",curr_cpu);
			return;
		}
		memcpy(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_IMMEDIATE) {
		int a1,a2;
		int *addr1, *addr2;
		UPTRVAL(a1,A1);
		IMMPTRVAL(a2,A2);
		addr1=verify_access(curr_cpu,a1,SREG(0),MEM_FLAG_WRITE);
		addr2=verify_access(curr_cpu,a2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Destination memory unsuitable",curr_cpu);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Source memory unsuitable",curr_cpu);
			return;
		}
		memcpy(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_UPTR) {
		int a1,a2;
		int *addr1, *addr2;
		UPTRVAL(a1,A1);
		UPTRVAL(a2,A2);
		addr1=verify_access(curr_cpu,a1,SREG(0),MEM_FLAG_WRITE);
		addr2=verify_access(curr_cpu,a2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Destination memory unsuitable",curr_cpu);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Source memory unsuitable",curr_cpu);
			return;
		}
		memcpy(addr1,addr2,SREG(0)*4);
		return;
	} else
	non_fatal(ERROR_BAD_PARAM,"CP: Bad parameter 2",curr_cpu);
}

// cmd_setstack

void cmd_setstack_immediate () {
  int t2;
  
  	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) {
		int a2;
		a2=A2;
		if (curr_cpu_p->uregs[0] > a2) {
			non_fatal(ERROR_BAD_PARAM,"SETSTACK: user stack ptr > stack size",curr_cpu);
			return;
		}
		curr_cpu_p->user_stack=A1;
		curr_cpu_p->user_size=a2;
		curr_cpu_p->user_ptr=curr_cpu_p->uregs[0];
		return;
	} else
	if (t2 == TYPE_UREG) {
		int a2;
		UREGVAL(a2,A2);
		if (curr_cpu_p->uregs[0] > a2) {
			non_fatal(ERROR_BAD_PARAM,"SETSTACK: user stack ptr > stack size",curr_cpu);
			return;
		}
		curr_cpu_p->user_stack=A1;
		curr_cpu_p->user_size=a2;
		curr_cpu_p->user_ptr=curr_cpu_p->uregs[0];
		return;
	} else
	if (t2 == TYPE_IMMPTR) {
		int a2;
		IMMPTRVAL(a2,A2);
		if (curr_cpu_p->uregs[0] > a2) {
			non_fatal(ERROR_BAD_PARAM,"SETSTACK: user stack ptr > stack size",curr_cpu);
			return;
		}
		curr_cpu_p->user_stack=A1;
		curr_cpu_p->user_size=a2;
		curr_cpu_p->user_ptr=curr_cpu_p->uregs[0];
		return;
	} else
	if (t2 == TYPE_UPTR) {
		int a2;
		UPTRVAL(a2,A2);
		if (curr_cpu_p->uregs[0] > a2) {
			non_fatal(ERROR_BAD_PARAM,"SETSTACK: user stack ptr > stack size",curr_cpu);
			return;
		}
		curr_cpu_p->user_stack=A1;
		curr_cpu_p->user_size=a2;
		curr_cpu_p->user_ptr=curr_cpu_p->uregs[0];
		return;
	} else \
	non_fatal(ERROR_BAD_PARAM,"SETSTACK: Bad parameter #2",curr_cpu);
}

void cmd_setstack_ureg () {
  int t2;
  
  	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) {
		int a1,a2;
		UREGVAL(a1,A1);
		a2=A2;
		if (curr_cpu_p->uregs[0] > a2) {
			non_fatal(ERROR_BAD_PARAM,"SETSTACK: user stack ptr > stack size",curr_cpu);
			return;
		}
		curr_cpu_p->user_stack=a1;
		curr_cpu_p->user_size=a2;
		curr_cpu_p->user_ptr=curr_cpu_p->uregs[0];
		return;
	} else
	if (t2 == TYPE_UREG) {
		int a1,a2;
		UREGVAL(a1,A1);
		UREGVAL(a2,A2);
		if (curr_cpu_p->uregs[0] > a2) {
			non_fatal(ERROR_BAD_PARAM,"SETSTACK: user stack ptr > stack size",curr_cpu);
			return;
		}
		curr_cpu_p->user_stack=a1;
		curr_cpu_p->user_size=a2;
		curr_cpu_p->user_ptr=curr_cpu_p->uregs[0];
		return;
	} else
	if (t2 == TYPE_IMMPTR) {
		int a1,a2;
		UREGVAL(a1,A1);
		IMMPTRVAL(a2,A2);
		if (curr_cpu_p->uregs[0] > a2) {
			non_fatal(ERROR_BAD_PARAM,"SETSTACK: user stack ptr > stack size",curr_cpu);
			return;
		}
		curr_cpu_p->user_stack=a1;
		curr_cpu_p->user_size=a2;
		curr_cpu_p->user_ptr=curr_cpu_p->uregs[0];
		return;
	} else
	if (t2 == TYPE_UPTR) {
		int a1,a2;
		UREGVAL(a1,A1);
		UPTRVAL(a2,A2);
		if (curr_cpu_p->uregs[0] > a2) {
			non_fatal(ERROR_BAD_PARAM,"SETSTACK: user stack ptr > stack size",curr_cpu);
			return;
		}
		curr_cpu_p->user_stack=a1;
		curr_cpu_p->user_size=a2;
		curr_cpu_p->user_ptr=curr_cpu_p->uregs[0];
		return;
	} else
	non_fatal(ERROR_BAD_PARAM,"SETSTACK: Bad parameter #2",curr_cpu);
}

void cmd_setstack_immptr () {
  int t2;

  	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) {
		int a1,a2;
		IMMPTRVAL(a1,A1);
		a2=A2;
		if (curr_cpu_p->uregs[0] > a2) {
			non_fatal(ERROR_BAD_PARAM,"SETSTACK: user stack ptr > stack size",curr_cpu);
			return;
		}
		curr_cpu_p->user_stack=a1;
		curr_cpu_p->user_size=a2;
		curr_cpu_p->user_ptr=curr_cpu_p->uregs[0];
		return;
	} else
	if (t2 == TYPE_UREG) {
		int a1,a2;
		IMMPTRVAL(a1,A1);
		UREGVAL(a2,A2);
		if (curr_cpu_p->uregs[0] > a2) {
			non_fatal(ERROR_BAD_PARAM,"SETSTACK: user stack ptr > stack size",curr_cpu);
			return;
		}
		curr_cpu_p->user_stack=a1;
		curr_cpu_p->user_size=a2;
		curr_cpu_p->user_ptr=curr_cpu_p->uregs[0];
		return;
	} else
	if (t2 == TYPE_IMMPTR) {
		int a1,a2;
		IMMPTRVAL(a1,A1);
		IMMPTRVAL(a2,A2);
		if (curr_cpu_p->uregs[0] > a2) {
			non_fatal(ERROR_BAD_PARAM,"SETSTACK: user stack ptr > stack size",curr_cpu);
			return;
		}
		curr_cpu_p->user_stack=a1;
		curr_cpu_p->user_size=a2;
		curr_cpu_p->user_ptr=curr_cpu_p->uregs[0];
		return;
	} else
	if (t2 == TYPE_UPTR) {
		int a1,a2;
		IMMPTRVAL(a1,A1);
		UPTRVAL(a2,A2);
		if (curr_cpu_p->uregs[0] > a2) {
			non_fatal(ERROR_BAD_PARAM,"SETSTACK: user stack ptr > stack size",curr_cpu);
			return;
		}
		curr_cpu_p->user_stack=a1;
		curr_cpu_p->user_size=a2;
		curr_cpu_p->user_ptr=curr_cpu_p->uregs[0];
		return;
	} else
	non_fatal(ERROR_BAD_PARAM,"SETSTACK: Bad parameter #2",curr_cpu);
}

void cmd_setstack_uptr () {
  int t2;

	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) {
		int a1,a2;
		UPTRVAL(a1,A1);
		a2=A2;
		if (curr_cpu_p->uregs[0] > a2) {
			non_fatal(ERROR_BAD_PARAM,"SETSTACK: user stack ptr > stack size",curr_cpu);
			return;
		}
		curr_cpu_p->user_stack=a1;
		curr_cpu_p->user_size=a2;
		curr_cpu_p->user_ptr=curr_cpu_p->uregs[0];
		return;
	} else
	if (t2 == TYPE_UREG) {
		int a1,a2;
		UPTRVAL(a1,A1);
		UREGVAL(a2,A2);
		if (curr_cpu_p->uregs[0] > a2) {
			non_fatal(ERROR_BAD_PARAM,"SETSTACK: user stack ptr > stack size",curr_cpu);
			return;
		}
		curr_cpu_p->user_stack=a1;
		curr_cpu_p->user_size=a2;
		curr_cpu_p->user_ptr=curr_cpu_p->uregs[0];
		return;
	} else
	if (t2 == TYPE_IMMPTR) {
		int a1,a2;
		UPTRVAL(a1,A1);
		IMMPTRVAL(a2,A2);
		if (curr_cpu_p->uregs[0] > a2) {
			non_fatal(ERROR_BAD_PARAM,"SETSTACK: user stack ptr > stack size",curr_cpu);
			return;
		}
		curr_cpu_p->user_stack=a1;
		curr_cpu_p->user_size=a2;
		curr_cpu_p->user_ptr=curr_cpu_p->uregs[0];
		return;
	} else
	if (t2 == TYPE_UPTR) {
		int a1,a2;
		UPTRVAL(a1,A1);
		UPTRVAL(a2,A2);
		if (curr_cpu_p->uregs[0] > a2) {
			non_fatal(ERROR_BAD_PARAM,"SETSTACK: user stack ptr > stack size",curr_cpu);
			return;
		}
		curr_cpu_p->user_stack=a1;
		curr_cpu_p->user_size=a2;
		curr_cpu_p->user_ptr=curr_cpu_p->uregs[0];
		return;
	} else
	non_fatal(ERROR_BAD_PARAM,"SETSTACK: Bad parameter #2",curr_cpu);
}

// cmd_push

void cmd_push_immediate () {

	if (!curr_cpu_p->user_size) {
		non_fatal(ERROR_NOUSTACK,"PUSH, but no user stack",curr_cpu);
		return;
	}

	if (curr_cpu_p->user_ptr>=curr_cpu_p->user_size) {
		non_fatal(ERROR_STACK_OVER,"Cannot push - stack overflow",curr_cpu);
		return;
	}

	set_mem_value(curr_cpu,curr_cpu_p->user_stack+curr_cpu_p->user_ptr,A1);
	curr_cpu_p->user_ptr++;
}

void cmd_push_ureg () {
  int a1;

	if (!curr_cpu_p->user_size) {
		non_fatal(ERROR_NOUSTACK,"PUSH, but no user stack",curr_cpu);
		return;
	}

	if (curr_cpu_p->user_ptr>=curr_cpu_p->user_size) {
		non_fatal(ERROR_STACK_OVER,"Cannot push - stack overflow",curr_cpu);
		return;
	}

	UREGVAL(a1,A1);
	set_mem_value(curr_cpu,curr_cpu_p->user_stack+curr_cpu_p->user_ptr,a1);
	curr_cpu_p->user_ptr++;
}

void cmd_push_sreg () {
  int a1;

	if (!curr_cpu_p->user_size) {
		non_fatal(ERROR_NOUSTACK,"PUSH, but no user stack",curr_cpu);
		return;
	}

	if (curr_cpu_p->user_ptr>=curr_cpu_p->user_size) {
		non_fatal(ERROR_STACK_OVER,"Cannot push - stack overflow",curr_cpu);
		return;
	}

	SREGVAL(a1,A1);
	set_mem_value(curr_cpu,curr_cpu_p->user_stack+curr_cpu_p->user_ptr,a1);
	curr_cpu_p->user_ptr++;
}

void cmd_push_freg () {
  int a1;

	if (!curr_cpu_p->user_size) {
		non_fatal(ERROR_NOUSTACK,"PUSH, but no user stack",curr_cpu);
		return;
	}

	if (curr_cpu_p->user_ptr>=curr_cpu_p->user_size) {
		non_fatal(ERROR_STACK_OVER,"Cannot push - stack overflow",curr_cpu);
		return;
	}

	FREGVAL(a1,A1);
	set_mem_value(curr_cpu,curr_cpu_p->user_stack+curr_cpu_p->user_ptr,a1);
	curr_cpu_p->user_ptr++;
}

void cmd_push_immptr () {
  int a1;

	if (!curr_cpu_p->user_size) {
		non_fatal(ERROR_NOUSTACK,"PUSH, but no user stack",curr_cpu);
		return;
	}

	if (curr_cpu_p->user_ptr>=curr_cpu_p->user_size) {
		non_fatal(ERROR_STACK_OVER,"Cannot push - stack overflow",curr_cpu);
		return;
	}

	IMMPTRVAL(a1,A1);
	set_mem_value(curr_cpu,curr_cpu_p->user_stack+curr_cpu_p->user_ptr,a1);
	curr_cpu_p->user_ptr++;
}

void cmd_push_uptr () {
  int a1;

	if (!curr_cpu_p->user_size) {
		non_fatal(ERROR_NOUSTACK,"PUSH, but no user stack",curr_cpu);
		return;
	}

	if (curr_cpu_p->user_ptr>=curr_cpu_p->user_size) {
		non_fatal(ERROR_STACK_OVER,"Cannot push - stack overflow",curr_cpu);
		return;
	}

	UPTRVAL(a1,A1);
	set_mem_value(curr_cpu,curr_cpu_p->user_stack+curr_cpu_p->user_ptr,a1);
	curr_cpu_p->user_ptr++;
}

// com_pop

void cmd_pop_ureg () {
  int work;

	if (!curr_cpu_p->user_size) {
		non_fatal(ERROR_NOUSTACK,"POP, but no user stack",curr_cpu);
		return;
	}

	if (curr_cpu_p->user_ptr<=0) {
		non_fatal(ERROR_STACK_OVER,"Cannot pop - stack underflow",curr_cpu);
		return;
	}

	curr_cpu_p->user_ptr--;
	work=get_mem_value(curr_cpu,curr_cpu_p->user_stack+curr_cpu_p->user_ptr);
	CHECK_EXCEPT;
	UREG(A1)=work;
}

void cmd_pop_sreg () {
  int work;

	if (!curr_cpu_p->user_size) {
		non_fatal(ERROR_NOUSTACK,"POP, but no user stack",curr_cpu);
		return;
	}

	if (curr_cpu_p->user_ptr<=0) {
		non_fatal(ERROR_STACK_OVER,"Cannot pop - stack underflow",curr_cpu);
		return;
	}

	curr_cpu_p->user_ptr--;
	work=get_mem_value(curr_cpu,curr_cpu_p->user_stack+curr_cpu_p->user_ptr);
	CHECK_EXCEPT;
	SREG(A1)=work;
}

void cmd_pop_freg () {
  int work;

	if (!curr_cpu_p->user_size) {
		non_fatal(ERROR_NOUSTACK,"POP, but no user stack",curr_cpu);
		return;
	}

	if (curr_cpu_p->user_ptr<=0) {
		non_fatal(ERROR_STACK_OVER,"Cannot pop - stack underflow",curr_cpu);
		return;
	}

	curr_cpu_p->user_ptr--;
	work=get_mem_value(curr_cpu,curr_cpu_p->user_stack+curr_cpu_p->user_ptr);
	CHECK_EXCEPT;
	FREG(A1)=work;
}

void cmd_pop_immptr () {
  int work;

	if (!curr_cpu_p->user_size) {
		non_fatal(ERROR_NOUSTACK,"POP, but no user stack",curr_cpu);
		return;
	}

	if (curr_cpu_p->user_ptr<=0) {
		non_fatal(ERROR_STACK_OVER,"Cannot pop - stack underflow",curr_cpu);
		return;
	}

	curr_cpu_p->user_ptr--;
	work=get_mem_value(curr_cpu,curr_cpu_p->user_stack+curr_cpu_p->user_ptr);
	CHECK_EXCEPT;
	set_mem_value(curr_cpu,A1,work);
}

void cmd_pop_uptr () {
  int work;

	if (!curr_cpu_p->user_size) {
		non_fatal(ERROR_NOUSTACK,"POP, but no user stack",curr_cpu);
		return;
	}

	if (curr_cpu_p->user_ptr<=0) {
		non_fatal(ERROR_STACK_OVER,"Cannot pop - stack underflow",curr_cpu);
		return;
	}

	curr_cpu_p->user_ptr--;
	work=get_mem_value(curr_cpu,curr_cpu_p->user_stack+curr_cpu_p->user_ptr);
	CHECK_EXCEPT;
	set_mem_value(curr_cpu,UREG(A1),work);
}

// cmd_invalid

void cmd_invalid () {

	non_fatal(ERROR_BAD_INSTR,"Incorrect opcode",curr_cpu);
}
