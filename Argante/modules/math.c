/*

   Argante virtual OS
   ------------------

   Mathematical routines
   
   Status: done, but do not use it; will be rewritten
 
   Author:     Maurycy Prodeus <z33d@eth-security.net>
   Maintainer: Maurycy Prodeus <z33d@eth-security.net> 
   
    The wages of sin() is death.
*/
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "task.h"
#include "bcode.h"
#include "module.h"
#include "memory.h"
#include "console.h"
#include "syscall.h"
#include "acman.h"

/*
  32bit<-> |31 30 29 28 27 26 25 24 23 22 21 20 19 18 17| -- integer part
  |16 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0| -- fraction
*/

#define ACCURATE_BITS	17
#define ACCURATE	131072 // multiply value (2^17)
#define CACHE_STEP	0.0001
#define CACHE_SIZE	(int)((2*3.14)/CACHE_STEP)
#define ARC_CACHE_SIZE	(int)(2/CACHE_STEP)

int sin_table[CACHE_SIZE];
int cos_table[CACHE_SIZE];
int tan_table[CACHE_SIZE/2];
int asin_table[ARC_CACHE_SIZE+1];
int acos_table[ARC_CACHE_SIZE+1];
int atan_table[ARC_CACHE_SIZE+1]; // not so really bad ;>

void init_cache()
{
    int i;
    double a=0;
    
    // preparing table of sines and cosines
    for (i=0;i<CACHE_SIZE;i++){
	sin_table[i]=(int)rint(sin(a)*ACCURATE);
	cos_table[i]=(int)rint(cos(a)*ACCURATE);
	a+=CACHE_STEP;
    }
    a=0;
    // preparing table of tangents
    for (i=0;i<CACHE_SIZE/2;i++){
	tan_table[i]=(int)rint(tan(a)*ACCURATE);
	a+=CACHE_STEP;
    }
    a=-1;
    // preparing table of arcsines and arccosines
    for (i=0;i<=ARC_CACHE_SIZE;i++){
	asin_table[i]=(int)rint(asin(a)*ACCURATE);
	acos_table[i]=(int)rint(acos(a)*ACCURATE);
	atan_table[i]=(int)rint(atan(a)*ACCURATE); // stinky ...
	a+=CACHE_STEP;
    }
}

int fix(int a){
  return (a<0 && a>-ACCURATE) ? 0 : a>>ACCURATE_BITS;
}

// u0 - type (0 - noncached, 1 - cached value), value from cache may be
// inaccurate !
// f0 - value in radians
// return: f0 - sine of given value
void sinus(int c)
{
/*
    "(...) Synus ...
	to nie lyczba (...)"
		    Aleksander Dobrzycki
*/
    if (cpu[c].uregs[0]==0)
	cpu[c].fregs[0]=(float)sin((double)cpu[c].fregs[0]);
    else {
	if (fabs(cpu[c].fregs[0])==cpu[c].fregs[0])
	  cpu[c].fregs[0]=(float)sin_table[(int)rint(cpu[c].fregs[0]/CACHE_STEP) 
	      % CACHE_SIZE]/ACCURATE;
	else cpu[c].fregs[0]=(float)-sin_table[(-(int)rint(cpu[c].fregs[0]/CACHE_STEP))
	    % CACHE_SIZE]/ACCURATE;
    }
}

// u0 - type (0 - noncached, 1 - cached value), value from cache may be
// inaccurate !
// f0 - value in radians
// return: f0 - cosine of given value
void cosinus(int c)
{
    if (cpu[c].uregs[0]==0)
	cpu[c].fregs[0]=(float)cos((double)cpu[c].fregs[0]);
    else {
	if (fabs(cpu[c].fregs[0])==cpu[c].fregs[0])
	  cpu[c].fregs[0]=(float)cos_table[(int)rint(cpu[c].fregs[0]/CACHE_STEP)
	      % CACHE_SIZE]/ACCURATE;
	else cpu[c].fregs[0]=(float)cos_table[(-(int)rint(cpu[c].fregs[0]/CACHE_STEP))
	    % CACHE_SIZE]/ACCURATE;
    }
}

// u0 - type (0 - noncached, 1 - cached value), value from cache may be
// inaccurate !
// f0 - value in radians
// return: f0 - tangent of given value
void tangens(int c)
{
    if (cpu[c].uregs[0]==0)
	cpu[c].fregs[0]=(float)tan((double)cpu[c].fregs[0]);
    else {
	if (fabs(cpu[c].fregs[0])==cpu[c].fregs[0])
	  cpu[c].fregs[0]=(float)tan_table[(int)rint(cpu[c].fregs[0]/CACHE_STEP)
	      % (CACHE_SIZE/2)]/ACCURATE;
	else cpu[c].fregs[0]=(float)-tan_table[-(int)rint(cpu[c].fregs[0]/CACHE_STEP)
	    % (CACHE_SIZE/2)]/ACCURATE;
    }

}

// u0 - type (0 - noncached, 1 - cached value), value from cache may be
// inaccurate !
// f0 - value (-1..1)
// return: f0 - arcsine of given value
void asinus(int c)
{
    if (cpu[c].fregs[0]<-1 || cpu[c].fregs[0]>1){
	non_fatal(ERROR_MATH_RANGE,"asin: given value is out of range",c);
	failure=1;
	return;
    }
    if (cpu[c].uregs[0]==0)
	cpu[c].fregs[0]=(float)asin((double)cpu[c].fregs[0]);
    else {
	    cpu[c].fregs[0]=(float)asin_table[(int)((int)rint(cpu[c].fregs[0]/CACHE_STEP)
		+(int)1/CACHE_STEP)]/ACCURATE;
    }
}

