#include <assert.h>
#include <limits.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "private.h"
#include "uthread.h"

/* TODO */
#define UTHREAD_STACK_SIZE 32768
#include "queue.h"
ucontext_t ctx[USHRT_MAX];
uthread_t TID = 0;
queue_t scheduler;


int uthread_start(int preempt)
{
	if(preempt == 0)
	{

	}

	scheduler = queue_create();
	queue_enqueue(scheduler, &TID);
	TID++;

	return -1;
}

int uthread_stop(void)
{
	/* TODO */
	return -1;
}

int uthread_create(uthread_func_t func)
{
	char threadstack[UTHREAD_STACK_SIZE]; //IDK maybe malloc?
	uthread_ctx_init(&ctx[TID], &threadstack, func); //IDK about & sign for stack
	
	return TID;
	return -1;
}

void uthread_yield(void)
{
	/* TODO */
}

uthread_t uthread_self(void)
{
	/* TODO */
	return -1;
}

void uthread_exit(int retval)
{
	if(retval == 0)
	{

	}
}

int uthread_join(uthread_t tid, int *retval)
{
	if(tid ==0 && retval == 0)
	{

	}
	return -1;
}

