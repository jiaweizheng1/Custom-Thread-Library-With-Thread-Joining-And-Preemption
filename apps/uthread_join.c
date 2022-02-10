#include <stdio.h>
#include <stdlib.h>

#include <uthread.h>

int thread3(void)
{
	printf("thread%d finished\n", uthread_self());
	return 7777;
}

int thread2(void)
{
	int retval;
	uthread_join(uthread_create(thread3), &retval);
	printf("thread 3 return %d\n", retval);
	printf("thread%d finished\n", uthread_self());
	return 7788888;
}

int thread1(void)
{
	int retval;
	uthread_join(uthread_create(thread2), &retval);
	printf("thread 2 return %d\n", retval);
	printf("thread%d finished\n", uthread_self());
	return 653345345;
}

int main(void)
{
	int retval;
	uthread_start(1);
	uthread_join(uthread_create(thread1), &retval);
	printf("thread 1 return %d\n", retval);
	uthread_stop();
	printf("main exiting\n");

	return 0;
}