// u0 - type (0 - noncached, 1 - cached value), value from cache may be
// inaccurate !
// f0 - value (-1..1)
// return: f0 - arccosine of given value
void acosinus(int c)
{
    if (cpu[c].fregs[0]<-1 || cpu[c].fregs[0]>1){
	non_fatal(ERROR_MATH_RANGE,"acos: given value is out of range",c);
	failure=1;
	return;
    }
    if (cpu[c].uregs[0]==0)
	cpu[c].fregs[0]=(float)acos((double)cpu[c].fregs[0]);
    else {
	    cpu[c].fregs[0]=(float)acos_table[(int)((int)rint(cpu[c].fregs[0]/CACHE_STEP)
	    +(int)1/CACHE_STEP)]/ACCURATE;
    }
}

// u0 - type (0 - noncached, 1 - cached value), value from cache may be
// inaccurate !
// f0 - value (-1..1)
// return: f0 - arctangent of given value
void atangens(int c)
{
    if (cpu[c].fregs[0]<-1 || cpu[c].fregs[0]>1){
	non_fatal(ERROR_MATH_RANGE,"atan: given value is out of range",c);
	failure=1;
	return;
    }
    if (cpu[c].uregs[0]==0)
	cpu[c].fregs[0]=(float)atan((double)cpu[c].fregs[0]);
    else {
	    cpu[c].fregs[0]=(float)atan_table[(int)((int)rint(cpu[c].fregs[0]/CACHE_STEP)
	    +(int)1/CACHE_STEP)]/ACCURATE;
    }
}

// u0 - address of buffer
// s0 - count of sin value
// f0 - first value, f1 - 'step'
// u1 - type (0 - noncached, 1 - cached)
// u2 - type of data ( 0 - int, 1 - float, 2 - unsigned char)
// u3 - value to multiply with results (0 is like 1) ;>
// s1 - step of index (must be bigger than 0)
void fillsin(int c)
{
    unsigned int l,*a, *a_orig;
    unsigned char *a2=NULL, *a2_orig;
    int m,k2,inx;
    float k,tmp;
    
    if (cpu[c].sregs[1]==0){
	non_fatal(ERROR_BAD_PARAM,"fillsin: Bad parameter.",c);
	failure=1;
	return;
    }
    
    if (cpu[c].uregs[2]==2){
	a2=verify_access(c,cpu[c].uregs[0],(cpu[c].sregs[0]*cpu[c].sregs[1]+3)/4,
			MEM_FLAG_WRITE);
	a=(unsigned int*)a2;
    } else a=verify_access(c,cpu[c].uregs[0],(cpu[c].sregs[0]*cpu[c].sregs[1]),
			MEM_FLAG_WRITE);
    if (!a){
	non_fatal(ERROR_PROTFAULT,"fillsin: Attempt to access protected memory",c);
	failure=1;
	return;
    }
    k=cpu[c].fregs[0];
    k2=((int)rint(k/CACHE_STEP)) % CACHE_SIZE; // security, it is a basic!
    inx=((int)rint(cpu[c].fregs[1]/CACHE_STEP)) % CACHE_SIZE;
    if (inx==0){
      if (fabs(cpu[c].fregs[1])==cpu[c].fregs[1])
        inx=1; else inx=-1;
    }
    if (cpu[c].uregs[3]==0)
	m=1;
    else m=cpu[c].uregs[3];
    l=cpu[c].sregs[0]*cpu[c].sregs[1];
    a_orig=a; a2_orig=a2;
    // noncached
    if (cpu[c].uregs[1]==0){
	if (cpu[c].uregs[2]==1)
	while(l>a-a_orig){
          tmp=(float)sin(k)*m;
          memcpy(a,&tmp,4);
          k+=cpu[c].fregs[1];
          a+=cpu[c].sregs[1];
	}
	else if (cpu[c].uregs[2]==0)
	while(l>a-a_orig){
          *a=(unsigned int)rint(sin(k)*m);
          k+=cpu[c].fregs[1];
          a+=cpu[c].sregs[1];
	}
	else while(l>a2-a2_orig){
          *a2=(unsigned char)rint(sin(k)*m);
          k+=cpu[c].fregs[1];
          a2+=cpu[c].sregs[1];
	}
	
    } else { // cached
	if (cpu[c].uregs[2]==1)
	while(l>a-a_orig){
	  if (fabs(k2)==k2)
	    tmp=(float)sin_table[k2]*m/ACCURATE;
	  else tmp=(float)-sin_table[-k2]*m/ACCURATE;
	  memcpy(a,&tmp,4);
	  k2+=inx;
	  k2%=CACHE_SIZE;
	  a+=cpu[c].sregs[1];
	}
	else if (cpu[c].uregs[2]==0)
	while(l>a-a_orig){
	  if (fabs(k2)==k2)
	    *a=(unsigned int)fix(sin_table[k2]*m);
	  else *a=(unsigned int)fix(-sin_table[-k2]*m);
	  k2+=inx;
	  k2%=CACHE_SIZE;
	  a+=cpu[c].sregs[1];
	}
	else while(l>a2-a2_orig){
	      if (fabs(k2)==k2)
		*a2=(unsigned char)fix(sin_table[k2]*m);
	      else *a2=(unsigned char)fix(-sin_table[-k2]*m);
	      k2+=inx;
	      k2%=CACHE_SIZE;
	      a2+=cpu[c].sregs[1];
	    }
    }
}

