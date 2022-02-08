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
#include "queue.h"

queue_t scheduler;
queue_t blocked;

uthread_t global_tid_size; 

typedef enum {RUNNING, READY, BLOCKED, EXITED} state_t;

typedef struct tcb tcb_t;

struct tcb
{
	uthread_t mytid;
	state_t mystate;
	uthread_ctx_t myctx;
	int myexit;
};

int print_item_info(queue_t q, void *data, void *arg)	//for debugging
{
	(void)q;
	(void)arg;
	struct tcb* thread = data;

	printf("tid %d\n", thread->mytid);
	printf("state %d\n", thread->mystate);
	printf("exit status %d\n", thread->myexit);

	return 0;
}

int find_next_available_ready_thread(queue_t q, void *data, void *arg)
{
	(void)q;
	(void)arg;
	struct tcb* thread = data;

	if(thread->mystate != RUNNING && thread->mystate != EXITED)
	{
		return 1;
		
	}
	else
	{
		struct tcb* requeue_thread;
		queue_dequeue(scheduler, (void**)&requeue_thread);
		queue_enqueue(scheduler, requeue_thread);
		return 0;
	}
}

int return_head(queue_t q, void *data, void *arg)
{
	(void)q;
	(void)data;
	(void)arg;
	
	return 1;
}

int requeue_head(queue_t q, void *data, void *arg)	//not used idk
{
	(void)q;
	(void)data;
	(void)arg;
	
	struct tcb* requeue_thread;
	queue_dequeue(scheduler, (void**)&requeue_thread);
	queue_enqueue(scheduler, requeue_thread);

	return 1;
}

int find_thread(queue_t q, void *data, void *arg)
{
	(void)q;
	uthread_t tid = (uthread_t)(long) arg;	//triple cast
	struct tcb* thread = data;

	if(thread->mytid == tid)
	{
		return 1;
	}

	return 0;
}

int uthread_start(int preempt)
{
	if(preempt == 0)
	{
	}

	scheduler = queue_create();
	blocked = queue_create();
	tcb_t* tcb_main = (tcb_t*)malloc(sizeof(struct tcb));

	//if(tcb_main == NULL || scheduler == NULL) return -1;

	queue_enqueue(scheduler, tcb_main);
	global_tid_size=0;
	tcb_main->mytid = global_tid_size++;
	tcb_main->mystate = RUNNING;
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

	queue_enqueue(scheduler, tcb_thread);
	tcb_thread->mytid = global_tid_size++;
	tcb_thread->mystate = READY;
	uthread_ctx_init(&tcb_thread->myctx, uthread_ctx_alloc_stack(), func);

	return tcb_thread->mytid;
}

void uthread_yield(void)
{
	struct tcb* ptr_curr;
	struct tcb* ptr_next = NULL;
	queue_iterate(scheduler, return_head, NULL, (void**)&ptr_curr);
	queue_iterate(scheduler, find_next_available_ready_thread, NULL, (void**)&ptr_next);
	if(ptr_curr->mystate != EXITED)
	{
		ptr_curr->mystate=READY;
	}
	ptr_next->mystate=RUNNING;
	printf("running tid %d\n", ptr_next->mytid);
	uthread_ctx_switch(&ptr_curr->myctx, &ptr_next->myctx);
}

uthread_t uthread_self(void)
{
	struct tcb* ptr_curr;
	queue_iterate(scheduler, return_head, NULL, (void**)&ptr_curr);
	return ptr_curr->mytid;
}

void uthread_exit(int retval)
{
	if(retval == 0)
	{

	}
	struct tcb* ptr_curr;
	queue_iterate(scheduler, return_head, NULL, (void**)&ptr_curr);
	ptr_curr->mystate = EXITED;
	ptr_curr->myexit = retval;
	printf("tid %d has exited\n", ptr_curr->mytid);
	uthread_yield();
}

int uthread_join(uthread_t tid, int *retval)
{
	struct tcb* ptr_curr;
	struct tcb* ptr_wait_thread;
	queue_iterate(scheduler, return_head, NULL, (void**)&ptr_curr);
	queue_iterate(scheduler, find_thread, (void*)(long)tid, (void**)&ptr_wait_thread); //not sure if int casting is needed
	printf("waiting for tid %d\n", ptr_wait_thread->mytid);
	while(ptr_wait_thread->mystate != EXITED)
	{
		uthread_yield(); //HERE; somewhere around here is the problem and need to implement priority of ready over blocked 
		//OR move blocked queue to back of queue after it becomes unblocked
	}

	queue_iterate(scheduler, print_item_info, NULL, NULL);
	
	printf("done waiting for %d\n", ptr_wait_thread->mytid);
	
	retval = &ptr_wait_thread->myexit;

	return 0;

	if(tid == 0 || retval == 0)
	{

	}
	return -1;
}