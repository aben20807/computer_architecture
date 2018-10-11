#include <stdio.h>
#include <string.h>
#include "typedefs.h"

#define WRITE_FILE 1

int h2d(char c);

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "%s%s",
                "Not enough arguments, please follow:\n",
                "`$ ./main <input> <output> `\n");
        goto exit_prog;
    }

    FILE *fin, *fout;
    if ((fin = fopen(argv[1], "r")) == NULL) {
        fprintf(stderr, "Error: Can't open `%s` for reading.\n", argv[1]);
        goto exit_prog;
    }
    if ((fout = fopen(argv[2], "w")) == NULL) {
        fprintf(stderr, "Error: Can't open `%s` for writing.\n", argv[2]);
        goto close_in;
    }

    char line[20];
    inst_t inst = 0;
    while (!feof(fin)) {
        if (fgets(line, 20, fin) != NULL) {
            inst = ((h2d(line[0]) << 28) + (h2d(line[1]) << 24) +
                    (h2d(line[3]) << 20) + (h2d(line[4]) << 16) +
                    (h2d(line[6]) << 12) + (h2d(line[7]) << 8 ) +
                    (h2d(line[9]) << 4 ) + (h2d(line[10])));
        } else {
            goto close_out;
        }
        op_t op = get_op(inst);
        funct_t funct;
        switch (op) {
        case OP_R:
            funct = get_funct(inst);
            switch (funct) {
            case F_SYSCALL:
            case F_BREAK:
                out("%s\n", funct_table[funct]);
                goto nxt;

            case F_JR:
                out("%s %s\n",
                        funct_table[funct],
                        reg_table[get_rs(inst)]);
                goto nxt;

            case F_JALR:
                out("%s %s, %s\n",
                        funct_table[funct],
                        reg_table[get_rd(inst)],
                        reg_table[get_rs(inst)]);
                goto nxt;

            case F_SLL:
            case F_SRL:
                if (inst == 0) {
                    out("nop\n");
                    goto nxt;
                }
                out("%s %s, %s, %d\n",
                        funct_table[funct],
                        reg_table[get_rd(inst)],
                        reg_table[get_rt(inst)],
                        get_shamt(inst));
                goto nxt;

            default:
                out("%s %s, %s, %s\n",
                        funct_table[funct],
                        reg_table[get_rd(inst)],
                        reg_table[get_rs(inst)],
                        reg_table[get_rt(inst)]);
                goto nxt;
            }
        case OP_BEQ:
        case OP_BNE:
            out("%s %s, %s, %d\n",
                    op_table[op],
                    reg_table[get_rs(inst)],
                    reg_table[get_rt(inst)],
                    get_imm(inst));
            break;

        case OP_BLTZ:
        case OP_BLEZ:
        case OP_BGTZ:
            out("%s %s, %d\n",
                    op_table[op],
                    reg_table[get_rs(inst)],
                    get_imm(inst));
            break;

        case OP_ADDI:
        case OP_ADDIU:
        case OP_SLTI:
        case OP_SLTIU:
        case OP_ANDI:
        case OP_ORI:
        case OP_XORI:
            out("%s %s, %s, %d\n",
                    op_table[op],
                    reg_table[get_rt(inst)],
                    reg_table[get_rs(inst)],
                    get_imm(inst));
            break;

        case OP_J:
        case OP_JAL:
            out("%s 0x%08x\n",
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
            out("%s %s, %d(%s)\n",
                    op_table[op],
                    reg_table[get_rt(inst)],
                    get_imm(inst),
                    reg_table[get_rs(inst)]);
            break;

        default:
            out("ERR\n");
        }
nxt:;
    }

close_out:
    fclose(fout);

close_in:
    fclose(fin);

exit_prog:
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
