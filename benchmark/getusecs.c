#include <stdio.h>
#include <sys/time.h>
int main(void)
{
	struct timeval time_now;
	gettimeofday(&time_now,NULL);
	printf ("%lu\n",time_now.tv_sec*(uint64_t)1000000+time_now.tv_usec);
	return 0;
}
