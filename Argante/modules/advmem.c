/*

   Argante virtual OS
   ------------------

   Advanced str* / mem* routines (for bounded strings)
   
   Status: done
   
   Author:     Maurycy Prodeus <z33d@eth-security.net>
   Maintainer: Maurycy Prodeus <z33d@eth-security.net>
   Patched:    Bulba <bulba@intelcom.pl>
    
    "One kiss, one kiss, of your lily-white lips,
     One kiss all I crave..."
                         English Madrigals
*/

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "config.h"
#include "task.h"
#include "bcode.h"
#include "module.h"
#include "memory.h"
#include "console.h"
#include "syscall.h"
#include "acman.h"

// u0 - destination, u1 - offset of destination (in dword)
// u2 - source, u3 - offset of source (in dword), u4 - size in bytes
void mem_strcpy(int c)
{
    char *a, *b;
    if (cpu[c].uregs[1]<0 || cpu[c].uregs[3]<0){
	non_fatal(ERROR_MEM_OFFSET,"strcpy: Illegal offset",c);
	failure=1;
	return;
    }
    a=verify_access(c,cpu[c].uregs[0]+(cpu[c].uregs[1]/4),
      (cpu[c].uregs[4]+3)/4,MEM_FLAG_WRITE);
    if (!a){
	non_fatal(ERROR_PROTFAULT,"strcpy: Destination memory unsuitable",c); 
	failure=1;
	return;
    }
    b=verify_access(c,cpu[c].uregs[2]+(cpu[c].uregs[3]/4),
      (cpu[c].uregs[4]+3)/4,MEM_FLAG_READ);
    if (!b){
	non_fatal(ERROR_PROTFAULT,"strcpy: Source memory unsuitable",c);
	failure=1;
	return;
    }
    a+=cpu[c].uregs[1] % 4;
    b+=cpu[c].uregs[3] % 4;
    // so we must add offset (0..3)
    memcpy(a,b,cpu[c].uregs[4]);
}


void mem_getchar(int c) {
  char* a;
  a=verify_access(c,cpu[c].uregs[0]+(cpu[c].uregs[1]/4),1,MEM_FLAG_READ);
  if (!a){
    non_fatal(ERROR_PROTFAULT,"getchar: source memory unsuitable",c); 
    failure=1;
    return;
  }
  a += ( cpu[c].uregs[1] % 4 );
  cpu[c].uregs[0]=*a;
}


// u0 - address, u1 - offset, u2 - byte, u3 - size in bytes
void mem_memset(int c)
{
    char *a;
    if (cpu[c].uregs[1]<0){
	non_fatal(ERROR_MEM_OFFSET,"memset: Illegal offset",c);
	failure=1;
	return;
    }
    a=verify_access(c,cpu[c].uregs[0]+(cpu[c].uregs[1]/4),
      (cpu[c].uregs[3]+3)/4,MEM_FLAG_WRITE);
    if (!a){
	non_fatal(ERROR_PROTFAULT,"memset: Attempt to access protected memory",c);
	failure=1;
	return;
    }
    a+=cpu[c].uregs[1] % 4;
    memset(a,cpu[c].uregs[2],cpu[c].uregs[3]);
}

#define ENDIAN_BIG 0
#define ENDIAN_LITTLE 1
#define ENDIAN_NATIVE 2

#if __BYTE_ORDER == __BIG_ENDIAN
#  define ENDIAN_NOW ENDIAN_BIG
#else
#  define ENDIAN_NOW ENDIAN_LITTLE
#endif

