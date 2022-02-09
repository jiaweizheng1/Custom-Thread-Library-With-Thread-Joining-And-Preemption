#include <stdio.h>
#include <stdlib.h>

#include <uthread.h>

int thread1(void)
{
	printf("%d\n", uthread_self());
	uthread_join(uthread_create(thread1), NULL);
	return 0;
}

int main(void)
{
	uthread_start(0);
	uthread_join(uthread_create(thread1), NULL);
	uthread_stop();

	return 0;
}