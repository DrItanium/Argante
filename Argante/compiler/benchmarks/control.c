#include <stdio.h>
#include <sys/time.h>

#define LOOPCT 10000000

void nofunc() { return; }

void calcTime(struct timeval *t1, struct timeval *t2, float n)
{
	float sec;
	sec=t1->tv_sec - t2->tv_sec + ((float) t1->tv_usec - t2->tv_usec) / 1000000;

	printf("%f\n", n / sec); 
}

int main()
{
	struct timeval t1, t2;
	int i;

	printf("Function calls / second:");
	gettimeofday(&t1, NULL);
	i=LOOPCT;
	while(i)
	{
		nofunc();
		i--;
	}
	gettimeofday(&t2, NULL);
	calcTime(&t2, &t1, LOOPCT);

	printf("Loops / second:");
	gettimeofday(&t1, NULL);
	i=LOOPCT * 20;
	while(i)
		i--;
	gettimeofday(&t2, NULL);
	calcTime(&t2, &t1, LOOPCT * 20);

	return 0;
}
    
