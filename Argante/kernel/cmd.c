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

#define CHECK_EXCEPT	if (got_nonfatal_round) return;

#define EVAL_A2_T2	t2=cpu[c].bytecode[cpu[c].IP*12+2]; \
			a2=*(((int*)&cpu[c].bytecode[cpu[c].IP*12+4])+1)

#define EVAL_T2		t2=cpu[c].bytecode[cpu[c].IP*12+2]

#define	EVAL_A1		a1=*((int*)&cpu[c].bytecode[cpu[c].IP*12+4])

#define	A1		*((int*)&cpu[c].bytecode[cpu[c].IP*12+4])

#define A2		*(((int*)&cpu[c].bytecode[cpu[c].IP*12+4])+1)

extern int change;
extern int got_nonfatal_round;

void cmd_nop (int c) {
	return;
}

// cmd_mov_ureg

void cmd_mov_ureg_immediate (int c) {

	UREG(A1)=A2;
}

void cmd_mov_ureg_ureg (int c) {

	UREG(A1)=UREG(A2);
}

void cmd_mov_ureg_sreg (int c) {

	UREG(A1)=SREG(A2);
}

void cmd_mov_ureg_freg (int c) {

	UREG(A1)=FFREG(A2);
}

void cmd_mov_ureg_immptr (int c) {
  int a2;
  	
	IMMPTRVAL(a2,A2);
	UREG(A1)=a2;
}

void cmd_mov_ureg_uptr (int c) {
  int a2;
	
	UPTRVAL(a2,A2);
	UREG(A1)=a2;
}


// cmd_mov_sreg

void cmd_mov_sreg_immediate (int c) {

	SREG(A1)=A2;
}

void cmd_mov_sreg_ureg (int c) {

	SREG(A1)=UREG(A2);
}

void cmd_mov_sreg_sreg (int c) {

	SREG(A1)=SREG(A2);
}

void cmd_mov_sreg_freg (int c) {

	SREG(A1)=FFREG(A2);
}

void cmd_mov_sreg_immptr (int c) {
  int a2;
  	
	IMMPTRVAL(a2,A2);
	SREG(A1)=a2;
}

void cmd_mov_sreg_uptr (int c) {
  int a2;
	
	UPTRVAL(a2,A2);
	SREG(A1)=a2;
}


// cmd_mov_immptr

void cmd_mov_immptr_immediate (int c) {
	
	set_mem_value(c,A1,A2);
}

void cmd_mov_immptr_ureg (int c) {

	set_mem_value(c,A1,UREG(A2));
}

void cmd_mov_immptr_sreg (int c) {
	
	set_mem_value(c,A1,SREG(A2));
}

void cmd_mov_immptr_freg (int c) {

	set_mem_value(c,A1,FREG(A2));
}

void cmd_mov_immptr_immptr (int c) {
  int a2;

  	IMMPTRVAL(a2,A2);
	set_mem_value(c,A1,a2);
}

void cmd_mov_immptr_uptr (int c) {
  int a2;
  	
	UPTRVAL(a2,A2);
	set_mem_value(c,A1,a2);
}


// cmd_mov_uptr

void cmd_mov_uptr_immediate (int c) {
	
	set_mem_value(c,UREG(A1),A2);
}

void cmd_mov_uptr_ureg (int c) {

	set_mem_value(c,UREG(A1),UREG(A2));
}

void cmd_mov_uptr_sreg (int c) {

	set_mem_value(c,UREG(A1),SREG(A2));
}

void cmd_mov_uptr_freg (int c) {

	set_mem_value(c,UREG(A1),FREG(A2));
}

void cmd_mov_uptr_immptr (int c) {
  int a2;

  	IMMPTRVAL(a2,A2);
	set_mem_value(c,UREG(A1),a2);
}

void cmd_mov_uptr_uptr (int c) {
  int a2;
  	
	UPTRVAL(a2,A2);
	set_mem_value(c,UREG(A1),a2);
}


// cmd_mov_freg

void cmd_mov_freg_immediate (int c) {
	FREG(A1)=*((int*)&A2);
}

void cmd_mov_freg_ureg (int c) {
	FFREG(A1)=UREG(A2);
}

void cmd_mov_freg_sreg (int c) {
	FFREG(A1)=SREG(A2);
}

void cmd_mov_freg_freg (int c) {
	FFREG(A1)=FFREG(A2);
}

void cmd_mov_freg_immptr (int c) {
  int a2;

	IMMPTRVAL(a2,A2);
	FFREG(A1)=*((float *)&a2);
}

void cmd_mov_freg_uptr (int c) {
  int a2;

	UPTRVAL(a2,A2);
	FFREG(A1)=*((float *)&a2);
}


// cmd_add_ureg

void cmd_add_ureg_immediate (int c) {

	UREG(A1)+=A2;
}

void cmd_add_ureg_ureg (int c) {

	UREG(A1)+=UREG(A2);
}

void cmd_add_ureg_sreg (int c) {

	UREG(A1)+=SREG(A2);
}

void cmd_add_ureg_freg (int c) {

	UREG(A1)+=FFREG(A2);
}

void cmd_add_ureg_immptr (int c) {
  int a2;

  	IMMPTRVAL(a2,A2);
	UREG(A1)+=a2;
}

void cmd_add_ureg_uptr (int c) {
  int a2;

  	UPTRVAL(a2,A2);
	UREG(A1)+=a2;
}


// cmd_add_sreg

void cmd_add_sreg_immediate (int c) {

	SREG(A1)+=A2;
}

void cmd_add_sreg_ureg (int c) {

	SREG(A1)+=UREG(A2);
}

void cmd_add_sreg_sreg (int c) {

	SREG(A1)+=SREG(A2);
}

void cmd_add_sreg_freg (int c) {

	SREG(A1)+=FFREG(A2);
}

void cmd_add_sreg_immptr (int c) {
  int a2;

	IMMPTRVAL(a2,A2);
	SREG(A1)+=a2;
}

void cmd_add_sreg_uptr (int c) {
  int a2;

	UPTRVAL(a2,A2);
	SREG(A1)+=a2;
}


// cmd_add_immptr

void cmd_add_immptr_immediate (int c) {
  int a1;
	
	EVAL_A1;
	set_mem_value(c,a1,A2+get_mem_value(c,a1));
}

void cmd_add_immptr_ureg (int c) {
  int a1;

  	EVAL_A1;
	set_mem_value(c,a1,UREG(A2)+get_mem_value(c,a1));
}

void cmd_add_immptr_sreg (int c) {
  int a1;

  	EVAL_A1;
	set_mem_value(c,a1,SREG(A2)+get_mem_value(c,a1));
}

void cmd_add_immptr_freg (int c) {
  int a1;
  float f;

  	EVAL_A1;
	*((int *)&f)=get_mem_value(c,a1);
	f+=FFREG(A2);
	set_mem_value(c,a1,*((int *)&f));
}

void cmd_add_immptr_immptr (int c) {
  int a1, a2;

  	IMMPTRVAL(a2,A2);
	EVAL_A1;
	set_mem_value(c,a1,a2+get_mem_value(c,a1));
}

void cmd_add_immptr_uptr (int c) {
  int a1, a2;

  	UPTRVAL(a2,A2);
	EVAL_A1;
	set_mem_value(c,a1,a2+get_mem_value(c,a1));
}


// cmd_add_uptr

void cmd_add_uptr_immediate (int c) {
  int a1;

	UREGVAL(a1,A1);
	set_mem_value(c,a1,A2+get_mem_value(c,a1));
}

void cmd_add_uptr_ureg (int c) {
  int a1;

  	UREGVAL(a1,A1);
	set_mem_value(c,a1,UREG(A2)+get_mem_value(c,a1));
}

void cmd_add_uptr_sreg (int c) {
  int a1;

  	UREGVAL(a1,A1);
	set_mem_value(c,a1,SREG(A2)+get_mem_value(c,a1));
}

void cmd_add_uptr_freg (int c) {
  int a1;
  float f;
  
  	UREGVAL(a1,A1);
	*((int *)&f)=get_mem_value(c,a1);
	f=FFREG(A2)+f;
	set_mem_value(c,a1,*((int *)&f));
}

void cmd_add_uptr_immptr (int c) {
  int a1, a2;

  	UREGVAL(a1,A1);
	IMMPTRVAL(a2,A2);
	set_mem_value(c,a1,a2+get_mem_value(c,a1));
}

void cmd_add_uptr_uptr (int c) {
  int a1, a2;

  	UREGVAL(a1,A1);
	UPTRVAL(a2,A2);
	set_mem_value(c,a1,a2+get_mem_value(c,a1));
}


// cmd_add_freg

void cmd_add_freg_immediate (int c) {

	FFREG(A1)+=*((float*)&A2);
}

void cmd_add_freg_ureg (int c) {

	FFREG(A1)+=UREG(A2);
}

void cmd_add_freg_sreg (int c) {

	FFREG(A1)+=SREG(A2);
}


void cmd_add_freg_freg (int c) {

	FFREG(A1)+=FFREG(A2);
}

void cmd_add_freg_immptr (int c) {
  int a2;

	IMMPTRVAL(a2,A2);
	FFREG(A1)+=*((float *)&a2);
}

void cmd_add_freg_uptr (int c) {
  int a2;

	UPTRVAL(a2,A2);
	FFREG(A1)+=*((float *)&a2);
}


// cmd_sub_ureg

void cmd_sub_ureg_immediate (int c) {

	UREG(A1)-=A2;
}

void cmd_sub_ureg_ureg (int c) {

	UREG(A1)-=UREG(A2);
}

void cmd_sub_ureg_sreg (int c) {

	UREG(A1)-=SREG(A2);
}

void cmd_sub_ureg_freg (int c) {

	UREG(A1)-=FFREG(A2);
}

void cmd_sub_ureg_immptr (int c) {
  int a2;

  	IMMPTRVAL(a2,A2);
	UREG(A1)-=a2;
}

void cmd_sub_ureg_uptr (int c) {
  int a2;
	
	UPTRVAL(a2,A2);
	UREG(A1)-=a2;
}


// cmd_sub_sreg

void cmd_sub_sreg_immediate (int c) {

	SREG(A1)-=A2;
}

void cmd_sub_sreg_ureg (int c) {

	SREG(A1)-=UREG(A2);
}

void cmd_sub_sreg_sreg (int c) {

	SREG(A1)-=SREG(A2);
}

void cmd_sub_sreg_freg (int c) {

	SREG(A1)-=FFREG(A2);
}

void cmd_sub_sreg_immptr (int c) {
  int a2;

  	IMMPTRVAL(a2,A2);
	SREG(A1)-=a2;
}

void cmd_sub_sreg_uptr (int c) {
  int a2;
	
	UPTRVAL(a2,A2);
	SREG(A1)-=a2;
}


// cmd_sub_immptr

void cmd_sub_immptr_immediate (int c) {
  int a1;
	
	EVAL_A1;
	set_mem_value(c,a1,get_mem_value(c,a1)-A2);
}

void cmd_sub_immptr_ureg (int c) {
  int a1;

  	EVAL_A1;
	set_mem_value(c,a1,get_mem_value(c,a1)-UREG(A2));
}

void cmd_sub_immptr_sreg (int c) {
  int a1;

  	EVAL_A1;
	set_mem_value(c,a1,get_mem_value(c,a1)-SREG(A2));
}

void cmd_sub_immptr_freg (int c) {
  int a1;
  float f;

  	EVAL_A1;
	*((int *)&f)=get_mem_value(c,a1);
	f-=FFREG(A2);
	set_mem_value(c,a1,*((int *)&f));
}

void cmd_sub_immptr_immptr (int c) {
  int a1, a2;

  	IMMPTRVAL(a2,A2);
	EVAL_A1;
	set_mem_value(c,a1,get_mem_value(c,a1)-a2);
}

