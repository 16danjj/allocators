#include "stack.h"

#define DEFAULT_ALIGNMENT (2 * sizeof(void *))

bool is_power_of_2(uintptr_t x) {
    return (x & (x - 1)) == 0;
}

void stack_init(Stack *s, void *backing_buffer, size_t backing_buffer_length) {
    s->buf = (unsigned char *)backing_buffer;
    s->buf_len = backing_buffer_length;
    s->curr_offset = 0;
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

void *stack_alloc_align(Stack *s, size_t size, size_t alignment) { 
    uintptr_t curr_addr, new_addr;
    Header *header;
    assert(is_power_of_2(alignment));

    if (alignment > 128) {
        alignment = 128;
    }

    curr_addr = (uintptr_t)s->buf + (uintptr_t)s->curr_offset;
    size_t padding = calc_padding_with_header(curr_addr, alignment, sizeof(Header));
    
    if (s->curr_offset + padding + size > s->buf_len) { 
        return NULL;
    }

    s->curr_offset += padding;
    new_addr = curr_addr + padding;

    header = (Header *)(new_addr - sizeof(Header));
    header->padding = padding;

    s->curr_offset += size;
    return memset((void *)new_addr, 0, size);
}

void *stack_alloc(Stack *s, size_t size) {
    return stack_alloc_align(s, size, DEFAULT_ALIGNMENT);
}

void stack_free(Stack *s, void *ptr) {
    if (ptr != NULL) {

        uintptr_t start, end, curr_addr;
    
        start = (uintptr_t)s->buf;
        end = start + (uintptr_t)s->buf_len;
        curr_addr = (uintptr_t)ptr;
    
        if (curr_addr < start || curr_addr > end) {
            assert(0 && "Address to free is out of bounds");
            return;
        }
    
        if (curr_addr >= start + s->curr_offset) {
            assert(0 && "Avoid freeing out of order");
            return;
        }
    
        Header *header = (Header *)(curr_addr - sizeof(Header));
    
        s->curr_offset = curr_addr - header->padding - start;
    }
}

void *stack_resize_align(Stack *s, void *ptr, size_t old_size, size_t new_size, size_t alignment){
    if (ptr == NULL) {
        return stack_alloc(s, new_size);
    } else if (new_size == 0) {
        stack_free(s, ptr);
        return NULL;
    } else if (old_size == new_size) {
        return ptr;
    } else {
        uintptr_t start, end, curr_addr;

        start = (uintptr_t)s->buf;
        end = start + (uintptr_t)s->buf_len;
        curr_addr = (uintptr_t)ptr;

        size_t minimum_size = old_size < new_size ? old_size : new_size;

        if (curr_addr < start || curr_addr > end) {
            assert(0 && "Address out of bounds");
            return NULL;
        }

        if (curr_addr > start + s->curr_offset) {
            assert(0 && "Avoid freeing out of order");
            return NULL;
        }

        void *new_addr = stack_alloc_align(s, new_size, alignment);

        if (new_addr != NULL) {
            memmove(new_addr, ptr, minimum_size);
            return new_addr;
        } else {
            return NULL;
        }
    }
}

void *stack_resize(Stack *s, void *ptr, size_t old_size, size_t new_size) {
    return stack_resize_align(s, ptr, old_size, new_size, DEFAULT_ALIGNMENT);
}

void stack_free_all(Stack *s) {
    s->curr_offset = 0;
}

int main () {

    unsigned char backing_buffer[256];
    Stack s = {0};

    stack_init(&s, backing_buffer, 256);

    for (int i = 0; i<10; i++) {
        int *x;
		float *f;
		char *str;

		stack_free_all(&s);

		x = (int *)stack_alloc(&s, sizeof(int));
		f = (float *)stack_alloc(&s, sizeof(float));
		str = stack_alloc(&s, 10);

		*x = 123;
		*f = 987;
		memmove(str, "Hello", 5);

		printf("%p: %d\n", x, *x);
		printf("%p: %f\n", f, *f);
		printf("%p: %s\n", str, str);
    }

    stack_free_all(&s);

    return 0;
}