CFLAGS=-O3 -g -Wall -Wextra -no-pie

.PHONY: all
all: mandelbrot
mandelbrot: mandelbrot.c mandelbrot.S
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: clean
clean:
	rm -f mandelbrot
