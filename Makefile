CC      = clang
CFLAGS  = -Wall -Wpedantic -Wextra -Werror
LFLAGS  = -lm

.PHONY: all clean format

all: encode decode entropy

encode: encode.c node.c io.c pq.c code.c huffman.c stack.c
	$(CC) encode.c node.c io.c pq.c code.c huffman.c stack.c $(CFLAGS) -o encode

decode: decode.c io.c code.c huffman.c stack.c pq.c node.c
	$(CC) decode.c io.c code.c huffman.c stack.c pq.c node.c  $(CFLAGS) -o decode

entropy: entropy.c
	$(CC) entropy.c $(CFLAGS) $(LFLAGS) -o entropy

format:
	clang-format -i -style=file *.[ch]

clean:
	rm -rf encode decode entropy

scan-build: clean
	scan-build make
