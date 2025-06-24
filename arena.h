#include <stdlib.h>
#include <stdio.h>


typedef struct {
    void *base;
    size_t buf_len;
    size_t curr_offset;
} Arena; 