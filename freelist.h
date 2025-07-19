#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <math.h>

struct Free_List_Node{
    struct Free_List_Node *next;
    size_t block_size;
};

typedef struct Free_List_Node Free_List_Node;

typedef struct {
    size_t block_size;
    size_t padding;
} Free_List_Allocation_Header;

typedef enum {
    Placement_Policy_Find_First,
    Placement_Policy_Find_Best
} Placement_Policy;

typedef struct {
    void *data;
    size_t size;
    size_t used;

    Placement_Policy policy;
    Free_List_Node *head;
} Free_List;

void free_list_node_remove(Free_List_Node **phead, Free_List_Node *prev_node, Free_List_Node *del_node);
void free_list_node_insert(Free_List_Node **phead, Free_List_Node *prev_node, Free_List_Node *new_node);