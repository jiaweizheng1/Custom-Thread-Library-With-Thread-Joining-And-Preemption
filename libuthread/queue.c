#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

typedef struct node node_t;

struct node
{
	void *data;	//void * to some generalize data, string, char, int, etc
	node_t *nxtnode;	//each node has a ptr to its data and a ptr to the nxtnode
};

struct queue 
{
	node_t *head, *tail;	//the queue has ptrs to head and tail for easier enqueueing and dequeuing
	int length;	//size variable for making queue_length() function O(1)
};

queue_t queue_create(void)
{
	//only need to allocate space for the struct
	queue_t queue = (queue_t)malloc(sizeof(struct queue));

	if(queue != NULL)	//initialized queue variables
	{
		queue->head = NULL;
		queue->tail = NULL;
		queue->length = 0;
	}

	return queue;
}

int queue_destroy(queue_t queue)
{
	if(queue->length == 0 && queue != NULL)	//free queue once all nodes are deallocated
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
		struct node* temp_head_ptr = queue->head;	//a temporary ptr to the head
		queue->head = queue->head->nxtnode;	//so we can delete the head after making another node the head
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
			struct node* temp_prev_node_ptr;	//iterate through queue with two ptrs so if a node is deleted, its easier to link
			int found = 0;						//adjacent nodes back together

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
	if(queue == NULL || func == NULL) return -1;

	node_t* curr = queue->head;	//iterate through a queue in a similar fashion to queue_delete with two ptrs
								//in case current node gets deleted we still have a ptr to the next node
	while (curr != NULL)
	{
		void* curr_data = curr->data;
		node_t* curr_next = curr->nxtnode;
		int ret = (*func)(queue, curr_data, arg); //curr may be deleted here
		if (ret == 1)
		{
			if (data != NULL) *data = curr_data;
			break;
		}
		curr = curr_next;
	}

	return 0;
}

int queue_length(queue_t queue)
{
	if(queue != NULL)
	{
		return queue->length;
	}

	return -1;
}
