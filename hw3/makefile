CC = gcc
CFLAGS = -Wall -Werror -std=c99
EXEC = cache
OBJS = F74046284.o

.PHONY: all clean test

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(EXEC) *.o

test: $(EXEC)
	./cache 2048 64 64 l < trace/t.txt