void cmd_sub_immptr_uptr (int c) {
  int a1, a2;

  	UPTRVAL(a2,A2);
	EVAL_A1;
	set_mem_value(c,a1,get_mem_value(c,a1)-a2);
}


// cmd_sub_uptr

void cmd_sub_uptr_immediate (int c) {
  int a1;

	UREGVAL(a1,A1);
	set_mem_value(c,a1,get_mem_value(c,a1)-A2);
}

void cmd_sub_uptr_ureg (int c) {
  int a1;

	UREGVAL(a1,A1);
	set_mem_value(c,a1,get_mem_value(c,a1)-UREG(A2));
}

void cmd_sub_uptr_sreg (int c) {
  int a1;

  	UREGVAL(a1,A1);
	set_mem_value(c,a1,get_mem_value(c,a1)-SREG(A2));
}

void cmd_sub_uptr_freg (int c) {
  int a1;
  float f;

	UREGVAL(a1,A1);
	*((int *)&f)=get_mem_value(c,a1);
	f-=FFREG(A2);
	set_mem_value(c,a1,*((int *)&f));
}

void cmd_sub_uptr_immptr (int c) {
  int a1,a2;

  	IMMPTRVAL(a2,A2);
	UREGVAL(a1,A1);
	set_mem_value(c,a1,get_mem_value(c,a1)-a2);
}

void cmd_sub_uptr_uptr (int c) {
  int a1,a2;
  
  	UPTRVAL(a2,A2);
	UREGVAL(a1,A1);
	set_mem_value(c,a1,get_mem_value(c,a1)-a2);
}


// cmd_sub_freg

void cmd_sub_freg (int c) {
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
           non_fatal(ERROR_BAD_PARAM,"SUB: Bad parameter #2",c);
           return;
         }

	if ((t2 == TYPE_FREG) || (t2==TYPE_IMMEDIATE) || (t2==TYPE_IMMPTR)) {
		FREG(A1)-=*((float *)&work2);
	} else {
		FREG(A1)-=work2;
	}
}


// cmd_mul

void cmd_mul_ureg (int c) {
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
           non_fatal(ERROR_BAD_PARAM,"MUL: Bad parameter #2",c);
           return;
         }

         UREG(A1)*=work2;
}


void cmd_mul_sreg (int c) {
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
           non_fatal(ERROR_BAD_PARAM,"MUL: Bad parameter #2",c);
           return;
         }

        SREG(A1)*=work2;
}


void cmd_mul_immptr (int c) {
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
           non_fatal(ERROR_BAD_PARAM,"MUL: Bad parameter #2",c);
	   return;
         }

	 EVAL_A1;
	 set_mem_value(c,a1,work2*get_mem_value(c,a1));
}


void cmd_mul_uptr (int c) {
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
           non_fatal(ERROR_BAD_PARAM,"MUL: Bad parameter #2",c);
           return;
         }

	 EVAL_A1;
	 set_mem_value(c,UREG(a1),work2*get_mem_value(c,UREG(a1)));
}


void cmd_mul_freg (int c) {
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
           non_fatal(ERROR_BAD_PARAM,"MUL: Bad parameter #2",c);
           return;
         }

	if ((t2 == TYPE_FREG) || (t2==TYPE_IMMEDIATE) || (t2==TYPE_IMMPTR)) {
		FREG(A1)*=*((float *)&work2);
	} else {
		FREG(A1)*=work2;
	}
}


// cmd_xor

void cmd_xor_ureg (int c) {
  int work2;
  int a2;
  int t2;

    EVAL_A2_T2;
         if (t2 == TYPE_IMMEDIATE) { work2=a2; } else
         if (t2 == TYPE_UREG) { UREGVAL(work2,a2) } else
         if (t2 == TYPE_SREG) { SREGVAL(work2,a2) } else
         if (t2 == TYPE_IMMPTR) { IMMPTRVAL(work2,a2) } else
         if (t2 == TYPE_UPTR)  { UPTRVAL(work2,a2) } else {
           non_fatal(ERROR_BAD_PARAM,"XOR: Bad parameter #2",c);
           return;
         }

	UREG(A1)^=work2;
}


void cmd_xor_sreg (int c) {
  int work2;
  int a2;
  int t2;

	EVAL_A2_T2;
         if (t2 == TYPE_IMMEDIATE) { work2=a2; } else
         if (t2 == TYPE_UREG) { UREGVAL(work2,a2) } else
         if (t2 == TYPE_SREG) { SREGVAL(work2,a2) } else
         if (t2 == TYPE_IMMPTR) { IMMPTRVAL(work2,a2) } else
         if (t2 == TYPE_UPTR)  { UPTRVAL(work2,a2) } else {
           non_fatal(ERROR_BAD_PARAM,"XOR: Bad parameter #2",c);
           return;
         }

	SREG(A1)^=work2; 
}


void cmd_xor_immptr (int c) {
  int work2;
  int a1,a2;
  int t2;

	EVAL_A2_T2;
         if (t2 == TYPE_IMMEDIATE) { work2=a2; } else
         if (t2 == TYPE_UREG) { UREGVAL(work2,a2) } else
         if (t2 == TYPE_SREG) { SREGVAL(work2,a2) } else
         if (t2 == TYPE_IMMPTR) { IMMPTRVAL(work2,a2) } else
         if (t2 == TYPE_UPTR)  { UPTRVAL(work2,a2) } else {
           non_fatal(ERROR_BAD_PARAM,"XOR: Bad parameter #2",c);
	   return;
         }

	 EVAL_A1;
	 set_mem_value(c,a1,get_mem_value(c,a1) ^ work2);
}


void cmd_xor_uptr (int c) {
  int work2;
  int a1,a2;
  int t2;

	EVAL_A2_T2;
         if (t2 == TYPE_IMMEDIATE) { work2=a2; } else
         if (t2 == TYPE_UREG) { UREGVAL(work2,a2) } else
         if (t2 == TYPE_SREG) { SREGVAL(work2,a2) } else
         if (t2 == TYPE_IMMPTR) { IMMPTRVAL(work2,a2) } else
         if (t2 == TYPE_UPTR)  { UPTRVAL(work2,a2) } else {
           non_fatal(ERROR_BAD_PARAM,"XOR: Bad parameter #2",c);
           return;
         }

	 EVAL_A1;
	 set_mem_value(c,UREG(a1),get_mem_value(c,UREG(a1)) ^ work2);
}


// cmd_or

void cmd_or_ureg (int c) {
  int work2;
  int a2;
  int t2;

    EVAL_A2_T2;
         if (t2 == TYPE_IMMEDIATE) { work2=a2; } else
         if (t2 == TYPE_UREG) { UREGVAL(work2,a2) } else
         if (t2 == TYPE_SREG) { SREGVAL(work2,a2) } else
         if (t2 == TYPE_IMMPTR) { IMMPTRVAL(work2,a2) } else
         if (t2 == TYPE_UPTR)  { UPTRVAL(work2,a2) } else {
           non_fatal(ERROR_BAD_PARAM,"OR: Bad parameter #2",c);
           return;
         }

	UREG(A1)|=work2;
}


void cmd_or_sreg (int c) {
  int work2;
  int a2;
  int t2;

	EVAL_A2_T2;
         if (t2 == TYPE_IMMEDIATE) { work2=a2; } else
         if (t2 == TYPE_UREG) { UREGVAL(work2,a2) } else
         if (t2 == TYPE_SREG) { SREGVAL(work2,a2) } else
         if (t2 == TYPE_IMMPTR) { IMMPTRVAL(work2,a2) } else
         if (t2 == TYPE_UPTR)  { UPTRVAL(work2,a2) } else {
           non_fatal(ERROR_BAD_PARAM,"OR: Bad parameter #2",c);
           return;
         }

	SREG(A1)|=work2;
}


void cmd_or_immptr (int c) {
  int work2;
  int a1,a2;
  int t2;


	EVAL_A2_T2;
         if (t2 == TYPE_IMMEDIATE) { work2=a2; } else
         if (t2 == TYPE_UREG) { UREGVAL(work2,a2) } else
         if (t2 == TYPE_SREG) { SREGVAL(work2,a2) } else
         if (t2 == TYPE_IMMPTR) { IMMPTRVAL(work2,a2) } else
         if (t2 == TYPE_UPTR)  { UPTRVAL(work2,a2) } else {
           non_fatal(ERROR_BAD_PARAM,"OR: Bad parameter #2",c);
	   return;
         }

	 EVAL_A1;
	 set_mem_value(c,a1,get_mem_value(c,a1) | work2);
}


void cmd_or_uptr (int c) {
  int work2;
  int a1,a2;
  int t2;

	EVAL_A2_T2;
         if (t2 == TYPE_IMMEDIATE) { work2=a2; } else
         if (t2 == TYPE_UREG) { UREGVAL(work2,a2) } else
         if (t2 == TYPE_SREG) { SREGVAL(work2,a2) } else
         if (t2 == TYPE_IMMPTR) { IMMPTRVAL(work2,a2) } else
         if (t2 == TYPE_UPTR)  { UPTRVAL(work2,a2) } else {
           non_fatal(ERROR_BAD_PARAM,"OR: Bad parameter #2",c);
           return;
         }

	 EVAL_A1;
	 set_mem_value(c,UREG(a1),get_mem_value(c,UREG(a1)) | work2);
}


// cmd_and

void cmd_and_ureg (int c) {
  int work2;
  int a2;
  int t2;

	EVAL_A2_T2;
         if (t2 == TYPE_IMMEDIATE) { work2=a2; } else
         if (t2 == TYPE_UREG) { UREGVAL(work2,a2) } else
         if (t2 == TYPE_SREG) { SREGVAL(work2,a2) } else
         if (t2 == TYPE_IMMPTR) { IMMPTRVAL(work2,a2) } else
         if (t2 == TYPE_UPTR)  { UPTRVAL(work2,a2) } else {
           non_fatal(ERROR_BAD_PARAM,"AND: Bad parameter #2",c);
           return;
         }

	UREG(A1)&=work2;
}


void cmd_and_sreg (int c) {
  int work2;
  int a2;
  int t2;

	EVAL_A2_T2;
         if (t2 == TYPE_IMMEDIATE) { work2=a2; } else
         if (t2 == TYPE_UREG) { UREGVAL(work2,a2) } else
         if (t2 == TYPE_SREG) { SREGVAL(work2,a2) } else
         if (t2 == TYPE_IMMPTR) { IMMPTRVAL(work2,a2) } else
         if (t2 == TYPE_UPTR)  { UPTRVAL(work2,a2) } else {
           non_fatal(ERROR_BAD_PARAM,"AND: Bad parameter #2",c);
           return;
         }

	SREG(A1)&=work2;
}


void cmd_and_immptr (int c) {
  int work2;
  int a1,a2;
  int t2;


	EVAL_A2_T2;
         if (t2 == TYPE_IMMEDIATE) { work2=a2; } else
         if (t2 == TYPE_UREG) { UREGVAL(work2,a2) } else
         if (t2 == TYPE_SREG) { SREGVAL(work2,a2) } else
         if (t2 == TYPE_IMMPTR) { IMMPTRVAL(work2,a2) } else
         if (t2 == TYPE_UPTR)  { UPTRVAL(work2,a2) } else {
           non_fatal(ERROR_BAD_PARAM,"AND: Bad parameter #2",c);
	   return;
         }

	 EVAL_A1;
	 set_mem_value(c,a1,get_mem_value(c,a1) & work2);
}