void mem_endian(int c)
{
    int *a;
    int cip=0,cm;
    int s=cpu[c].uregs[2],t=cpu[c].uregs[3];
    a=(int*)verify_access(c,cpu[c].uregs[0],(cpu[c].uregs[1]+3)/4,MEM_FLAG_WRITE);
    if (!a){
	non_fatal(ERROR_PROTFAULT,"endian: Attempt to access protected memory",c);
	failure=1;
	return;
    }
   if (s==ENDIAN_NATIVE) s=ENDIAN_NOW;
   if (t==ENDIAN_NATIVE) t=ENDIAN_NOW;
   if (s==t) return; // Do nothing request ;>
   if (s!=0 && s!=1) {
     non_fatal(ERROR_BAD_SYS_PARAM,"endian: Invalid source endian specified",c);
     failure=1;
     return;
   }
   if (t!=0 && t!=1) {
     non_fatal(ERROR_BAD_SYS_PARAM,"endian: Invalid destination endian specified",c);
     failure=1;
     return;
   }

   // Party time!
   cm=cpu[c].uregs[1]/4;
   for (cip=0;cip<cm;cip++) {
     *a= ( ( *a & 0xff ) << 24 ) +
         ( ( *a & 0xff00 ) << 8 ) +
         ( ( *a & 0xff0000 ) >> 8 ) +
         ( ( *a & 0xff000000 ) >> 24) ;
     a++;
   }

}








// u0 - address, u1 - offset, u2 - size
void mem_bzero(int c)
{
    char *a;
    if (cpu[c].uregs[1]<0){
	non_fatal(ERROR_MEM_OFFSET,"bzero: Illegal offset",c);
	failure=1;
	return;
    }
    a=verify_access(c,cpu[c].uregs[0]+(cpu[c].uregs[1]/4),
      (cpu[c].uregs[2]+3)/4,MEM_FLAG_WRITE);
    if (!a){
	non_fatal(ERROR_PROTFAULT,"bzero: Attempt to access protected memory",c);
	failure=1;
	return;
    }
    a+=cpu[c].uregs[1] % 4;
    memset(a,0,cpu[c].uregs[2]);
}
// u0 - address, u1 - offset, u2 - char, u3 - size in bytes
// return: u0 - pointer to the matching dword, u1 - offset to specific byte
// in this dword, (only when u2 is != 0 -> char is found)
void mem_strchr(int c)
{
    char *a, *b;
    
    if (cpu[c].uregs[1]<0){
	non_fatal(ERROR_MEM_OFFSET,"strchr: Illegal offset",c);
	failure=1;
	return;
    }
    a=verify_access(c,cpu[c].uregs[0]+(cpu[c].uregs[1]/4),
      (cpu[c].uregs[3]+3)/4,MEM_FLAG_READ);
    if (!a){
	non_fatal(ERROR_PROTFAULT,"strchr: Attempt to access protected memory",c);
	failure=1;
	return;
    }
    a+=cpu[c].uregs[1] % 4;
    b=memchr(a,cpu[c].uregs[2],cpu[c].uregs[3]);
    if (b){
      cpu[c].uregs[0]+=(b-a)/4+(cpu[c].uregs[1]/4);
      cpu[c].uregs[1]=(b-a) % 4;
      cpu[c].uregs[2]=1;
    } else {
      cpu[c].uregs[2]=0;
    }
}
// u0 - address, u1 - offset, u2 - char, u3 - size in bytes
// return: u0 - pointer to the matching dword, u1 - offset to specific byte
// in this dword (only when u2 is != 0 -> char is found)
void mem_strrchr(int c)
{
    char *a, *a_old;
    
    if (cpu[c].uregs[1]<0){
	non_fatal(ERROR_MEM_OFFSET,"strrchr: Illegal offset",c);
	failure=1;
	return;
    }
    a=verify_access(c,cpu[c].uregs[0]+(cpu[c].uregs[1]/4),
      (cpu[c].uregs[3]+3)/4,MEM_FLAG_READ);
    if (!a){
	non_fatal(ERROR_PROTFAULT,"strrchr: Attempt to access protected memory",c);
	failure=1;
	return;
    }
    a+=cpu[c].uregs[1] % 4;
    a_old=a;
    a+=cpu[c].uregs[3]-1;
    if (a_old>a){
	non_fatal(ERROR_PROTFAULT,"strrchr: Attempt to access protected memory",c);
	failure=1;
	return;
    }
    
    while(*a!=cpu[c].uregs[2] && a>=a_old)
	a--;
    if (a>=a_old){
      cpu[c].uregs[0]+=(a-a_old)/4+(cpu[c].uregs[1]/4);
      cpu[c].uregs[1]=(a-a_old) % 4;
      cpu[c].uregs[2]=1;
    } else {
      cpu[c].uregs[2]=0;
    }
}

