#include <stdio.h>
#include <stdlib.h>

#include <uthread.h>

uthread_t tid[2];
int thread2(void)
{
    int ret;
    printf("thread2\n");
    uthread_join(tid[0], &ret);
    printf("thread1 returned %d\n", ret);
    return 2;
}
int thread1(void)
{
    tid[1] = uthread_create(thread2);
    printf("thread1\n");
    return 1;
}
int main(void)
{
    int ret;
    uthread_start(0);
    tid[0] = uthread_create(thread1);
    uthread_yield();
    uthread_join(tid[1], &ret);
    printf("thread2 returned %d\n", ret);
    uthread_stop();
    return 0;
}