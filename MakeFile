CC      = gcc
CFLAGS  = -Wall -O2 -std=c99
LDFLAGS = -lm

SRC = src/heavy_guardian.c \
      src/count_min_sketch.c \
      src/elastic_sketch.c \
      src/traffic_gen.c

all: main benchmark

main: $(SRC) src/main.c
	$(CC) $(CFLAGS) -o elastic_sketch $(SRC) src/main.c $(LDFLAGS)

benchmark: $(SRC) tests/benchmark.c
	$(CC) $(CFLAGS) -o benchmark $(SRC) tests/benchmark.c $(LDFLAGS)

clean:
	rm -f elastic_sketch benchmark

run: main
	./elastic_sketch

test: benchmark
	./benchmark