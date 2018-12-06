/* file : ND_bintree.h
 * This file realize bintree arithmetic 
 * 
 * version 1.0 
 * author : neil 
 * 2005-11-4
 */

/*
 * porting from linux , and change function name .
 *
 desc :
 Linux's rbtree implementation lives in the file "lib/rbtree.c".  To use it,
 "#include <linux/rbtree.h>".

 The Linux rbtree implementation is optimized for speed, and thus has one
 less layer of indirection (and better cache locality) than more traditional
 tree implementations.  Instead of using pointers to separate rb_node and data
 structures, each instance of struct rb_node is embedded in the data structure
 it organizes.  And instead of using a comparison callback function pointer,
 users are expected to write their own tree search and insert functions
 which call the provided rbtree functions.  Locking is also left up to the
 user of the rbtree code.

 Creating a new rbtree
 ---------------------

 Data nodes in an rbtree tree are structures containing a struct rb_node member:
 [CODE]
 struct mytype {
 struct rb_node node;
 char *keystring;
 };
 [/CODE]
 When dealing with a pointer to the embedded struct rb_node, the containing data
 structure may be accessed with the standard container_of() macro.  In addition,
 individual members may be accessed directly via rb_entry(node, type, member).

 At the root of each rbtree is an rb_root structure, which is initialized to be
 empty via:
 [CODE]
 struct rb_root mytree = RB_ROOT;
 [/CODE]
 Searching for a value in an rbtree
 ----------------------------------

 Writing a search function for your tree is fairly straightforward: start at the
 root, compare each value, and follow the left or right branch as necessary.

 Example:
 [CODE]
 struct mytype *my_search(struct rb_root *root, char *string)
 {
	 struct rb_node *node = root->rb_node;

	 while (node) {
		 struct mytype *data = container_of(node, struct mytype, node);
		 int result;

		 result = ndstrcmp(string, data->keystring);

		 if (result < 0)
			node = node->rb_left;
		 else if (result > 0)
			node = node->rb_right;
		 else
			return data;
	}
	return NULL;
 }
 [/CODE]
 Inserting data into an rbtree
 -----------------------------

 Inserting data in the tree involves first searching for the place to insert the
 new node, then inserting the node and rebalancing ("recoloring") the tree.

 The search for insertion differs from the previous search by finding the
 location of the pointer on which to graft the new node.  The new node also
 needs a link to its parent node for rebalancing purposes.

 Example:
 [CODE]
 int my_insert(struct rb_root *root, struct mytype *data)
 {
 struct rb_node **new = &(root->rb_node), *parent = NULL;

  //Figure out where to put new node 
	while (*new) {
		struct mytype *this = container_of(*new, struct mytype, node);
		int result = ndstrcmp(data->keystring, this->keystring);

		parent = *new;
		if (result < 0)
			new = &((*new)->rb_left);
		else if (result > 0)
			new = &((*new)->rb_right);
		else
			return FALSE;
	}

	//Add new node and rebalance tree. 
	rb_link_node(data->node, parent, new);
	rb_insert_color(data->node, root);

	return TRUE;
  }
  [/CODE]
  Removing or replacing existing data in an rbtree
	  ------------------------------------------------

	  To remove an existing node from a tree, call:
  [CODE]
  void rb_erase(struct rb_node *victim, struct rb_root *tree);
  [/CODE]
Example:
  [CODE]
  struct mytype *data = mysearch(mytree, "walrus");

  if (data) {
	  rb_erase(data->node, mytree);
	  myfree(data);
  }
  [/CODE]
  To replace an existing node in a tree with a new one with the same key, call:
  [CODE]
  void rb_replace_node(struct rb_node *old, struct rb_node *new,
  struct rb_root *tree);
  [/CODE]
  Replacing a node this way does not re-sort the tree: If the new node doesn't
	  have the same key as the old node, the rbtree will probably become corrupted.

	  Iterating through the elements stored in an rbtree (in sort order)
	  ------------------------------------------------------------------

	  Four functions are provided for iterating through an rbtree's contents in
	  sorted order.  These work on arbitrary trees, and should not need to be
	  modified or wrapped (except for locking purposes):
  [CODE]
  struct rb_node *rb_first(struct rb_root *tree);
  struct rb_node *rb_last(struct rb_root *tree);
  struct rb_node *rb_next(struct rb_node *node);
  struct rb_node *rb_prev(struct rb_node *node);
  [/CODE]
  To start iterating, call rb_first() or rb_last() with a pointer to the root
	  of the tree, which will return a pointer to the node structure contained in
	  the first or last element in the tree.  To continue, fetch the next or previous
	  node by calling rb_next() or rb_prev() on the current node.  This will return
	  NULL when there are no more nodes left.

	  The iterator functions return a pointer to the embedded struct rb_node, from
	  which the containing data structure may be accessed with the container_of()
	  macro, and individual members may be accessed directly via
	  rb_entry(node, type, member).

Example:
  [CODE]
  struct rb_node *node;
  for (node = rb_first(&mytree); node; node = rb_next(node))
	  printk("key=%s\n", rb_entry(node, int, keystring));

  [/CODE]
 */

