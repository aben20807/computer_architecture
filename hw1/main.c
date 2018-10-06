#include <stdio.h>
#include <string.h>
#include "typedefs.h"

static const char *table[] = {
    "0000", "0001", "0010", "0011",
    "0100", "0101", "0110", "0111",
    "1000", "1001", "1010", "1011",
    "1100", "1101", "1110", "1111",
};

int h2d(char c);
int b2d(char *s, int start, int end);

int main(int argc, char *argv[])
{
    FILE *fin = fopen(argv[1], "r");
    FILE *fout = fopen(argv[2], "w");

    char line[20];
    char bin[32];
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
            sprintf(bin, "%s%s%s%s%s%s%s%s",
                    table[h2d(line[0])],
                    table[h2d(line[1])],
                    table[h2d(line[3])],
                    table[h2d(line[4])],
                    table[h2d(line[6])],
                    table[h2d(line[7])],
                    table[h2d(line[9])],
                    table[h2d(line[10])]);
            printf("%s\n", bin);
        }
        if (strcmp(bin, "00000000000000000000000000000000") == 0) {
            printf("nop\n");
            goto nxt;
        }
        int op = b2d(bin, 0, 6);
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
        // printf("%d ", b2d(bin, 0, 6));
        // printf("%d ", b2d(bin, 6, 11));
        // printf("%d ", b2d(bin, 11, 16));
        // printf("%d ", b2d(bin, 16, 21));
        // printf("%d ", b2d(bin, 21, 26));
        // printf("%d\n", b2d(bin, 26, 32));
nxt:;
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

int b2d(char *s, int start, int end)
{
    int ret = 0;
    for (int i = end - 1, e = 1; i >= start; i--, e *= 2) {
        ret += ((s[i] - '0') * e);
    }
    return ret;
}
