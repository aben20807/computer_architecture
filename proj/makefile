CC = gcc
CFLAGS = -Wall
EXEC = cache
OBJS = F74046284.o

.PHONY: all clean test

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(EXEC) *.o sim.txt victim_bit.txt

test: $(EXEC)
	./cache testcase/6_bits/cache.txt testcase/6_bits/ReferenceList.txt

test2: $(EXEC)
	./cache testcase/6_bits/cache2.txt testcase/6_bits/ReferenceList2.txt
