// Author: 黃柏瑄 Huang Po-Hsuan <aben20807@gmail.com>
// Filename: F74046284.c

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    if (argc != 5) {
        fprintf(stderr, "Need 4 arguments but get %d\n", argc - 1);
        exit(1);
    }
    int nk = atoi(argv[1]);
    int assoc = atoi(argv[2]);
    int blocksize = atoi(argv[3]);
    char repl = argv[4][0];
    printf("%d %d %d %c\n", nk, assoc, blocksize, repl);
    return 0;
}
