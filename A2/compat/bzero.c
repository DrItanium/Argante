#include "autocfg.h"
#include "compat/bzero.h"

/* I'm not sure why it needs to be like this, but... */
void bzero(void *block, size_t size) {
	while(size) switch(size) {
		if (size >= sizeof(int)) {
			*((int *) block)=0;
			size-=sizeof(int);
			block+=sizeof(int);
			break;
		} else if (size >= sizeof(short)) {
			*((short *) block)=0;
			size-=sizeof(short);
			block+=sizeof(short);
			break;
		} else {
			*((char *) block)=0;
			size--;
			block++;
			break;
		}
	}
}