// u0 - address of buffer
// s0 - count
// f0 - first value, f1 - step
// u1 - type (0 - noncached, 1 - cached)
// u2 - type of data ( 0 - int, 1 - float, 2 - unsigned char)
// u3 - value to multiply with results (0 is like 1) ;>
// s1 - step of index (must be bigger than 0)
void fillcos(int c)
{
    unsigned int l,*a, *a_orig;
    unsigned char *a2=NULL, *a2_orig;
    int m,k2,inx;
    float k,tmp;
    
    if (cpu[c].sregs[1]==0){
	non_fatal(ERROR_BAD_PARAM,"fillcos: Bad parameter.",c);
	failure=1;
	return;
    }
    if (cpu[c].uregs[2]==2){
	a2=verify_access(c,cpu[c].uregs[0],(cpu[c].sregs[0]*cpu[c].sregs[1]+3)/4,
			MEM_FLAG_WRITE);
	a=(unsigned int*)a2;
    } else a=verify_access(c,cpu[c].uregs[0],(cpu[c].sregs[0]*cpu[c].sregs[1]),
			    MEM_FLAG_WRITE);
    if (!a){
	non_fatal(ERROR_PROTFAULT,"fillcos: Attempt to access protected memory",c);
	failure=1;
	return;
    }
    k=cpu[c].fregs[0];
    k2=(int)rint(k/CACHE_STEP) % CACHE_SIZE; // security, it is a basic!
    inx=(int)rint(cpu[c].fregs[1]/CACHE_STEP) % CACHE_SIZE;
    if (inx==0){
      if (fabs(cpu[c].fregs[1])==cpu[c].fregs[1])
        inx=1; else inx=-1;
    }
    if (cpu[c].uregs[3]==0)
	m=1;
    else m=cpu[c].uregs[3];
    l=cpu[c].sregs[0]*cpu[c].sregs[1];
    a_orig=a; a2_orig=a2;
    // noncached
    if (cpu[c].uregs[1]==0){
	if (cpu[c].uregs[2]==1)
	while (l>a-a_orig){
          tmp=(float)cos(k)*m;
          memcpy(a,&tmp,4);
          k+=cpu[c].fregs[1];
          a+=cpu[c].sregs[1];
	}
	else if (cpu[c].uregs[2]==0)
	while (l>a-a_orig){
          *a=(unsigned int)rint(cos(k)*m);
          k+=cpu[c].fregs[1];
          a+=cpu[c].sregs[1];
	}
	else while(l>a2-a2_orig){
              *a2=(unsigned char)rint(cos(k)*m);
              k+=cpu[c].fregs[1];
              a2+=cpu[c].sregs[1];
	}
    } else {// cached
	if (cpu[c].uregs[2]==1) // floaty
	while(l>a-a_orig){
          if (fabs(k2)==k2)
	    tmp=cos_table[k2]*m/ACCURATE;
	  else tmp=cos_table[-k2]*m/ACCURATE;
          memcpy(a,&tmp,4);
          k2+=inx;
          k2%=CACHE_SIZE;
	  a+=cpu[c].sregs[1];
	}
	else if (cpu[c].uregs[2]==0) // inty
	while(l>a-a_orig){
          if (fabs(k2)==k2)
	    *a=(unsigned int)fix(cos_table[k2]*m);
	  else *a=(unsigned int)fix(cos_table[-k2]*m);
          k2+=inx;
	  k2%=CACHE_SIZE;
          a+=cpu[c].sregs[1];
	}
	else while(l>a2-a2_orig){ // chary
              if (fabs(k2)==k2)
		*a2=(unsigned char)fix(cos_table[k2]*m);
	      else *a2=(unsigned char)fix(cos_table[-k2]*m);
              k2+=inx;
	      k2%=CACHE_SIZE;
              a2+=cpu[c].sregs[1];
	}
    }
}