// u0 - address of first string, u1 - offset of first string
// u2 - address of second string, u3 - offset of second string
// u4 - size in bytes of first string
// return: like strcmp, strncmp or memcmp ...
void mem_strcmp(int c)
{
    char *a, *b;

    if (cpu[c].uregs[1]<0 || cpu[c].uregs[3]<0){
        non_fatal(ERROR_MEM_OFFSET,"strcmp: Illegal offset",c);
        failure=1;
	return;
    }
    a=verify_access(c,cpu[c].uregs[0]+(cpu[c].uregs[1]/4),
      (cpu[c].uregs[4]+3)/4,MEM_FLAG_READ);
    if (!a){
        non_fatal(ERROR_PROTFAULT,"strcmp: First string unsuitable",c);
        failure=1;
	return;
    }
    b=verify_access(c,cpu[c].uregs[2]+(cpu[c].uregs[3]/4),
      (cpu[c].uregs[4]+3)/4,MEM_FLAG_READ);
    if (!b){
	non_fatal(ERROR_PROTFAULT,"strcmp: Second string unsuitable",c);
	failure=1;
	return;
    }
    a+=cpu[c].uregs[1] % 4;
    b+=cpu[c].uregs[3] % 4;
    // so we must add offset (0..3)
    cpu[c].uregs[0]=memcmp(a,b,cpu[c].uregs[4]);
}

// u0 - address of first string, u1 - offset of first string
// u2 - address of second string, u3 - offset of second string
// u4 - size in bytes of first string
// return: like strcmp, strncmp or memcmp ...
void mem_strcasecmp(int c)
{
    char *a, *b;
    const unsigned char *p1;
    const unsigned char *p2;
    unsigned char c1, c2;
 
    
    if (cpu[c].uregs[1]<0 || cpu[c].uregs[3]<0){
	non_fatal(ERROR_MEM_OFFSET,"strcasecmp: Illegal offset",c);
	failure=1;
	return;
    }
    a=verify_access(c,cpu[c].uregs[0]+(cpu[c].uregs[1]/4),
      (cpu[c].uregs[4]+3)/4,MEM_FLAG_READ);
    if (!a){
	non_fatal(ERROR_PROTFAULT,"strcasecmp: First string unsuitable",c); 
	failure=1;
	return;
    }
    b=verify_access(c,cpu[c].uregs[2]+(cpu[c].uregs[3]/4),
      (cpu[c].uregs[4]+3)/4,MEM_FLAG_READ);
    if (!b){
	non_fatal(ERROR_PROTFAULT,"strcasecmp: Second string unsuitable",c);
	failure=1;
	return;
    }
    a+=cpu[c].uregs[1] % 4;
    b+=cpu[c].uregs[3] % 4;
    // so we must add offset (0..3)
    p1 = (const unsigned char *) a;
    p2 = (const unsigned char *) b;
  
    if (p1 == p2){
      cpu[c].uregs[0]=0;
      return;
    }
    do
      {
        c1 = tolower(*p1++);
        c2 = tolower(*p2++);
	if ((int)((char*)p1-(char*)a)==cpu[c].uregs[4])
          break;
      }
    while (c1 == c2);
    cpu[c].uregs[0]=c1 - c2;
}