void cmd_and_uptr (int c) {
  int work2;
  int a1,a2;
  int t2;

	EVAL_A2_T2;
         if (t2 == TYPE_IMMEDIATE) { work2=a2; } else
         if (t2 == TYPE_UREG) { UREGVAL(work2,a2) } else
         if (t2 == TYPE_SREG) { SREGVAL(work2,a2) } else
         if (t2 == TYPE_IMMPTR) { IMMPTRVAL(work2,a2) } else
         if (t2 == TYPE_UPTR)  { UPTRVAL(work2,a2) } else {
           non_fatal(ERROR_BAD_PARAM,"AND: Bad parameter #2",c);
           return;
         }

	 EVAL_A1;
	 set_mem_value(c,UREG(a1),get_mem_value(c,UREG(a1)) & work2);
}


// cmd_div

void cmd_div_ureg (int c) {
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
           non_fatal(ERROR_BAD_PARAM,"DIV: Bad parameter #2",c);
           return;
         }

	 if (!work2) {
		 non_fatal(ERROR_BAD_PARAM,"DIV: Division by zero",c);
		 return;
	 }
	UREG(A1)/=work2;
}


void cmd_div_sreg (int c) {
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
           non_fatal(ERROR_BAD_PARAM,"DIV: Bad parameter #2",c);
           return;
         }

	 if (!work2) {
		 non_fatal(ERROR_BAD_PARAM,"DIV: Division by zero",c);
		 return;
	 }
	SREG(A1)/=work2;
}


void cmd_div_immptr (int c) {
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
           non_fatal(ERROR_BAD_PARAM,"DIV: Bad parameter #2",c);
	   return;
         }

	 if (!work2) {
		 non_fatal(ERROR_BAD_PARAM,"DIV: Division by zero",c);
		 return;
	 }
	 EVAL_A1;
	 set_mem_value(c,a1,get_mem_value(c,a1)/work2);
}


void cmd_div_uptr (int c) {
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
           non_fatal(ERROR_BAD_PARAM,"DIV: Bad parameter #2",c);
           return;
         }

	 if (!work2) {
		 non_fatal(ERROR_BAD_PARAM,"DIV: Division by zero",c);
		 return;
	 }
	 EVAL_A1;
	 set_mem_value(c,UREG(a1),get_mem_value(c,UREG(a1))/work2);
}


void cmd_div_freg (int c) {
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
           non_fatal(ERROR_BAD_PARAM,"DIV: Bad parameter #2",c);
           return;
         }

	 if (!work2) {
		 non_fatal(ERROR_BAD_PARAM,"DIV: Division by zero",c);
		 return;
	 }

	if ((t2 == TYPE_FREG) || (t2==TYPE_IMMEDIATE) || (t2==TYPE_IMMPTR)) {
		FREG(A1)/=*((float *)&work2);
	} else {
		FREG(A1)/=work2;
	}
}


// cmd_mod

void cmd_mod_ureg (int c) {
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
           non_fatal(ERROR_BAD_PARAM,"MOD: Bad parameter #2",c);
           return;
         }

	 if (!work2) {
		 non_fatal(ERROR_BAD_PARAM,"MOD: Division by zero",c);
		 return;
	 }
	UREG(A1)%=work2;
}


void cmd_mod_sreg (int c) {
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
           non_fatal(ERROR_BAD_PARAM,"MOD: Bad parameter #2",c);
           return;
         }

	 if (!work2) {
		 non_fatal(ERROR_BAD_PARAM,"MOD: Division by zero",c);
		 return;
	 }
	 SREG(A1)%=work2;
}


void cmd_mod_immptr (int c) {
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
           non_fatal(ERROR_BAD_PARAM,"MOD: Bad parameter #2",c);
	   return;
         }

	 if (!work2) {
		 non_fatal(ERROR_BAD_PARAM,"MOD: Division by zero",c);
		 return;
	 }
	 EVAL_A1;
	 set_mem_value(c,a1,get_mem_value(c,a1)%work2);
}


void cmd_mod_uptr (int c) {
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
           non_fatal(ERROR_BAD_PARAM,"MOD: Bad parameter #2",c);
           return;
         }

	 if (!work2) {
		 non_fatal(ERROR_BAD_PARAM,"MOD: Division by zero",c);
		 return;
	 }
	 EVAL_A1;
	 set_mem_value(c,UREG(a1),get_mem_value(c,UREG(a1))%work2);
}

// cmd_jmp

void cmd_jmp_immediate (int c) {

	cpu[c].IP=A1;
	change=0;
}

void cmd_jmp_ureg (int c) {

	cpu[c].IP=UREG(A1);
	change=0;
}

void cmd_jmp_immptr (int c) {
  int work;

	IMMPTRVAL(work,A1);
	cpu[c].IP=work;
	change=0;
}

void cmd_jmp_uptr (int c) {
  int work;

	UPTRVAL(work,A1);
	cpu[c].IP=work; 
	change=0;
}

// cmd_call

void cmd_call_immediate (int c) {

	push_on_stack(c,cpu[c].IP);
	CHECK_EXCEPT;
	cpu[c].IP=A1;
	change=0;
}

void cmd_call_ureg (int c) {

	push_on_stack(c,cpu[c].IP);
	CHECK_EXCEPT;
	cpu[c].IP=UREG(A1);
	change=0;
}

void cmd_call_immptr (int c) {
  int work;
	
	IMMPTRVAL(work,A1);
	push_on_stack(c,cpu[c].IP);
	CHECK_EXCEPT;
	cpu[c].IP=work;
	change=0;
}

void cmd_call_uptr (int c) {
  int work;

	UPTRVAL(work,A1);
	push_on_stack(c,cpu[c].IP);
	CHECK_EXCEPT;
	cpu[c].IP=work; 
	change=0;
}

// cmd_loop

void cmd_loop_immediate (int c) {

	if (cpu[c].sregs[0]>0) {
		cpu[c].sregs[0]--;
		cpu[c].IP=A1;
                change=0;
	}
}

void cmd_loop_ureg (int c) {

		if (cpu[c].sregs[0]>0) {
			cpu[c].sregs[0]--;
			cpu[c].IP=UREG(A1);
                        change=0;
		} 
}

void cmd_loop_immptr (int c) {
  int work;
	
	if (cpu[c].sregs[0]>0) {
		IMMPTRVAL(work,A1);
		cpu[c].sregs[0]--;
		cpu[c].IP=work;
                change=0;
	}
}

void cmd_loop_uptr (int c) {
  int work;

		if (cpu[c].sregs[0]>0) {
			UPTRVAL(work,A1);
			cpu[c].sregs[0]--;
			cpu[c].IP=work;
                        change=0;
		}
}

// cmd_onfail

void cmd_onfail_immediate (int c) {

	(*cpu[c].ex_st)[cpu[c].stack_ptr]=A1;
}

void cmd_onfail_ureg (int c) {

		(*cpu[c].ex_st)[cpu[c].stack_ptr]=UREG(A1);
}

void cmd_onfail_immptr (int c) {
  int work;
  	
	IMMPTRVAL(work,A1);
	(*cpu[c].ex_st)[cpu[c].stack_ptr]=work;

}

void cmd_onfail_uptr (int c) {
  int work;
	
		UPTRVAL(work,A1);
		(*cpu[c].ex_st)[cpu[c].stack_ptr]=work;
}

// cmd_syscall

void cmd_syscall_immediate (int c) {

	do_syscall(c,A1);

}

void cmd_syscall_ureg (int c) {

		do_syscall(c,UREG(A1));
}

void cmd_syscall_immptr (int c) {
  int work;
  	
	IMMPTRVAL(work,A1);
	do_syscall(c,work);
}

void cmd_syscall_uptr (int c) {
  int work;

		UPTRVAL(work,A1);
		do_syscall(c,work);
}

// cmd_ret

void cmd_ret_immediate (int c) {
  int a1,work;

  	EVAL_A1;

	if (!a1) a1=1;
	if (a1>MAX_STACK) {
		non_fatal(ERROR_BAD_PARAM,"RET: Bad parameter",c);
		return;
	}
	// Lemme guess - it's buggy!
	cpu[c].in_handler-=a1;
	if (!cpu[c].in_handler) cpu[c].uregs[0]=cpu[c].u0_saved;
	if (cpu[c].in_handler<0) cpu[c].in_handler=0;

	for (work=0;work<a1;work++) {
		cpu[c].IP=pop_from_stack(c)+1;
		change=0;

		if (cpu[c].handling_failure && (cpu[c].IP-1==cpu[c].first_except_ip)) {
				cpu[c].handling_failure=0;
		}
	}
}


void cmd_ret_ureg (int c) {
  int a1,work;

	a1=UREG(A1);

	if (!a1) a1=1;
	if (a1>MAX_STACK) {
		non_fatal(ERROR_BAD_PARAM,"RET: Bad parameter",c);
		return;
	}
	// Lemme guess - it's buggy!
	cpu[c].in_handler-=a1;
	if (!cpu[c].in_handler) cpu[c].uregs[0]=cpu[c].u0_saved;
	if (cpu[c].in_handler<0) cpu[c].in_handler=0;

	for (work=0;work<a1;work++) {
		cpu[c].IP=pop_from_stack(c)+1;
		change=0;

		if (cpu[c].handling_failure && (cpu[c].IP-1==cpu[c].first_except_ip)) {
				cpu[c].handling_failure=0;
		}
	}
}

void cmd_ret_immptr (int c) {
  int a1,work;

	IMMPTRVAL(a1,A1);

	if (!a1) a1=1;
	if (a1>MAX_STACK) {
		non_fatal(ERROR_BAD_PARAM,"RET: Bad parameter",c);
		return;
	}
	// Lemme guess - it's buggy!
	cpu[c].in_handler-=a1;
	if (!cpu[c].in_handler) cpu[c].uregs[0]=cpu[c].u0_saved;
	if (cpu[c].in_handler<0) cpu[c].in_handler=0;

	for (work=0;work<a1;work++) {
		cpu[c].IP=pop_from_stack(c)+1;
		change=0;

		if (cpu[c].handling_failure && (cpu[c].IP-1==cpu[c].first_except_ip)) {
				cpu[c].handling_failure=0;
		}
	}
}

void cmd_ret_uptr (int c) {
  int a1,work;

	UPTRVAL(a1,A1);

	if (!a1) a1=1;
	if (a1>MAX_STACK) {
		non_fatal(ERROR_BAD_PARAM,"RET: Bad parameter",c);
		return;
	}
	// Lemme guess - it's buggy!

	cpu[c].in_handler-=a1;
	if (!cpu[c].in_handler) cpu[c].uregs[0]=cpu[c].u0_saved;
	if (cpu[c].in_handler<0) cpu[c].in_handler=0;

	for (work=0;work<a1;work++) {
		cpu[c].IP=pop_from_stack(c)+1;
		change=0;

		if (cpu[c].handling_failure && (cpu[c].IP-1==cpu[c].first_except_ip)) {
				cpu[c].handling_failure=0;
		}
	}
}

// cmd_raise

void cmd_raise_immediate (int c) {

	non_fatal(A1,"RAISE: User-raised exception",c);
}

void cmd_raise_ureg (int c) {

	non_fatal(UREG(A1),"RAISE: User-raised exception",c);
}

void cmd_raise_immptr (int c) {
  int work;

  	IMMPTRVAL(work,A1);
	non_fatal(work,"RAISE: User-raised exception",c);
}

void cmd_raise_uptr (int c) {
  int work;

  	UPTRVAL(work,A1);
	non_fatal(work,"RAISE: User-raised exception",c);
}

// cmd_ifeq

void cmd_ifeq_immediate (int c) {
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
           non_fatal(ERROR_BAD_PARAM,"IFEQ: Bad parameter #2",c);
           return;
}

void cmd_ifeq_ureg (int c) {
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
           non_fatal(ERROR_BAD_PARAM,"IFEQ: Bad parameter #2",c);
           return;
}

void cmd_ifeq_sreg (int c) {
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
           non_fatal(ERROR_BAD_PARAM,"IFEQ: Bad parameter #2",c);
           return;
}

void cmd_ifeq_freg (int c) {
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
           non_fatal(ERROR_BAD_PARAM,"IFEQ: Bad parameter #2",c);
           return;
}


