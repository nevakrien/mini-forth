CC = gcc
CFLAGS = -Wall -Wextra -g

test: test_basic_ops
	./test_basic_ops

test_basic_ops: test_basic_ops.c vm.h compile.h basic_ops.h utils.h mem_alloc.h
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f test_basic_ops

.PHONY: test clean
