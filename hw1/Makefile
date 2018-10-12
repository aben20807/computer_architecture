main: main.c typedefs.h
	gcc main.c -o main -Wall -std=c99

clean:
	rm -f main out/*.txt

test: main
	./main trace/trace1.txt out/trace1_ans.txt

test2: main
	./main trace/more.txt out/more_ans.txt