void cmd_ifeq_immptr (int c) {
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
           non_fatal(ERROR_BAD_PARAM,"IFEQ: Bad parameter #2",c);
           return;
}

void cmd_ifeq_uptr (int c) {
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
           non_fatal(ERROR_BAD_PARAM,"IFEQ: Bad parameter #2",c);
           return;
}

// cmd_ifneq

void cmd_ifneq_immediate (int c) {
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
           non_fatal(ERROR_BAD_PARAM,"IFNEQ: Bad parameter #2",c);
           return;
}

void cmd_ifneq_ureg (int c) {
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
           non_fatal(ERROR_BAD_PARAM,"IFNEQ: Bad parameter #2",c);
           return;
}

void cmd_ifneq_sreg (int c) {
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
           non_fatal(ERROR_BAD_PARAM,"IFNEQ: Bad parameter #2",c);
           return;
}

void cmd_ifneq_freg (int c) {
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
           non_fatal(ERROR_BAD_PARAM,"IFNEQ: Bad parameter #2",c);
           return;
}


void cmd_ifneq_immptr (int c) {
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
           non_fatal(ERROR_BAD_PARAM,"IFNEQ: Bad parameter #2",c);
           return;
}

void cmd_ifneq_uptr (int c) {
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
           non_fatal(ERROR_BAD_PARAM,"IFNEQ: Bad parameter #2",c);
           return;
}

// cmd_ifabo

void cmd_ifabo_immediate (int c) {
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
           non_fatal(ERROR_BAD_PARAM,"IFABO: Bad parameter #2",c);
           return;
}

void cmd_ifabo_ureg (int c) {
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
           non_fatal(ERROR_BAD_PARAM,"IFABO: Bad parameter #2",c);
           return;
}

void cmd_ifabo_sreg (int c) {
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
           non_fatal(ERROR_BAD_PARAM,"IFABO: Bad parameter #2",c);
           return;
}

void cmd_ifabo_freg (int c) {
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
           non_fatal(ERROR_BAD_PARAM,"IFABO: Bad parameter #2",c);
           return;
}


void cmd_ifabo_immptr (int c) {
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
           non_fatal(ERROR_BAD_PARAM,"IFABO: Bad parameter #2",c);
           return;
}

void cmd_ifabo_uptr (int c) {
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
           non_fatal(ERROR_BAD_PARAM,"IFABO: Bad parameter #2",c);
           return;
}

// cmd_ifbel

void cmd_ifbel_immediate (int c) {
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
           non_fatal(ERROR_BAD_PARAM,"IFBEL: Bad parameter #2",c);
           return;
}

void cmd_ifbel_ureg (int c) {
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
           non_fatal(ERROR_BAD_PARAM,"IFBEL: Bad parameter #2",c);
           return;
}

void cmd_ifbel_sreg (int c) {
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
           non_fatal(ERROR_BAD_PARAM,"IFBEL: Bad parameter #2",c);
           return;
}

void cmd_ifbel_freg (int c) {
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
           non_fatal(ERROR_BAD_PARAM,"IFBEL: Bad parameter #2",c);
           return;
}


void cmd_ifbel_immptr (int c) {
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
           non_fatal(ERROR_BAD_PARAM,"IFBEL: Bad parameter #2",c);
           return;
}

void cmd_ifbel_uptr (int c) {
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
           non_fatal(ERROR_BAD_PARAM,"IFBEL: Bad parameter #2",c);
           return;
}

// cmd_halt

void cmd_halt (int c) {

	printk("=> Task #%d commited suicide (HALT command).\n",c);
	destroy_task_respawn(c);
	killed_proc--; exit_proc++;
}

// cmd_nofail

void cmd_nofail (int c) {

	(*cpu[c].ex_st)[cpu[c].stack_ptr]=0;
}

// cmd_not

void cmd_not_ureg (int c) {

	UREG(A1)^=0xffffffff;
}

void cmd_not_sreg (int c) {

	SREG(A1)^=0xffffffff;
}

void cmd_not_immptr (int c) {
  int a1;

	EVAL_A1;
	set_mem_value(c,a1,get_mem_value(c,a1)^0xffffffff);
}

void cmd_not_uptr (int c) {
  int a1;

  	EVAL_A1;
	set_mem_value(c,UREG(a1),get_mem_value(c,UREG(a1)));
}

// cmd_sleepfor

void cmd_sleepfor_immediate (int c) {

	cpu[c].state |= VCPU_STATE_SLEEPFOR;
	cpu[c].cnt_down = A1;
}

void cmd_sleepfor_ureg (int c) {

	cpu[c].state |= VCPU_STATE_SLEEPFOR;
	cpu[c].cnt_down = UREG(A1);
}

void cmd_sleepfor_sreg (int c) {

	cpu[c].state |= VCPU_STATE_SLEEPFOR;
	cpu[c].cnt_down = SREG(A1);
}

void cmd_sleepfor_freg (int c) {

	cpu[c].state |= VCPU_STATE_SLEEPFOR;
	cpu[c].cnt_down = FREG(A1);
}


void cmd_sleepfor_immptr (int c) {
  int work;

  	IMMPTRVAL(work,A1);
	cpu[c].state |= VCPU_STATE_SLEEPFOR;
	cpu[c].cnt_down = work;
}

void cmd_sleepfor_uptr (int c) {
  int work;

  	UPTRVAL(work,A1);
	cpu[c].state |= VCPU_STATE_SLEEPFOR;
	cpu[c].cnt_down = work;
}

// cmd_waittill

void cmd_waittill_immediate (int c) {
  struct timezone tz; struct timeval tv;

	gettimeofday(&tv,&tz);
	cpu[c].state |= VCPU_STATE_SLEEPUNTIL;
	cpu[c].wake_on=tv.tv_sec*1000000+tv.tv_usec+A1;
}

void cmd_waittill_ureg (int c) {
  struct timezone tz; struct timeval tv;

	gettimeofday(&tv,&tz);
	cpu[c].state |= VCPU_STATE_SLEEPUNTIL;
	cpu[c].wake_on=tv.tv_sec*1000000+tv.tv_usec+UREG(A1);
}

void cmd_waittill_sreg (int c) {
  struct timezone tz; struct timeval tv;

	gettimeofday(&tv,&tz);
	cpu[c].state |= VCPU_STATE_SLEEPUNTIL;
	cpu[c].wake_on=tv.tv_sec*1000000+tv.tv_usec+SREG(A1);
}

void cmd_waittill_freg (int c) {
  struct timezone tz; struct timeval tv;

	gettimeofday(&tv,&tz);
	cpu[c].state |= VCPU_STATE_SLEEPUNTIL;
	cpu[c].wake_on=tv.tv_sec*1000000+tv.tv_usec+FREG(A1);
}


void cmd_waittill_immptr (int c) {
  struct timezone tz; struct timeval tv;
  int work;

  	IMMPTRVAL(work,A1);
	gettimeofday(&tv,&tz);
	cpu[c].state |= VCPU_STATE_SLEEPUNTIL;
	cpu[c].wake_on=tv.tv_sec*1000000+tv.tv_usec+work;
}

void cmd_waittill_uptr (int c) {
  struct timezone tz; struct timeval tv;
  int work;

  	UPTRVAL(work,A1);
	gettimeofday(&tv,&tz);
	cpu[c].state |= VCPU_STATE_SLEEPUNTIL;
	cpu[c].wake_on=tv.tv_sec*1000000+tv.tv_usec+work;
}

// cmd_ldb

void cmd_ldb_ureg (int c) {
  int t2;

	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) {
		UREG(A1)=(cpu[c].bytecode[cpu[c].IP*12+8+(SREG(0)&3)]);
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
	non_fatal(ERROR_BAD_PARAM,"LDB: Bad parameter #2",c);
}

void cmd_ldb_sreg (int c) {
  int t2;

	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) {
		SREG(A1)=(cpu[c].bytecode[cpu[c].IP*12+8+(SREG(0)&3)]);
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
	non_fatal(ERROR_BAD_PARAM,"LDB: Bad parameter #2",c);
}

void cmd_ldb_freg (int c) {
  int t2;

	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) {
		FREG(A1)=(cpu[c].bytecode[cpu[c].IP*12+8+(SREG(0)&3)]);
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
	non_fatal(ERROR_BAD_PARAM,"LDB: Bad parameter #2",c);
}

void cmd_ldb_immptr (int c) {
  int t2;

	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) {
		set_mem_value(c,A1,(cpu[c].bytecode[cpu[c].IP*12+8+(SREG(0)&3)]));
		return;
	} else 
	if (t2 == TYPE_UREG) {
		set_mem_value(c,A1,*(((char*)&(UREG(A2)))+(SREG(0)&3)));
		return;
	} else
	if (t2 == TYPE_SREG) {
		set_mem_value(c,A1,*(((char*)&(SREG(A2)))+(SREG(0)&3)));
	} else 
	if (t2 == TYPE_IMMPTR) {
		int work;
		IMMPTRVAL(work, (A2+(SREG(0)>>2)));
		set_mem_value(c,A1,*(((char*)&work)+(SREG(0)&3)));
	} else
	if (t2 == TYPE_UPTR) {
		int work;
		// BLACK MAGIC!!! :)))
		IMMPTRVAL(work, ((UREG(A2))+(SREG(0)>>2)));
		set_mem_value(c,A1,*(((char*)&work)+(SREG(0)&3)));
	} else \
	non_fatal(ERROR_BAD_PARAM,"LDB: Bad parameter #2",c);
}

void cmd_ldb_uptr (int c) {
  int t2;

	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) {
		set_mem_value(c,UREG(A1),(cpu[c].bytecode[cpu[c].IP*12+8+(SREG(0)&3)]));
		return;
	} else 
	if (t2 == TYPE_UREG) {
		set_mem_value(c,UREG(A1),*(((char*)&(UREG(A2)))+(SREG(0)&3)));
		return;
	} else
	if (t2 == TYPE_SREG) {
		set_mem_value(c,UREG(A1),*(((char*)&(SREG(A2)))+(SREG(0)&3)));
	} else 
	if (t2 == TYPE_IMMPTR) {
		int work;
		IMMPTRVAL(work, (A2+(SREG(0)>>2)));
		set_mem_value(c,UREG(A1),*(((char*)&work)+(SREG(0)&3)));
	} else
	if (t2 == TYPE_UPTR) {
		int work;
		// BLACK MAGIC!!! :)))
		IMMPTRVAL(work, ((UREG(A2))+(SREG(0)>>2)));
		set_mem_value(c,UREG(A1),*(((char*)&work)+(SREG(0)&3)));
	} else \
	non_fatal(ERROR_BAD_PARAM,"LDB: Bad parameter #2",c);
}

// cmd_stob_ureg

void cmd_stob_ureg (int c) {
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
		non_fatal(ERROR_BAD_PARAM,"STOB: Bad parameter #2",c);
}

void cmd_stob_sreg (int c) {
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
		non_fatal(ERROR_BAD_PARAM,"STOB: Bad parameter #2",c);
}

void cmd_stob_immptr (int c) {
  int a1,t2,work;

	EVAL_T2;
	EVAL_A1;
	IMMPTRVAL(work,(a1+(SREG(0)>>2)));
	if (t2 == TYPE_IMMEDIATE) {
		*(((char *)&work)+(SREG(0)&3))=(char)A2;
		set_mem_value(c,(a1+(SREG(0)>>2)),work);
	} else
	if (t2 == TYPE_UREG) {
		*(((char *)&work)+(SREG(0)&3))=(char)UREG(A2);
		set_mem_value(c,(a1+(SREG(0)>>2)),work);
	} else
	if (t2 == TYPE_SREG) {
		*(((char *)&work)+(SREG(0)&3))=(char)SREG(A2);
		set_mem_value(c,(a1+(SREG(0)>>2)),work);
	} else
	if (t2 == TYPE_IMMPTR) {
		int work2;
		IMMPTRVAL(work2,A2);
		*(((char *)&work)+(SREG(0)&3))=(char)work2;
		set_mem_value(c,(a1+(SREG(0)>>2)),work);
	} else
	if (t2 == TYPE_UPTR) {
		int work2;
		UPTRVAL(work2,A2);
		*(((char *)&work)+(SREG(0)&3))=(char)work2;
		set_mem_value(c,(a1+(SREG(0)>>2)),work);
	} else \
		non_fatal(ERROR_BAD_PARAM,"STOB: Bad parameter #2",c);
}