#ifndef _NDBINTREE_H_
#define _NDBINTREE_H_

struct nd_rb_node
{
	struct nd_rb_node *rb_right;
	struct nd_rb_node *rb_left;
	size_t  rb_parent_color;
#define	RB_RED		0
#define	RB_BLACK	1
} ;//__attribute__((aligned(sizeof(long))));
/* The alignment might seem pointless, but allegedly CRIS needs it */

struct nd_rb_root
{
	struct nd_rb_node *rb_node;
};

#define rb_parent(r)   ((struct nd_rb_node *)((r)->rb_parent_color & ~3))
#define rb_color(r)   ((r)->rb_parent_color & 1)
#define rb_is_red(r)   (!rb_color(r))
#define rb_is_black(r) rb_color(r)
#define rb_set_red(r)  do { (r)->rb_parent_color &= ~1; } while (0)
#define rb_set_black(r)  do { (r)->rb_parent_color |= 1; } while (0)

static __inline__ void rb_set_parent(struct nd_rb_node *rb, struct nd_rb_node *p)
{
	rb->rb_parent_color = (rb->rb_parent_color & 3) | (size_t)p;
}
static __inline__ void rb_set_color(struct nd_rb_node *rb, size_t color)
{
	rb->rb_parent_color = (rb->rb_parent_color & ~1) | color;
}

#define RB_ROOT	(struct rb_root) { NULL, }
#define	rb_entry(ptr, type, member) ((type *)((char *)(ptr)-(char*)(&((type *)0)->member)))

#define RB_EMPTY_ROOT(root)	((root)->rb_node == NULL)
#define RB_EMPTY_NODE(node)	(rb_parent(node) == node)
#define RB_CLEAR_NODE(node)	(rb_set_parent(node, node))

static __inline__ void rb_init_node(struct nd_rb_node *rb)
{
	rb->rb_parent_color = 0;
	rb->rb_right = NULL;
	rb->rb_left = NULL;
	RB_CLEAR_NODE(rb);
}

ND_COMMON_API void rb_insert_color(struct nd_rb_node *, struct nd_rb_root *);
ND_COMMON_API void rb_erase(struct nd_rb_node *, struct nd_rb_root *);

typedef void (*rb_augment_f)(struct nd_rb_node *node, void *data);

ND_COMMON_API void rb_augment_insert(struct nd_rb_node *node,	rb_augment_f func, void *data);
ND_COMMON_API struct nd_rb_node *rb_augment_erase_begin(struct nd_rb_node *node);
ND_COMMON_API void rb_augment_erase_end(struct nd_rb_node *node,	rb_augment_f func, void *data);

/* Find logical next and previous nodes in a tree */
ND_COMMON_API struct nd_rb_node *rb_next(const struct nd_rb_node *);
ND_COMMON_API struct nd_rb_node *rb_prev(const struct nd_rb_node *);
ND_COMMON_API struct nd_rb_node *rb_first(const struct nd_rb_root *);
ND_COMMON_API struct nd_rb_node *rb_last(const struct nd_rb_root *);

/* Fast replacement of a single node without remove/rebalance/add/rebalance */
ND_COMMON_API void rb_replace_node(struct nd_rb_node *victim, struct nd_rb_node *newnode,struct nd_rb_root *root);

static __inline__ void rb_link_node(struct nd_rb_node * node, struct nd_rb_node * parent,struct nd_rb_node ** rb_link)
{
	node->rb_parent_color = (size_t)parent;
	node->rb_left = node->rb_right = NULL;
	*rb_link = node;
}

#endif 
