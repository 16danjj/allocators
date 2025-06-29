#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

typedef struct {
    unsigned char *base;
    size_t buf_len;
    size_t curr_offset;
} Arena; 