void cmd_stob_uptr (int c) {
  int a1,t2,work;

  	EVAL_T2;
	EVAL_A1;
	// BLACK MAGIC!!! :)))
	IMMPTRVAL(work,((UREG(a1))+(SREG(0)>>2)));
	if (t2 == TYPE_IMMEDIATE) {
		*(((char *)&work)+(SREG(0)&3))=(char)A2;
		set_mem_value(c,(UREG(a1)+(SREG(0)>>2)),work);
	} else
	if (t2 == TYPE_UREG) {
		*(((char *)&work)+(SREG(0)&3))=(char)UREG(A2);
		set_mem_value(c,(UREG(a1)+(SREG(0)>>2)),work);
	} else
	if (t2 == TYPE_SREG) {
		*(((char *)&work)+(SREG(0)&3))=(char)SREG(A2);
		set_mem_value(c,(UREG(a1)+(SREG(0)>>2)),work);
	} else
	if (t2 == TYPE_IMMPTR) {
		int work2;
		IMMPTRVAL(work2,A2);
		*(((char *)&work)+(SREG(0)&3))=(char)work2;
		set_mem_value(c,(UREG(a1)+(SREG(0)>>2)),work);
	} else
	if (t2 == TYPE_UPTR) {
		int work2;
		UPTRVAL(work2,A2);
		*(((char *)&work)+(SREG(0)&3))=(char)work2;
		set_mem_value(c,(UREG(a1)+(SREG(0)>>2)),work);
	} else \
		non_fatal(ERROR_BAD_PARAM,"STOB: Bad parameter #2",c);
}

// cmd_alloc

void cmd_alloc_immediate (int c) {
  int t2;

	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) {
		int x=mem_alloc(c,A1,A2);
		if (x>=0) {
			UREG(1)=(*cpu[c].mem)[x].map_addr;
			UREG(0)=x;
		}
	} else
	if (t2 == TYPE_UREG) {
		int x=mem_alloc(c,A1,UREG(A2));
		if (x>=0) {
			UREG(1)=(*cpu[c].mem)[x].map_addr;
			UREG(0)=x;
		}
	} else
	if (t2 == TYPE_SREG) {
		int x=mem_alloc(c,A1,SREG(A2));
		if (x>=0) {
			UREG(1)=(*cpu[c].mem)[x].map_addr;
			UREG(0)=x;
		}
	} else
	if (t2 == TYPE_IMMPTR) {
		int work;
		int x;
		IMMPTRVAL(work,A2);
		x=mem_alloc(c,A1,work);
		if (x>=0) {
			UREG(1)=(*cpu[c].mem)[x].map_addr;
			UREG(0)=x;
		}
	} else 
	if (t2 == TYPE_UPTR) {
		int work;
		int x;
		UPTRVAL(work,A2);
		x=mem_alloc(c,A1,work);
		if (x>=0) {
			UREG(1)=(*cpu[c].mem)[x].map_addr;
			UREG(0)=x;
		}
	} else \
	non_fatal(ERROR_BAD_PARAM,"ALLOC: Bad parameter #2",c);
}

void cmd_alloc_ureg (int c) {
  int t2;

	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) {
		int x=mem_alloc(c,UREG(A1),A2);
		if (x>=0) {
			UREG(1)=(*cpu[c].mem)[x].map_addr;
			UREG(0)=x;
		}
	} else
	if (t2 == TYPE_UREG) {
		int x=mem_alloc(c,UREG(A1),UREG(A2));
		if (x>=0) {
			UREG(1)=(*cpu[c].mem)[x].map_addr;
			UREG(0)=x;
		}
	} else
	if (t2 == TYPE_SREG) {
		int x=mem_alloc(c,UREG(A1),SREG(A2));
		if (x>=0) {
			UREG(1)=(*cpu[c].mem)[x].map_addr;
			UREG(0)=x;
		}
	} else
	if (t2 == TYPE_IMMPTR) {
		int work;
		int x;
		IMMPTRVAL(work,A2);
		x=mem_alloc(c,UREG(A1),work);
		if (x>=0) {
			UREG(1)=(*cpu[c].mem)[x].map_addr;
			UREG(0)=x;
		}
	} else 
	if (t2 == TYPE_UPTR) {
		int work;
		int x;
		UPTRVAL(work,A2);
		x=mem_alloc(c,UREG(A1),work);
		if (x>=0) {
			UREG(1)=(*cpu[c].mem)[x].map_addr;
			UREG(0)=x;
		}
	} else \
	non_fatal(ERROR_BAD_PARAM,"ALLOC: Bad parameter #2",c);
}

void cmd_alloc_sreg (int c) {
  int t2;

	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) {
		int x=mem_alloc(c,SREG(A1),A2);
		if (x>=0) {
			UREG(1)=(*cpu[c].mem)[x].map_addr;
			UREG(0)=x;
		}
	} else
	if (t2 == TYPE_UREG) {
		int x=mem_alloc(c,SREG(A1),UREG(A2));
		if (x>=0) {
			UREG(1)=(*cpu[c].mem)[x].map_addr;
			UREG(0)=x;
		}
	} else
	if (t2 == TYPE_SREG) {
		int x=mem_alloc(c,SREG(A1),SREG(A2));
		if (x>=0) {
			UREG(1)=(*cpu[c].mem)[x].map_addr;
			UREG(0)=x;
		}
	} else
	if (t2 == TYPE_IMMPTR) {
		int work;
		int x;
		IMMPTRVAL(work,A2);
		x=mem_alloc(c,SREG(A1),work);
		if (x>=0) {
			UREG(1)=(*cpu[c].mem)[x].map_addr;
			UREG(0)=x;
		}
	} else 
	if (t2 == TYPE_UPTR) {
		int work;
		int x;
		UPTRVAL(work,A2);
		x=mem_alloc(c,SREG(A1),work);
		if (x>=0) {
			UREG(1)=(*cpu[c].mem)[x].map_addr;
			UREG(0)=x;
		}
	} else \
	non_fatal(ERROR_BAD_PARAM,"ALLOC: Bad parameter #2",c);
}

void cmd_alloc_immptr (int c) {
  int t2;
  int work;

  	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) {
		int x;
		IMMPTRVAL(work,A1);
		x=mem_alloc(c,work,A2);
		if (x>=0) {
			UREG(1)=(*cpu[c].mem)[x].map_addr;
			UREG(0)=x;
		}
	} else 
	if (t2 == TYPE_UREG) {
		int x;
		IMMPTRVAL(work,A1);
		x=mem_alloc(c,work,UREG(A2));
		if (x>=0) {
			UREG(1)=(*cpu[c].mem)[x].map_addr;
			UREG(0)=x;
		}
	} else 
	if (t2 == TYPE_SREG) {
		int x;
		IMMPTRVAL(work,A1);
		x=mem_alloc(c,work,SREG(A2));
		if (x>=0) {
			UREG(1)=(*cpu[c].mem)[x].map_addr;
			UREG(0)=x;
		}
	} else 
	if (t2 == TYPE_IMMPTR) {
		int x;
		int work2;
		IMMPTRVAL(work,A1);
		IMMPTRVAL(work2,A2);
		x=mem_alloc(c,work,work2);
		if (x>=0) {
			UREG(1)=(*cpu[c].mem)[x].map_addr;
			UREG(0)=x;
		}
	} else 
	if (t2 == TYPE_UPTR) {
		int x;
		int work2;
		IMMPTRVAL(work,A1);
		UPTRVAL(work2,A2);
		x=mem_alloc(c,work,work2);
		if (x>=0) {
			UREG(1)=(*cpu[c].mem)[x].map_addr;
			UREG(0)=x;
		}
	} else \
	non_fatal(ERROR_BAD_PARAM,"ALLOC: Bad parameter #2",c);
}

void cmd_alloc_uptr (int c) {
  int t2;
  int work;

  	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) {
		int x;
		UPTRVAL(work,A1);
		x=mem_alloc(c,work,A2);
		if (x>=0) {
			UREG(1)=(*cpu[c].mem)[x].map_addr;
			UREG(0)=x;
		}
	} else 
	if (t2 == TYPE_UREG) {
		int x;
		UPTRVAL(work,A1);
		x=mem_alloc(c,work,UREG(A2));
		if (x>=0) {
			UREG(1)=(*cpu[c].mem)[x].map_addr;
			UREG(0)=x;
		}
	} else 
	if (t2 == TYPE_SREG) {
		int x;
		UPTRVAL(work,A1);
		x=mem_alloc(c,work,SREG(A2));
		if (x>=0) {
			UREG(1)=(*cpu[c].mem)[x].map_addr;
			UREG(0)=x;
		}
	} else 
	if (t2 == TYPE_IMMPTR) {
		int x;
		int work2;
		UPTRVAL(work,A1);
		IMMPTRVAL(work2,A2);
		x=mem_alloc(c,work,work2);
		if (x>=0) {
			UREG(1)=(*cpu[c].mem)[x].map_addr;
			UREG(0)=x;
		}
	} else 
	if (t2 == TYPE_UPTR) {
		int x;
		int work2;
		UPTRVAL(work,A1);
		UPTRVAL(work2,A2);
		x=mem_alloc(c,work,work2);
		if (x>=0) {
			UREG(1)=(*cpu[c].mem)[x].map_addr;
			UREG(0)=x;
		}
	} else \
	non_fatal(ERROR_BAD_PARAM,"ALLOC: Bad parameter #2",c);
}

// cmd_realloc

void cmd_realloc_immediate (int c) {
  int t2;

	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) mem_realloc(c,A1,A2); else
	if (t2 == TYPE_UREG) mem_realloc(c,A1,UREG(A2)); else
	if (t2 == TYPE_SREG) mem_realloc(c,A1,SREG(A2)); else
	if (t2 == TYPE_IMMPTR) {
		int work;
		IMMPTRVAL(work,A2);
		mem_realloc(c,A1,work);
	} else
	if (t2 == TYPE_UPTR) {
		int work;
		UPTRVAL(work,A2);
		mem_realloc(c,A1,work);
	} else \
	non_fatal(ERROR_BAD_PARAM,"REALLOC: Bad parameter #2",c);
}

void cmd_realloc_ureg (int c) {
  int t2;

	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) mem_realloc(c,UREG(A1),A2); else
	if (t2 == TYPE_UREG) mem_realloc(c,UREG(A1),UREG(A2)); else
	if (t2 == TYPE_SREG) mem_realloc(c,UREG(A1),SREG(A2)); else
	if (t2 == TYPE_IMMPTR) {
		int work;
		IMMPTRVAL(work,A2);
		mem_realloc(c,UREG(A1),work);
	} else
	if (t2 == TYPE_UPTR) {
		int work;
		UPTRVAL(work,A2);
		mem_realloc(c,UREG(A1),work);
	} else \
	non_fatal(ERROR_BAD_PARAM,"REALLOC: Bad parameter #2",c);
}

