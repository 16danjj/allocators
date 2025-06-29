#include "arena.h"

#define DEFAULT_ALIGNMENT (2 * sizeof(void*))

bool is_power_of_two(uintptr_t x) {
    return (x & (x - 1)) == 0;
}

uintptr_t align_forward(uintptr_t ptr, size_t align) {
    uintptr_t p,a,m;
    assert(is_power_of_two(align));

    p = ptr;
    a = (uintptr_t)align;
    m = p & (a-1);

    if (m != 0) {
        p += a - m;
    }

    return p;
}

void *arena_alloc_align(Arena *a, size_t size, size_t align)  {
    uintptr_t curr_ptr = (uintptr_t)a->base + (uintptr_t)a->curr_offset;
    uintptr_t offset = align_forward(curr_ptr, align);

    offset -= (uintptr_t)a->base;

    if (offset + size <= a->buf_len) {
        void *ptr = &a->base[offset];
        a->curr_offset = offset + size;
        memset(ptr, 0, size);
        return ptr;
    } 

    return NULL;
}

void arena_init(Arena *a, void *backing_buffer, size_t backing_buffer_length) {
    a->base = backing_buffer;
    a->buf_len = backing_buffer_length;
    a->curr_offset = 0;
}

void *arena_alloc(Arena *a, size_t size) {
    return arena_alloc_align(a, size, DEFAULT_ALIGNMENT);
}

/*
void *arena_resize_align(Arena *a, void *old_memory, size_t old_size, size_t new_size, size_t align) {
    // TBD 
} */

void arena_free(Arena *a, void *ptr) {
    // do nothing
    (void)a;
    (void)ptr;
}

void arena_free_all(Arena *a) {
    a->curr_offset = 0;
}

int main () {

    unsigned char backing_buffer[256];
    Arena a = {0};
    arena_init(&a, backing_buffer, 256);

    for (int i = 0; i<10; i++) {
        int *x;
		float *f;
		char *str;

		// Reset all arena offsets for each loop
		arena_free_all(&a);

		x = (int *)arena_alloc(&a, sizeof(int));
		f = (float *)arena_alloc(&a, sizeof(float));
		str = arena_alloc(&a, 10);

		*x = 123;
		*f = 987;
		memmove(str, "Hellope", 7);

		printf("%p: %d\n", x, *x);
		printf("%p: %f\n", f, *f);
		printf("%p: %s\n", str, str);
    }

    arena_free_all(&a);
    return 0;
}