// u0 - address, u1 - offset, u2 - size
// u3 - address, u4 - offset, u5 - size 
// return: u0 - addres , u1 - offset
// only when u2 is !=0 -> substring is found
void mem_strstr(int c)
{
    char *a, *b, *a_size, *a_old;
    
    if (cpu[c].uregs[1]<0 || cpu[c].uregs[4]<0 || cpu[c].uregs[5]<0
    || cpu[c].uregs[2]<0){
	non_fatal(ERROR_MEM_OFFSET,"strstr: Illegal offset",c);
	failure=1;
	return;
    }
    a=verify_access(c,cpu[c].uregs[0]+(cpu[c].uregs[1]/4),
      (cpu[c].uregs[2]+3)/4,MEM_FLAG_READ);
    if (!a){
	non_fatal(ERROR_PROTFAULT,"strstr: First string unsuitable",c); 
	failure=1;
	return;
    }
    b=verify_access(c,cpu[c].uregs[3]+(cpu[c].uregs[4]/4),
      (cpu[c].uregs[5]+3)/4,MEM_FLAG_READ);
    if (!b){
	non_fatal(ERROR_PROTFAULT,"strstr: Second string unsuitable",c);
	failure=1;
	return;
    }
    if (cpu[c].uregs[2]==0 || cpu[c].uregs[2]<cpu[c].uregs[5]){
	cpu[c].uregs[2]=0;
	return;
    }
    if (cpu[c].uregs[5]==0)
	return;
    a+=cpu[c].uregs[1] % 4;
    b+=cpu[c].uregs[4] % 4;
    // so we must add offset (0..3)
    a_size=a+cpu[c].uregs[2];
    a_old=a;
    while(a<a_size && memcmp(a,b,cpu[c].uregs[5]))
	a++;
    if (a<a_size){
	cpu[c].uregs[0]+=(int)(a-a_old)/4+(cpu[c].uregs[1] / 4);
	cpu[c].uregs[1]=(int)(a-a_old) % 4;    
	cpu[c].uregs[2]=1;
    } else cpu[c].uregs[2]=0;
}
// u0 - address, u1 - offset, u2 - size
// u3 - address, u4 - offset, u5 - size 
// return: u0 - addres , u1 - offset
// only when u2 is != 0 -> substring is found
void mem_strrstr(int c)
{
    char *a, *b, *a_old;
    
    if (cpu[c].uregs[1]<0 || cpu[c].uregs[4]<0 || cpu[c].uregs[5]<0
    || cpu[c].uregs[2]<0){
	non_fatal(ERROR_MEM_OFFSET,"strrstr: Illegal offset",c);
	failure=1;
	return;
    }
    a=verify_access(c,cpu[c].uregs[0]+(cpu[c].uregs[1]/4),
      (cpu[c].uregs[2]+3)/4,MEM_FLAG_READ);
    if (!a){
	non_fatal(ERROR_PROTFAULT,"strrstr: First string unsuitable",c); 
	failure=1;
	return;
    }
    b=verify_access(c,cpu[c].uregs[3]+(cpu[c].uregs[4]/4),
      (cpu[c].uregs[5]+3)/4,MEM_FLAG_READ);
    if (!b){
	non_fatal(ERROR_PROTFAULT,"strrstr: Second string unsuitable",c);
	failure=1;
	return;
    }
    if (cpu[c].uregs[2]==0 || cpu[c].uregs[2]<cpu[c].uregs[5]){
	cpu[c].uregs[2]=0;
	return;
    }
    if (cpu[c].uregs[5]==0)
	return;
    a+=cpu[c].uregs[1] % 4;
    b+=cpu[c].uregs[4] % 4;
    // so we must add offset (0..3)
    a_old=a;
    a=a+cpu[c].uregs[2]-cpu[c].uregs[5];
    if (a_old>a){
	non_fatal(ERROR_PROTFAULT,"strrstr: Second string unsuitable",c);
	failure=1;
	return;
    }
    while(a>0 && memcmp(a,b,cpu[c].uregs[5]))
	a--;
    if (!memcmp(a,b,cpu[c].uregs[5])){
	cpu[c].uregs[0]+=(int)(a-a_old)/4+(cpu[c].uregs[1] / 4);
	cpu[c].uregs[1]=(int)(a-a_old) % 4;    
	cpu[c].uregs[2]=1;
    } else cpu[c].uregs[2]=0;
}

// u0 - address, u1 - offset, u2 - size in bytes 
void mem_toupper(int c)
{
    char *a;
    int i;
    
    a=verify_access(c,cpu[c].uregs[0]+(cpu[c].uregs[1]/4),(cpu[c].uregs[2]+3)/4,
		    MEM_FLAG_READ|MEM_FLAG_WRITE);
    if (!a){
	non_fatal(ERROR_PROTFAULT,"mem_tolower: Attempt to access protected memory",c);
	failure=1;
	return;
    }
    a+=cpu[c].uregs[1] % 4;
    for (i=0;i<cpu[c].uregs[2];i++){
	*a=(char)toupper((int)*a);
	a++;
    }
}
// u0 - address, u1 - offset, u2 - size in bytes 
void mem_tolower(int c)
{
    char *a;
    int i;
    
    a=verify_access(c,cpu[c].uregs[0]+(cpu[c].uregs[1]/4),(cpu[c].uregs[2]+3)/4,
		    MEM_FLAG_READ|MEM_FLAG_WRITE);
    if (!a){
	non_fatal(ERROR_PROTFAULT,"mem_tolower: Attempt to access protected memory",c);
	failure=1;
	return;
    }
    a+=cpu[c].uregs[1] % 4;
    for (i=0;i<cpu[c].uregs[2];i++){
	*a=(char)tolower((int)*a);
	a++;
    }
}

