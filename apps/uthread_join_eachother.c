#include <stdio.h>
#include <stdlib.h>

#include <uthread.h>

int thread1(void);
int thread2(void);

int main(void)
{
	uthread_start(0);
	uthread_create(thread1);
	uthread_stop();
	printf("main exiting\n");

	return 0;
}

int thread1(void)
{
    uthread_join(uthread_create(thread2), NULL);
    return 1;
}

int thread2(void)
{
    uthread_join(uthread_create(thread1), NULL);
    return 2;
}