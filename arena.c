#include "arena.h"

// Some Macros 
#define DEFAULT_ALIGNMENT (2 * (sizeof void*))

uintptr_t align_forward(uintptr_t ptr, size_t align) {}

void *arena_alloc_align(Arena *a, size_t size, size_t align)  {}

void arena_init(Arena *a, void *backing_buffer, size_t backing_buffer_length) {}

void *arena_resize_align(Arena *a, void *old_memory, size_t old_size, size_t new_size, size_t align) {}

void arena_free(Arena *a, void *ptr) {}

void arena_free_all(Arena *a) {}

int main () {

    return 0;
}