// u0 - address of buffer
// s0 - count
// f0 - first value, f1 - step
// u1 - type (0 - noncached, 1 - cached)
// u2 - type of data ( 0 - int, 1 - float, 2 - unsigned char)
// u3 - value to multiply with results (0 is like 1) ;>
// s1 - step of index (must be bigger than 0)
void filltan(int c)
{
    unsigned int l,*a, *a_orig;
    unsigned int *a2=NULL, *a2_orig;
    int m,k2,inx;
    float k,tmp;
    
    if (cpu[c].sregs[1]==0){
	non_fatal(ERROR_BAD_PARAM,"filltan: Bad parameter.",c);
	failure=1;
	return;
    }
    if (cpu[c].uregs[2]==2){
	a2=verify_access(c,cpu[c].uregs[0],(cpu[c].sregs[0]*cpu[c].sregs[1]+3)/4,
			MEM_FLAG_WRITE);
	a=(unsigned int*)a2;
    } else a=verify_access(c,cpu[c].uregs[0],(cpu[c].sregs[0]*cpu[c].sregs[1]),
			MEM_FLAG_WRITE);
    if (!a){
	non_fatal(ERROR_PROTFAULT,"filltan: Attempt to access protected memory",c);
	failure=1;
	return;
    }
    k=cpu[c].fregs[0];
    k2=(int)rint(k/CACHE_STEP) % (CACHE_SIZE/2); // security, it is a basic!
    inx=(int)rint(cpu[c].fregs[1]/CACHE_STEP) % (CACHE_SIZE/2);
    if (inx==0){
      if (fabs(cpu[c].fregs[1])==cpu[c].fregs[1])
        inx=1; else inx=-1;
    }
    if (cpu[c].uregs[3]==0)
	m=1;
    else m=cpu[c].uregs[3];
    l=cpu[c].sregs[0]*cpu[c].sregs[1];
    a_orig=a; a2_orig=a2;
    // noncached
    if (cpu[c].uregs[1]==0){
	if (cpu[c].uregs[2]==1)
	while(l>a-a_orig){
          tmp=(float)tan(k)*m;
          memcpy(a,&tmp,4);
          k+=cpu[c].fregs[1];
          a+=cpu[c].sregs[1];
	}
	else if (cpu[c].uregs[2]==0)
	while (l>a-a_orig){
          *a=(unsigned int)rint(tan(k)*m);
          k+=cpu[c].fregs[1];
          a+=cpu[c].sregs[1];
	}
	else while(l>a2-a2_orig){
              *a2=(unsigned char)rint(tan(k)*m);
              k+=cpu[c].fregs[1];
              a2+=cpu[c].sregs[1];
	}
    } else { // cached
	if (cpu[c].uregs[2]==1) // floaty
	while(l>a-a_orig){
          if (fabs(k2)==k2)
	    tmp=tan_table[k2]*m/ACCURATE;
	  else tmp=-tan_table[-k2]*m/ACCURATE;
          memcpy(a,&tmp,4);
          k2+=inx;
	  k2%=(CACHE_SIZE/2);
          a+=cpu[c].sregs[1];
	}
	else if (cpu[c].uregs[2]==0) // inty
	while (l>a2-a2_orig){
          if (fabs(k2)==k2)
	      *a=(unsigned int)fix(tan_table[k2]*m);
	  else *a=(unsigned int)fix(-tan_table[-k2]*m);
          k2+=inx;
	  k2%=(CACHE_SIZE/2);
          a+=cpu[c].sregs[1];
	}
	else while (l>(cpu[c].sregs[0]*cpu[c].sregs[1])){ // chary
              if (fabs(k2)==k2)
	          *a2=(unsigned char)fix(tan_table[k2]*m);
	      else *a2=(unsigned char)fix(-tan_table[-k2]*m);
              k2+=inx;
              k2%=(CACHE_SIZE/2);
	      a2+=cpu[c].sregs[1];
	}
    }
}
// jej oczy niebieskie [...]

void tablemul_byte(int c)
{
    unsigned char *a,*b,*b_orig,i;
    int m;
    
    a=verify_access(c,cpu[c].uregs[0],(cpu[c].uregs[1]+3)/4, MEM_FLAG_READ|MEM_FLAG_WRITE);
    if (!a){
	non_fatal(ERROR_PROTFAULT,"table_mul: Attempt to access protected memory",c);
	failure=1;
	return;
    }
    b=verify_access(c,cpu[c].uregs[3],(cpu[c].uregs[4]+3)/4, MEM_FLAG_READ);
    if (!b){
	non_fatal(ERROR_PROTFAULT,"table_mul: Attempt to access protected memory",c);
	failure=1;
	return;
    }
    if (cpu[c].uregs[4]<=0 || cpu[c].uregs[1]<=0)
	return;
    b_orig=b;
    if (cpu[c].uregs[7]==0)
	m=1;
    else m=cpu[c].uregs[7];
    for (i=0;i<cpu[c].uregs[1];i++){
	if (b-b_orig==cpu[c].uregs[4])
	  b=b_orig;
	*a=(*a)*(*b)*m;	
	a++;
	b++;
    }
}
// u0 - address of first table, u1 - size of table 
// u2 - type (0-int, 1-float, 2-unsigned char)
// u3 - address of second table, u4 - size 
// u5 - type (0-int, 1-float, 2-unsigned char)
// u6 - type of results (0 - int, 1 - float, 2 - unsigned char)
// u7 - value to multiply with results (0 is like 1 ;>)
void tablemul(int c)
{
    unsigned int *a,*b,*b_orig,i;
    int m;
    
    // long, very long night ;>
    if (cpu[c].uregs[2]==2 || cpu[c].uregs[5]==2 || cpu[c].uregs[6]==2){
      if (cpu[c].uregs[2]!=2 || cpu[c].uregs[5]!=2 || cpu[c].uregs[6]!=2){
          non_fatal(ERROR_MEM_FORMAT,"table_mul: Sorry, byte conversion is unimplemented.",c);
	  failure=1;
	  return;
      } else {
          tablemul_byte(c);
    	  return;
	}
    }
    a=verify_access(c,cpu[c].uregs[0],cpu[c].uregs[1], MEM_FLAG_READ|MEM_FLAG_WRITE);
    if (!a){
	non_fatal(ERROR_PROTFAULT,"table_mul: Attempt to access protected memory",c);
	failure=1;
	return;
    }
    b=verify_access(c,cpu[c].uregs[3],cpu[c].uregs[4], MEM_FLAG_READ);
    if (!b){
	non_fatal(ERROR_PROTFAULT,"table_mul: Attempt to access protected memory",c);
	failure=1;
	return;
    }
    if (cpu[c].uregs[4]<=0 || cpu[c].uregs[1]<=0)
	return;
    b_orig=b;
    if (cpu[c].uregs[7]==0)
	m=1;
    else m=cpu[c].uregs[7];
    
    if (cpu[c].uregs[6]==0){
    // yeah, and now we have 4 cases
    if (cpu[c].uregs[2]==0 && cpu[c].uregs[5]==0)
      for (i=0;i<cpu[c].uregs[1];i++){
	if (b-b_orig==cpu[c].uregs[4])
	  b=b_orig;
	*a=(*a)*(*b)*m;	
	a++;
	b++;
      }
    else if (cpu[c].uregs[2]==1 && cpu[c].uregs[5]==1)
	    for (i=0;i<cpu[c].uregs[1];i++){
		if (b-b_orig==cpu[c].uregs[4])
	          b=b_orig;
		*a=(int)rint((*(float*)a)*(*(float*)b)*m);	
		a++;
		b++;
            }
    else if (cpu[c].uregs[2]==0 && cpu[c].uregs[5]==1)
	    for (i=0;i<cpu[c].uregs[1];i++){
		if (b-b_orig==cpu[c].uregs[4])
	          b=b_orig;
		*a=(int)rint((*a)*(*(float*)b)*m);	
		a++;
		b++;
            }
    else if (cpu[c].uregs[2]==1 && cpu[c].uregs[5]==0)
	    for (i=0;i<cpu[c].uregs[1];i++){
		if (b-b_orig==cpu[c].uregs[4])
	          b=b_orig;
		*a=(int)rint((*(float*)a)*(*b)*m);	
		a++;
		b++;
            }
    } else { // end of type int
    // yeah, and now we have another 4 cases
    if (cpu[c].uregs[2]==0 && cpu[c].uregs[5]==0)
      for (i=0;i<cpu[c].uregs[1];i++){
	if (b-b_orig==cpu[c].uregs[4])
	  b=b_orig;
	*(float*)a=(float)(*a)*(*b)*m;	
	a++;
	b++;
      }
    else if (cpu[c].uregs[2]==1 && cpu[c].uregs[5]==1)
	    for (i=0;i<cpu[c].uregs[1];i++){
		if (b-b_orig==cpu[c].uregs[4])
	          b=b_orig;
		*(float*)a=(float)((*(float*)a)*(*(float*)b)*m);	
		a++;
		b++;
            }
    else if (cpu[c].uregs[2]==0 && cpu[c].uregs[5]==1)
	    for (i=0;i<cpu[c].uregs[1];i++){
		if (b-b_orig==cpu[c].uregs[4])
	          b=b_orig;
		*(float*)a=(float)((*a)*(*(float*)b)*m);	
		a++;
		b++;
            }
    else if (cpu[c].uregs[2]==1 && cpu[c].uregs[5]==0)
	    for (i=0;i<cpu[c].uregs[1];i++){
		if (b-b_orig==cpu[c].uregs[4])
	          b=b_orig;
		*(float*)a=(float)((*(float*)a)*(*b)*m);	
		a++;
		b++;
            }
    }
}

