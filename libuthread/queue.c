#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

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
		queue->head = NULL;	//IDK if necessary
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
	node_t *node = (node_t*)malloc(sizeof(node_t));

	if(queue != NULL && data != NULL && node != NULL)
	{
		node->data = data;
		node->nxtnode = NULL; //IDK if necessary
		queue->length++;

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

		return 0;
	}

	return -1;
}

int queue_dequeue(queue_t queue, void **data)
{
	if(queue != NULL && data != NULL && queue->length != 0)
	{
		queue->length--;

		if(queue->head == queue->tail) //one node left in queue
		{
			data = &(queue->head->data);	//IDK if correct &
			free(queue->head); //IDK if need &
			queue->head = NULL; //IDK if necessary
			queue->tail = NULL; //IDK if necessary
		}
		else //more than one node in queue
		{
			data = &(queue->head->data); //IDK if correct &
			struct node* temp_head_ptr = queue->head;
			queue->head = queue->head->nxtnode;
			free(temp_head_ptr);
		}

		return 0;
	}

	return -1;
}

int queue_delete(queue_t queue, void *data)
{
	if(queue != NULL && data != NULL)
	{
		struct node* temp_node_ptr = queue->head;	//iterate starting from head
		struct node* temp_prev_node_ptr = NULL;

		if(queue->length == 1)	//dont need to iterate anymore because queue is of size 1
		{
			if(temp_node_ptr->data == data)
			{
				free(queue->head);
				queue->head = NULL; //IDK if necessary
				queue->tail = NULL; //IDK if necessary
				return 0;
			}
		}
		else	//need to iterate because queue has more than 1 nodes
		{
			for(int i=0; i < queue->length; i++)
			{
			}
		}
	}

	return -1;
}

int queue_iterate(queue_t queue, queue_func_t func, void *arg, void **data)
{
	/* TODO */
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