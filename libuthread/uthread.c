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

typedef enum {RUNNING, READY, BLOCKED, EXITED} state_t;	//states for threads

static ucontext_t ctx[USHRT_MAX];	//ctx for ushrt_max threads

static void* top_of_stack[USHRT_MAX];	//top of stack void ptr array for freeing contexes

typedef struct tcb tcb_t;

struct tcb
{
	uthread_t mytid;
	state_t mystate;
	int myexit;
	int myjoiner;	//initially set to -1 for invalid tid as joiner
};

static tcb_t* curr_thread;	//global varaible to current thread makes it easier to swap context with the next ready thread

static queue_t q_scheduler;	//a ready/running queue

static queue_t q_blocked;	//queue for threads who called join on other threads. helps make scheduling O(1) because don't have to iterate through blocked threads

static queue_t q_exited;	//queue for threads that has finished execution. helps make scheduling O(1) because don't have to iterate through exited threads

static uthread_t global_tid_size; //makes it easier to assign tids to new threads

static int find_next_ready_thread(queue_t q, void *data, void *arg)	//helper function to find ready threads in q_scheduler
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

static int find_thread(queue_t q, void *data, void *arg)	//helper function to find a particular thread in a queue by proving tid of the thread
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

int uthread_start(int preempt)	//create the three queues, set main as first thread of scheduler
{
	q_scheduler = queue_create();
	q_blocked = queue_create();
	q_exited = queue_create();
	tcb_t* tcb_main = (tcb_t*)malloc(sizeof(struct tcb));

	if(q_scheduler == NULL || q_blocked == NULL || q_exited == NULL || tcb_main == NULL) return -1;

	curr_thread = tcb_main; 
	global_tid_size = 0;
	tcb_main->mytid = global_tid_size++;
	tcb_main->myjoiner = -1;
	tcb_main->mystate = RUNNING;	//main is currently in running state
	queue_enqueue(q_scheduler, tcb_main);

	if(preempt)	//start preempt if needed
	{
		preempt_start();
	}

	return 0;
}

int uthread_stop(void)	//free three queues + main thread
{
	if(curr_thread->mytid == (uthread_t)0 && queue_length(q_scheduler) == 1 && queue_length(q_blocked) == 0 && queue_length(q_exited) == 0)	//only main thread is left
	{
		preempt_disable();	//stop preemption if was enabled
		preempt_stop();
		queue_dequeue(q_scheduler, (void**)&curr_thread);
		free(curr_thread);
		queue_destroy(q_scheduler);
		queue_destroy(q_blocked);
		queue_destroy(q_exited);
		return 0;
	}

	return -1;
}

int uthread_create(uthread_func_t func)	//alloc memory for tcb for another thread, push thread to end of scheduler
{
	preempt_enable();
	tcb_t* tcb_thread = (tcb_t*)malloc(sizeof(struct tcb));

	if(tcb_thread != NULL && global_tid_size != USHRT_MAX)
	{

		tcb_thread->mytid = global_tid_size++;

		preempt_disable();
		top_of_stack[tcb_thread->mytid] = uthread_ctx_alloc_stack();	//initialize ctx for the thread
		uthread_ctx_init(&ctx[tcb_thread->mytid], top_of_stack[tcb_thread->mytid], func);
		queue_enqueue(q_scheduler, tcb_thread);
		preempt_enable();
		
		tcb_thread->myjoiner = -1;
		tcb_thread->mystate = READY;
		return tcb_thread->mytid;
	}
	return -1;
}

void uthread_yield(void)
{
	struct tcb* prev_thread = curr_thread;	//tmp tcb ptr to keep track of thread that is to be moved to the end of scheduler queue(or the other queues)

	preempt_disable();
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
	preempt_enable();

	preempt_disable();
	queue_iterate(q_scheduler, find_next_ready_thread, NULL, (void**)&curr_thread); //update curr_thread to next thread to run
	curr_thread->mystate=RUNNING;
	preempt_enable();

	preempt_disable();
	if(prev_thread != curr_thread)	//dont need to swap context if choose the same thread
	{
		uthread_ctx_switch(&ctx[prev_thread->mytid], &ctx[curr_thread->mytid]);	
	}
	preempt_enable();
}

uthread_t uthread_self(void)
{
	return curr_thread->mytid;
}

void uthread_exit(int retval)	//set thread's state to exit then call yield to do the rest of the work(move this thread to exited queue, etc)
{
	preempt_enable();
	curr_thread->mystate = EXITED;
	curr_thread->myexit = retval;
	if(curr_thread->myjoiner != -1)	//schedule parent back to end of q_scheduler and make it ready to run
	{
		struct tcb* ptr_parent;
		queue_iterate(q_blocked, find_thread, (void*)(long)(int)curr_thread->myjoiner, (void**)&ptr_parent);
		queue_delete(q_blocked, ptr_parent);
		queue_enqueue(q_scheduler, ptr_parent);
		ptr_parent->mystate = READY;
	}
	uthread_yield();
}

int uthread_join(uthread_t tid, int *retval)
{
	preempt_enable();
	if(tid == 0 || curr_thread->mytid == tid) return -1;
	struct tcb* ptr_wait_thread = NULL;

	preempt_disable();
	queue_iterate(q_scheduler, find_thread, (void*)(long)(int)tid, (void**)&ptr_wait_thread); 
	preempt_enable();

	preempt_disable();	//to avoid two threads accessing child->myjoiner at the same time
	if(ptr_wait_thread != NULL)	//child is in q_scheduler
	{
		if(ptr_wait_thread->myjoiner != -1) return -1;	//child is already being joined
		ptr_wait_thread->myjoiner = curr_thread->mytid;	//set that child thread's joined parent to current thread
		curr_thread->mystate = BLOCKED;
		uthread_yield(); 
	}
	else	//child has already exited into q_exited
	{
		queue_iterate(q_exited, find_thread, (void*)(long)(int)tid, (void**)&ptr_wait_thread); 
		if(ptr_wait_thread == NULL) return -1; //cant find tid
		if(ptr_wait_thread->myjoiner != -1) return -1;	//child is already being joined
		ptr_wait_thread->myjoiner = curr_thread->mytid; //set that child thread's joined parent to current thread
	}
	preempt_enable();

	if(retval != NULL)	//set retval to the return value of the joined child
	{
		*retval = ptr_wait_thread->myexit;//pass a hard copy of the return value of child to retval
	}
	uthread_ctx_destroy_stack(top_of_stack[ptr_wait_thread->mytid]);	//free child's resources
	queue_delete(q_exited, ptr_wait_thread);
	free(ptr_wait_thread);
	return 0;
}