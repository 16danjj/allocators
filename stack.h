#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <math.h>

typedef struct {
    unsigned char *buf;
    size_t buf_len;
    size_t curr_offset;
    size_t prev_offset;
} Stack;

typedef struct {
    uint8_t padding;
    size_t prev_offset;
} Header;