void cmd_realloc_sreg (int c) {
  int t2;

	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) mem_realloc(c,SREG(A1),A2); else
	if (t2 == TYPE_UREG) mem_realloc(c,SREG(A1),UREG(A2)); else
	if (t2 == TYPE_SREG) mem_realloc(c,SREG(A1),SREG(A2)); else
	if (t2 == TYPE_IMMPTR) {
		int work;
		IMMPTRVAL(work,A2);
		mem_realloc(c,SREG(A1),work);
	} else
	if (t2 == TYPE_UPTR) {
		int work;
		UPTRVAL(work,A2);
		mem_realloc(c,SREG(A1),work);
	} else \
	non_fatal(ERROR_BAD_PARAM,"REALLOC: Bad parameter #2",c);
}

void cmd_realloc_immptr (int c) {
  int work, t2;

  	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) {
		IMMPTRVAL(work,A1);
		mem_realloc(c,work,A2);
	} else
	if (t2 == TYPE_UREG) {
		IMMPTRVAL(work,A1);
		mem_realloc(c,work,UREG(A2));
	} else
	if (t2 == TYPE_SREG) {
		IMMPTRVAL(work,A1);
		mem_realloc(c,work,SREG(A2));
	} else
	if (t2 == TYPE_IMMPTR) {
		int work2;
		IMMPTRVAL(work,A1);
		IMMPTRVAL(work2,A2);
		mem_realloc(c,work,work2);
	} else
	if (t2 == TYPE_UPTR) {
		int work2;
		IMMPTRVAL(work,A1);
		UPTRVAL(work2,A2);
		mem_realloc(c,work,work2);
	} else \
	non_fatal(ERROR_BAD_PARAM,"REALLOC: Bad parameter #2",c);
}

void cmd_realloc_uptr (int c) {
  int work, t2;

  	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) {
		UPTRVAL(work,A1);
		mem_realloc(c,work,A2);
	} else
	if (t2 == TYPE_UREG) {
		UPTRVAL(work,A1);
		mem_realloc(c,work,UREG(A2));
	} else
	if (t2 == TYPE_SREG) {
		UPTRVAL(work,A1);
		mem_realloc(c,work,SREG(A2));
	} else
	if (t2 == TYPE_IMMPTR) {
		int work2;
		UPTRVAL(work,A1);
		IMMPTRVAL(work2,A2);
		mem_realloc(c,work,work2);
	} else
	if (t2 == TYPE_UPTR) {
		int work2;
		UPTRVAL(work,A1);
		UPTRVAL(work2,A2);
		mem_realloc(c,work,work2);
	} else \
	non_fatal(ERROR_BAD_PARAM,"REALLOC: Bad parameter #2",c);
}

// cmd_dealloc

void cmd_dealloc_immediate (int c) {

	mem_dealloc(c,A1);
}

void cmd_dealloc_ureg (int c) {

	mem_dealloc(c,UREG(A1));
}

void cmd_dealloc_sreg (int c) {

	mem_dealloc(c,SREG(A1));
}

void cmd_dealloc_immptr (int c) {
  int work;

  	IMMPTRVAL(work,A1);
	mem_dealloc(c,work);
}

void cmd_dealloc_uptr (int c) {
  int work;

  	UPTRVAL(work,A1);
	mem_dealloc(c,work);
}

// cmd_cmpcnt

void cmd_cmpcnt_immediate (int c) {
  int t2;

  	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) {
		int *addr1, *addr2;
		addr1=verify_access(c,A1,SREG(0),MEM_FLAG_READ);
		addr2=verify_access(c,A2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Cmp1 memory unsuitable",c);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Cmp2 memory unsuitable",c);
			return;
		}
		SREG(0)=memcmp(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_UREG) {
		int *addr1, *addr2;
		addr1=verify_access(c,A1,SREG(0),MEM_FLAG_READ);
		addr2=verify_access(c,UREG(A2),SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Cmp1 memory unsuitable",c);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Cmp2 memory unsuitable",c);
			return;
		}
		SREG(0)=memcmp(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_SREG) {
		int *addr1, *addr2;
		addr1=verify_access(c,A1,SREG(0),MEM_FLAG_READ);
		addr2=verify_access(c,SREG(A2),SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Cmp1 memory unsuitable",c);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Cmp2 memory unsuitable",c);
			return;
		}
		SREG(0)=memcmp(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_IMMEDIATE) {
		int a2;
		int *addr1, *addr2;
		IMMPTRVAL(a2,A2);
		addr1=verify_access(c,A1,SREG(0),MEM_FLAG_READ);
		addr2=verify_access(c,a2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Cmp1 memory unsuitable",c);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Cmp2 memory unsuitable",c);
			return;
		}
		SREG(0)=memcmp(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_UPTR) {
		int a2;
		int *addr1, *addr2;
		UPTRVAL(a2,A2);
		addr1=verify_access(c,A1,SREG(0),MEM_FLAG_READ);
		addr2=verify_access(c,a2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Cmp1 memory unsuitable",c);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Cmp2 memory unsuitable",c);
			return;
		}
		SREG(0)=memcmp(addr1,addr2,SREG(0)*4);
		return;
	} else
	non_fatal(ERROR_BAD_PARAM,"CP: Bad parameter 2",c);
}

void cmd_cmpcnt_ureg (int c) {
  int t2;

  	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) {
		int *addr1, *addr2;
		addr1=verify_access(c,UREG(A1),SREG(0),MEM_FLAG_READ);
		addr2=verify_access(c,A2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Cmp1 memory unsuitable",c);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Cmp2 memory unsuitable",c);
			return;
		}
		SREG(0)=memcmp(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_UREG) {
		int *addr1, *addr2;
		addr1=verify_access(c,UREG(A1),SREG(0),MEM_FLAG_READ);
		addr2=verify_access(c,UREG(A2),SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Cmp1 memory unsuitable",c);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Cmp2 memory unsuitable",c);
			return;
		}
		SREG(0)=memcmp(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_SREG) {
		int *addr1, *addr2;
		addr1=verify_access(c,UREG(A1),SREG(0),MEM_FLAG_READ);
		addr2=verify_access(c,SREG(A2),SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Cmp1 memory unsuitable",c);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Cmp2 memory unsuitable",c);
			return;
		}
		SREG(0)=memcmp(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_IMMEDIATE) {
		int a2;
		int *addr1, *addr2;
		IMMPTRVAL(a2,A2);
		addr1=verify_access(c,UREG(A1),SREG(0),MEM_FLAG_READ);
		addr2=verify_access(c,a2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Cmp1 memory unsuitable",c);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Cmp2 memory unsuitable",c);
			return;
		}
		SREG(0)=memcmp(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_UPTR) {
		int a2;
		int *addr1, *addr2;
		UPTRVAL(a2,A2);
		addr1=verify_access(c,UREG(A1),SREG(0),MEM_FLAG_READ);
		addr2=verify_access(c,a2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Cmp1 memory unsuitable",c);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Cmp2 memory unsuitable",c);
			return;
		}
		SREG(0)=memcmp(addr1,addr2,SREG(0)*4);
		return;
	} else
	non_fatal(ERROR_BAD_PARAM,"CP: Bad parameter 2",c);
}

void cmd_cmpcnt_sreg (int c) {
  int t2;

  	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) {
		int *addr1, *addr2;
		addr1=verify_access(c,SREG(A1),SREG(0),MEM_FLAG_READ);
		addr2=verify_access(c,A2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Cmp1 memory unsuitable",c);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Cmp2 memory unsuitable",c);
			return;
		}
		SREG(0)=memcmp(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_UREG) {
		int *addr1, *addr2;
		addr1=verify_access(c,SREG(A1),SREG(0),MEM_FLAG_READ);
		addr2=verify_access(c,UREG(A2),SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Cmp1 memory unsuitable",c);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Cmp2 memory unsuitable",c);
			return;
		}
		SREG(0)=memcmp(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_SREG) {
		int *addr1, *addr2;
		addr1=verify_access(c,SREG(A1),SREG(0),MEM_FLAG_READ);
		addr2=verify_access(c,SREG(A2),SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Cmp1 memory unsuitable",c);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Cmp2 memory unsuitable",c);
			return;
		}
		SREG(0)=memcmp(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_IMMEDIATE) {
		int a2;
		int *addr1, *addr2;
		IMMPTRVAL(a2,A2);
		addr1=verify_access(c,SREG(A1),SREG(0),MEM_FLAG_READ);
		addr2=verify_access(c,a2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Cmp1 memory unsuitable",c);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Cmp2 memory unsuitable",c);
			return;
		}
		SREG(0)=memcmp(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_UPTR) {
		int a2;
		int *addr1, *addr2;
		UPTRVAL(a2,A2);
		addr1=verify_access(c,SREG(A1),SREG(0),MEM_FLAG_READ);
		addr2=verify_access(c,a2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Cmp1 memory unsuitable",c);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Cmp2 memory unsuitable",c);
			return;
		}
		SREG(0)=memcmp(addr1,addr2,SREG(0)*4);
		return;
	} else
	non_fatal(ERROR_BAD_PARAM,"CP: Bad parameter 2",c);
}

void cmd_cmpcnt_immptr (int c) {
  int t2;

  	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) {
		int a1;
		int *addr1, *addr2;
		IMMPTRVAL(a1,A1);
		addr1=verify_access(c,a1,SREG(0),MEM_FLAG_READ);
		addr2=verify_access(c,A2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Cmp1 memory unsuitable",c);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Cmp2 memory unsuitable",c);
			return;
		}
		SREG(0)=memcmp(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_UREG) {
		int a1;
		int *addr1, *addr2;
		IMMPTRVAL(a1,A1);
		addr1=verify_access(c,a1,SREG(0),MEM_FLAG_READ);
		addr2=verify_access(c,UREG(A2),SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Cmp1 memory unsuitable",c);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Cmp2 memory unsuitable",c);
			return;
		}
		SREG(0)=memcmp(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_SREG) {
		int a1;
		int *addr1, *addr2;
		IMMPTRVAL(a1,A1);
		addr1=verify_access(c,a1,SREG(0),MEM_FLAG_READ);
		addr2=verify_access(c,SREG(A2),SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Cmp1 memory unsuitable",c);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Cmp2 memory unsuitable",c);
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
		addr1=verify_access(c,a1,SREG(0),MEM_FLAG_READ);
		addr2=verify_access(c,a2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Cmp1 memory unsuitable",c);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Cmp2 memory unsuitable",c);
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
		addr1=verify_access(c,a1,SREG(0),MEM_FLAG_READ);
		addr2=verify_access(c,a2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Cmp1 memory unsuitable",c);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Cmp2 memory unsuitable",c);
			return;
		}
		SREG(0)=memcmp(addr1,addr2,SREG(0)*4);
		return;
	} else
	non_fatal(ERROR_BAD_PARAM,"CP: Bad parameter 2",c);
}

void cmd_cmpcnt_uptr (int c) {
  int t2;

  	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) {
		int a1;
		int *addr1, *addr2;
		UPTRVAL(a1,A1);
		addr1=verify_access(c,a1,SREG(0),MEM_FLAG_READ);
		addr2=verify_access(c,A2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Cmp1 memory unsuitable",c);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Cmp2 memory unsuitable",c);
			return;
		}
		SREG(0)=memcmp(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_UREG) {
		int a1;
		int *addr1, *addr2;
		UPTRVAL(a1,A1);
		addr1=verify_access(c,a1,SREG(0),MEM_FLAG_READ);
		addr2=verify_access(c,UREG(A2),SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Cmp1 memory unsuitable",c);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Cmp2 memory unsuitable",c);
			return;
		}
		SREG(0)=memcmp(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_SREG) {
		int a1;
		int *addr1, *addr2;
		UPTRVAL(a1,A1);
		addr1=verify_access(c,a1,SREG(0),MEM_FLAG_READ);
		addr2=verify_access(c,SREG(A2),SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Cmp1 memory unsuitable",c);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Cmp2 memory unsuitable",c);
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
		addr1=verify_access(c,a1,SREG(0),MEM_FLAG_READ);
		addr2=verify_access(c,a2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Cmp1 memory unsuitable",c);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Cmp2 memory unsuitable",c);
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
		addr1=verify_access(c,a1,SREG(0),MEM_FLAG_READ);
		addr2=verify_access(c,a2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Cmp1 memory unsuitable",c);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Cmp2 memory unsuitable",c);
			return;
		}
		SREG(0)=memcmp(addr1,addr2,SREG(0)*4);
		return;
	} else
	non_fatal(ERROR_BAD_PARAM,"CP: Bad parameter 2",c);
}


