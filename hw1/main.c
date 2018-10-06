#include <stdio.h>
#include <string.h>

typedef enum _OP {
    OP_R        = 0,
    OP_BLTZ     = 1,
    OP_J        = 2,
    OP_Jal      = 3,
    OP_BEQ      = 4,
    OP_BNE      = 5,
    OP_BLEZ     = 6,
    OP_BGTZ     = 7,
    OP_ADDI     = 8,
    OP_ADDIU    = 9,
    OP_SLTI     = 10,
    OP_SLTIU    = 11,
    OP_ANDI     = 12,
    OP_ORI      = 13,
    OP_XORI     = 14,
    OP_LUI      = 15,
    OP_TLB      = 16,
    OP_FIPT     = 17,
    OP_LB       = 32,
    OP_LH       = 33,
    OP_LWI      = 34,
    OP_LW       = 35,
    OP_LBU      = 36,
    OP_LHU      = 37,
    OP_LWR      = 38,
    OP_SB       = 40,
    OP_SH       = 41,
    OP_SWI      = 42,
    OP_SW       = 43,
    OP_SWR      = 46,
    OP_LL       = 48,
    OP_LWC1     = 49,
    OP_SC       = 56,
    OP_SWC1     = 67,
} op_t;

typedef enum _FUNCT {
    F_SLL       = 0,
    F_SRL       = 2,
    F_SRA       = 3,
    F_SLLV      = 4,
    F_SRLV      = 6,
    F_SRAV      = 7,
    F_JR        = 8,
    F_JALR      = 9,
    F_SYSCALL   = 12,
    F_BREAK     = 13,
    F_MFHI      = 16,
    F_MTHI      = 17,
    F_MFLO      = 18,
    F_MTLO      = 19,
    F_MULT      = 24,
    F_MULTU     = 25,
    F_DIV       = 26,
    F_DIVU      = 27,
    F_ADD       = 32,
    F_ADDU      = 33,
    F_SUB       = 34,
    F_SUBU      = 35,
    F_AND       = 36,
    F_OR        = 37,
    F_XOR       = 38,
    F_NOR       = 39,
    F_SLT       = 42,
    F_SLTU      = 43,
} funct_t;

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
    while (!feof(fin)) {
        if (fgets(line, 20, fin) != NULL) {
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
