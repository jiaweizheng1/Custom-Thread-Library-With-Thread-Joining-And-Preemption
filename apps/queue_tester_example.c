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

	TEST_ASSERT(queue_create() != NULL);
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
	queue_dequeue(q, (void**)&data3);
	TEST_ASSERT(ptr == &data);
	TEST_ASSERT(ptr2 == &data2);
	TEST_ASSERT(ptr3 == &data3);
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
}

//delete beginning
void test_queue_delete_beg(void)
{
	int data = 3, data2 = 4, data3 = 5, *ptr, *ptr2, *ptr3;
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
}

//delete mid1
void test_queue_delete_mid(void)
{
	int data = 3, data2 = 4, data3 = 5, *ptr, *ptr2, *ptr3;
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
}

//delete mid2
void test_queue_delete_mid2(void)
{
	int data = 3, data2 = 4, data3 = 5, *ptr, *ptr2, *ptr3;
	queue_t q;

	printf("*** TEST queue_delete_mid ***\n");

	q = queue_create();
	queue_enqueue(q, &data);
	queue_enqueue(q, &data2);
	queue_enqueue(q, &data3);
	queue_dequeue(q, (void**)&ptr);
	queue_delete(q, &data2);
	queue_dequeue(q, (void**)&ptr3);
	TEST_ASSERT(ptr == &data);
	TEST_ASSERT(ptr3 == &data3);
}

//delete end1
void test_queue_delete_end(void)
{
	int data = 3, data2 = 4, data3 = 5, *ptr, *ptr2, *ptr3;
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
}

//delete end2
void test_queue_delete_end(void)
{
	int data = 3, data2 = 4, data3 = 5, *ptr, *ptr2, *ptr3;
	queue_t q;

	printf("*** TEST queue_delete_end ***\n");

	q = queue_create();
	queue_enqueue(q, &data);
	queue_enqueue(q, &data2);
	queue_enqueue(q, &data3);
	queue_dequeue(q, (void**)&ptr);
	queue_delete(q, &data3);
	queue_dequeue(q, (void**)&ptr2);
	TEST_ASSERT(ptr == &data);
	TEST_ASSERT(ptr2 == &data2);
}

//delete end3
void test_queue_delete_end(void)
{
	int data = 3, data2 = 4, data3 = 5, *ptr, *ptr2, *ptr3;
	queue_t q;

	printf("*** TEST queue_delete_end ***\n");

	q = queue_create();
	queue_enqueue(q, &data);
	queue_enqueue(q, &data2);
	queue_enqueue(q, &data3);
	queue_dequeue(q, (void**)&ptr);
	queue_dequeue(q, (void**)&ptr2);
	queue_delete(q, &data3);
	TEST_ASSERT(ptr == &data);
	TEST_ASSERT(ptr2 == &data2);
}

int main(void)
{
	test_create();
	test_queue_simple();
	test_queue_complex();
	test_queue_complex2();
	test_queue_delete_beg();
	test_queue_delete_mid();
	test_queue_delete_mid2();
	test_queue_delete_end();

	return 0;
}
