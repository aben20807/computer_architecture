main: main.c typedefs.h
	gcc main.c -o main -Wall -std=c99

clean:
	rm -f main out

test:
	./main trace/trace1.txt out.txt

test2:
	./main trace/more.txt trace/more_ans.txt
