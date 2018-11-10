CC = gcc
CFLAGS = -Wall -Werror
EXEC = cache
OBJS = F74046284.o

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(EXEC) *.o

test1: $(EXEC)
	./cache 2048 64 64 l
