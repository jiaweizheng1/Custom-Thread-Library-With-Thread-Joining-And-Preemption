#include <stdio.h>
#include <stdlib.h>
#include <uthread.h>
#include <unistd.h>

int func(void)
{
	while(1)
	{
		printf("Thread %d say hello\n", uthread_self());
	}
}

int main(void)
{
	printf("main starts\n");
	uthread_start(1); //start preemption
	uthread_create(func);
	uthread_join(uthread_create(func), NULL);
	printf("main ends\n");
	uthread_stop();
	return 0;
}