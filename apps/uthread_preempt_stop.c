#include <stdio.h>
#include <stdlib.h>
#include <uthread.h>
#include <unistd.h>

int i = 5;

int func(void)
{
	while(1)
	{
		printf("Thread %d say hello\n", uthread_self());
	}
}

int func2(void)
{
	i += 45;
	return 0;
}

int func3(void)
{
	i += 50;
	uthread_join(uthread_create(func2), NULL);
	return 0;
}

int main(void)
{
	uthread_start(1); //start preemption
	uthread_join(uthread_create(func3), NULL);
	printf("i=%d\n", i);
	printf("preemption stopped sucessfully if 0 = %d\n", uthread_stop()); //test if stop preemption was successful
	sleep(4); //check if timer was properly shut off

	uthread_start(0);	//dont start preemption this time
	uthread_create(func);
	uthread_join(uthread_create(func), NULL);	//only one thread should be printing hello because preemption is off

	return 0;
}