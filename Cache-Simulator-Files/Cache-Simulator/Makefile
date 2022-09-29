################################################################################
#              Makefile for the CS 3410 Cache Project 5                        #
################################################################################

# Additional flags for the compiler
# always enable debugging because its more convenient
CFLAGS := -std=c99 -D_GNU_SOURCE -Wall -g3
LFLAGS := -lm

.PHONY: all clean run

all: clean p5

p5: cache.o cache_stats.o simulator.o print_helpers.o
	gcc $(CFLAGS) -o $@ $@.c $^ $(LFLAGS)

# Wildcard rule that allows for the compilation of a *.c file to a *.o file
%.o : %.c
	gcc -c $(CFLAGS) $< -o $@

# Removes any executables and compiled object files
clean:
	rm -f p5 *.o

test1: all
	./p5 -t trace.1t.short.txt -cache 9 5 1 > actual/actual1.txt
	-diff actual/actual1.txt expect/expect1.txt -b

test2: all
	./p5 -t trace.1t.short.txt -cache 12 6 2 > actual/actual2.txt
	-diff actual/actual2.txt expect/expect2.txt -b

test3: all
	./p5 -t trace.1t.short.txt -cache 16 4 2 > actual/actual3.txt
	-diff actual/actual3.txt expect/expect3.txt -b

multi1: all
	./p5 -n 2 -t trace.2t.short.txt -cache 9 5 1 -limit 1542 > actual/actualm1.txt
	clion diff actual/actualm1.txt expect/expectm1.txt

multi2: all
	./p5 -n 2 -t trace.2t.short.txt -cache 12 6 2 -limit 1542 > actual/actualm2.txt
	clion diff actual/actualm2.txt expect/expectm2.txt

multi3: all
	./p5 -n 2 -t trace.2t.short.txt -cache 16 4 2 -limit 1542 > actual/actualm3.txt
	clion diff actual/actualm3.txt expect/expectm3.txt

multi_vi: all
	./p5 -t trace.2t.tiny.txt -cache 16 4 2 -v -n 2 -p vi > actual/actualm_vi.txt
	clion diff actual/actualm_vi.txt expect/expectm_vi.txt

multi_msi: all
	./p5 -t trace.2t.tiny.txt -cache 16 4 2 -v -n 2 -p msi > actual/actualm_msi.txt
	clion diff actual/actualm_msi.txt expect/expectm_msi.txt

test_singles: test1 test2 test3