// cmd_cpcnt

void cmd_cpcnt_immediate (int c) {
  int t2;

  	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) {
		int *addr1, *addr2;
		addr1=verify_access(c,A1,SREG(0),MEM_FLAG_WRITE);
		addr2=verify_access(c,A2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Destination memory unsuitable",c);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Source memory unsuitable",c);
			return;
		}
		memcpy(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_UREG) {
		int *addr1, *addr2;
		addr1=verify_access(c,A1,SREG(0),MEM_FLAG_WRITE);
		addr2=verify_access(c,UREG(A2),SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Destination memory unsuitable",c);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Source memory unsuitable",c);
			return;
		}
		memcpy(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_SREG) {
		int *addr1, *addr2;
		addr1=verify_access(c,A1,SREG(0),MEM_FLAG_WRITE);
		addr2=verify_access(c,SREG(A2),SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Destination memory unsuitable",c);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Source memory unsuitable",c);
			return;
		}
		memcpy(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_IMMEDIATE) {
		int a2;
		int *addr1, *addr2;
		IMMPTRVAL(a2,A2);
		addr1=verify_access(c,A1,SREG(0),MEM_FLAG_WRITE);
		addr2=verify_access(c,a2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Destination memory unsuitable",c);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Source memory unsuitable",c);
			return;
		}
		memcpy(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_UPTR) {
		int a2;
		int *addr1, *addr2;
		UPTRVAL(a2,A2);
		addr1=verify_access(c,A1,SREG(0),MEM_FLAG_WRITE);
		addr2=verify_access(c,a2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Destination memory unsuitable",c);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Source memory unsuitable",c);
			return;
		}
		memcpy(addr1,addr2,SREG(0)*4);
		return;
	} else
	non_fatal(ERROR_BAD_PARAM,"CP: Bad parameter 2",c);
}

void cmd_cpcnt_ureg (int c) {
  int t2;

  	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) {
		int *addr1, *addr2;
		addr1=verify_access(c,UREG(A1),SREG(0),MEM_FLAG_WRITE);
		addr2=verify_access(c,A2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Destination memory unsuitable",c);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Source memory unsuitable",c);
			return;
		}
		memcpy(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_UREG) {
		int *addr1, *addr2;
		addr1=verify_access(c,UREG(A1),SREG(0),MEM_FLAG_WRITE);
		addr2=verify_access(c,UREG(A2),SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Destination memory unsuitable",c);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Source memory unsuitable",c);
			return;
		}
		memcpy(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_SREG) {
		int *addr1, *addr2;
		addr1=verify_access(c,UREG(A1),SREG(0),MEM_FLAG_WRITE);
		addr2=verify_access(c,SREG(A2),SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Destination memory unsuitable",c);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Source memory unsuitable",c);
			return;
		}
		memcpy(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_IMMEDIATE) {
		int a2;
		int *addr1, *addr2;
		IMMPTRVAL(a2,A2);
		addr1=verify_access(c,UREG(A1),SREG(0),MEM_FLAG_WRITE);
		addr2=verify_access(c,a2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Destination memory unsuitable",c);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Source memory unsuitable",c);
			return;
		}
		memcpy(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_UPTR) {
		int a2;
		int *addr1, *addr2;
		UPTRVAL(a2,A2);
		addr1=verify_access(c,UREG(A1),SREG(0),MEM_FLAG_WRITE);
		addr2=verify_access(c,a2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Destination memory unsuitable",c);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Source memory unsuitable",c);
			return;
		}
		memcpy(addr1,addr2,SREG(0)*4);
		return;
	} else
	non_fatal(ERROR_BAD_PARAM,"CP: Bad parameter 2",c);
}

void cmd_cpcnt_sreg (int c) {
  int t2;

  	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) {
		int *addr1, *addr2;
		addr1=verify_access(c,SREG(A1),SREG(0),MEM_FLAG_WRITE);
		addr2=verify_access(c,A2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Destination memory unsuitable",c);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Source memory unsuitable",c);
			return;
		}
		memcpy(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_UREG) {
		int *addr1, *addr2;
		addr1=verify_access(c,SREG(A1),SREG(0),MEM_FLAG_WRITE);
		addr2=verify_access(c,UREG(A2),SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Destination memory unsuitable",c);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Source memory unsuitable",c);
			return;
		}
		memcpy(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_SREG) {
		int *addr1, *addr2;
		addr1=verify_access(c,SREG(A1),SREG(0),MEM_FLAG_WRITE);
		addr2=verify_access(c,SREG(A2),SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Destination memory unsuitable",c);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Source memory unsuitable",c);
			return;
		}
		memcpy(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_IMMEDIATE) {
		int a2;
		int *addr1, *addr2;
		IMMPTRVAL(a2,A2);
		addr1=verify_access(c,SREG(A1),SREG(0),MEM_FLAG_WRITE);
		addr2=verify_access(c,a2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Destination memory unsuitable",c);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Source memory unsuitable",c);
			return;
		}
		memcpy(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_UPTR) {
		int a2;
		int *addr1, *addr2;
		UPTRVAL(a2,A2);
		addr1=verify_access(c,SREG(A1),SREG(0),MEM_FLAG_WRITE);
		addr2=verify_access(c,a2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Destination memory unsuitable",c);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Source memory unsuitable",c);
			return;
		}
		memcpy(addr1,addr2,SREG(0)*4);
		return;
	} else
	non_fatal(ERROR_BAD_PARAM,"CP: Bad parameter 2",c);
}

void cmd_cpcnt_immptr (int c) {
  int t2;

  	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) {
		int a1;
		int *addr1, *addr2;
		IMMPTRVAL(a1,A1);
		addr1=verify_access(c,a1,SREG(0),MEM_FLAG_WRITE);
		addr2=verify_access(c,A2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Destination memory unsuitable",c);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Source memory unsuitable",c);
			return;
		}
		memcpy(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_UREG) {
		int a1;
		int *addr1, *addr2;
		IMMPTRVAL(a1,A1);
		addr1=verify_access(c,a1,SREG(0),MEM_FLAG_WRITE);
		addr2=verify_access(c,UREG(A2),SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Destination memory unsuitable",c);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Source memory unsuitable",c);
			return;
		}
		memcpy(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_SREG) {
		int a1;
		int *addr1, *addr2;
		IMMPTRVAL(a1,A1);
		addr1=verify_access(c,a1,SREG(0),MEM_FLAG_WRITE);
		addr2=verify_access(c,SREG(A2),SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Destination memory unsuitable",c);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Source memory unsuitable",c);
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
		addr1=verify_access(c,a1,SREG(0),MEM_FLAG_WRITE);
		addr2=verify_access(c,a2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Destination memory unsuitable",c);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Source memory unsuitable",c);
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
		addr1=verify_access(c,a1,SREG(0),MEM_FLAG_WRITE);
		addr2=verify_access(c,a2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Destination memory unsuitable",c);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Source memory unsuitable",c);
			return;
		}
		memcpy(addr1,addr2,SREG(0)*4);
		return;
	} else
	non_fatal(ERROR_BAD_PARAM,"CP: Bad parameter 2",c);
}

void cmd_cpcnt_uptr (int c) {
  int t2;

  	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) {
		int a1;
		int *addr1, *addr2;
		UPTRVAL(a1,A1);
		addr1=verify_access(c,a1,SREG(0),MEM_FLAG_WRITE);
		addr2=verify_access(c,A2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Destination memory unsuitable",c);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Source memory unsuitable",c);
			return;
		}
		memcpy(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_UREG) {
		int a1;
		int *addr1, *addr2;
		UPTRVAL(a1,A1);
		addr1=verify_access(c,a1,SREG(0),MEM_FLAG_WRITE);
		addr2=verify_access(c,UREG(A2),SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Destination memory unsuitable",c);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Source memory unsuitable",c);
			return;
		}
		memcpy(addr1,addr2,SREG(0)*4);
		return;
	} else
	if (t2 == TYPE_SREG) {
		int a1;
		int *addr1, *addr2;
		UPTRVAL(a1,A1);
		addr1=verify_access(c,a1,SREG(0),MEM_FLAG_WRITE);
		addr2=verify_access(c,SREG(A2),SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Destination memory unsuitable",c);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Source memory unsuitable",c);
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
		addr1=verify_access(c,a1,SREG(0),MEM_FLAG_WRITE);
		addr2=verify_access(c,a2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Destination memory unsuitable",c);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Source memory unsuitable",c);
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
		addr1=verify_access(c,a1,SREG(0),MEM_FLAG_WRITE);
		addr2=verify_access(c,a2,SREG(0),MEM_FLAG_READ);
		if (!addr1) {
			non_fatal(ERROR_PROTFAULT,"Destination memory unsuitable",c);
			return;
		}
		if (!addr2) {
			non_fatal(ERROR_PROTFAULT,"Source memory unsuitable",c);
			return;
		}
		memcpy(addr1,addr2,SREG(0)*4);
		return;
	} else
	non_fatal(ERROR_BAD_PARAM,"CP: Bad parameter 2",c);
}

// cmd_setstack

void cmd_setstack_immediate (int c) {
  int t2;
  
  	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) {
		int a2;
		a2=A2;
		if (cpu[c].uregs[0] > a2) {
			non_fatal(ERROR_BAD_PARAM,"SETSTACK: user stack ptr > stack size",c);
			return;
		}
		cpu[c].user_stack=A1;
		cpu[c].user_size=a2;
		cpu[c].user_ptr=cpu[c].uregs[0];
		return;
	} else
	if (t2 == TYPE_UREG) {
		int a2;
		UREGVAL(a2,A2);
		if (cpu[c].uregs[0] > a2) {
			non_fatal(ERROR_BAD_PARAM,"SETSTACK: user stack ptr > stack size",c);
			return;
		}
		cpu[c].user_stack=A1;
		cpu[c].user_size=a2;
		cpu[c].user_ptr=cpu[c].uregs[0];
		return;
	} else
	if (t2 == TYPE_IMMPTR) {
		int a2;
		IMMPTRVAL(a2,A2);
		if (cpu[c].uregs[0] > a2) {
			non_fatal(ERROR_BAD_PARAM,"SETSTACK: user stack ptr > stack size",c);
			return;
		}
		cpu[c].user_stack=A1;
		cpu[c].user_size=a2;
		cpu[c].user_ptr=cpu[c].uregs[0];
		return;
	} else
	if (t2 == TYPE_UPTR) {
		int a2;
		UPTRVAL(a2,A2);
		if (cpu[c].uregs[0] > a2) {
			non_fatal(ERROR_BAD_PARAM,"SETSTACK: user stack ptr > stack size",c);
			return;
		}
		cpu[c].user_stack=A1;
		cpu[c].user_size=a2;
		cpu[c].user_ptr=cpu[c].uregs[0];
		return;
	} else \
	non_fatal(ERROR_BAD_PARAM,"SETSTACK: Bad parameter #2",c);
}

