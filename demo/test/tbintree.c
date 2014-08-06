/* file tbin_tree.c
 * test tbin_tree 
 *
 */

#include "nd_common/nd_common.h"
#include <time.h>

#define MAX_TREENODE 32768 
struct tbin_node __buf_bin[MAX_TREENODE];

struct tbin_tree __tree_root ;

LIST_HEAD(__free_tree) ;


struct tbin_node* alloc_tbin() 
{
	struct list_head *pos = __free_tree.next ;
	if(pos!=&__free_tree) {
		list_del_init(pos) ;
		return (struct tbin_node*) pos ;
	}
	return NULL ;
}

void free_tbin(struct tbin_node* node)
{
	struct list_head *pos = (struct list_head *)node ;
	INIT_LIST_HEAD(pos); 

	list_add_tail(pos,&__free_tree) ;
}

void init_tree()
{
	int i ;
	struct list_head *pos  ;
	INIT_LIST_HEAD(&__free_tree) ;
	tbin_init(&__tree_root,alloc_tbin, free_tbin) ;
	srand((int)time(NULL)) ;
	for (i=0; i<MAX_TREENODE; i++) {
		
		pos = (struct list_head *) (__buf_bin+i) ;
		INIT_LIST_HEAD(pos) ;

		list_add(pos, &__free_tree) ;
	}
}

void walk_entry(const struct tbin_node*node) 
{
	printf("walk item =%d\n", node->data) ;
}
void insert_node() 
{
	int total = 0 ;
	while (total <=(MAX_TREENODE-1)) {
		TTREE_DATA  item =(TTREE_DATA) rand()  ;
		if(tbin_insert(&__tree_root, item) ) {
			++total ;
		}
	}
}

void list_each()
{
	int i ;
	struct tbin_node *node ;

	printf("press ANY KEY to list for each in desc\n") ;
	i = 0 ;
	if(__tree_root.root) {
		tbin_for_each_desc(&__tree_root,node) {
			printf("%d item = %d\n", i , node->data) ;
		}
	}
	printf("press ANY KEY to  asc list for each asc\n") ;

	//getch() ;
	i = 0 ;
	if(__tree_root.root) {
		
		tbin_for_each_desc(&__tree_root,node) {
			printf("%d item = %d\n", i , node->data) ;
		}
	}
	printf("press ANY KEY to walk for each in asc\n") ;
	//getch() ;

	tbin_walk(&__tree_root, walk_entry) ;

}

int tbintree_test()
{
	int i ;
	for (i=0; i<10; i++)
	{
		init_tree() ;
		insert_node() ;
		list_each() ;
		tbin_destroy(&__tree_root) ;
	}

	return 0 ;
}