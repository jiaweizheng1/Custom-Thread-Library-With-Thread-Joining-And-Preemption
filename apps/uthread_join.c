#include <stdio.h>
#include <stdlib.h>

#include <uthread.h>

int thread3(void)
{
	uthread_yield();
	printf("thread%d\n", uthread_self());
	return 7777;
}

int thread2(void)
{
	int retval;
	uthread_join(uthread_create(thread3), &retval);
	printf("%d\n", retval);
	uthread_yield();
	printf("thread%d\n", uthread_self());
	return 7788888;
}

int thread1(void)
{
	int retval;
	uthread_join(uthread_create(thread1), &retval);
	printf("%d\n", retval);
	uthread_yield();
	printf("thread%d\n", uthread_self());
	uthread_yield();
	return 653345345;
}

int main(void)
{
	int retval;
	uthread_start(0);
	uthread_join(uthread_create(thread1), &retval);
	printf("%d\n", retval);
	uthread_stop();

	printf("main exiting\n");

	return 0;
}