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
#include "queue.h"
typedef struct tcb* tcb_t;

struct tcb
{
	uthread_t TID;
	int state;	//IDK 0=running, 1=ready, 2=blocked
	uthread_ctx_t* ctx;	//IDK is this stack + registers?
};

queue_t scheduler;

uthread_t GLOBAL_TID_COUNT = 0;

int uthread_start(int preempt)
{
	if(preempt == 0)
	{
		scheduler = queue_create();
		tcb_t tcb_main = (tcb_t)malloc(sizeof(struct tcb));
		int retval = queue_enqueue(scheduler, &tcb_main);

		if(tcb_main == NULL || scheduler == NULL || retval == -1) return -1;

		tcb_main->TID = GLOBAL_TID_COUNT;
		tcb_main->state = 0;
		GLOBAL_TID_COUNT++;
		return 0;
	}
	else if(preempt == 1)
	{

	}

	return -1;
}

int uthread_stop(void)
{
	if(queue_length(scheduler) == 1 && queue_dequeue(scheduler, NULL) == 0 && queue_destroy(scheduler) == 0)	//only main thread is scheduled
	{
		return 0; //free the scheduler after dequeueing main thread
	}

	return -1;
}

int uthread_create(uthread_func_t func)
{
	tcb_t tcb_thread = (tcb_t)malloc(sizeof(struct tcb));
	int retval = queue_enqueue(scheduler, &tcb_thread);

	if(tcb_thread == NULL || retval == -1 || GLOBAL_TID_COUNT == USHRT_MAX) return -1; //IDK overflow when to negative number?

	uthread_ctx_init(tcb_thread->ctx, uthread_ctx_alloc_stack(), func);
	tcb_thread->TID = GLOBAL_TID_COUNT;
	tcb_thread->state = 1;
	GLOBAL_TID_COUNT++;
	return tcb_thread->TID;
}

void uthread_yield(void)
{
	/* TODO */
}

uthread_t uthread_self(void)
{
	//return scheduler->head->data->TID;
	return -1;
}

void uthread_exit(int retval)
{
	if(retval == 0)
	{

	}
	exit(0);	//IDK
}

int uthread_join(uthread_t tid, int *retval)
{
	struct tcb* tcb;
	struct tcb* tcb_next;
	queue_dequeue(scheduler, (void**)&tcb);
	queue_dequeue(scheduler, (void**)&tcb_next);

	uthread_ctx_switch(tcb->ctx, tcb_next->ctx);

	if(tid == 0 || retval == 0)
	{

	}
	return -1;
}