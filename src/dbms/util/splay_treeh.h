/*
 * A splay tree is a binary search tree with the additional property 
 * that recently accessed elements are quick to access again. 
 * https://en.wikipedia.org/wiki/Splay_tree
 */

#ifndef SPLAY_TREE
#define SPLAY_TREE

#include <stdlib.h>
#include <stdio.h>



struct STreeNode{
    struct STreeNode * left;
    struct STreeNode * right;
    struct STreeNode * parent;
    long key;
    void * content;
};

struct STree{
    struct STreeNode * root;
    int size;
	/* The user of this list can inform the print and free functions */
	void (* print_function)(void *); /* Each type of data can have different forms of printing */
	void (* free_function) (void *); /* and different forms of deallocation (free) */
};

int comp(long key1, long key2);

void splay_tree_print(struct STree * tree);
void splay_tree_print2(struct STreeNode * root, int space);

void splay(struct STreeNode * x, struct STree * tree);
void left_rotate(struct STreeNode * x, struct STree * tree);
void right_rotate(struct STreeNode * x, struct STree * tree);

void splay_tree_replace(struct STreeNode *u, struct STreeNode *v, struct STree * tree);
struct STreeNode * subtree_minimum(struct STreeNode *u);
struct STreeNode * subtree_maximum(struct STreeNode *u);

struct STreeNode * splay_tree_find(struct STree * tree, long key);
struct STreeNode * splay_tree_erase(struct STree * tree, struct STreeNode * z);

struct STree* splay_tree_create(void (*print_function)(void*), void (*free_function)(void*)){
	struct STree* tree = (struct STree*) malloc(sizeof(struct STree));
	tree->root = NULL;
	tree->size = 0;
	tree->print_function = print_function;
	tree->free_function = free_function;
	return tree;
}

struct STreeNode * splay_tree_create_node(void* content, long key){
	struct STreeNode * new_node = (struct STreeNode*) malloc(sizeof(struct STreeNode));
	new_node->left = NULL;
	new_node->right = NULL;
  new_node->parent = NULL;
	new_node->content = content;
	new_node->key = key;
	return new_node;
}

void splay_tree_insert_node(struct STree * tree, struct STreeNode * new_node) {
	struct STreeNode * z = tree->root;
    struct STreeNode * p = NULL;
    
    while (z != NULL) {
      p = z;
      if (comp(z->key, new_node->key) == 1){
		z = z->right;
	  } else {
		z = z->left;
	  }
    }
    
    z = new_node; //new node(key);
    z->parent = p;
    
    if (p == NULL) {
		tree->root = z;
	}else if (comp(p->key, z->key)){
		p->right = z;
	} else {
		p->left = z;
	}
    splay(z, tree);
    tree->size++;
}


void splay(struct STreeNode * x, struct STree * tree) {
    while (x->parent) {
      if (!x->parent->parent) {
        if (x->parent->left == x) right_rotate(x->parent, tree);
        else left_rotate(x->parent, tree);
      } else if (x->parent->left == x && x->parent->parent->left == x->parent) {
        right_rotate(x->parent->parent, tree);
        right_rotate(x->parent, tree);
      } else if (x->parent->right == x && x->parent->parent->right == x->parent) {
        left_rotate(x->parent->parent, tree);
        left_rotate(x->parent, tree);
      } else if (x->parent->left == x && x->parent->parent->right == x->parent) {
        right_rotate(x->parent, tree);
        left_rotate(x->parent, tree);
      } else {
        left_rotate(x->parent, tree);
        right_rotate(x->parent, tree);
      }
    }
  }

void left_rotate(struct STreeNode *x, struct STree * tree) {
    struct STreeNode *y = x->right;
    if (y) {
      x->right = y->left;
      if (y->left) y->left->parent = x;
      y->parent = x->parent;
    }
    
    if (!x->parent) tree->root = y;
    else if (x == x->parent->left) x->parent->left = y;
    else x->parent->right = y;
    if (y) y->left = x;
    x->parent = y;
  }

void right_rotate(struct STreeNode *x, struct STree * tree) {
    struct STreeNode *y = x->left;
    if (y) {
      x->left = y->right;
      if (y->right) y->right->parent = x;
      y->parent = x->parent;
    }
    if (!x->parent) tree->root = y;
    else if (x == x->parent->left) x->parent->left = y;
    else x->parent->right = y;
    if (y) y->right = x;
    x->parent = y;
}


void splay_tree_replace(struct STreeNode *u, struct STreeNode *v, struct STree * tree) {
    if (!u->parent) tree->root = v;
    else if (u == u->parent->left) u->parent->left = v;
    else u->parent->right = v;
    if (v) v->parent = u->parent;
}
  
struct STreeNode * subtree_minimum(struct STreeNode *u) {
    while (u->left) u = u->left;
    return u;
}
  
struct STreeNode * subtree_maximum(struct STreeNode *u) {
    while (u->right) u = u->right;
    return u;
}


struct STreeNode * splay_tree_find(struct STree * tree, long key) {
    struct STreeNode * z = tree->root;
    while (z) {
      if (comp(z->key, key)) z = z->right;
      else if (comp(key, z->key)) z = z->left;
      else return z;
    }
    return NULL;
}
        
struct STreeNode * splay_tree_erase(struct STree * tree, struct STreeNode * z) {
    //struct STreeNode *z = splay_tree_find(tree, key);
    if (!z) return NULL;
    
    splay(z, tree);
    
    if (!z->left) splay_tree_replace(z, z->right, tree);
    else if (!z->right) splay_tree_replace(z, z->left, tree);
    else {
      struct STreeNode *y = subtree_minimum(z->right);
      if (y->parent != z) {
        splay_tree_replace(y, y->right, tree);
        y->right = z->right;
        y->right->parent = y;
      }
      splay_tree_replace(z, y, tree);
      y->left = z->left;
      y->left->parent = y;
    }
    z->left = NULL;
    z->right = NULL;
    z->parent = NULL;
    tree->size--;
    return z;
}




void splay_tree_print(struct STree * tree){
	splay_tree_print2(tree->root, 0);
}
#define COUNT 2
void splay_tree_print2(struct STreeNode * root, int space){
  
    if (root == NULL)
        return;

    space += COUNT;
  
   	splay_tree_print2(root->right, space);
 
    printf("\n");
    for (int i = COUNT; i < space; i++){
        printf(" ");
	}
    printf("%d\n", root->key);
 
    splay_tree_print2(root->left, space);
}
 
int comp(long key1, long key2){
	if(key1 < key2){
		return 1;
	}else{
		return 0;
	}
}

#endif