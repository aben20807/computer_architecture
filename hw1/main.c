#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "typedefs.h"

int h2d(char c);
op_t get_op(inst_t inst);
int get_rs(inst_t inst);
int get_rt(inst_t inst);
int get_rd(inst_t inst);
int get_shamt(inst_t inst);
int get_imm(inst_t inst);
int get_addr(inst_t inst);
funct_t get_funct(inst_t inst);

int main(int argc, char *argv[])
{
        // for (int i = 0; i < 32; i++){
        //     printf("%s\n", reg_table[i]);
        // }
        // for (int i = 0; i < 64; i++){
        //     if (strlen(op_table[i]) == 0) continue;
        //     printf("%s\n", op_table[i]);
        // }
        // for (int i = 0; i < 64; i++){
        //     if (strlen(funct_table[i]) == 0) continue;
        //     printf("%s\n", funct_table[i]);
        // }
    if (argc < 2)
        exit(0);
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
        // printf("%s\n", reg_table[get_rs(inst)]);
        int op = get_op(inst);
        int funct;
        switch (op) {
            case OP_R:
                funct = get_funct(inst);
                printf("R: %d\n", funct);
                printf("%s %s %s %s\n",
                        funct_table[get_funct(inst)],
                        reg_table[get_rd(inst)],
                        reg_table[get_rs(inst)],
                        reg_table[get_rt(inst)]);
                break;
            case 1:
                break;
            case OP_J:
                printf("J: %d\n", get_addr(inst));
                break;
        }
        printf("\n");
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

op_t get_op(inst_t inst)
{
    return inst >> 26;
}

int get_rs(inst_t inst)
{
    return (inst >> 21) & 31;
}

int get_rt(inst_t inst)
{
    return (inst >> 16) & 31;
}

int get_rd(inst_t inst)
{
    return (inst >> 11) & 31;
}

int get_shamt(inst_t inst)
{
    return (inst >> 6) & 31;
}

int get_imm(inst_t inst)
{
    return inst & 65535; /* 16-bit */
}

int get_addr(inst_t inst)
{
    return inst & 67108863; /* 26-bit */
}

funct_t get_funct(inst_t inst)
{
    return inst & 63;
}
