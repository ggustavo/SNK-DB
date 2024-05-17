/*
 * 
 * Splay Tree Testing
 *
 */
#ifndef POLICY_H_INCLUDED
#define POLICY_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include "../db_buffer.h"
#include "../../db_config.h"
#include "../../util/splay_treeh.h"


struct STree * tree;

/*
 * This function is called after initializing the buffer and before page requests.
 * 
 * Initialize the structures used in the page replacement policy here.
 */
void buffer_policy_start(){
    printf("\nBuffer Replacement Policy: %s", __FILE__);
    printf("\n---------------------------------------------------------------------------------------------------");
	
    tree = splay_tree_create(NULL, NULL);
}


struct Page * buffer_request_page(int file_id, long block_id, char operation){

	// It is mandatory to call this two functions (buffer_find_page, buffer_computes_request_statistics)
	//--------------------------------------------------------
	struct Page * page = buffer_find_page(file_id, block_id);
	buffer_computes_request_statistics(page, operation);
	//--------------------------------------------------------

	if(page != NULL){ /* HIT - Update MRU */

		struct STreeNode * hitted = (struct STreeNode*) page->extended_attributes;
        hitted = splay_tree_erase(tree, hitted);
        splay_tree_insert_node(tree, hitted);

	} else { /* MISS - page is not in Buffer (struct Page * page == NULL) */

		if (buffer_is_full() == FALSE) {

			page = buffer_get_free_page();
			struct STreeNode * new_node = splay_tree_create_node(page, block_id);
			buffer_load_page(file_id, block_id, page); // Read the data from storage media
			
            splay_tree_insert_node(tree, new_node);
            page->extended_attributes = new_node;
            

		} else { /* Need a replacement */

			struct STreeNode * node_victim = subtree_minimum(tree->root);
            node_victim = splay_tree_erase(tree, node_victim);
			
            struct Page * victim = (struct Page *) node_victim->content; /* Get the LRU Page */
			debug("\n ---- REPLACEMENT victim: %c[%d-%d]", victim->dirty_flag, victim->file_id, victim->block_id);

			buffer_flush_page(victim); /* Flush the data to the secondary storage media if is dirty */

			page = buffer_reset_page(victim); /* To avoid malloc a new page we reuse the victim page */

			buffer_load_page(file_id, block_id, page); /* Read new data from storage media */
			
            node_victim->key = block_id;
            splay_tree_insert_node(tree, node_victim);

		}

	}
	set_dirty(page, operation);
	return page;
}

void buffer_print_policy(){
	
}

#endif