// u0 - address, u1 - offset, u2 - size in bytes
// return: u0 - integer or call exception
void mem_strtoint(int c)
{
    char *a,tmp[128];
    int r;
    
    if (cpu[c].uregs[1]<0){
	non_fatal(ERROR_MEM_OFFSET,"strtoint: Illegal offset",c);
	failure=1;
	return;
    }
    a=verify_access(c,cpu[c].uregs[0]+(cpu[c].uregs[1]/4),
      (cpu[c].uregs[2]+3)/4,MEM_FLAG_READ);
    if (!a){
	non_fatal(ERROR_PROTFAULT,"strtoint: String unsuitable",c); 
	failure=1;
	return;
    }
    a+=cpu[c].uregs[1]%4; 
    if (cpu[c].uregs[2]>=sizeof(tmp)-1){
	memcpy(&tmp,a,sizeof(tmp)-1);
	tmp[sizeof(tmp)-1]='\0';
    } else {
	memcpy(&tmp,a,cpu[c].uregs[2]);
	tmp[cpu[c].uregs[2]]='\0';
    }
    if(!sscanf(tmp,"%d",&r)){
	non_fatal(ERROR_MEM_FORMAT,"strtoint: It isn't int",c);
	failure=1;
	return;
    }
    cpu[c].uregs[0]=r;
}
// u0 - address, u1 - offset, u2 - size in bytes
// return: s0 - hex or call exception
void mem_strtohex(int c)
{
    char *a,tmp[128];
    unsigned int r;
    
    if (cpu[c].uregs[1]<0){
	non_fatal(ERROR_MEM_OFFSET,"strtohex: Illegal offset",c);
	failure=1;
	return;
    }
    a=verify_access(c,cpu[c].uregs[0]+(cpu[c].uregs[1]/4),
      (cpu[c].uregs[2]+3)/4,MEM_FLAG_READ);
    if (!a){
	non_fatal(ERROR_PROTFAULT,"strtohex: String unsuitable",c); 
	failure=1;
	return;
    }
    a+=cpu[c].uregs[1]%4;
    if (cpu[c].uregs[2]>=sizeof(tmp)-1){
	memcpy(&tmp,a,sizeof(tmp)-1);
	tmp[sizeof(tmp)-1]='\0';
    } else {
	memcpy(&tmp,a,cpu[c].uregs[2]);
	tmp[cpu[c].uregs[2]]='\0';
    }
    if(!sscanf(tmp,"%x",&r)){
	non_fatal(ERROR_MEM_FORMAT,"strtohex: It isn't hex",c);
	failure=1;
	return;
    }
    cpu[c].sregs[0]=r;
}
// u0 - address, u1 - offset, u2 - size in bytes
// return: f0 - float or call exception
void mem_strtofloat(int c)
{
    char *a,tmp[128];
    float r;
    
    if (cpu[c].uregs[1]<0){
	non_fatal(ERROR_MEM_OFFSET,"strtofloat: Illegal offset",c);
	failure=1;
	return;
    }
    a=verify_access(c,cpu[c].uregs[0]+(cpu[c].uregs[1]/4),
      (cpu[c].uregs[2]+3)/4,MEM_FLAG_READ);
    if (!a){
	non_fatal(ERROR_PROTFAULT,"strtofloat: String unsuitable",c); 
	failure=1;
	return;
    }
    a+=cpu[c].uregs[1]%4;
    if (cpu[c].uregs[2]>=sizeof(tmp)-1){
	memcpy(&tmp,a,sizeof(tmp)-1);
	tmp[sizeof(tmp)-1]='\0';
    } else {
	memcpy(&tmp,a,cpu[c].uregs[2]);
	tmp[cpu[c].uregs[2]]='\0';
    }
    if(!sscanf(tmp,"%f",&r)){
	non_fatal(ERROR_MEM_FORMAT,"strtofloat: It isn't float",c);
	failure=1;
	return;
    }
    cpu[c].fregs[0]=r;
}
// u0 - address, u1 - offset, u2 - size in bytes
// return: s0 - integer or call exception
void mem_strhexint(int c)
{
    char *a,tmp[128];
    unsigned int r;
    
    if (cpu[c].uregs[1]<0){
	non_fatal(ERROR_MEM_OFFSET,"strhexint: Illegal offset",c);
	failure=1;
	return;
    }
    a=verify_access(c,cpu[c].uregs[0]+(cpu[c].uregs[1]/4),
      (cpu[c].uregs[2]+3)/4,MEM_FLAG_READ);
    if (!a){
	non_fatal(ERROR_PROTFAULT,"strhexint: String unsuitable",c); 
	failure=1;
	return;
    }
    a+=cpu[c].uregs[1]%4;
    if (cpu[c].uregs[2]>=sizeof(tmp)-1){
	memcpy(&tmp,a,sizeof(tmp)-1);
	tmp[sizeof(tmp)-1]='\0';
    } else {
	memcpy(&tmp,a,cpu[c].uregs[2]);
	tmp[cpu[c].uregs[2]]='\0';
    }
    if(!sscanf(tmp,"%i",&r)){
	non_fatal(ERROR_MEM_FORMAT,"strhexint: It isn't hex or int",c);
	failure=1;
	return;
    }
    cpu[c].sregs[0]=r;

}
// u0 - address, u1 - offset, u2 - size of buf, s0 - value to convert
// return: converted hex value in buf, s0 - number of written bytes
void mem_hextostr(int c)
{
    char *a,tmp[128];
    unsigned int r;
    
    if (cpu[c].uregs[1]<0){
	non_fatal(ERROR_MEM_OFFSET,"hextostr: Illegal offset",c);
	failure=1;
	return;
    }
    a=verify_access(c,cpu[c].uregs[0]+(cpu[c].uregs[1]/4),
      (cpu[c].uregs[2]+3)/4,MEM_FLAG_WRITE);
    if (!a){
	non_fatal(ERROR_PROTFAULT,"hextostr: Attempt to access protected memory",c); 
	failure=1;
	return;
    }
    a+=cpu[c].uregs[1]%4;
    r=snprintf(tmp,sizeof(tmp),"%x",cpu[c].sregs[0]);
    if (r<=cpu[c].uregs[2]){
      memcpy(a,tmp,r);    
      cpu[c].sregs[0]=r;
    }					
    else {
	memcpy(a,tmp,cpu[c].uregs[2]);
	cpu[c].sregs[0]=cpu[c].uregs[2];
    }
}
// u0 - address, u1 - offset, u2 - size of buf, u3 - value to convert
// return: converted int value in buf, s0 - number of written bytes
void mem_inttostr(int c)
{
    char *a,tmp[128];
    unsigned int r;
    
    if (cpu[c].uregs[1]<0){
	non_fatal(ERROR_MEM_OFFSET,"inttostr: Illegal offset",c);
	failure=1;
	return;
    }
    a=verify_access(c,cpu[c].uregs[0]+(cpu[c].uregs[1]/4),
      (cpu[c].uregs[2]+3)/4,MEM_FLAG_WRITE);
    if (!a){
	non_fatal(ERROR_PROTFAULT,"inttostr: Attempt to access protected memory",c); 
	failure=1;
	return;
    }
    a+=cpu[c].uregs[1]%4;
    r=snprintf(tmp,sizeof(tmp),"%d",cpu[c].uregs[3]);
    if (r<=cpu[c].uregs[2]){
      memcpy(a,tmp,r);    
      cpu[c].sregs[0]=r;
    }					
    else {
	memcpy(a,tmp,cpu[c].uregs[2]);
	cpu[c].sregs[0]=cpu[c].uregs[2];
    }
}
// u0 - address, u1 - offset, u2 - size of buf, f0 - value to convert
// return: converted float value in buf, s0 - number of written bytes
void mem_floattostr(int c)
{
    char *a,tmp[128];
    unsigned int r;
    
    if (cpu[c].uregs[1]<0){
	non_fatal(ERROR_MEM_OFFSET,"floattostr: Illegal offset",c);
	failure=1;
	return;
    }
    a=verify_access(c,cpu[c].uregs[0]+(cpu[c].uregs[1]/4),
      (cpu[c].uregs[2]+3)/4,MEM_FLAG_WRITE);
    if (!a){
	non_fatal(ERROR_PROTFAULT,"floattostr: Attempt to access protected memory",c); 
	failure=1;
	return;
    }
    a+=cpu[c].uregs[1]%4;
    r=snprintf(tmp,sizeof(tmp),"%f",cpu[c].fregs[0]);
    if (r<=cpu[c].uregs[2]){
      memcpy(a,tmp,r);    
      cpu[c].sregs[0]=r;
    }					
    else {
	memcpy(a,tmp,cpu[c].uregs[2]);
	cpu[c].sregs[0]=cpu[c].uregs[2];
    }
}

