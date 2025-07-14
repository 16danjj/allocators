#include "pool.h"

#define CHUNK_ALIGNMENT 8

uintptr_t align_forward_uintptr(uintptr_t ptr, uintptr_t alignment) {
    uintptr_t p, a, modulo;

    p = ptr;
    a = alignment;

    modulo = p & (a - 1);

    if (modulo != 0) {
        p += a - modulo;
    }

    return p;
}

size_t align_forward_size(size_t chunk_size , size_t alignment) {
    size_t p, a, modulo;

    p = chunk_size;
    a = alignment;

    modulo = p & (a - 1);

    if (modulo != 0) {
        p += a - modulo;
    }

    return p;
}

void pool_free_all(Pool *p){
    size_t chunks = p->buf_len / p->chunk_size;

    for (size_t i = 0; i < chunks; i++) {
        void *ptr = &p->buf[i * p->chunk_size];
        Pool_Node *node = (Pool_Node *)ptr;
        node->next = p->head;
        p->head = node;
    }
}

void pool_init(Pool *p, void *backing_buffer, size_t backing_buffer_length, size_t chunk_size, size_t chunk_alignment) {
    uintptr_t initial_start = (uintptr_t)backing_buffer;

    // Align backing buffer to chunk alignment
    uintptr_t actual_start = align_forward_uintptr(initial_start, (uintptr_t)chunk_alignment);
    backing_buffer_length -= (actual_start - initial_start);

    // Align chunk size to chunk alignment
    chunk_size = align_forward_size(chunk_size, chunk_alignment);

    assert(chunk_size >= sizeof(Pool_Node) && "Chunk size should be greater than size of pool node");
    assert(backing_buffer_length >= chunk_size && "Buffer length should be greater than chunk size");

    p->buf = (void *)actual_start;
    p->buf_len = backing_buffer_length;
    p->chunk_size = chunk_size;
    p->head = NULL;

    pool_free_all(p);
}


void *pool_alloc(Pool *p) {

    Pool_Node *node = p->head;

    if (node == NULL) {
        assert(0 && "Pool allocator has no free memory");
        return NULL;
    }

    p->head = node->next;

    void *ptr = (void *)node;
    return memset(ptr, 0, p->chunk_size);
}

void pool_free(Pool *p, void *ptr) {

    void *start = &p->buf;
    void *end = &p->buf[p->buf_len];

    if (ptr == NULL) {
        return;
    }

    if (ptr < start || ptr > end) {
        assert(0 && "Out of bounds");
        return;
    }

    Pool_Node *node = (Pool_Node *)ptr;

    node->next = p->head;
    p->head = node;
}

int main(void) {

	unsigned char backing_buffer[1024];
	Pool p;
	void *a, *b, *c, *d, *e, *f;
    int *g;

	pool_init(&p, backing_buffer, 1024, 64, CHUNK_ALIGNMENT);

	a = pool_alloc(&p);
	b = pool_alloc(&p);
	c = pool_alloc(&p);
	d = pool_alloc(&p);
	e = pool_alloc(&p);
	f = pool_alloc(&p);

	pool_free(&p, f);
	pool_free(&p, c);
	pool_free(&p, b);
	pool_free(&p, d);

	d = pool_alloc(&p);

	pool_free(&p, a);

	a = pool_alloc(&p);
    
	pool_free(&p, e);
	pool_free(&p, a);
	pool_free(&p, d);

    g = (int *)pool_alloc(&p);
    *g = 16;
    printf("%p: %d\n", g, *g);

	return 0;
}