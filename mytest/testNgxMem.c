#include <stdio.h>
#include "../src/core/ngx_core.h"
#include "../src/core/ngx_config.h"
#include "../src/core/ngx_conf_file.h"
#include "../src/os/unix/ngx_errno.h"
#include "../src/core/ngx_string.h"
#include "../src/core/ngx_palloc.h"
#include "../src/core/ngx_array.h"
#include "../src/core/ngx_queue.h"


volatile ngx_cycle_t* ngx_cycle;

void ngx_log_error_core(ngx_uint_t level, ngx_log_t* log,
		                  ngx_err_t err, const char* fmt, ...)
{

};

void allocInPool(ngx_pool_t* pool, ngx_uint_t nNum);
void testNgxMemPool();
void testNgxArray();
void testNgxQueue();
void printSmallDataChunks(ngx_pool_t* pool);
void printLargeDataChunks(ngx_pool_t* pool);

typedef struct example_s{
	int n;
	char* str;
}example_t;

typedef struct employees_s{
	ngx_queue_t queue;
} employees_t;

typedef struct employee_s{
	ngx_uint_t id;
	u_char* name;
	ngx_queue_t queue;
}employee_t;

// 排序比较函数，id大的就大
ngx_int_t compare_employee(const ngx_queue_t* p, const ngx_queue_t* q){
	employee_t* pdata = (employee_t*)ngx_queue_data(p, employee_t, queue);
	employee_t* qdata = (employee_t*)ngx_queue_data(q, employee_t, queue);
	return ((pdata->id > qdata->id) ? 1 : 0);
}

int main(){
	testNgxMemPool();
	testNgxArray();
	testNgxQueue();
}

void testNgxMemPool(){
	printf("\n\n****testNgxMemPool****\n");
	ngx_pool_t* pool = NULL;
	example_t* exp  = NULL;
	char* s  = NULL;

	printf("The pool header takes space %d\n", sizeof(ngx_pool_t));
	size_t size = 1024;
	pool = ngx_create_pool(size, NULL);
	if(pool == NULL){
		return;
	}
	printf("Available Memory Pool Space is %d now!\n", (ngx_uint_t)(pool->d.end - pool->d.last));

	exp = ngx_palloc(pool, sizeof(exp));
	s = ngx_pcalloc(pool, sizeof("Hello World! LALALA"));
	printf("Available Memory Pool Space is %d now!\n", (ngx_uint_t)(pool->d.end - pool->d.last));


	exp->n = 1;
	exp->str = s;
	strncpy(s, "Hello World!", strlen("Hello World!"));
	printf("Pool max is %d.\n", pool->max);
	printf("exp->n = %d, exp->str = %s\n", exp->n, s);
	printf("pool->d.next = %d\n", (ngx_uint_t)pool->d.next);
	printf("Pool current is %d.\n", pool->current);
	printf("Pool large is %d.\n", pool->large);

	int i = 0;
	ngx_uint_t num = 2;
	for(i = 0; i < 10; i++){

		allocInPool(pool, num);
		num *= 2;
	}

	num = 241;
	for(i = 0; i < 10; i++){

		allocInPool(pool, num);
	}

	ngx_destroy_pool(pool);

}

void allocInPool(ngx_pool_t* pool, ngx_uint_t nNum){
	ngx_uint_t* iarr;
	iarr = ngx_palloc(pool, sizeof(ngx_uint_t)*nNum);
	printf("-------After alloc %d:------------\n", sizeof(ngx_uint_t)*nNum);
	printf("Pool space is %d, max is %d\n", (pool->d.end - pool->d.last), pool->max);
	//printf("Pool current is %d, large is %d.\n", pool->current, pool->large);
	printf("pool->d.failed = %d\n", (ngx_uint_t)pool->d.failed);

	printSmallDataChunks(pool);
	printLargeDataChunks(pool);
}

void printSmallDataChunks(ngx_pool_t* pool){
	ngx_pool_t* next = pool->d.next;
	printf("==List of next data block:\n");
	do{
		printf("d.next(%d) ->", (ngx_uint_t)next);
		if(next) next = next->d.next;

	}while(next != 0);
	printf("NULL\n");
}

