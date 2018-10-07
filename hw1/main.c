#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "typedefs.h"

#define WRITE_FILE 1

int h2d(char c);

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "not enough argument\n");
        exit(0);
    }

    FILE *fin = fopen(argv[1], "r");
    FILE *fout = fopen(argv[2], "w");

    char line[20];
    inst_t inst = 0;
    while (!feof(fin)) {
        if (fgets(line, 20, fin) != NULL) {
            inst = ((h2d(line[0]) << 28) + (h2d(line[1]) << 24) +
                    (h2d(line[3]) << 20) + (h2d(line[4]) << 16) +
                    (h2d(line[6]) << 12) + (h2d(line[7]) << 8 ) +
                    (h2d(line[9]) << 4 ) + (h2d(line[10])));
        } else {
            break;
        }
        int op = get_op(inst);
        int funct, rd, rs, rt, shamt;
        switch (op) {
        case OP_R:
            funct = get_funct(inst);
            switch (funct) {
            case F_SYSCALL:
            case F_BREAK:
                out("%s\n", funct_table[funct]);
                goto nxt;

            case F_JR:
                rs = get_rs(inst);
                out("%s %s\n",
                        funct_table[funct],
                        reg_table[rs]);
                goto nxt;

            case F_JALR:
                rd = get_rd(inst);
                rs = get_rs(inst);
                out("%s %s, %s\n",
                        funct_table[funct],
                        reg_table[rd],
                        reg_table[rs]);
                goto nxt;

            case F_SLL:
            case F_SRL:
                if (inst == 0) {
                    out("nop\n");
                    goto nxt;
                }
                rd = get_rd(inst);
                rt = get_rt(inst);
                shamt = get_shamt(inst);
                out("%s %s, %s, %d\n",
                        funct_table[funct],
                        reg_table[rd],
                        reg_table[rt],
                        shamt);
                goto nxt;

            default:
                rd = get_rd(inst);
                rs = get_rs(inst);
                rt = get_rt(inst);
                out("%s %s, %s, %s\n",
                        funct_table[funct],
                        reg_table[rd],
                        reg_table[rs],
                        reg_table[rt]);
                goto nxt;
            }
        case OP_BEQ:
        case OP_BNE:
            rs = get_rs(inst);
            rt = get_rt(inst);
            out("%s %s, %s, %d\n",
                    op_table[op],
                    reg_table[rs],
                    reg_table[rt],
                    get_imm(inst));
            break;

        case OP_BLTZ:
        case OP_BLEZ:
        case OP_BGTZ:
            rs = get_rs(inst);
            out("%s %s, %d\n",
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
            out("%s %s, %s, %d\n",
                    op_table[op],
                    reg_table[rt],
                    reg_table[rs],
                    get_imm(inst));
            break;

        case OP_J:
        case OP_JAL:
            out("%s %d\n",
                    op_table[op],
                    get_addr(inst));
            break;

        case OP_LB:
        case OP_LH:
        case OP_LWL:
        case OP_LW:
        case OP_LBU:
        case OP_LHU:
        case OP_LWR:
        case OP_SB:
        case OP_SH:
        case OP_SWL:
        case OP_SW:
        case OP_SWR:
            rs = get_rs(inst);
            rt = get_rt(inst);
            out("%s %s, %d(%s)\n",
                    op_table[op],
                    reg_table[rt],
                    get_imm(inst),
                    reg_table[rs]);
            break;

        default:
            out("ERR\n");
        }
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
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    return -1;
}
