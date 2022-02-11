# Project 2 Report

Contributors: Haixu Qin, Jiawei Zheng

[Phase I](#Phase-I-queue-API) 

[Phase II](#Phase-II-uthread-API) 

[Phase III](#Phase-III-uthreadjoin) 

[Phase IV](#Phase-IV-preemption-and-test_preemptx) 

[References](#References)

## Phase I: queue API 
A FIFO(first in, first out) queue is a type of structure for storing 
information; the first item added to the queue will be the first one to be 
removed. For the queue, we chose to use a linked list of nodes. Each node 
contains a pointer to the next node and a pointer to data. The queue structure 
enables manipulating this linked list by having a pointer to the head and tail 
of the linked list and a size variable for how many nodes are currently in the 
queue. For the base case, for example, when there is only one node in the 
queue, the head and tail points to the same node, of which the next node is set 
to NULL. First, one benefit of using a linked list is that memory for nodes are 
dynamically allocated to only store however many data the user wants to store 
and thus, it is memory efficient because we do not use extra memory when we do 
not need to. Dynamic allocation also makes our queue structure more flexible 
because we do not need to know how many data the user wants to store ahead of 
time. Second, using a linked list also satisfies all the runtime limitations 
for the queue functions. For example, queue_enqueue() is O(1) because we just 
use the tail node pointer of the queue to set the tail node’s next node pointer 
to the new node and update tail node pointer to point to the new node; we do 
not have to traverse the entire linked list to add a node to the end. 
Similarly, for queue_dequeue(), it is also O(1) because we just update the head 
node pointer of the queue to the next node of the current head node and we free 
the old head node by keeping a temporary pointer to it before performing those 
operations. In addition, queue_length(), which returns size of queue, is also O
(1) because we update the size variable of the queue structure everytime we use 
queue_enqueue() or queue_dequeue() so we can just return this size variable. 
For queue_iterate() and queue_delete(), we made these functions resistant to 
the current node or data being deleted by iterating through the queue with two 
pointers, a pointer to the current node and a pointer to the node before the 
current node. Thus, when we do find the data we want to delete(the current node)
, we just rejoin the linked list by updating the next node pointer of the 
previous node to the next node pointer of the current node and then we are safe 
to delete the current node. Last, there are pitfalls to our queue structure. 
Our queue_iterate() and queue_delete() is O(n), n is the size of the queue, 
because the functions always traverse from the oldest node of the queue to the 
newest node of the queue. Thus, there are scenarios in which the data we want 
to delete or operate on might be the last node in the queue and so we have to 
iterate through the entire queue. 
## Phase II: uthread API 
Threads are sequences of instructions that run concurrently within a single 
process and they share the same global variables, heap memory, and open file 
descriptors. To deal with the creation and scheduling of threads, we created a 
structure called thread control block or TCB to hold all the important 
information for a thread. Our TCB stores the thread’s TID, context, state(ready,
blocked,exited,etc), exit status, and an int variable for storing the TID of 
another thread who has joined this thread(if any). TID is important for 
differentiating threads, an exit status variable allows us to retrieve a 
thread’s return value in O(1), and an int variable for storing joined parent 
thread’s TID is helpful for making sure that no two threads can join the same 
thread. Storing context is also important because it is the only way to resume 
the execution of a thread(via context_swap() function). To ensure maximum 
efficiency scheduling, we have decided to use three queues: a ready/running 
queue(or q_scheduler), a blocked queue, and an exited queue. The ready/running 
queue only stores threads that are ready to run or are currently running on the 
cpu. The blocked queue only stores blocked threads who are waiting for other 
threads to finish executing, and the exited queue only stores threads that have 
already finished execution and no longer requires cpu time. We chose to 
separate storing of ready/running threads from blocked and exited threads 
because this ensures that scheduling threads for execution via queue_iterate() 
and q_scheduler queue is always O(1). Because our yield() function always 
places blocked threads and exited threads into their respective queues first 
before we look for a thread to schedule, it is always guaranteed that the next 
thread after the currently running thread in q_scheduler is ready to be 
scheduled. We declared the first TCB node within q_scheduler to be the 
currently running thread because this is also helpful for making scheduling as 
efficient as possible; at most, we only have to iterate through or dequeue/
enqueue the first thread to find the next ready thread. Further, separation of 
blocked threads from exited threads is important because this ensures the best 
runtime possible for bringing back blocked threads within the blocked queue to 
the q_scheduler queue for rescheduling once their children threads have exited; 
we would have to iterate through exited threads to find blocked threads for 
rescheduling if the blocked and exited queues were combined. Furthermore, this 
separation is also good for when a blocked thread has been rescheduled into 
q_scheduler so they can collect and free their child’s resource; this ensures 
the best runtime for finding the blocked thread’s child within the exited queue 
because there are no blocked threads in this queue but only other exited 
threads.
## Phase III: uthread_join() 
A thread A joining thread B means that thread A should be blocked from 
execution until thread B has exited. We decided to make a blocked queue to 
store these blocked threads and only retrieve these blocked threads from the 
blocked queue to q_scheduler queue once their child has exited. We chose this 
path because it was more cpu cycle efficient than continuously rescheduling 
blocked threads for execution to check if their child has exited yet. When a 
thread joins another thread, we realize there are two scenarios we have to 
cover. The child thread they are trying to join is in the q_scheduler or 
blocked queue, in which case the parent should be sent to the blocked queue, 
yield, and wait for their child to exit. Or their child thread has already 
exited into exited queue, so the parent does not have to yield and can 
immediately collect and free their child’s resources. Looking for the child is O
(n), where n is the number of alive threads(ready, blocked, exited), which is a 
limitation. However, we still try to make looking for the child as efficient as 
possible by not iterating the other queues if we have already found the child 
in one of the queues; which is still O(n) because the child might be in the 
last queue we search through or the child do not even exist.
## Phase IV: preemption and test_preempt.x
We decided to disable preemption for when there is a thread context switch or 
initialization of thread context, when the queues are being dequeued or 
enqueued, and when a thread exits by calling uthread_exit(). For example, we 
think thread contexts will become corrupt if preempted midway during 
initialization or context swapping. A queue might be dequeued, becomes empty, 
preemption happens, then a thread tries to read from an empty queue. Protecting 
uthread_exit() from preemption is important because we do not want a child 
thread to be preempted during uthread_exit() after rescheduling their parent to 
q_scheduler, and have not moved to exited queue yet but the parent is going to 
try to free the child’s resources in exited queue when it is not there. For 
test_preempt.x, we made main create two threads that are stuck in while loops 
and then join one of them. Since main joins a non returning thread, main is 
stuck forever in the blocked queue so it will never be scheduled again. Then, 
the only threads that can be scheduled are the two threads that main has 
created. Each thread prints out themselves saying hello with their TID within 
the while loop. When you run the program, you can see from the terminal that it 
oscillates between thread 1 saying hello and thread 2 saying hello; thus, our 
preemption works.
## References
Lecture/discussion slides \
Piazza \
https://linux.die.net/man/3/swapcontext \
https://opensource.apple.com/source/libplatform/libplatform-126.1.2/src/ucontext/generic/swapcontext.c \
https://linux.die.net/man/2/setitimer \
https://man7.org/linux/man-pages/man2/sigprocmask.2.html \
https://man7.org/linux/man-pages/man2/sigaction.2.html \
https://stackoverflow.com/questions/16493224/block-alarm-in-c-in-linux
