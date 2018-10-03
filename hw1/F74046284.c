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
    while (!feof(fin)) {
        if (fgets(line, 20, fin) != NULL) {
            for (int i = 0; i < 11; i++) {
                int ii = c2i(line[i]);
                if (ii != -1) {
                    printf("%d", ii);
                    printf("(%s)", table[ii]);
                }
            }
            printf("\n");
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
