#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "private.h"
#include "uthread.h"

/*
 * Frequency of preemption
 * 100Hz is 100 times per second
 */
#define HZ 100

struct sigaction sa;
struct itimerval timer;

void alarm_handler (int signum)
{
	if(signum == SIGVTALRM)
	{
		uthread_yield();
		printf("swapped\n");
	}
}

void preempt_start()
{
	printf("preempt started\n");
 	//install alarm_handler as the signal handler for SIGVTALRM

	sa.sa_handler = alarm_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGVTALRM, &sa, NULL);

	timer.it_value.tv_sec = 0;	//rn 1 sec, change to 10000 us later
	timer.it_value.tv_usec = 10000;
	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = 10000;
	//start a virtual timer
	setitimer (ITIMER_VIRTUAL, &timer, NULL);
}

void preempt_stop(void)
{
	preempt_disable();
	signal(SIGILL, SIG_DFL);	//reset to default behavior for signals
	signal(SIGABRT, SIG_DFL);
}

void preempt_enable(void)
{
	sigprocmask(SIG_UNBLOCK, &sa.sa_mask, NULL);
}

void preempt_disable(void)
{
	sigprocmask(SIG_BLOCK, &sa.sa_mask, NULL);
}