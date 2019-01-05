# Computer Architecture Project

### Descirption

+ Design a LRU cache with a FIFO victim cache.

### Usage

1. use makefile

```bash
$ make test
```

2. the other method

+ arguments: `./cache <Cache.txt> <ReferenceList.txt>`
+ command
```bash
$ make
$ ./cache testcase/6_bits/cache.txt testcase/6_bits/ReferenceList.txt
```

+ result

`victim_bit.txt`:
```
# Student ID: F74046284
# Cache Organization
# Addressing Bus : 6 bits (a31 a30 ... a1 a0 )
M 6
# Main Cache Size: 16 bytes
MCS 16
# Entry Size: 4 bytes
ES 4
# 1-way associative
A 1
# index bit
IB 1 0
# victim-cache size: 8 bytes
VCS 8
# Time Unit
TU 5
# Maximum Change per Time Unit
MCTU 2
# Alpha Beta Gmama
ABG 1 3 15
# Configuration of Victim-Bits: total change count
CVB 4
# Cycle Time Configuration of Victim-Bits
1 1111
4 1111
7 1111
10 1111
```

`sim.txt`:
```
# Student ID: F74046284
# total Main-Cache miss
MCM 7
# total Victim-Cache miss
VCM 5
# Total Cycle Time
TCT 84
# CPU Time(ms)
T 0
.benchmark testcase1
#input_address	main_cache status	victim_cache status
000100	m	m
000100	h	n
001000	m	m
000100	m	h
001011	m	m
000110	m	m
001011	h	n
001110	m	m
000110	m	h
000110	h	n
.end
```