void printLargeDataChunks(ngx_pool_t* pool){
	ngx_pool_large_t* large = pool->large;
	printf("==List of large data block:\n");
	do{
		printf("large(%d) ->", (ngx_uint_t)large);
		if(large) large = large->next;

	}while(large != 0);
	printf("NULL\n");
}

void testNgxArray(){
	printf("\n\n****testNgxArray****\n");
	ngx_pool_t* pool = NULL;
	ngx_array_t* arr = NULL;
	int n = 0;
	int* ele = NULL;

	printf("ngx_create_pool\n");
	pool = ngx_create_pool(1024, NULL);
	if(pool == NULL){
		printf("create pool failed\n");
		return;
	}
	printf("ngx_create_pool\n");

	printf("ngx_array_create\n");
	arr = ngx_array_create(pool, 1, sizeof(ngx_uint_t));
	if(arr == NULL){
		printf("create array failed\n");
		return;
	}
	printf("ngx_array_create\n");

	printf("ngx_array_push\n");
	for(n = 0; n < 5; n++){
		ele = (int*) ngx_array_push(arr);
		if(ele == NULL){
			printf("create array ele failed\n");
			return;
		}
		*ele = n;
		printf("new element %d added\n", n);
	}
	printf("arr->nelts = %d, arr->nalloc = %d\n", arr->nelts, arr->nalloc);

	ngx_uint_t size = 1;
	for(n = 0; n <12 ; n++){
		ele = (int*) ngx_array_push_n(arr, size);
		*ele = n;
		printf("pool space = %d(max = %d), ", (pool->d.end - pool->d.last), pool->max);
		printf("%d new elements added, ", size);
		printf("arr->nelts = %d, arr->nalloc = %d\n", arr->nelts, arr->nalloc);
		printSmallDataChunks(pool);
		printLargeDataChunks(pool);
		size += 50;
	}
	//printf("arr->nelts = %d, arr->nalloc = %d\n", arr->nelts, arr->nalloc);

	ngx_array_destroy(arr);
	ngx_destroy_pool(pool);
}

void testNgxQueue(){
	printf("\n\n****testNgxQueue****\n");
	ngx_pool_t* pool = NULL;
	employee_t* one = NULL;
	employees_t* all = NULL;
	ngx_queue_t* q = NULL;
	pool = ngx_create_pool(1024*sizeof(employee_t), NULL);
	if(pool == NULL){
		printf("create pool failed\n");
		return;
	}

	//create queue
	const ngx_str_t names[] = {ngx_string("Joe"), ngx_string("Bob"), ngx_string("Lily")};
	const ngx_uint_t ids[] = {1,2,3};

	all = ngx_palloc(pool, sizeof(employees_t));
	ngx_queue_init(&all->queue);

	int i;
	for(i = 0; i < 3; i++){
		one = (employee_t*)ngx_palloc(pool, sizeof(employee_t));
		one->id = ids[i];
		one->name = (u_char*)ngx_pstrdup(pool, (ngx_str_t*)&(names[i]));
		ngx_queue_init(&one->queue);
	    // 从头部进入队列
		ngx_queue_insert_head(&all->queue, &one->queue);
	}

	// 从尾部遍历输出
	for(q = ngx_queue_last(&all->queue);
		q != ngx_queue_sentinel(&all->queue);
		q = ngx_queue_prev(q)){
		one = ngx_queue_data(q, employee_t, queue);
		printf("No. %d, name is %s\n", one->id, one->name);
	}

	// 排序从头部输出
	ngx_queue_sort(&all->queue, compare_employee);
	printf("Sorting...\n");
	for(q = ngx_queue_prev(&all->queue);
		q != ngx_queue_sentinel(&all->queue);
		q = ngx_queue_last(q)){
		one = ngx_queue_data(q, employee_t, queue);
		printf("No. %d, name is %s\n", one->id, one->name);
	}
}

