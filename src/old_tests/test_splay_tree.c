/*
#include <stdio.h>
#include <stdlib.h>
#include "../dbms/util/splay_treeh.h"
#include <time.h>



int main(void) {

   printf("STARTED\n");

   struct STree * s = splay_tree_create(NULL,NULL);
   struct STreeNode * n1 = splay_tree_create_node(NULL, 10);
   struct STreeNode * n2 = splay_tree_create_node(NULL, 12);
   struct STreeNode * n3 = splay_tree_create_node(NULL, 9);
   struct STreeNode * n4 = splay_tree_create_node(NULL, 20);
   struct STreeNode * n5 = splay_tree_create_node(NULL, 30);

   splay_tree_insert_node(s, n1);
   splay_tree_insert_node(s, n2);
   printf("--------------------------------");
   splay_tree_print(s);
   splay_tree_insert_node(s, n3);
   printf("--------------------------------");
   splay_tree_print(s);
   splay_tree_insert_node(s, n4);
   printf("--------------------------------");
   splay_tree_print(s);
   splay_tree_insert_node(s, n5);

   printf("--------------------------------");
   splay_tree_print(s);

   printf("\nroot %d",s->root->key);
   printf("\nmin %d", subtree_minimum(s->root)->key);
   printf("\nmax %d", subtree_maximum(s->root)->key);

   
   struct STreeNode * x = splay_tree_erase(s, 12);
   if(x != NULL){
      printf("\nREMOVED: %d", x->key);
   }else{
      printf("\nNULL");
   }
   
   printf("--------------------------------");
   splay_tree_print(s);

   x = splay_tree_erase(s, 9);
   if(x != NULL){
      printf("\nREMOVED: %d", x->key);
   }else{
      printf("\nNULL");
   }

   printf("--------------------------------");
   splay_tree_print(s);

   x = splay_tree_erase(s, 10);
   if(x != NULL){
      printf("\nREMOVED: %d", x->key);
   }else{
      printf("\nNULL");
   }

   x = splay_tree_erase(s, 20);
   if(x != NULL){
      printf("\nREMOVED: %d", x->key);
   }else{
      printf("\nNULL");
   }

   x = splay_tree_erase(s, 10);
   if(x != NULL){
      printf("\nREMOVED: %d", x->key);
   }else{
      printf("\nNULL");
   }

   printf("--------------------------------");
   splay_tree_print(s);
   
}
*/