#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/*
#include <stdio.h>
#include <assert.h>
#include <stdio.h>
typedef struct queue* queue_t;
queue_t queue_create(void);
int queue_destroy(queue_t queue);
int queue_enqueue(queue_t queue, void *data);
int queue_dequeue(queue_t queue, void **data);
int queue_delete(queue_t queue, void *data);
typedef int (*queue_func_t)(queue_t queue, void *data, void *arg);
int queue_iterate(queue_t queue, queue_func_t func, void *arg, void **data);
int queue_length(queue_t queue);
*/

typedef struct node node_t;

struct node
{
	void *data;
	node_t *nxtnode;	//each node has a ptr to its data and a ptr to the nxtnode
};

struct queue 
{
	node_t *head, *tail;	//the queue has a ptrs to head and tail for easier enqueueing and dequeuing
	int length;
};

queue_t queue_create(void)
{
	queue_t queue = (queue_t)malloc(sizeof(queue_t));

	if(queue != NULL)
	{
		queue->head = NULL;
		queue->tail = NULL;	//IDK very likely can delete because overwritten later anyways
		queue->length = 0;
	}

	return queue;
}

int queue_destroy(queue_t queue)
{
	if(queue->length == 0 && queue != NULL)
	{
		free(queue);
		return 0;
	}
	
	return -1;
}

int queue_enqueue(queue_t queue, void *data)
{
	if(queue != NULL && data != NULL)
	{
		node_t *node = (node_t*)malloc(sizeof(node_t));

		if(node != NULL)
		{
			node->data = data;
			node->nxtnode = NULL; 

			if(queue->head == NULL)	//first node
			{
				queue->head = node;
				queue->tail = node;
			}
			else	//nodes after the first node
			{
				queue->tail->nxtnode = node;
				queue->tail = node;
			}

			queue->length++;
			return 0;
		}
	}

	return -1;
}

int queue_dequeue(queue_t queue, void **data)
{
	if(queue != NULL && data != NULL && queue->length != 0)
	{
		*data = queue->head->data;
		struct node* temp_head_ptr = queue->head;
		queue->head = queue->head->nxtnode;
		free(temp_head_ptr);
		queue->length--;
		return 0;
	}

	return -1;
}

int queue_delete(queue_t queue, void *data)
{
	if(queue != NULL && data != NULL)
	{
		struct node* temp_node_ptr = queue->head;	//iterate starting from head

		if(temp_node_ptr->data == data)	//head has the same data
		{
			queue->head = queue->head->nxtnode;
			free(temp_node_ptr);
			queue->length--;
			return 0;
		}
		else	//need to iterate because queue has atleast 2 nodes and head does not have the same data
		{
			struct node* temp_prev_node_ptr;
			int found = 0;

			for(int i=1; (i < queue->length) && !found; i++)	//skip first node
			{
				temp_prev_node_ptr = temp_node_ptr;
				temp_node_ptr = temp_node_ptr->nxtnode;
				if(temp_node_ptr->data == data)
				{
					found = 1;
				}
			}

			if(found)
			{
				if(temp_node_ptr->nxtnode == NULL)	//is tail node
				{
					queue->tail = temp_prev_node_ptr;
					queue->tail->nxtnode = NULL;
				}
				else	//is not last node
				{
					temp_prev_node_ptr->nxtnode = temp_node_ptr->nxtnode;
				}

				free(temp_node_ptr);
				queue->length--;
				return 0;
			}
		}
	}

	return -1;
}

int queue_iterate(queue_t queue, queue_func_t func, void *arg, void **data)
{
	if(queue != NULL && func != NULL)
	{
		if(queue->head == NULL) //return 0 prematurely for queue of size 0
		{
			return 0;
		}

		struct node* temp_node_ptr = queue->head->nxtnode;	//keep a ptr for next node in case current node gets deleted
		struct node* temp_prev_node_ptr = queue->head;

		int retval;
		int done = 0;
		retval = func(queue, temp_prev_node_ptr->data, arg);
		while(!retval && !done)	//IDK data might need * or &
		{
			if(temp_node_ptr == NULL)
			{
				done = 1;
			}
			else
			{
				temp_prev_node_ptr = temp_node_ptr;
				temp_node_ptr = temp_node_ptr->nxtnode;
				retval = func(queue, temp_prev_node_ptr->data, arg);
			}
		}

		if(retval && data != NULL)
		{
			*data = temp_prev_node_ptr->data;
		}
		
		return 0;
	}
	
	return -1;
}

int queue_length(queue_t queue)
{
	if(queue != NULL)
	{
		return queue->length;
	}

	return -1;
}