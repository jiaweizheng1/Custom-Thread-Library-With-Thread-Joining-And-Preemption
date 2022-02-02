/*
 * Example of using the getcontext()/swapcontext() API. This API, provided by
 * ucontext.h, is not part of POSIX anymore because of portability issues.
 *
 * Create two threads from the main function and start a predefined sequence of
 * context switches.
 */

#include <assert.h>
#include <stdio.h>
#include <ucontext.h>

/*
 * 3 context objects (for the "main" thread and the two extra function threads)
 */
static ucontext_t ctx[3];

/*
 * Context switching takes a snapshot of currently running @from' context before
 * stopping its execution, and resumes the execution of @to by restoring its
 * saved context.
 */
static void context_switch(int from, int to)
{
	swapcontext(&ctx[from], &ctx[to]);
}

/*
 * Thread f1
 */
static void f1(void)
{
	printf("start f1\n");

	/* Context switch from f1 to f2 */
	context_switch(1, 2);

	printf("finish f1\n");

	/* Context switch from f1 to main */
	context_switch(1, 0);

	assert(0); /* We should never come back here */
}

/*
 * Thread f2
 */
static void f2(void)
{
	printf("start f2\n");

	/* Context switch from f2 to f1 */
	context_switch(2, 1);

	printf("finish f2\n");

	/* Context switch from f2 to f1 */
	context_switch(2, 1);

	assert(0); /* We should never come back here */
}

#define STACK_SIZE 32768
typedef void (*func_t) ();

int main(int argc, char *argv[])
{
	char stack1[STACK_SIZE], stack2[STACK_SIZE];

	/*
	 * Initialize context object for thread f1
	 * - use stack1 as stack and fill out stack size
	 * - pass one argument (23) to thread f1
	 */
	getcontext(&ctx[1]);
	ctx[1].uc_stack.ss_sp   = stack1;
	ctx[1].uc_stack.ss_size = STACK_SIZE;
	makecontext(&ctx[1], (func_t)f1, 0);

	/*
	 * Initialize context object for thread f2
	 * - use stack2 as stack and fill out stack size
	 * - pass one argument (42) to thread f2
	 */
	getcontext(&ctx[2]);
	ctx[2].uc_stack.ss_sp   = stack2;
	ctx[2].uc_stack.ss_size = STACK_SIZE;
	makecontext(&ctx[2], (func_t)f2, 0);

	/* Before we start scheduling */
	printf("The beginning...\n");

	/*
	 * Start the predefined scheduling sequence
	 * - Context switch from main to f2
	 */
	context_switch(0, 2);

	/* Back to main and quit */
	printf("The end...\n");

	return 0;
}

