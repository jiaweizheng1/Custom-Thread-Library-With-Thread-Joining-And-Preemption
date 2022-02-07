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
#include <unistd.h>	//sleep
#include "queue.h"

typedef enum {RUNNING, READY, BLOCKED, EXITED} state_t;

ucontext_t ctx[USHRT_MAX];

typedef struct tcb tcb_t;

struct tcb
{
	uthread_t mytid;
	state_t mystate;
	uthread_ctx_t myctx;
};

int print_item_info(queue_t q, void *data, void *arg)
{
	(void)q;
	(void)arg;
	struct tcb* a = data;

	printf("tid %d\n", a->mytid);
	printf("state %d\n", a->mystate);

	return 0;
}

queue_t scheduler;

int uthread_start(int preempt)
{
	if(preempt == 0)
	{
	}

	scheduler = queue_create();
	tcb_t* tcb_main = (tcb_t*)malloc(sizeof(struct tcb));

	//if(tcb_main == NULL || scheduler == NULL) return -1;

	tcb_main->mytid = 5;
	tcb_main->mystate = RUNNING;
	queue_enqueue(scheduler, tcb_main);
	return 0;

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
	tcb_t* tcb_thread = (tcb_t*)malloc(sizeof(struct tcb));

	//if(tcb_thread == NULL || GLOBAL_TID_COUNT == USHRT_MAX) //return -1;

	tcb_thread->mytid = 3;
	tcb_thread->mystate = READY;
	uthread_ctx_init(&tcb_thread->myctx, uthread_ctx_alloc_stack(), func);
	queue_enqueue(scheduler, tcb_thread);

	return tcb_thread->mytid;
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
	queue_iterate(scheduler, print_item_info, NULL, NULL);

	if(tid == 0 || retval == 0)
	{

	}
	return -1;
}
