WARNINGS_ARE_ERRORS := -Wall -Wextra -Werror
CC=gcc
C_FLAGS := -I. $(WARNINGS_ARE_ERRORS)

run: arena
	./arena
	
arena: arena.o
	$(CC) $(C_FLAGS) -o arena arena.o

arena.o: arena.c
	$(CC) $(C_FLAGS) -c arena.c