void tablediv_byte(int c)
{
    unsigned char *a,*b,*b_orig,i;
    int m;
    
    a=verify_access(c,cpu[c].uregs[0],(cpu[c].uregs[1]+3)/4, MEM_FLAG_READ|MEM_FLAG_WRITE);
    if (!a){
	non_fatal(ERROR_PROTFAULT,"table_div: Attempt to access protected memory",c);
	failure=1;
	return;
    }
    b=verify_access(c,cpu[c].uregs[3],(cpu[c].uregs[4]+3)/4, MEM_FLAG_READ);
    if (!b){
	non_fatal(ERROR_PROTFAULT,"table_div: Attempt to access protected memory",c);
	failure=1;
	return;
    }
    if (cpu[c].uregs[4]<=0 || cpu[c].uregs[1]<=0)
	return;
    b_orig=b;
    if (cpu[c].uregs[7]==0)
	m=1;
    else m=cpu[c].uregs[7];
    for (i=0;i<cpu[c].uregs[1];i++){
	if (b-b_orig==cpu[c].uregs[4])
	  b=b_orig;
	*a=(*a)/(*b)*m;	
	a++;
	b++;
    }
}
// u0 - address of first table, u1 - size of table
// u2 - type (0-int, 1-float, 2-unsigned char)
// u3 - address of second table, u4 - size
// u5 - type (0-int, 1-float, 2-unsigned char)
// u6 - type of results (0-int, 1-float, 2-unsigned char)
// u7 - value to multiply with results (0 is like 1 ;>)
void tablediv(int c)
{
    unsigned int *a,*b,*b_orig,i;
    int m;
    
    // long, very long night ;>
    if (cpu[c].uregs[2]==2 || cpu[c].uregs[5]==2 || cpu[c].uregs[6]==2){
      if (cpu[c].uregs[2]!=2 || cpu[c].uregs[5]!=2 || cpu[c].uregs[6]!=2){
          non_fatal(ERROR_MEM_FORMAT,"table_div: Sorry, byte conversion is unimplemented.",c);
	  failure=1;
	  return;
      } else {
        tablediv_byte(c);
        return;
      }
    }
    a=verify_access(c,cpu[c].uregs[0],cpu[c].uregs[1], MEM_FLAG_READ|MEM_FLAG_WRITE);
    if (!a){
	non_fatal(ERROR_PROTFAULT,"table_div: Attempt to access protected memory",c);
	failure=1;
	return;
    }
    b=verify_access(c,cpu[c].uregs[3],cpu[c].uregs[4], MEM_FLAG_READ);
    if (!b){
	non_fatal(ERROR_PROTFAULT,"table_div: Attempt to access protected memory",c);
	failure=1;
	return;
    }
    if (cpu[c].uregs[4]<=0 || cpu[c].uregs[1]<=0)
	return;
    b_orig=b;
    for (i=0;i<cpu[c].uregs[4];i++)
      if (*(b++)==0x00000000){
        non_fatal(ERROR_MATH_DIV,"table_div: Division by zero",c);
	failure=1;
	return;
      }
    b=b_orig;

    if (cpu[c].uregs[7]==0)
	m=1;
    else m=cpu[c].uregs[7];
    
    if (cpu[c].uregs[6]==0){
    // yeah, and now we have 4 cases
    if (cpu[c].uregs[2]==0 && cpu[c].uregs[5]==0)
      for (i=0;i<cpu[c].uregs[1];i++){
	if (b-b_orig==cpu[c].uregs[4])
	  b=b_orig;
	*a=(*a)/(*b)*m;	
	a++;
	b++;
      }
    else if (cpu[c].uregs[2]==1 && cpu[c].uregs[5]==1)
	    for (i=0;i<cpu[c].uregs[1];i++){
		if (b-b_orig==cpu[c].uregs[4])
	          b=b_orig;
		*a=(int)rint((*(float*)a)/(*(float*)b)*m);	
		a++;
		b++;
            }
    else if (cpu[c].uregs[2]==0 && cpu[c].uregs[5]==1)
	    for (i=0;i<cpu[c].uregs[1];i++){
		if (b-b_orig==cpu[c].uregs[4])
	          b=b_orig;
		*a=(int)rint((*a)/(*(float*)b)*m);	
		a++;
		b++;
            }
    else if (cpu[c].uregs[2]==1 && cpu[c].uregs[5]==0)
	    for (i=0;i<cpu[c].uregs[1];i++){
		if (b-b_orig==cpu[c].uregs[4])
	          b=b_orig;
		*a=(int)rint((*(float*)a)/(*b)*m);	
		a++;
		b++;
            }
    } else { // end of type int
    // yeah, and now we have another 4 cases
    if (cpu[c].uregs[2]==0 && cpu[c].uregs[5]==0)
      for (i=0;i<cpu[c].uregs[1];i++){
	if (b-b_orig==cpu[c].uregs[4])
	  b=b_orig;
	*(float*)a=(float)(*a)/(*b)*m;	
	a++;
	b++;
      }
    else if (cpu[c].uregs[2]==1 && cpu[c].uregs[5]==1)
	    for (i=0;i<cpu[c].uregs[1];i++){
		if (b-b_orig==cpu[c].uregs[4])
	          b=b_orig;
		*(float*)a=(float)((*(float*)a)/(*(float*)b)*m);	
		a++;
		b++;
            }
    else if (cpu[c].uregs[2]==0 && cpu[c].uregs[5]==1)
	    for (i=0;i<cpu[c].uregs[1];i++){
		if (b-b_orig==cpu[c].uregs[4])
	          b=b_orig;
		*(float*)a=(float)((*a)/(*(float*)b)*m);	
		a++;
		b++;
            }
    else if (cpu[c].uregs[2]==1 && cpu[c].uregs[5]==0)
	    for (i=0;i<cpu[c].uregs[1];i++){
		if (b-b_orig==cpu[c].uregs[4])
	          b=b_orig;
		*(float*)a=(float)((*(float*)a)/(*b)*m);	
		a++;
		b++;
            }
    }

}

