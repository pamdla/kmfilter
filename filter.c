#include <time.h>
#include "filter.h"

/*
int main(){
	char *ret;
	tree_init();
	tree_add("abc");
	tree_add("ABC");
	ret = tree_filter("abcdssdACABCeeessssssabc");

	printf("ret:%s\n",ret);
	printf("size:%d\n",pool->size);
	printf("total:%d\n",pool->pl_total);
}
*/
bool tree_init(){
	pool = (void *)malloc(sizeof(_pool));
	pool->size = POOL_SIZE;
	pool->list = (node *)malloc(pool->size*sizeof(node));
	memset(pool->list,0,pool->size*sizeof(node));
	pool->pl_total = 1;
	pool->pl_cursor = 1;
	recycle_init();
	pool->header = &(pool->list[0]);
	pool->end = (void *)malloc(sizeof(node));
	del_init();
	return true;
}

bool tree_add(char *dword){
	//printf("add:%s\n",dword);
	int i,d_len,chr,isend;
	node *p,*new;
	isend = 0;
	p = pool->header;
	d_len = strlen(dword);
	for(i=0;i<d_len;i++){
		chr = dword[i];
		if(p->nexts[chr] == NULL){
			if(pool->pl_total == pool->size){
				pool_expand();
			}
			if(i != (d_len-1)){
				if(pool->recycle_size == 0){
					new = &(pool->list[pool->pl_cursor]);
					pool->pl_cursor++;
				}else{
					new = recycle_extract();
				}
				p->nexts[chr] = new;
				pool->pl_total++;
			}else{
				p->nexts[chr] = pool->end;
			}
		}
		p = p->nexts[chr];
	}
}

bool tree_del(char *dword){
	//printf("del:%s\n",dword);
	node *p;
	int i,d_len,counter;
	del_node *delp;
	d_len = strlen(dword);
	p=pool->header;
	for(i=0;i<d_len;i++){
		//if(p->nexts[dword[i]]!=pool->end){
		if(p!=pool->end){
			del_push(p,dword[i]);
			p = p->nexts[dword[i]];
		}else{
			break;
		}
	}
	if(i != d_len || p!=pool->end){
		del_flush();
		return false;
	}
	while(1){
		counter = 0;
		delp = del_stack->top;
		for(i=0;i<NODE_WIDTH;i++){
			if(i == delp->index){
				continue;
			}
			if(delp->dnode->nexts[i] != NULL){
				counter++;
				break;
			}
		}
		if(counter>=1){
			if(delp->dnode == pool->header){
				delp->dnode->nexts[delp->index] = NULL;
			}
			break;
		}else{
			//add to the recycle queue
			recycle_insert(delp->dnode);
			delp->next->dnode->nexts[delp->next->index] = pool->end;
			pool->pl_total--;
			del_pop();
		}
	}
	del_flush();
	return true;
}


bool tree_flush() {
	memset(pool->list,0,pool->size*sizeof(node));
	pool->pl_total = 1;
	pool->pl_cursor = 1;
	recycle_flush();
}

char *tree_filter(char *target) {
	//printf("filter:%s\n",target);
	int i,j,t_len;
	char *ret,*tmpstr;
	tmpstr = (char *)malloc(100);
	ret = (char *)malloc(1024);
	memset(ret,0,1024);
	t_len = strlen(target);
	node *p;
	for(i=0;i<t_len;i++){
		p = pool->header;
		j = i;
		while(1){
			if(p->nexts[target[j]] == NULL){
				break;
			}
			//meet the end of a dirty word.
			if(p->nexts[target[j]] == pool->end){
				sprintf(tmpstr,"%d:%d;",i,j);
				ret = strcat(ret,tmpstr);
				break;
			}
			p = p->nexts[target[j]];
			j++;
		}
	}
	return ret;
}

bool pool_expand(){
	pool->list = realloc(pool->list,pool->size*2*sizeof(node));
	pool->size *= 2;
}

bool recycle_init() {
	pool->recycle_head = NULL;
	pool->recycle_tail = NULL;
	pool->recycle_size = 0;
	return true;
}

bool del_init() {
	del_stack = (_del_stack *)malloc(sizeof(_del_stack));
	del_stack->top = NULL;
}


bool del_flush(){
	del_node *p;
	while(del_stack->top != NULL){
		p = del_pop();
		free(p);
	}
	return true;
}

bool del_push(node *pnode,int index) {
	del_node *p;
	p = (del_node *)malloc(sizeof(del_node));
	if(p == NULL){
		return false;
	}
	memset(p,0,sizeof(del_node));
	p->dnode = pnode;
	p->index = index;
	p->next = del_stack->top;
	del_stack->top = p;
}

del_node *del_pop() {
	del_node *p;
	if(del_stack->top == NULL){
		return NULL;
	}
	p = del_stack->top;
	del_stack->top = p->next;
	return p;
}


bool recycle_insert(node *pnode){
	//printf("recycle_insert\n");
	if(pool->recycle_head == NULL){
		pool->recycle_head = pnode;
		pool->recycle_tail = pnode;
	}else{
		pool->recycle_head->nexts[0] = pnode;
		pool->recycle_head = pnode;
	}
	pnode->nexts[0] = NULL;
	pool->recycle_size++;
	//printf("recycle_size:%d\n",pool->recycle_size);
	return false;
}

node *recycle_extract(){
	//printf("recycle_extract\n");
	node *p;
	if(pool->recycle_tail == NULL){
		return NULL;
	}else{
		p = pool->recycle_tail;
		pool->recycle_tail = p->nexts[0];
		if(pool->recycle_head == p){
			pool->recycle_head == NULL;
		}
		pool->recycle_size--;
		//printf("recycle_size:%d\n",pool->recycle_size);
		return p;
	}
}

bool recycle_flush(){
	pool->recycle_head = NULL;
	pool->recycle_tail = NULL;
	pool->recycle_size = 0;
	return true;
}