void syscall_load(int* x)
{
    *(x++)=SYSCALL_MEM_STRCPY;
    *(x++)=SYSCALL_MEM_GETCHAR;
    *(x++)=SYSCALL_MEM_BZERO;
    *(x++)=SYSCALL_MEM_MEMSET;
    *(x++)=SYSCALL_MEM_ENDIAN;
    *(x++)=SYSCALL_MEM_STRCHR;
    *(x++)=SYSCALL_MEM_STRCMP;
    *(x++)=SYSCALL_MEM_STRCASECMP;
    *(x++)=SYSCALL_MEM_STRSTR;
    *(x++)=SYSCALL_MEM_TOUPPER;
    *(x++)=SYSCALL_MEM_TOLOWER;
    *(x++)=SYSCALL_MEM_STRTOINT;
    *(x++)=SYSCALL_MEM_STRTOHEX;
    *(x++)=SYSCALL_MEM_STRTOFLOAT;
    *(x++)=SYSCALL_MEM_STRHEXINT;
    *(x++)=SYSCALL_MEM_HEXTOSTR;
    *(x++)=SYSCALL_MEM_INTTOSTR;
    *(x++)=SYSCALL_MEM_FLOATTOSTR;
    *(x++)=SYSCALL_MEM_STRRCHR;
    *(x++)=SYSCALL_MEM_STRRSTR;
    *(x)=SYSCALL_ENDLIST;
    printk(">> Advmem module loaded.\n");
} 
void syscall_handler(int c, int nr)
{
    switch(nr) {
        case SYSCALL_MEM_STRCPY: mem_strcpy(c);break;
        case SYSCALL_MEM_GETCHAR: mem_getchar(c);break;
	case SYSCALL_MEM_BZERO: mem_bzero(c);break;
	case SYSCALL_MEM_MEMSET: mem_memset(c);break;
	case SYSCALL_MEM_ENDIAN: mem_endian(c);break;
	case SYSCALL_MEM_STRCHR: mem_strchr(c);break;
	case SYSCALL_MEM_STRCMP: mem_strcmp(c);break;
	case SYSCALL_MEM_STRCASECMP: mem_strcasecmp(c);break;
	case SYSCALL_MEM_STRSTR: mem_strstr(c);break;
	case SYSCALL_MEM_TOUPPER: mem_toupper(c);break;
	case SYSCALL_MEM_TOLOWER: mem_tolower(c);break;
	case SYSCALL_MEM_STRTOINT: mem_strtoint(c);break;
	case SYSCALL_MEM_STRTOHEX: mem_strtohex(c);break;
	case SYSCALL_MEM_STRTOFLOAT: mem_strtofloat(c);break;
	case SYSCALL_MEM_STRHEXINT: mem_strhexint(c);break;
	case SYSCALL_MEM_HEXTOSTR: mem_hextostr(c);break;
	case SYSCALL_MEM_INTTOSTR: mem_inttostr(c);break;
	case SYSCALL_MEM_FLOATTOSTR: mem_floattostr(c);break;
	case SYSCALL_MEM_STRRCHR: mem_strrchr(c);break;
	case SYSCALL_MEM_STRRSTR: mem_strrstr(c);break;
    }
}
