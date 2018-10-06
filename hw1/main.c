#include <stdio.h>
#include <string.h>
#include "typedefs.h"

int h2d(char c);

int main(int argc, char *argv[])
{
    FILE *fin = fopen(argv[1], "r");
    FILE *fout = fopen(argv[2], "w");

    char line[20];
    inst_t inst = 0;
    while (!feof(fin)) {
        if (fgets(line, 20, fin) != NULL) {
            inst = ((h2d(line[0]) << 28) +
                    (h2d(line[1]) << 24) +
                    (h2d(line[3]) << 20) +
                    (h2d(line[4]) << 16) +
                    (h2d(line[6]) << 12) +
                    (h2d(line[7]) << 8 ) +
                    (h2d(line[9]) << 4 ) +
                    (h2d(line[10])));
            printf("%d\n", inst);
        }
        int op = inst >> 26;
        switch (op) {
            case OP_R: /* R-format */
                printf("R\n");
                break;
            case 1:
                break;
            case OP_J:
                printf("J\n");
                break;
        }
    }

    fclose(fout);
    fclose(fin);
    return 0;
}

int h2d(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return -1;
}