void cmd_setstack_ureg (int c) {
  int t2;
  
  	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) {
		int a1,a2;
		UREGVAL(a1,A1);
		a2=A2;
		if (cpu[c].uregs[0] > a2) {
			non_fatal(ERROR_BAD_PARAM,"SETSTACK: user stack ptr > stack size",c);
			return;
		}
		cpu[c].user_stack=a1;
		cpu[c].user_size=a2;
		cpu[c].user_ptr=cpu[c].uregs[0];
		return;
	} else
	if (t2 == TYPE_UREG) {
		int a1,a2;
		UREGVAL(a1,A1);
		UREGVAL(a2,A2);
		if (cpu[c].uregs[0] > a2) {
			non_fatal(ERROR_BAD_PARAM,"SETSTACK: user stack ptr > stack size",c);
			return;
		}
		cpu[c].user_stack=a1;
		cpu[c].user_size=a2;
		cpu[c].user_ptr=cpu[c].uregs[0];
		return;
	} else
	if (t2 == TYPE_IMMPTR) {
		int a1,a2;
		UREGVAL(a1,A1);
		IMMPTRVAL(a2,A2);
		if (cpu[c].uregs[0] > a2) {
			non_fatal(ERROR_BAD_PARAM,"SETSTACK: user stack ptr > stack size",c);
			return;
		}
		cpu[c].user_stack=a1;
		cpu[c].user_size=a2;
		cpu[c].user_ptr=cpu[c].uregs[0];
		return;
	} else
	if (t2 == TYPE_UPTR) {
		int a1,a2;
		UREGVAL(a1,A1);
		UPTRVAL(a2,A2);
		if (cpu[c].uregs[0] > a2) {
			non_fatal(ERROR_BAD_PARAM,"SETSTACK: user stack ptr > stack size",c);
			return;
		}
		cpu[c].user_stack=a1;
		cpu[c].user_size=a2;
		cpu[c].user_ptr=cpu[c].uregs[0];
		return;
	} else
	non_fatal(ERROR_BAD_PARAM,"SETSTACK: Bad parameter #2",c);
}

void cmd_setstack_immptr (int c) {
  int t2;

  	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) {
		int a1,a2;
		IMMPTRVAL(a1,A1);
		a2=A2;
		if (cpu[c].uregs[0] > a2) {
			non_fatal(ERROR_BAD_PARAM,"SETSTACK: user stack ptr > stack size",c);
			return;
		}
		cpu[c].user_stack=a1;
		cpu[c].user_size=a2;
		cpu[c].user_ptr=cpu[c].uregs[0];
		return;
	} else
	if (t2 == TYPE_UREG) {
		int a1,a2;
		IMMPTRVAL(a1,A1);
		UREGVAL(a2,A2);
		if (cpu[c].uregs[0] > a2) {
			non_fatal(ERROR_BAD_PARAM,"SETSTACK: user stack ptr > stack size",c);
			return;
		}
		cpu[c].user_stack=a1;
		cpu[c].user_size=a2;
		cpu[c].user_ptr=cpu[c].uregs[0];
		return;
	} else
	if (t2 == TYPE_IMMPTR) {
		int a1,a2;
		IMMPTRVAL(a1,A1);
		IMMPTRVAL(a2,A2);
		if (cpu[c].uregs[0] > a2) {
			non_fatal(ERROR_BAD_PARAM,"SETSTACK: user stack ptr > stack size",c);
			return;
		}
		cpu[c].user_stack=a1;
		cpu[c].user_size=a2;
		cpu[c].user_ptr=cpu[c].uregs[0];
		return;
	} else
	if (t2 == TYPE_UPTR) {
		int a1,a2;
		IMMPTRVAL(a1,A1);
		UPTRVAL(a2,A2);
		if (cpu[c].uregs[0] > a2) {
			non_fatal(ERROR_BAD_PARAM,"SETSTACK: user stack ptr > stack size",c);
			return;
		}
		cpu[c].user_stack=a1;
		cpu[c].user_size=a2;
		cpu[c].user_ptr=cpu[c].uregs[0];
		return;
	} else
	non_fatal(ERROR_BAD_PARAM,"SETSTACK: Bad parameter #2",c);
}

void cmd_setstack_uptr (int c) {
  int t2;

	EVAL_T2;
	if (t2 == TYPE_IMMEDIATE) {
		int a1,a2;
		UPTRVAL(a1,A1);
		a2=A2;
		if (cpu[c].uregs[0] > a2) {
			non_fatal(ERROR_BAD_PARAM,"SETSTACK: user stack ptr > stack size",c);
			return;
		}
		cpu[c].user_stack=a1;
		cpu[c].user_size=a2;
		cpu[c].user_ptr=cpu[c].uregs[0];
		return;
	} else
	if (t2 == TYPE_UREG) {
		int a1,a2;
		UPTRVAL(a1,A1);
		UREGVAL(a2,A2);
		if (cpu[c].uregs[0] > a2) {
			non_fatal(ERROR_BAD_PARAM,"SETSTACK: user stack ptr > stack size",c);
			return;
		}
		cpu[c].user_stack=a1;
		cpu[c].user_size=a2;
		cpu[c].user_ptr=cpu[c].uregs[0];
		return;
	} else
	if (t2 == TYPE_IMMPTR) {
		int a1,a2;
		UPTRVAL(a1,A1);
		IMMPTRVAL(a2,A2);
		if (cpu[c].uregs[0] > a2) {
			non_fatal(ERROR_BAD_PARAM,"SETSTACK: user stack ptr > stack size",c);
			return;
		}
		cpu[c].user_stack=a1;
		cpu[c].user_size=a2;
		cpu[c].user_ptr=cpu[c].uregs[0];
		return;
	} else
	if (t2 == TYPE_UPTR) {
		int a1,a2;
		UPTRVAL(a1,A1);
		UPTRVAL(a2,A2);
		if (cpu[c].uregs[0] > a2) {
			non_fatal(ERROR_BAD_PARAM,"SETSTACK: user stack ptr > stack size",c);
			return;
		}
		cpu[c].user_stack=a1;
		cpu[c].user_size=a2;
		cpu[c].user_ptr=cpu[c].uregs[0];
		return;
	} else
	non_fatal(ERROR_BAD_PARAM,"SETSTACK: Bad parameter #2",c);
}

// cmd_push

void cmd_push_immediate (int c) {

	if (!cpu[c].user_size) {
		non_fatal(ERROR_NOUSTACK,"PUSH, but no user stack",c);
		return;
	}

	if (cpu[c].user_ptr>=cpu[c].user_size) {
		non_fatal(ERROR_STACK_OVER,"Cannot push - stack overflow",c);
		return;
	}

	set_mem_value(c,cpu[c].user_stack+cpu[c].user_ptr,A1);
	cpu[c].user_ptr++;
}

void cmd_push_ureg (int c) {
  int a1;

	if (!cpu[c].user_size) {
		non_fatal(ERROR_NOUSTACK,"PUSH, but no user stack",c);
		return;
	}

	if (cpu[c].user_ptr>=cpu[c].user_size) {
		non_fatal(ERROR_STACK_OVER,"Cannot push - stack overflow",c);
		return;
	}

	UREGVAL(a1,A1);
	set_mem_value(c,cpu[c].user_stack+cpu[c].user_ptr,a1);
	cpu[c].user_ptr++;
}

void cmd_push_sreg (int c) {
  int a1;

	if (!cpu[c].user_size) {
		non_fatal(ERROR_NOUSTACK,"PUSH, but no user stack",c);
		return;
	}

	if (cpu[c].user_ptr>=cpu[c].user_size) {
		non_fatal(ERROR_STACK_OVER,"Cannot push - stack overflow",c);
		return;
	}

	SREGVAL(a1,A1);
	set_mem_value(c,cpu[c].user_stack+cpu[c].user_ptr,a1);
	cpu[c].user_ptr++;
}

void cmd_push_freg (int c) {
  int a1;

	if (!cpu[c].user_size) {
		non_fatal(ERROR_NOUSTACK,"PUSH, but no user stack",c);
		return;
	}

	if (cpu[c].user_ptr>=cpu[c].user_size) {
		non_fatal(ERROR_STACK_OVER,"Cannot push - stack overflow",c);
		return;
	}

	FREGVAL(a1,A1);
	set_mem_value(c,cpu[c].user_stack+cpu[c].user_ptr,a1);
	cpu[c].user_ptr++;
}

void cmd_push_immptr (int c) {
  int a1;

	if (!cpu[c].user_size) {
		non_fatal(ERROR_NOUSTACK,"PUSH, but no user stack",c);
		return;
	}

	if (cpu[c].user_ptr>=cpu[c].user_size) {
		non_fatal(ERROR_STACK_OVER,"Cannot push - stack overflow",c);
		return;
	}

	IMMPTRVAL(a1,A1);
	set_mem_value(c,cpu[c].user_stack+cpu[c].user_ptr,a1);
	cpu[c].user_ptr++;
}

void cmd_push_uptr (int c) {
  int a1;

	if (!cpu[c].user_size) {
		non_fatal(ERROR_NOUSTACK,"PUSH, but no user stack",c);
		return;
	}

	if (cpu[c].user_ptr>=cpu[c].user_size) {
		non_fatal(ERROR_STACK_OVER,"Cannot push - stack overflow",c);
		return;
	}

	UPTRVAL(a1,A1);
	set_mem_value(c,cpu[c].user_stack+cpu[c].user_ptr,a1);
	cpu[c].user_ptr++;
}

// com_pop

void cmd_pop_ureg (int c) {
  int work;

	if (!cpu[c].user_size) {
		non_fatal(ERROR_NOUSTACK,"POP, but no user stack",c);
		return;
	}

	if (cpu[c].user_ptr<=0) {
		non_fatal(ERROR_STACK_OVER,"Cannot pop - stack underflow",c);
		return;
	}

	cpu[c].user_ptr--;
	work=get_mem_value(c,cpu[c].user_stack+cpu[c].user_ptr);
	CHECK_EXCEPT;
	UREG(A1)=work;
}

void cmd_pop_sreg (int c) {
  int work;

	if (!cpu[c].user_size) {
		non_fatal(ERROR_NOUSTACK,"POP, but no user stack",c);
		return;
	}

	if (cpu[c].user_ptr<=0) {
		non_fatal(ERROR_STACK_OVER,"Cannot pop - stack underflow",c);
		return;
	}

	cpu[c].user_ptr--;
	work=get_mem_value(c,cpu[c].user_stack+cpu[c].user_ptr);
	CHECK_EXCEPT;
	SREG(A1)=work;
}

void cmd_pop_freg (int c) {
  int work;

	if (!cpu[c].user_size) {
		non_fatal(ERROR_NOUSTACK,"POP, but no user stack",c);
		return;
	}

	if (cpu[c].user_ptr<=0) {
		non_fatal(ERROR_STACK_OVER,"Cannot pop - stack underflow",c);
		return;
	}

	cpu[c].user_ptr--;
	work=get_mem_value(c,cpu[c].user_stack+cpu[c].user_ptr);
	CHECK_EXCEPT;
	FREG(A1)=work;
}

void cmd_pop_immptr (int c) {
  int work;

	if (!cpu[c].user_size) {
		non_fatal(ERROR_NOUSTACK,"POP, but no user stack",c);
		return;
	}

	if (cpu[c].user_ptr<=0) {
		non_fatal(ERROR_STACK_OVER,"Cannot pop - stack underflow",c);
		return;
	}

	cpu[c].user_ptr--;
	work=get_mem_value(c,cpu[c].user_stack+cpu[c].user_ptr);
	CHECK_EXCEPT;
	set_mem_value(c,A1,work);
}

void cmd_pop_uptr (int c) {
  int work;

	if (!cpu[c].user_size) {
		non_fatal(ERROR_NOUSTACK,"POP, but no user stack",c);
		return;
	}

	if (cpu[c].user_ptr<=0) {
		non_fatal(ERROR_STACK_OVER,"Cannot pop - stack underflow",c);
		return;
	}

	cpu[c].user_ptr--;
	work=get_mem_value(c,cpu[c].user_stack+cpu[c].user_ptr);
	CHECK_EXCEPT;
	set_mem_value(c,UREG(A1),work);
}

// cmd_invalid

void cmd_invalid (int c) {

	non_fatal(ERROR_BAD_INSTR,"Incorrect opcode",c);
}
