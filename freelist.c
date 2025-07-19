#include "freelist.h"

bool is_power_of_2(uintptr_t x) {
    return (x & (x - 1)) == 0;
}

void free_list_free_all(Free_List *fl) {
    fl->used = 0;
    Free_List_Node *node = (Free_List_Node *)fl->data;
    node->block_size = fl->size;
    node->next = NULL;
    fl->head = node;
}

void free_list_init(Free_List *fl, void *data, size_t size) {
    fl->data = data;
    fl->size = size;
    free_list_free_all(fl);
}

size_t calc_padding_with_header(uintptr_t ptr, uintptr_t alignment, size_t header_size) {
    uintptr_t p, a, modulo, padding, needed_header_space;

    assert(is_power_of_2(alignment)); 
    p = ptr;
    a = alignment; 
    padding = 0; 
    needed_header_space = (size_t)header_size;

    modulo = p & (a - 1); 

    if (modulo != 0) {
        padding = a - modulo; 
    }

    if (padding < needed_header_space) {

        needed_header_space -= padding;
    
        if ((needed_header_space & (a - 1)) != 0 ) {
            p += ceil(needed_header_space / a) * a;
        } else {
            p += needed_header_space;
        }
    }

    return (size_t)padding;
}

Free_List_Node *free_list_find_first(Free_List *fl, size_t size, size_t alignment, size_t *padding_, Free_List_Node **prev_node_) {
    Free_List_Node *node = fl->head;
    Free_List_Node *prev_node = NULL;

    size_t padding = 0;

    while (node != NULL) {
        padding = calc_padding_with_header((uintptr_t)node, alignment, sizeof(Free_List_Allocation_Header));
        size_t required_size = size + padding;

        if (node->block_size >= required_size) {
            break;
        }

        prev_node = node;
        node = node->next;
    }
    
    *prev_node_ = prev_node;
    *padding_ = padding;
    return node;
}

Free_List_Node *free_list_find_best(Free_List *fl, size_t size, size_t alignment, size_t *padding_, Free_List_Node **prev_node_) {
    Free_List_Node *node = fl->head;
    Free_List_Node *prev_node = NULL;
    Free_List_Node *best_node = NULL; 

    size_t padding = 0;

    while (node != NULL) {
        padding = calc_padding_with_header((uintptr_t)node, alignment, sizeof(Free_List_Allocation_Header));
        size_t required_size = size + padding;
        size_t smallest_diff = ~0;

        if (node->block_size >= required_size && (node->block_size - required_size < smallest_diff)) {
            smallest_diff = node->block_size - required_size;
            best_node = node;
        }

        prev_node = node;
        node = node->next;
    }

    *padding_ = padding;
    *prev_node_ = prev_node;
    return best_node;
}

void *free_list_alloc(Free_List *fl, size_t size, size_t alignment) { 
    Free_List_Node *prev_node = NULL;
    Free_List_Node *node = NULL;
    size_t padding = 0;
    Free_List_Allocation_Header *header;

    if (size < sizeof(Free_List_Node)) {
        size = sizeof(Free_List_Node);
    }
    if (alignment < 8) {
        alignment = 8;
    }

    if (fl->policy == Placement_Policy_Find_First) {
        node = free_list_find_best(fl, size, alignment, &padding, &prev_node);
    } else {
        node = free_list_find_first(fl, size, alignment, &padding, &prev_node);
    }

    if (node == NULL) {
        assert(0 && "Free list has no free memory");
        return NULL;
    }

    size_t used_size = padding + size;
    size_t remaining_size = node->block_size - used_size;
    fl->used += used_size;
    
    if (remaining_size > 0) {
        Free_List_Node *new_node = (Free_List_Node *)(node + used_size);
        new_node->block_size = remaining_size;
        free_list_node_insert(&fl->head, node, new_node);
    }

    node->block_size -= used_size;
    free_list_node_remove(&fl->head, prev_node, node);

    header = (Free_List_Allocation_Header *)((void *)node + padding - sizeof(Free_List_Allocation_Header));
    header->padding = padding;
    header->block_size = size;

    return (void *)(node + padding);
}

void free_list_coalescence(Free_List *fl, Free_List_Node *prev_node, Free_List_Node *free_node) {
    if (free_node->next != NULL && (free_node + free_node->block_size) == free_node->next) {
        free_node->block_size += free_node->next->block_size;
        free_list_node_remove(&fl->head, free_node, free_node->next);
    }

    if (prev_node->next != NULL && (prev_node + prev_node->block_size) == free_node) {
        prev_node->block_size += prev_node->next->block_size;
        free_list_node_remove(&fl->head, prev_node, free_node);
    }
}

void free_list_free(Free_List *fl, void *ptr) {
    Free_List_Node *curr_node = fl->head;
    Free_List_Node *prev_node = fl->head;
    Free_List_Allocation_Header *header;

    if (ptr == NULL) {
        return;
    }

    Free_List_Node *freed_node = (Free_List_Node *)ptr;
    header = (Free_List_Allocation_Header *)(freed_node - sizeof(Free_List_Allocation_Header));
    size_t used_size = header->padding + header->block_size;

    while (curr_node != NULL) {
        if (freed_node < curr_node) {
            free_list_node_insert(&fl->head, prev_node, freed_node);
            break;
        }
        prev_node = curr_node;
        curr_node = curr_node->next;
    }
    
    fl->used -= used_size;
    free_list_coalescence(fl, prev_node, freed_node);
}

void free_list_node_insert(Free_List_Node **phead, Free_List_Node *prev_node, Free_List_Node *new_node) {
    if (prev_node == NULL) {
        if (*phead == NULL) {
            *phead = new_node;
            new_node->next = NULL;
        } else {
            new_node->next = *phead;
            *phead = new_node;
        }
    } else {
        if (prev_node->next != NULL) {
            new_node->next = prev_node->next;
            prev_node->next = new_node;
        } else {
            prev_node->next = new_node;
            new_node->next = NULL;
        }
    }
}

void free_list_node_remove(Free_List_Node **phead, Free_List_Node *prev_node, Free_List_Node *del_node) {
    if (prev_node == NULL) {
        *phead = del_node->next;
    } else {
        prev_node->next = del_node->next;
    }
}

int main() {
    
    void *data = malloc(10 * sizeof(Free_List_Node));
    Free_List fl;
    fl.policy = Placement_Policy_Find_First;
    free_list_init(&fl, data, 10 * sizeof(Free_List_Node));

    int *x = (int *)free_list_alloc(&fl, sizeof(int), 8);
    *x = 16;

    printf("%p: %d\n", x, *x);

    free_list_free_all(&fl); 

    return 0;
}