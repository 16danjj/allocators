#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <math.h>

typedef struct Pool_Node Pool_Node;

typedef struct {
    unsigned char *buf;
    size_t chunk_size;
    size_t buf_len;
    Pool_Node *head;
} Pool;

struct Pool_Node{
    Pool_Node *next;
};