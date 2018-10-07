#ifndef _TYPEDEFS_H_
#define _TYPEDEFS_H_

#define $V "$v0", "$v1"
#define $A "$a0", "$a1", "$a2", "$a3"
#define $T "$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7"
#define $S "$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7"
#define $Y "$t8", "$t9"
#define $K "$k0", "$k1"
#define REG "$zero", "$at", $V, $A, $T, $S, $Y, $K, "$gp", "$sp", "$fp", "$ra"
const char *reg_table[32] = { REG };

/*           000     001      010     011      100     101    110      111 */
#define o000 "r"   , "bltz" , "j"   , "jal"  , "beq" , "bne", "blez", "bgtz"
#define o001 "addi", "addiu", "slti", "sltiu", "andi", "ori", "xori", "lui"
#define o010 "tlb" , "fipt" , ""    , ""     , ""    , ""   , ""    , ""
#define o011 ""    , ""     , ""    , ""     , ""    , ""   , ""    , ""
#define o100 "lb"  , "lh"   , "lwi" , "lw"   , "lbu" , "lhu", "lwr" , ""
#define o101 "sb"  , "sh"   , "swi" , "sw"   , ""    , ""   , "swr" , ""
#define o110 "ll"  , "lwc1" , ""    , ""     , ""    , ""   , ""    , ""
#define o111 "sc"  , "swc1" , ""    , ""     , ""    , ""   , ""    , ""
const char *op_table[64] = { o000, o001, o010, o011, o100, o101, o110, o111 };

/*           000     001      010     011      100        101      110     111 */
#define f000 "sll" , ""     , "srl" , "sra"  , "sllv"   , ""     , "blez", "bgtz"
#define f001 "jr"  , "jalr" , ""    , ""     , "syscall", "break", ""    , ""
#define f010 "mfhi", "mthi" , "mflo", "mtlo" , ""       , ""     , ""    , ""
#define f011 "mult", "multu", "div" , "divu" , ""       , ""     , ""    , ""
#define f100 "add" , "addu" , "sub" , "subu" , "and"    , "or"   , "xor" , "nor"
#define f101 ""    , ""     , "slt" , "sltu" , ""       , ""     , "swr" , ""
#define f110 ""    , ""     , ""    , ""     , ""       , ""     , ""    , ""
#define f111 ""    , ""     , ""    , ""     , ""       , ""     , ""    , ""
const char *funct_table[64] = { f000, f001, f010, f011, f100, f101, f110, f111 };

#define get_op(x) x >> 26
#define get_rs(x) (x >> 21) & 31
#define get_rt(x) (x >> 16) & 31
#define get_rd(x) (x >> 11) & 31
#define get_shamt(x) (x >> 6) & 31
#define get_imm(x) x & 65535
#define get_addr(x) x & 67108863
#define get_funct(x) x & 63
#define out(...) \
        do { \
            if (WRITE_FILE) \
                fprintf(fout, __VA_ARGS__); \
            else \
                printf(__VA_ARGS__); \
        } while (0)

typedef unsigned int inst_t;

typedef enum _OP {
    OP_R        = 0,
    OP_BLTZ     = 1,
    OP_J        = 2,
    OP_JAL      = 3,
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

#endif /*  _TYPEDEFS_H_ */
