# Project 2 Report
<!--
summary
-->

Contributors: Haixu Qin, Jiawei Zheng

[Phase I](#Phase-I)

[Phase II](#Phase-II)

[Phase III](#Phase-III)

[Phase IV](#Phase-IV)

## Phase I
We chose to use a linked list instead of an array to implement the queue, because the size of the queue is unknown.
The queue keeps track of the head, tail, and length of the queue.
The head and tail are of type struct node, which contains its data and the next node (nxtnode).
For the base case when there is only one node in the queue, the head and tail point to the same node, of which the next node is set to NULL.
For queue_iterate, it stores the current node's data and the next node into the stack, because the heap may be overwritten
if the function deletes the current node and then calls queue_enqueue.
<!--TODO-->

## Phase II

## Phase III

## Phase IV