void tableadd_byte(int c)
{
    unsigned char *a,*b,*b_orig,i;
    int m;
    
    a=verify_access(c,cpu[c].uregs[0],(cpu[c].uregs[1]+3)/4, MEM_FLAG_READ|MEM_FLAG_WRITE);
    if (!a){
	non_fatal(ERROR_PROTFAULT,"table_add: Attempt to access protected memory",c);
	failure=1;
	return;
    }
    b=verify_access(c,cpu[c].uregs[3],(cpu[c].uregs[4]+3)/4, MEM_FLAG_READ);
    if (!b){
	non_fatal(ERROR_PROTFAULT,"table_add: Attempt to access protected memory",c);
	failure=1;
	return;
    }
    if (cpu[c].uregs[4]<=0 || cpu[c].uregs[1]<=0)
	return;
    b_orig=b;
    if (cpu[c].uregs[7]==0)
	m=1;
    else m=cpu[c].uregs[7];
    for (i=0;i<cpu[c].uregs[1];i++){
	if (b-b_orig==cpu[c].uregs[4])
	  b=b_orig;
	*a=((*a)+(*b))*m;	
	a++;
	b++;
    }

}

// u0 - address of first table, u1 - size of table
// u2 - type (0-int, 1-float, 2-unsigned char)
// u3 - address of second table, u4 - size 
// u5 - type (0-int, 1-float, 2-unsigned char)
// u6 - type of results (0-int, 1-float, 2-unsigned char)
// u7 - value to multiply with results (0 is like 1 ;>)
void tableadd(int c)
{
    unsigned int *a,*b,*b_orig,i;
    int m;
    
    // long, very long night ;>
    if (cpu[c].uregs[2]==2 || cpu[c].uregs[5]==2 || cpu[c].uregs[6]==2){
      if (cpu[c].uregs[2]!=2 || cpu[c].uregs[5]!=2 || cpu[c].uregs[6]!=2){
          non_fatal(ERROR_MEM_FORMAT,"table_add: Sorry, byte conversion is unimplemented.",c);
	  failure=1;
	  return;
      } else {
        tableadd_byte(c);
        return;
      }
    }
    
    a=verify_access(c,cpu[c].uregs[0],cpu[c].uregs[1], MEM_FLAG_READ|MEM_FLAG_WRITE);
    if (!a){
	non_fatal(ERROR_PROTFAULT,"table_add: Attempt to access protected memory",c);
	failure=1;
	return;
    }
    b=verify_access(c,cpu[c].uregs[3],cpu[c].uregs[4], MEM_FLAG_READ);
    if (!b){
	non_fatal(ERROR_PROTFAULT,"table_add: Attempt to access protected memory",c);
	failure=1;
	return;
    }
    if (cpu[c].uregs[4]<=0 || cpu[c].uregs[1]<=0)
	return;
    b_orig=b;
    if (cpu[c].uregs[7]==0)
	m=1;
    else m=cpu[c].uregs[7];
    
    if (cpu[c].uregs[6]==0){
    // yeah, and now we have 4 cases
    if (cpu[c].uregs[2]==0 && cpu[c].uregs[5]==0)
      for (i=0;i<cpu[c].uregs[1];i++){
	if (b-b_orig==cpu[c].uregs[4])
	  b=b_orig;
	*a=(*a)+(*b)*m;	
	a++;
	b++;
      }
    else if (cpu[c].uregs[2]==1 && cpu[c].uregs[5]==1)
	    for (i=0;i<cpu[c].uregs[1];i++){
		if (b-b_orig==cpu[c].uregs[4])
	          b=b_orig;
		*a=(int)rint((*(float*)a)+(*(float*)b)*m);	
		a++;
		b++;
            }
    else if (cpu[c].uregs[2]==0 && cpu[c].uregs[5]==1)
	    for (i=0;i<cpu[c].uregs[1];i++){
		if (b-b_orig==cpu[c].uregs[4])
	          b=b_orig;
		*a=(int)rint((*a)+(*(float*)b)*m);	
		a++;
		b++;
            }
    else if (cpu[c].uregs[2]==1 && cpu[c].uregs[5]==0)
	    for (i=0;i<cpu[c].uregs[1];i++){
		if (b-b_orig==cpu[c].uregs[4])
	          b=b_orig;
		*a=(int)rint((*(float*)a)+(*b)*m);	
		a++;
		b++;
            }
    } else { // end of type int
    // yeah, and now we have another 4 cases
    if (cpu[c].uregs[2]==0 && cpu[c].uregs[5]==0)
      for (i=0;i<cpu[c].uregs[1];i++){
	if (b-b_orig==cpu[c].uregs[4])
	  b=b_orig;
	*(float*)a=(float)(*a)+(*b)*m;	
	a++;
	b++;
      }
    else if (cpu[c].uregs[2]==1 && cpu[c].uregs[5]==1)
	    for (i=0;i<cpu[c].uregs[1];i++){
		if (b-b_orig==cpu[c].uregs[4])
	          b=b_orig;
		*(float*)a=(float)((*(float*)a)+(*(float*)b)*m);	
		a++;
		b++;
            }
    else if (cpu[c].uregs[2]==0 && cpu[c].uregs[5]==1)
	    for (i=0;i<cpu[c].uregs[1];i++){
		if (b-b_orig==cpu[c].uregs[4])
	          b=b_orig;
		*(float*)a=(float)((*a)+(*(float*)b)*m);	
		a++;
		b++;
            }
    else if (cpu[c].uregs[2]==1 && cpu[c].uregs[5]==0)
	    for (i=0;i<cpu[c].uregs[1];i++){
		if (b-b_orig==cpu[c].uregs[4])
	          b=b_orig;
		*(float*)a=(float)((*(float*)a)+(*b)*m);	
		a++;
		b++;
            }
    }
}

