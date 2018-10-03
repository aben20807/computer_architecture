#include <stdio.h>
#include <string.h>
#include <ctype.h>

static const char *table[] = {
    "0000", "0001", "0010", "0011",
    "0100", "0101", "0110", "0111",
    "1000", "1001", "1010", "1011",
    "1100", "1101", "1110", "1111",
};

int c2i(char c);

int main(int argc, char *argv[])
{
    FILE *fin = fopen(argv[1], "r");
    FILE *fout = fopen(argv[2], "w");

    char line[20];
    char bin[32];
    while (!feof(fin)) {
        if (fgets(line, 20, fin) != NULL) {
            sprintf(bin, "%s%s%s%s%s%s%s%s",
                    table[c2i(line[0])],
                    table[c2i(line[1])],
                    table[c2i(line[3])],
                    table[c2i(line[4])],
                    table[c2i(line[6])],
                    table[c2i(line[7])],
                    table[c2i(line[9])],
                    table[c2i(line[10])]);
            printf("%s\n", bin);
        }
        if (strcmp(bin, "00000000000000000000000000000000") == 0) {
            printf("nop");
        }
    }

    fclose(fout);
    fclose(fin);
    return 0;
}

int c2i(char c) {
    if (isdigit(c))
        return c - '0';
    if (isalpha(c))
        return c - 'A' + 10;
    return -1;
}
