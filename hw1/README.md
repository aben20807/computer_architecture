# Computer Architecture Homework 1

## Part 2: Programming for Machine Language Understanding

### Descirption

Disassemble hexadecimal numbers into MIPS instructions in input file.
Each line in input file are 8 hex number like `03 E0 00 08`, note that
there are serveral spaces between every two bytes.

### Usage

1. Compile src and get executable.
```bash
$ make      # gcc main.c -o main -Wall -std=c99
```

2. Test trace1.txt as input.
```bash
$ make test # ./main trace/trace1.txt out/trace1_ans.txt
```
