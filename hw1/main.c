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
        }
        int op = get_op(inst);
        int funct, rd, rs, rt;
        switch (op) {
        case OP_R:
            funct = get_funct(inst);
            rd = get_rd(inst);
            rs = get_rs(inst);
            rt = get_rt(inst);
            if (inst == 0) {
                printf("noop\n");
                break;
            }
            printf("%s %s %s %s\n",
                    funct_table[funct],
                    reg_table[rd],
                    reg_table[rs],
                    reg_table[rt]);
            break;
        case OP_BEQ:
        case OP_BNE:
            rs = get_rs(inst);
            rt = get_rt(inst);
            printf("%s %s, %s, %d\n",
                    op_table[op],
                    reg_table[rs],
                    reg_table[rt],
                    get_imm(inst));
            break;
        case OP_BLTZ:
        case OP_BLEZ:
        case OP_BGTZ:
            rs = get_rs(inst);
            printf("%s %s, %d\n",
                    op_table[op],
                    reg_table[rs],
                    get_imm(inst));
            break;
        case OP_ADDI:
        case OP_ADDIU:
        case OP_SLTI:
        case OP_SLTIU:
        case OP_ANDI:
        case OP_ORI:
        case OP_XORI:
            rt = get_rt(inst);
            rs = get_rs(inst);
            printf("%s %s, %s, %d\n",
                    op_table[op],
                    reg_table[rt],
                    reg_table[rs],
                    get_imm(inst));
            break;
        case OP_J:
        case OP_JAL:
            printf("%s %d\n",
                    op_table[op],
                    get_addr(inst));
            break;
        case OP_LB:
        case OP_LH:
        case OP_LWI:
        case OP_LW:
        case OP_LBU:
        case OP_LHU:
        case OP_LWR:
        case OP_SB:
        case OP_SH:
        case OP_SWI:
        case OP_SW:
        case OP_SWR:
            printf("%s %d(%s)\n",
                    op_table[op],
                    get_imm(inst),
                    reg_table[rs]);
            break;
        default:
            printf("ERR\n");
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
