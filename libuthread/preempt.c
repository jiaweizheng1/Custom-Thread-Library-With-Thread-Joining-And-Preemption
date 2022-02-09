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
#define us 10000	//100hz is 10000us

struct sigaction sa;
struct itimerval timer;

void alarm_handler(int signum)
{
	if(signum == SIGVTALRM)
	{
		uthread_yield();
	}
}

void preempt_start()
{
 	//install alarm_handler as the signal handler for SIGVTALRM
	sa.sa_handler = alarm_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGVTALRM, &sa, NULL);

	//set alarm interval
	timer.it_value.tv_sec = 0;	//100hz is 10000 us
	timer.it_value.tv_usec = 10000;
	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = 10000;
	//start a virtual timer
	setitimer (ITIMER_VIRTUAL, &timer, NULL);
}

void preempt_stop(void)
{
	setitimer(ITIMER_VIRTUAL, NULL, NULL);	//stop timer
	sa.sa_handler = SIG_DFL;
	sigaction(SIGVTALRM, &sa, NULL);	//reset handler for virtual alarm to default handler
}

void preempt_enable(void)
{
	sigprocmask(SIG_UNBLOCK, &sa.sa_mask, NULL);	//unblock sigvtalarm
}

void preempt_disable(void)
{
	sigprocmask(SIG_BLOCK, &sa.sa_mask, NULL);	//block sigvtalarm
}