CC = gcc
CFLAGS = -Wall -Wextra -g3  -Iinclude -I. -fsanitize=address

SRC_DIR = src
BIN_DIR = bin
TEST_DIR = tests

HEADERS = $(SRC_DIR)/vm.c $(SRC_DIR)/basic_ops.c \
          include/vm.h include/basic_ops.h include/compile.h include/utils.h

TEST_BINS = $(BIN_DIR)/test_basic_ops $(BIN_DIR)/test_lex
REPL = $(BIN_DIR)/repl

all: $(TEST_BINS) $(REPL)

test: $(TEST_BINS)
	@echo "Running test_basic_ops..."
	./$(BIN_DIR)/test_basic_ops
	@echo ""
	@echo "Running test_lex..."
	./$(BIN_DIR)/test_lex

$(BIN_DIR)/test_basic_ops: $(TEST_DIR)/test_basic_ops.c $(SRC_DIR)/vm.c $(SRC_DIR)/basic_ops.c $(HEADERS)
	$(CC) $(CFLAGS) -o $@ $^

$(BIN_DIR)/test_lex: $(TEST_DIR)/test_lex.c $(SRC_DIR)/vm.c $(SRC_DIR)/basic_ops.c $(HEADERS)
	$(CC) $(CFLAGS) -o $@ $^

$(BIN_DIR)/repl: $(SRC_DIR)/main.c $(SRC_DIR)/vm.c $(SRC_DIR)/basic_ops.c $(HEADERS)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(BIN_DIR)/*

.PHONY: test clean all