void tablesub_byte(int c)
{
    unsigned char *a,*b,*b_orig,i;
    int m;
    
    a=verify_access(c,cpu[c].uregs[0],(cpu[c].uregs[1]+3)/4, MEM_FLAG_READ|MEM_FLAG_WRITE);
    if (!a){
	non_fatal(ERROR_PROTFAULT,"table_sub: Attempt to access protected memory",c);
	failure=1;
	return;
    }
    b=verify_access(c,cpu[c].uregs[3],(cpu[c].uregs[4]+3)/4, MEM_FLAG_READ);
    if (!b){
	non_fatal(ERROR_PROTFAULT,"table_sub: Attempt to access protected memory",c);
	failure=1;
	return;
    }
    if (cpu[c].uregs[4]<=0 || cpu[c].uregs[1]<=0)
	return;
    b_orig=b;
    if (cpu[c].uregs[7]==0)
	m=1;
    else m=cpu[c].uregs[7];
    for (i=0;i<cpu[c].uregs[1];i++){
	if (b-b_orig==cpu[c].uregs[4])
	  b=b_orig;
	*a=((*a)-(*b))*m;	
	a++;
	b++;
    }
}

// u0 - address of first table, u1 - size of table
// u2 - type (0-int, 1-float, 2-unsigned char)
// u3 - address of second table, u4 - size
// u5 - type (0-int, 1-float, 2-unsigned char)
// u6 - type of results (0-int, 1-float, 2-unsigned char)
// u7 - value to multiply with results (0 is like 1 ;>)
void tablesub(int c)
{
    unsigned int *a,*b,*b_orig,i;
    int m;
    
    // long, very long night ;>
    if (cpu[c].uregs[2]==2 || cpu[c].uregs[5]==2 || cpu[c].uregs[6]==2){
      if (cpu[c].uregs[2]!=2 || cpu[c].uregs[5]!=2 || cpu[c].uregs[6]!=2){
          non_fatal(ERROR_MEM_FORMAT,"table_sub: Sorry, byte conversion is unimplemented.",c);
	  failure=1;
	  return;
      } else { 
        tablesub_byte(c);
        return;
      }
    }
    
    a=verify_access(c,cpu[c].uregs[0],cpu[c].uregs[1], MEM_FLAG_READ|MEM_FLAG_WRITE);
    if (!a){
	non_fatal(ERROR_PROTFAULT,"table_sub: Attempt to access protected memory",c);
	failure=1;
	return;
    }
    b=verify_access(c,cpu[c].uregs[3],cpu[c].uregs[4], MEM_FLAG_READ);
    if (!b){
	non_fatal(ERROR_PROTFAULT,"table_sub: Attempt to access protected memory",c);
	failure=1;
	return;
    }
    if (cpu[c].uregs[4]<=0 || cpu[c].uregs[1]<=0)
	return;
    b_orig=b;
    if (cpu[c].uregs[7]==0)
	m=1;
    else m=cpu[c].uregs[7];
    
    if (cpu[c].uregs[6]==0){
    // yeah, and now we have 4 cases
    if (cpu[c].uregs[2]==0 && cpu[c].uregs[5]==0)
      for (i=0;i<cpu[c].uregs[1];i++){
	if (b-b_orig==cpu[c].uregs[4])
	  b=b_orig;
	*a=(*a)-(*b)*m;	
	a++;
	b++;
      }
    else if (cpu[c].uregs[2]==1 && cpu[c].uregs[5]==1)
	    for (i=0;i<cpu[c].uregs[1];i++){
		if (b-b_orig==cpu[c].uregs[4])
	          b=b_orig;
		*a=(int)rint((*(float*)a)-(*(float*)b)*m);	
		a++;
		b++;
            }
    else if (cpu[c].uregs[2]==0 && cpu[c].uregs[5]==1)
	    for (i=0;i<cpu[c].uregs[1];i++){
		if (b-b_orig==cpu[c].uregs[4])
	          b=b_orig;
		*a=(int)rint((*a)-(*(float*)b)*m);	
		a++;
		b++;
            }
    else if (cpu[c].uregs[2]==1 && cpu[c].uregs[5]==0)
	    for (i=0;i<cpu[c].uregs[1];i++){
		if (b-b_orig==cpu[c].uregs[4])
	          b=b_orig;
		*a=(int)rint((*(float*)a)-(*b)*m);	
		a++;
		b++;
            }
    } else { // end of type int
    // yeah, and now we have another 4 cases
    if (cpu[c].uregs[2]==0 && cpu[c].uregs[5]==0)
      for (i=0;i<cpu[c].uregs[1];i++){
	if (b-b_orig==cpu[c].uregs[4])
	  b=b_orig;
	*(float*)a=(float)(*a)-(*b)*m;	
	a++;
	b++;
      }
    else if (cpu[c].uregs[2]==1 && cpu[c].uregs[5]==1)
	    for (i=0;i<cpu[c].uregs[1];i++){
		if (b-b_orig==cpu[c].uregs[4])
	          b=b_orig;
		*(float*)a=(float)((*(float*)a)-(*(float*)b)*m);	
		a++;
		b++;
            }
    else if (cpu[c].uregs[2]==0 && cpu[c].uregs[5]==1)
	    for (i=0;i<cpu[c].uregs[1];i++){
		if (b-b_orig==cpu[c].uregs[4])
	          b=b_orig;
		*(float*)a=(float)((*a)-(*(float*)b)*m);	
		a++;
		b++;
            }
    else if (cpu[c].uregs[2]==1 && cpu[c].uregs[5]==0)
	    for (i=0;i<cpu[c].uregs[1];i++){
		if (b-b_orig==cpu[c].uregs[4])
	          b=b_orig;
		*(float*)a=(float)((*(float*)a)-(*b)*m);	
		a++;
		b++;
            }
    }
}
void syscall_load(int *x)
{
    printk(">> WARNING: math module is an experimental code.\n");
    printk(">> It still requires a lot of debugging and optimizations.\n");
    printk(">> Please do not load this module if you do not need it.\n");
    printk(">> Math module loaded.\n");
    printk(">> Cache size: %d\n",CACHE_SIZE);
    init_cache();
    *(x++)=SYSCALL_MATH_SIN;
    *(x++)=SYSCALL_MATH_COS;
    *(x++)=SYSCALL_MATH_TAN;
    *(x++)=SYSCALL_MATH_ASIN;
    *(x++)=SYSCALL_MATH_ACOS;
    *(x++)=SYSCALL_MATH_ATAN;
    *(x++)=SYSCALL_MATH_FILLSIN;
    *(x++)=SYSCALL_MATH_FILLCOS;
    *(x++)=SYSCALL_MATH_FILLTAN;
    *(x++)=SYSCALL_MATH_TABLE_MUL;
    *(x++)=SYSCALL_MATH_TABLE_DIV;
    *(x++)=SYSCALL_MATH_TABLE_ADD;
    *(x++)=SYSCALL_MATH_TABLE_SUB;
    *(x)=SYSCALL_ENDLIST;
}
void syscall_handler(int c, int nr)
{
    switch(nr) {
	case SYSCALL_MATH_SIN: sinus(c); break;
	case SYSCALL_MATH_COS: cosinus(c); break;
	case SYSCALL_MATH_TAN: tangens(c); break;
	case SYSCALL_MATH_ASIN: asinus(c); break;
	case SYSCALL_MATH_ACOS: acosinus(c); break;
	case SYSCALL_MATH_ATAN: atangens(c); break;
	case SYSCALL_MATH_FILLSIN: fillsin(c); break;
	case SYSCALL_MATH_FILLCOS: fillcos(c); break;
	case SYSCALL_MATH_FILLTAN: filltan(c); break;
	case SYSCALL_MATH_TABLE_MUL: tablemul(c); break;
	case SYSCALL_MATH_TABLE_DIV: tablediv(c); break;
	case SYSCALL_MATH_TABLE_ADD: tableadd(c); break;
	case SYSCALL_MATH_TABLE_SUB: tablesub(c); break;
    }
}
