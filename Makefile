CC = gcc
CFLAGS = -Wall -Wextra -g -fsanitize=address

test: test_basic_ops
	./test_basic_ops

test_basic_ops: test_basic_ops.c vm.c basic_ops.c compile.h
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f test_basic_ops

.PHONY: test clean
