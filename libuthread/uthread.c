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

typedef enum {RUNNING, READY, BLOCKED, EXITED} state_t;	//states for threads; set these states to predefined constants

typedef struct tcb tcb_t;

struct tcb	//struct to store information about a thread
{
	uthread_t mytid;
	state_t mystate;
	int myexit;	//return value
	int myjoiner;	//initially set to -1 for invalid tid as joiner
	void* mystackptr;
	uthread_ctx_t myctx;
};

static tcb_t* curr_thread;	//global ptr to current thread makes it easier to swap context with the next ready thread

static queue_t q_scheduler;	//a ready/running threads queue

static queue_t q_blocked;	//queue for threads who called join on other threads. helps make scheduling O(1) because don't have to iterate through blocked threads

static queue_t q_exited;	//queue for threads that has finished execution. helps make scheduling O(1) because don't have to iterate through exited threads

static uthread_t global_tid_size; //makes it easier to assign tids to new threads

static int find_head_ready_thread(queue_t q, void *data, void *arg)	//helper function to retreive next ready thread in q_scheduler
{
	(void)q;
	(void)arg;	//unused
	(void)data;

	return 1;	//modify *data to head of queue
}

static int find_thread(queue_t q, void *data, void *arg)	//helper function to find a particular thread in a queue by proving tid of the thread
{
	(void)q;
	uthread_t tid = (uthread_t)(int)(long) arg;	//triple cast
	struct tcb* thread = data;

	if(thread->mytid == tid)	//set *data to thread if found thread with same input tid
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
	tcb_main->mystate = RUNNING;	//main is currently the running thread
	tcb_main->myjoiner = -1;
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
		preempt_disable(); //stop preemption if was enabled
		preempt_stop();	//reset handler for v alarm
		queue_destroy(q_blocked);
		queue_destroy(q_exited);
		queue_dequeue(q_scheduler, (void**)&curr_thread);
		queue_destroy(q_scheduler);
		free(curr_thread);	//free main's resources
		return 0;
	}

	return -1;
}

int uthread_create(uthread_func_t func)	//alloc memory for tcb of another thread, push thread to end of scheduler
{
	tcb_t* tcb_thread = (tcb_t*)malloc(sizeof(struct tcb));

	if(tcb_thread != NULL && global_tid_size != USHRT_MAX)
	{
		tcb_thread->mytid = global_tid_size++;

		preempt_disable();	//protect ctx creation for thread from being preempted
		tcb_thread->mystackptr = uthread_ctx_alloc_stack();	//initialize ctx for the thread
		uthread_ctx_init(&tcb_thread->myctx, tcb_thread->mystackptr, func);
		preempt_enable();

		queue_enqueue(q_scheduler, tcb_thread);	//enqueue thread to scheduler to be scheduled
		tcb_thread->myjoiner = -1;
		tcb_thread->mystate = READY;
		return tcb_thread->mytid;
	}

	return -1;
}

void uthread_yield(void)
{
	struct tcb* prev_thread = curr_thread;	//temp tcb ptr to keep track of thread that is to be moved to the end of scheduler queue(or the other queues)
	
	preempt_disable();	//these functions modify sensitive queue structures
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
	else	//else reschedule current thread to end of q_scheduler
	{
		queue_dequeue(q_scheduler, (void**)&prev_thread);
		queue_enqueue(q_scheduler, prev_thread);
		prev_thread->mystate = READY;
	}
	queue_iterate(q_scheduler, find_head_ready_thread, NULL, (void**)&curr_thread); //update curr_thread to next thread to run
	
	curr_thread->mystate=RUNNING;
	if(prev_thread != curr_thread)	//dont need to swap context if choose the same thread
	{
		uthread_ctx_switch(&prev_thread->myctx, &curr_thread->myctx);	//protect context swapping from being preempted
	}
	preempt_enable();
}

uthread_t uthread_self(void)
{
	return curr_thread->mytid;
}

void uthread_exit(int retval)	//set thread's state to exit then call yield to do the rest of the work(move this thread to exited queue, etc)
{
	preempt_disable();	//can't have parent be scheduled to end of scheduler but child hasn't exited to exited queue yet
	if(curr_thread->myjoiner != -1)	//schedule parent back to end of q_scheduler and make it ready to run
	{
		struct tcb* ptr_parent;	
		queue_iterate(q_blocked, find_thread, (void*)(long)(int)curr_thread->myjoiner, (void**)&ptr_parent);	//find parent in blocked queue
		queue_delete(q_blocked, ptr_parent);
		queue_enqueue(q_scheduler, ptr_parent);	//move blocked parent from blocked queue back to q_scheduler to be scheduled
		ptr_parent->mystate = READY;
	}
	curr_thread->mystate = EXITED;
	curr_thread->myexit = retval;
	uthread_yield();	//no need to renable preempt after child yields() because they will be in exited queue after this yield and wont ever be scheduled again
}

int uthread_join(uthread_t tid, int *retval)
{
	if(tid == 0 || curr_thread->mytid == tid) return -1;
	struct tcb* ptr_wait_thread = NULL;

	queue_iterate(q_scheduler, find_thread, (void*)(long)(int)tid, (void**)&ptr_wait_thread); //try to find child thread in scheduler queue
	if(ptr_wait_thread != NULL)	//child is in q_scheduler, then curr_thread must be placed into blocked queue
	{
		if(ptr_wait_thread->myjoiner != -1) return -1;	//child is already being joined
		ptr_wait_thread->myjoiner = curr_thread->mytid;	//set that child thread's joined parent to current thread
		curr_thread->mystate = BLOCKED;	
		uthread_yield(); //move current thread to blocked queue
	}
	else
	{
		queue_iterate(q_blocked, find_thread, (void*)(long)(int)tid, (void**)&ptr_wait_thread); //try to find child thread in blocked queue
		if(ptr_wait_thread != NULL)	//child is in blocked queue, then curr_thread must also be placed into blocked queue
		{
			if(ptr_wait_thread->myjoiner != -1) return -1;	//child is already being joined
			ptr_wait_thread->myjoiner = curr_thread->mytid;	//set that child thread's joined parent to current thread
			curr_thread->mystate = BLOCKED;	
			uthread_yield(); //move current thread to blocked queue
		}
		else	//child has already exited into q_exited so we dont need to yield and block current thread
		{
			queue_iterate(q_exited, find_thread, (void*)(long)(int)tid, (void**)&ptr_wait_thread); //try to find child thread in exited queue instead 
			if(ptr_wait_thread == NULL) return -1; //cant find tid in any queues
			if(ptr_wait_thread->myjoiner != -1) return -1;	//child is already being joined
			ptr_wait_thread->myjoiner = curr_thread->mytid; //set that child thread's joined parent to current thread
		}
	}

	//if blocked, parent will resume here after their child has placed them back into the q_scheduler
	if(retval != NULL)	//otherwise, child has already exited, parent dont have to be blocked, and parent can free the child immediately
	{
		*retval = ptr_wait_thread->myexit;	//set retval to the return value of the joined child
	}
	uthread_ctx_destroy_stack(ptr_wait_thread->mystackptr);	//free child's resources
	queue_delete(q_exited, ptr_wait_thread);
	free(ptr_wait_thread);
	return 0;
}