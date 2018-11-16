# Computer Architecture Homework 3

## Part2: Cache Simulator

### Descirption

+ A cache which can simulate the lru and random replacement policy.

### Usage

1. use makefile

```bash
$ make test
```

2. the other method

+ arguments: `./cache nk assoc blocksize repl`
  + nk: the capacity of the cache in kilobytes (an int)
  + assoc: the associativity of the cache (an int)
  + blocksize: the size of a single cache block in bytes (an int)
  + repl: the replacement policy (a char); 'l' means LRU, 'r' means random.
+ command
```bash
$ make
$ gzip -dc trace/429.mcf-184B.trace.txt.gz | ./cache 2048 64 64 l
```
or
```bash
$ make
$ ./cache 2048 64 64 l < some_trace.txt
```

+ result

```bash
gzip -dc trace/429.mcf-184B.trace.txt.gz | ./cache 2048 64 64 l
55752 5.575200% 55703 5.610155% 49 0.689752%
gzip -dc trace/429.mcf-184B.trace.txt.gz | ./cache 2048 64 64 r
57986 5.798600% 57929 5.834347% 57 0.802365%
```
