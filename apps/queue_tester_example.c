#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <queue.h>

#define TEST_ASSERT(assert)				\
do {									\
	printf("ASSERT: " #assert " ... ");	\
	if (assert) {						\
		printf("PASS\n");				\
	} else	{							\
		printf("FAIL\n");				\
		exit(1);						\
	}									\
} while(0)

/* Create */
void test_create(void)
{
	fprintf(stderr, "*** TEST create ***\n");

	queue_t q = queue_create();

	TEST_ASSERT(q != NULL);
	queue_destroy(q);
}

/* Enqueue/Dequeue simple */
void test_queue_simple(void)
{
	int data = 3, *ptr;
	queue_t q;

	fprintf(stderr, "*** TEST queue_simple ***\n");

	q = queue_create();
	queue_enqueue(q, &data);
	queue_dequeue(q, (void**)&ptr);
	TEST_ASSERT(ptr == &data);
	queue_destroy(q);
}

//enqueue NULL
void test_queue_enqueue_null(void)
{
	queue_t q;

	fprintf(stderr, "*** TEST queue_NULL ***\n");

	q = queue_create();
	TEST_ASSERT(queue_enqueue(q, NULL) == -1);
	queue_destroy(q);
}

//enqueue/dequeue complex
void test_queue_complex(void)
{
	int data = 3, data2 = 4, data3 = 5, *ptr, *ptr2, *ptr3;
	queue_t q;

	printf("*** TEST queue_complex ***\n");

	q = queue_create();
	queue_enqueue(q, &data);
	queue_enqueue(q, &data2);
	queue_enqueue(q, &data3);
	queue_dequeue(q, (void**)&ptr);
	queue_dequeue(q, (void**)&ptr2);
	queue_dequeue(q, (void**)&ptr3);
	TEST_ASSERT(ptr == &data);
	TEST_ASSERT(ptr2 == &data2);
	TEST_ASSERT(ptr3 == &data3);
	queue_destroy(q);
}

//enqueue/dequeue complex
void test_queue_complex2(void)
{
	int data = 3, data2 = 4, data3 = 5, *ptr, *ptr2, *ptr3;
	queue_t q;

	printf("*** TEST queue_complex2 ***\n");

	q = queue_create();
	queue_enqueue(q, &data);
	queue_dequeue(q, (void**)&ptr);
	queue_enqueue(q, &data2);
	queue_dequeue(q, (void**)&ptr2);
	queue_enqueue(q, &data3);
	queue_dequeue(q, (void**)&ptr3);
	TEST_ASSERT(ptr == &data);
	TEST_ASSERT(ptr2 == &data2);
	TEST_ASSERT(ptr3 == &data3);
	queue_destroy(q);
}

//delete nonexisting data in queue
void test_queue_delete_nonexist(void)
{
	int data = 3, data2 = 4, data3 = 5, *ptr, *ptr2, *ptr3;
	int data4 = 6, data5 = 7;
	queue_t q;

	printf("*** TEST queue_delete_nonexist ***\n");

	q = queue_create();
	queue_enqueue(q, &data);
	queue_enqueue(q, &data2);
	queue_enqueue(q, &data3);
	TEST_ASSERT(queue_delete(q, &data4) == -1);
	TEST_ASSERT(queue_delete(q, &data5) == -1);
	queue_dequeue(q, (void**)&ptr);
	queue_dequeue(q, (void**)&ptr2);
	queue_dequeue(q, (void**)&ptr3);
	queue_destroy(q);
}

//delete complex
void test_queue_delete_complex(void)
{
	int data = 3, data2 = 4, data3 = 5, data4 = 4, data5 = 4, data6 = 6, *ptr, *ptr2, *ptr3, *ptr4;
	queue_t q;

	printf("*** TEST queue_delete_complex ***\n");

	q = queue_create();
	queue_enqueue(q, &data);
	queue_enqueue(q, &data2);
	queue_enqueue(q, &data3);
	queue_enqueue(q, &data4);
	queue_enqueue(q, &data5);
	queue_enqueue(q, &data6);
	queue_delete(q, &data2);
	queue_dequeue(q, (void**)&ptr);
	queue_delete(q, &data4);
	queue_dequeue(q, (void**)&ptr2);
	queue_dequeue(q, (void**)&ptr3);
	queue_dequeue(q, (void**)&ptr4);
	TEST_ASSERT(ptr == &data);
	TEST_ASSERT(ptr2 == &data3);
	TEST_ASSERT(ptr3 == &data5);
	TEST_ASSERT(ptr4 == &data6);
	queue_destroy(q);
}

//delete beginning
void test_queue_delete_beg(void)
{
	int data = 3, data2 = 4, data3 = 5, *ptr2, *ptr3;
	queue_t q;

	printf("*** TEST queue_delete_beg ***\n");

	q = queue_create();
	queue_enqueue(q, &data);
	queue_enqueue(q, &data2);
	queue_enqueue(q, &data3);
	queue_delete(q, &data);
	queue_dequeue(q, (void**)&ptr2);
	queue_dequeue(q, (void**)&ptr3);
	TEST_ASSERT(ptr2 == &data2);
	TEST_ASSERT(ptr3 == &data3);
	queue_destroy(q);
}

//delete mid1
void test_queue_delete_mid(void)
{
	int data = 3, data2 = 4, data3 = 5, *ptr, *ptr3;
	queue_t q;

	printf("*** TEST queue_delete_mid ***\n");

	q = queue_create();
	queue_enqueue(q, &data);
	queue_enqueue(q, &data2);
	queue_enqueue(q, &data3);
	queue_delete(q, &data2);
	queue_dequeue(q, (void**)&ptr);
	queue_dequeue(q, (void**)&ptr3);
	TEST_ASSERT(ptr == &data);
	TEST_ASSERT(ptr3 == &data3);
	queue_destroy(q);
}

//delete mid2
void test_queue_delete_mid2(void)
{
	int data = 3, data2 = 4, data3 = 5, *ptr, *ptr3;
	queue_t q;

	printf("*** TEST queue_delete_mid2 ***\n");

	q = queue_create();
	queue_enqueue(q, &data);
	queue_enqueue(q, &data2);
	queue_enqueue(q, &data3);
	queue_dequeue(q, (void**)&ptr);
	queue_delete(q, &data2);
	queue_dequeue(q, (void**)&ptr3);
	TEST_ASSERT(ptr == &data);
	TEST_ASSERT(ptr3 == &data3);
	queue_destroy(q);
}

//delete end1
void test_queue_delete_end(void)
{
	int data = 3, data2 = 4, data3 = 5, *ptr, *ptr2;
	queue_t q;

	printf("*** TEST queue_delete_end ***\n");

	q = queue_create();
	queue_enqueue(q, &data);
	queue_enqueue(q, &data2);
	queue_enqueue(q, &data3);
	queue_delete(q, &data3);
	queue_dequeue(q, (void**)&ptr);
	queue_dequeue(q, (void**)&ptr2);
	TEST_ASSERT(ptr == &data);
	TEST_ASSERT(ptr2 == &data2);
	queue_destroy(q);
}

//delete end2
void test_queue_delete_end2(void)
{
	int data = 3, data2 = 4, data3 = 5, *ptr, *ptr2;
	queue_t q;

	printf("*** TEST queue_delete_end2 ***\n");

	q = queue_create();
	queue_enqueue(q, &data);
	queue_enqueue(q, &data2);
	queue_enqueue(q, &data3);
	queue_dequeue(q, (void**)&ptr);
	queue_delete(q, &data3);
	queue_dequeue(q, (void**)&ptr2);
	TEST_ASSERT(ptr == &data);
	TEST_ASSERT(ptr2 == &data2);
	queue_destroy(q);
}

//delete end3
void test_queue_delete_end3(void)
{
	int data = 3, data2 = 4, data3 = 5, *ptr, *ptr2;
	queue_t q;

	printf("*** TEST queue_delete_end3 ***\n");

	q = queue_create();
	queue_enqueue(q, &data);
	queue_enqueue(q, &data2);
	queue_enqueue(q, &data3);
	queue_dequeue(q, (void**)&ptr);
	queue_dequeue(q, (void**)&ptr2);
	queue_delete(q, &data3);
	TEST_ASSERT(ptr == &data);
	TEST_ASSERT(ptr2 == &data2);
	queue_destroy(q);
}

/* Callback function that increments integer items by a certain value (or delete
 * item if item is value 42) */
static int inc_item(queue_t q, void *data, void *arg)
{
    int *a = (int*)data;
    int inc = (int)(long)arg;

    if (*a == 42)
        queue_delete(q, data);
    else
        *a += inc;

    return 0;
}

/* Callback function that finds a certain item according to its value */
static int find_item(queue_t q, void *data, void *arg)
{
    int *a = (int*)data;
    int match = (int)(long)arg;
    (void)q; //unused

    if (*a == match)
        return 1;

    return 0;
}

void test_iterator(void)
{
    queue_t q;
    int data[] = {1, 2, 3, 4, 5, 42, 6, 7, 8, 9};
    size_t i;
    int *ptr;

	printf("*** TEST queue_iterator ***\n");

    /* Initialize the queue and enqueue items */
    q = queue_create();
    for (i = 0; i < sizeof(data) / sizeof(data[0]); i++)
        queue_enqueue(q, &data[i]);

    /* Add value '1' to every item of the queue, delete item '42' */
    queue_iterate(q, inc_item, (void*)1, NULL);
    TEST_ASSERT(data[0] == 2);
    TEST_ASSERT(queue_length(q) == 9);

    /* Find and get the item which is equal to value '5' */
    ptr = NULL;     // result pointer *must* be reset first
    queue_iterate(q, find_item, (void*)5, (void**)&ptr);
    TEST_ASSERT(ptr != NULL);
    TEST_ASSERT(*ptr == 5);
    TEST_ASSERT(ptr == &data[3]);
	for (i = 0; i < 9; i++)
        queue_dequeue(q, (void**)&ptr);
	queue_destroy(q);	//for valgrind checking memory leaks
}

void test_iterator2(void)
{
    queue_t q;
    int data[] = {1, 2, 3, 4, 5, 9, 6, 7, 8, 42};
    size_t i;
    int *ptr;

	printf("*** TEST queue_iterator2 ***\n");

    /* Initialize the queue and enqueue items */
    q = queue_create();
    for (i = 0; i < sizeof(data) / sizeof(data[0]); i++)
        queue_enqueue(q, &data[i]);

    /* Add value '1' to every item of the queue, delete item '42' */
    queue_iterate(q, inc_item, (void*)1, NULL);
    TEST_ASSERT(data[0] == 2);
    TEST_ASSERT(queue_length(q) == 9);

    /* Find and get the item which is equal to value '5' */
    ptr = NULL;     // result pointer *must* be reset first
    queue_iterate(q, find_item, (void*)5, (void**)&ptr);
    TEST_ASSERT(ptr != NULL);
    TEST_ASSERT(*ptr == 5);
    TEST_ASSERT(ptr == &data[3]);
	for (i = 0; i < 9; i++)
        queue_dequeue(q, (void**)&ptr);
	queue_destroy(q);	//for valgrind checking memory leaks
}

void test_iterator3(void)
{
    queue_t q;
    int data[] = {1, 2, 3, 4, 5, 9, 6, 7, 42, 8};
    size_t i;
    int *ptr;

	printf("*** TEST queue_iterator3 ***\n");

    /* Initialize the queue and enqueue items */
    q = queue_create();
    for (i = 0; i < sizeof(data) / sizeof(data[0]); i++)
        queue_enqueue(q, &data[i]);

    /* Add value '1' to every item of the queue, delete item '42' */
    queue_iterate(q, inc_item, (void*)1, NULL);
    TEST_ASSERT(data[0] == 2);
    TEST_ASSERT(queue_length(q) == 9);

    /* Find and get the item which is equal to value '5' */
    ptr = NULL;     // result pointer *must* be reset first
    queue_iterate(q, find_item, (void*)9, (void**)&ptr);
    TEST_ASSERT(ptr != NULL);
    TEST_ASSERT(*ptr == 9);
    TEST_ASSERT(ptr == &data[9]);
	for (i = 0; i < 9; i++)
        queue_dequeue(q, (void**)&ptr);
	queue_destroy(q);	//for valgrind checking memory leaks
}

int main(void)
{
	test_create();
	test_queue_simple();
	test_queue_enqueue_null();
	test_queue_complex();
	test_queue_complex2();
	test_queue_delete_nonexist();
	test_queue_delete_complex();
	test_queue_delete_beg();
	test_queue_delete_mid();
	test_queue_delete_mid2();
	test_queue_delete_end();
	test_queue_delete_end2();
	test_queue_delete_end3();
	test_iterator();
	test_iterator2();
	test_iterator3();

	return 0;
}
