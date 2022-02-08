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

typedef enum {RUNNING, READY, BLOCKED, EXITED} state_t;

typedef struct tcb tcb_t;

struct tcb
{
	uthread_t mytid;
	state_t mystate;
	uthread_ctx_t myctx;
	int myexit;
	int myjoiner;	//initially set to -1 for invalid tid as joiner
};

tcb_t* curr_thread;

queue_t q_scheduler;

queue_t q_blocked;

queue_t q_exited;

uthread_t global_tid_size; 

int print_item_info(queue_t q, void *data, void *arg)	//for debugging
{
	(void)q;
	(void)arg;
	struct tcb* thread = data;

	printf("tid %d\n", thread->mytid);
	printf("state %d\n", thread->mystate);
	printf("exit status %d\n", thread->myexit);
	printf("joiner %d\n", thread->myjoiner);

	return 0;
}

int find_next_ready_thread(queue_t q, void *data, void *arg)
{
	(void)q;
	(void)arg;
	struct tcb* thread = data;

	if(thread->mystate == READY)
	{
		return 1;
		
	}
	else
	{
		struct tcb* requeue_thread;
		queue_dequeue(q_scheduler, (void**)&requeue_thread);
		queue_enqueue(q_scheduler, requeue_thread);
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

	q_scheduler = queue_create();
	q_blocked = queue_create();
	q_exited = queue_create();
	tcb_t* tcb_main = (tcb_t*)malloc(sizeof(struct tcb));
	curr_thread = tcb_main; 

	//if(tcb_main == NULL || q_scheduler == NULL) return -1;

	queue_enqueue(q_scheduler, tcb_main);
	global_tid_size=0;
	tcb_main->mytid = global_tid_size++;
	tcb_main->myjoiner = -1;
	tcb_main->mystate = RUNNING;
	return 0;

	return -1;
}

int uthread_stop(void)
{
	if(queue_length(q_scheduler) == 1 && queue_dequeue(q_scheduler, NULL) == 0 && queue_destroy(q_scheduler) == 0)	//only main thread is scheduled
	{
		return 0; //free the q_scheduler after dequeueing main thread
	}

	return -1;
}

int uthread_create(uthread_func_t func)
{
	tcb_t* tcb_thread = (tcb_t*)malloc(sizeof(struct tcb));

	//if(tcb_thread == NULL || GLOBAL_TID_COUNT == USHRT_MAX) //return -1;

	queue_enqueue(q_scheduler, tcb_thread);
	tcb_thread->mytid = global_tid_size++;
	tcb_thread->myjoiner = -1;
	tcb_thread->mystate = READY;
	uthread_ctx_init(&tcb_thread->myctx, uthread_ctx_alloc_stack(), func);

	return tcb_thread->mytid;
}

void uthread_yield(void)
{
	struct tcb* prev_thread = curr_thread;

	if(prev_thread->mystate == EXITED)	//move curr thread to exited queue
	{
		queue_dequeue(q_scheduler, (void**)&prev_thread);
		queue_enqueue(q_exited, prev_thread);
		queue_iterate(q_scheduler, find_next_ready_thread, NULL, (void**)&curr_thread); //update curr_thread to next thread to run
	}
	else if(prev_thread->mystate == BLOCKED)	//move cur thread to blocked queue
	{
		queue_dequeue(q_scheduler, (void**)&prev_thread);
		queue_enqueue(q_blocked, prev_thread);
		queue_iterate(q_scheduler, find_next_ready_thread, NULL, (void**)&curr_thread); //update curr_thread to next thread to run
	}
	else	//reschedule current thread to end of q_scheduler
	{
		queue_dequeue(q_scheduler, (void**)&prev_thread);
		queue_enqueue(q_scheduler, prev_thread);
		prev_thread->mystate = READY;
	}
	queue_iterate(q_scheduler, find_next_ready_thread, NULL, (void**)&curr_thread); //update curr_thread to next thread to run
	curr_thread->mystate=RUNNING;
	printf("running tid %d\n", curr_thread->mytid);
	uthread_ctx_switch(&prev_thread->myctx, &curr_thread->myctx);
}

uthread_t uthread_self(void)
{
	return curr_thread->mytid;
}

void uthread_exit(int retval)
{
	if(retval == 0)
	{

	}
	curr_thread->mystate = EXITED;
	curr_thread->myexit = retval;
	if(curr_thread->myjoiner != -1)	//schedule parent back to end of q_scheduler
	{
		struct tcb* ptr_parent;
		queue_iterate(q_blocked, find_thread, (void*)(long)curr_thread->myjoiner, (void**)&ptr_parent);
		queue_delete(q_blocked, ptr_parent);
		queue_enqueue(q_scheduler, ptr_parent);
		ptr_parent->mystate = READY;
	}
	printf("tid %d has exited\n", curr_thread->mytid);
	uthread_yield();
}

int uthread_join(uthread_t tid, int *retval)
{
	struct tcb* ptr_wait_thread;
	queue_iterate(q_scheduler, find_thread, (void*)(long)tid, (void**)&ptr_wait_thread); //not sure if int casting is needed
	ptr_wait_thread->myjoiner = curr_thread->mytid;
	curr_thread->mystate = BLOCKED;
	printf("waiting for tid %d\n", ptr_wait_thread->mytid);
	
	uthread_yield(); 
	
	printf("done waiting for %d\n", ptr_wait_thread->mytid);

	//child has exited
	retval = &ptr_wait_thread->myexit;
	printf("%d\n", ptr_wait_thread->myexit);
	printf("%d\n", *retval);
	printf("scheduler info\n");
	queue_iterate(q_scheduler, print_item_info, NULL, NULL);
	printf("blocked info\n");
	queue_iterate(q_blocked, print_item_info, NULL, NULL);
	printf("exited info\n");
	queue_iterate(q_exited, print_item_info, NULL, NULL);
	queue_delete(q_exited, ptr_wait_thread);
	free(ptr_wait_thread);

	return 0;

	if(tid == 0 || retval == 0)
	{

	}
	return -1;
}