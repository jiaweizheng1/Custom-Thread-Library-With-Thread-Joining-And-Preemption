#include <assert.h>
#include <limits.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include <string.h>

#include "private.h"
#include "uthread.h"
#include "queue.h"

typedef enum {RUNNING, READY, BLOCKED, EXITED} state_t;

static ucontext_t ctx[USHRT_MAX];

static void* top_of_stack[USHRT_MAX];

typedef struct tcb tcb_t;

struct tcb
{
	uthread_t mytid;
	state_t mystate;
	int myexit;
	int myjoiner;	//initially set to -1 for invalid tid as joiner
};

static tcb_t* curr_thread;

static queue_t q_scheduler;

static queue_t q_blocked;

static queue_t q_exited;

static uthread_t global_tid_size; 

static int find_next_ready_thread(queue_t q, void *data, void *arg)
{
	(void)q;
	(void)arg;
	struct tcb* thread = data;

	if(thread->mystate == READY)
	{
		return 1;
		
	}
	struct tcb* requeue_thread;
	queue_dequeue(q, (void**)&requeue_thread);
	queue_enqueue(q, requeue_thread);
	return 0;
}

static int find_thread(queue_t q, void *data, void *arg)
{
	(void)q;
	uthread_t tid = (uthread_t)(int)(long) arg;	//triple cast
	struct tcb* thread = data;

	if(thread->mytid == tid)
	{
		return 1;
	}
	return 0;
}

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

int uthread_start(int preempt)
{
	q_scheduler = queue_create();
	q_blocked = queue_create();
	q_exited = queue_create();
	tcb_t* tcb_main = (tcb_t*)malloc(sizeof(struct tcb));

	if(q_scheduler == NULL || q_blocked == NULL || q_exited == NULL || tcb_main == NULL || queue_enqueue(q_scheduler, tcb_main) != 0) return -1;

	curr_thread = tcb_main; 
	global_tid_size=0;
	tcb_main->mytid = global_tid_size++;
	top_of_stack[tcb_main->mytid] = uthread_ctx_alloc_stack();
	uthread_ctx_init(&ctx[tcb_main->mytid], top_of_stack[tcb_main->mytid], NULL);
	tcb_main->myjoiner = -1;
	tcb_main->mystate = RUNNING;

	if(preempt)
	{
		preempt_start();
	}

	return 0;
}

int uthread_stop(void)
{
	if(curr_thread->mytid == (uthread_t)0 && queue_length(q_scheduler) == 1 && queue_length(q_blocked) == 0 && queue_length(q_exited) == 0 && queue_dequeue(q_scheduler, (void**)&curr_thread) == 0)	//only main thread is left
	{
		preempt_stop();
		uthread_ctx_destroy_stack(top_of_stack[curr_thread->mytid]);
		queue_dequeue(q_scheduler, NULL);
		free(curr_thread);
		queue_destroy(q_scheduler);
		queue_destroy(q_blocked);
		queue_destroy(q_exited);
		return 0;
	}

	return -1;
}

int uthread_create(uthread_func_t func)
{
	tcb_t* tcb_thread = (tcb_t*)malloc(sizeof(struct tcb));

	if(tcb_thread != NULL && global_tid_size != USHRT_MAX && queue_enqueue(q_scheduler, tcb_thread) == 0)
	{
		tcb_thread->mytid = global_tid_size++;
		top_of_stack[tcb_thread->mytid] = uthread_ctx_alloc_stack();
		uthread_ctx_init(&ctx[tcb_thread->mytid], top_of_stack[tcb_thread->mytid], func);
		tcb_thread->myjoiner = -1;
		tcb_thread->mystate = READY;
		return tcb_thread->mytid;
	}

	return -1;
}

void uthread_yield(void)
{
	struct tcb* prev_thread = curr_thread;

	if(prev_thread->mystate == EXITED)	//move curr thread to exited queue
	{
		queue_dequeue(q_scheduler, (void**)&prev_thread);
		queue_enqueue(q_exited, prev_thread);
	}
	else if(prev_thread->mystate == BLOCKED)	//move cur thread to blocked queue
	{
		queue_dequeue(q_scheduler, (void**)&prev_thread);
		queue_enqueue(q_blocked, prev_thread);
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
	uthread_ctx_switch(&ctx[prev_thread->mytid], &ctx[curr_thread->mytid]);
}

uthread_t uthread_self(void)
{
	return curr_thread->mytid;
}

void uthread_exit(int retval)
{
	curr_thread->mystate = EXITED;
	curr_thread->myexit = retval;
	if(curr_thread->myjoiner != -1)	//schedule parent back to end of q_scheduler
	{
		struct tcb* ptr_parent;
		queue_iterate(q_blocked, find_thread, (void*)(long)(int)curr_thread->myjoiner, (void**)&ptr_parent);
		queue_delete(q_blocked, ptr_parent);
		queue_enqueue(q_scheduler, ptr_parent);
		ptr_parent->mystate = READY;
	}
	printf("tid %d has exited\n", curr_thread->mytid);
	uthread_yield();
}

int uthread_join(uthread_t tid, int *retval)
{
	if(tid == 0 || curr_thread->mytid == tid) return -1;
	struct tcb* ptr_wait_thread = NULL;
	queue_iterate(q_scheduler, find_thread, (void*)(long)(int)tid, (void**)&ptr_wait_thread); //not sure if int casting is needed

	if(ptr_wait_thread != NULL)	//child is in q_scheduler
	{
		if(ptr_wait_thread->myjoiner != -1) return -1;	//child is already being joined
		ptr_wait_thread->myjoiner = curr_thread->mytid;
		curr_thread->mystate = BLOCKED;
		uthread_yield(); 

	}
	else	//child has already exited
	{
		queue_iterate(q_exited, find_thread, (void*)(long)(int)tid, (void**)&ptr_wait_thread); //not sure if int casting is needed
		if(ptr_wait_thread == NULL) return -1; //cant find tid
		if(ptr_wait_thread->myjoiner != -1) return -1;	//child is already being joined
	}
	
	if(retval != NULL)
	{
		memcpy(retval, &ptr_wait_thread->myexit, sizeof(ptr_wait_thread->myexit));	//create a hard copy of the return value
	}
	uthread_ctx_destroy_stack(top_of_stack[ptr_wait_thread->mytid]);
	queue_delete(q_exited, ptr_wait_thread);
	free(ptr_wait_thread);
	return 0;
}