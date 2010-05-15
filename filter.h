#ifndef HAVE_CONFIG
#include "config.h"
#define HAVE_CONFIG
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define POOL_SIZE 36864/*1048576*/	/* the initial size of the list(the number of tree nodes) */
#define NODE_WIDTH 256	/* the next pointer number of node */

typedef enum 
{ 
    true = 1, 
    false = 0,
}bool;


/* the basic unit struct for the filter tree */
typedef struct _node {
	struct _node *nexts[NODE_WIDTH];	/* the 256 next pointer for next nodes */
} node;


/* the basic unit struct of the del stack used in tree_del */
typedef struct _del_node {
	struct _node *dnode;
	int index;
	struct _del_node *next;
} del_node;


/* the link of the del stack */
typedef struct {
	del_node *top;
} _del_stack;

static _del_stack *del_stack;

/* the pool that tree units storged */
typedef struct {
	unsigned int size;			/* the array size of the tree node */
	node *list;					/* the array of the tree unit */
	unsigned int pl_total;		/* the total node number of the tree */
	unsigned int pl_cursor;		/* the cursor of the list */

	unsigned int recycle_size;	/* the array size of the recycle queue */

	node *recycle_head;			/* the recycle queue head */
	node *recycle_tail;			/* the recycle queue tial */

	void *header;				/* the header note of the tree */
	void *end;					/* the common end of the tree */
} _pool;

static _pool *pool;



/* APIs */

/* initialize the pool. 
 *	1.initialize the list array;
 *	2.make the pl_total=0, pl_curr=0;
 *	3.call the queue_init() function to initialize the recycle queue;
 *	4.initialize the header pointer;
 *	5.initialize the end pointer;
 *	If successful,the return is true.while failed,the return is false;
**/
bool tree_init();



/* add a dirty word to the tree 
 *	the dword is a char pointer to a string.
 *	If successful,the return is true.while failed,the return is false;
**/
bool tree_add(char *dword);



/* del a dirty word of the tree,according to a atack.
 *	the dword is a char pointer to a string.
 *	If successful,the return is true.while failed,the return is false;
**/
bool tree_del(char *dword);



/* flush the tree 
 *	If successful,the return is true.while failed,the return is false;
**/
bool tree_flush();



/* filter a sting using the tree existed 
 *	the target is the string ready to be filtered;
 *	the starts is a pointer of a int array.as the function finished,it contains the starts of the dwords in the target;
 *	the ends is a pointer of a int array.as the function finished,it contains the ends of the dwords in the target;
 *	If successful,the return is a plus int
 *		0:no dword in the string.
 *		n:n dwords in the string.
 *	while failed,the return is -1;
**/
char *tree_filter(char *target);



/* expand memory of the pool list 
 *	while the pool list is full,relloc it to make a bigger pool list.
 *	If successful,the return is true.while failed,the return is false;
**/
bool pool_expand();



/* OTHER FUNCTIONS*/
/* initialize the pl_total 
 *	pl_total=0;
**/
static bool pool_total_init();



/* increment the pl_total
 *	pl_total+=1;
**/
static bool pool_total_increment();



/* decrement the pl_total
 *	pl_total-=1;
**/
static bool pool_total_decrement();



/* get pl_total value */
static int pool_total_get();



/* initialize the pl_cursor
 * pl_cursor=0;
**/
static bool pool_cursor_init();



/* increment the pl_cursor 
 *	pl_cursor+=1;
**/
static bool pool_cursor_increment();



/* decrement the pl_cursor 
 *	pl_cursor-=1;
**/
static bool pool_cursor_decrement();



/* get pl_cursor value */
static int pool_cursor_get();



/* pl_cursor rollback to the head 
 *	pl_cursor=0;
**/
static bool pool_cursor_rollback();



/* initialize the del_stack.
**/
static bool del_init();


/* push a del_node to the top of the del_stack.
**/
static bool del_push();


/* pop a del_node from the top of the del_stack.
**/
static del_node *del_pop();


/* flush the del_stack.
**/
static bool del_flush();


/* QUEUE FUNCTIONS */
/* initialize the recycle queue 
 *	1.initialize the recycle_head and recycle_tail,recycle_head=NULL,recycle_tail=NULL.
 *	If successful,the return is true.while failed,the return is false;
**/
static bool recycle_init();


/* insert a node to the head of recycle queue
**/
static bool recycle_insert(node *pnode);


/* get a node from the tail of recycle queue
**/
static node *recycle_extract();


/* flush the recycle queue while the tree is rebuild.
**/
static bool recycle_flush();


