/*
 * Tests joining the blocked thread
 * 
 * If T1 joins T2, T2 creates T3 and yields, it's okay if T3 joins T1
 */

#include <stdio.h>
#include <stdlib.h>

#include <uthread.h>

int f3(void)
{
	int ret;
	printf("T3 joins T1; T3 yields to T2\n");
	uthread_join(1, &ret);
	printf("T1 ret = %d\n", ret);
	printf("back to T3\n");
    printf("T3 yields to main\n");
	return 3;
}

int f2(void)
{
	printf("T2 yields to T3\n");
	uthread_yield();
	printf("back to T2\n");
	return 2;
}

int f1(void)
{
	int ret;
	printf("T1 joins T2; T1 yield to T2\n");
	uthread_join(2, &ret);
    printf("back to T1\n");
	printf("T2 ret = %d\n", ret);

	return 1;
}

int main(void)
{
	uthread_start(0);
	uthread_create(f1);
	uthread_create(f2);
    printf("Main yield to t1\n");
	uthread_join(uthread_create(f3), NULL);
	printf("back at main\n");
	uthread_stop();
	printf("main exiting\n");
	return 0;
}