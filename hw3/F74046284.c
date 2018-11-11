// Author: 黃柏瑄 Huang Po-Hsuan <aben20807@gmail.com>
// Filename: F74046284.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define DEBUG
#ifdef DEBUG
#define debug(...) do { printf(__VA_ARGS__); } while(0)
#else
#define debug(...)
#endif

#define K 1024

typedef unsigned long long u64;
typedef struct _Addr {
    int tag;
    int index;
    // int offset;
} Addr;

typedef struct _Block {
    bool valid;
    int tag;
    // Data *data;
} Block;

typedef struct _Seq Seq;
struct _Seq {   /* the sequence of blocks be used */
    int block_index;
    Seq *pre;
    Seq *nxt;
};

typedef struct _Set {
    Seq *lru;
    Seq *mru;
    Block *blocks;
} Set;

typedef struct _Cache {
    int set_num;
    int block_num_per_set;
    Set *sets;
} Cache;

typedef void (*ReplFunc)(Set *set, Addr *addr);

Cache *create_cache(int set_num, int block_num_per_set);
void destroy_cache(Cache **cache);
Seq *create_seq_node(int block_index);
Seq *create_seq(int num);
Addr *get_addr(u64 real_addr, int set_num);
bool find_addr(Cache *cache, Addr *addr);
void load_from_mem(Cache *cache, Addr *addr, ReplFunc repl);

int main(int argc, char *argv[])
{
    if (argc != 5) {
        fprintf(stderr, "Need 4 arguments but get %d\n", argc - 1);
        exit(1);
    }
    /* Get the values of arguments */
    int nk = atoi(argv[1]);
    int assoc = atoi(argv[2]);
    int blocksize = atoi(argv[3]);
    char repl = argv[4][0];
    debug("%d %d %d %c\n", nk, assoc, blocksize, repl);

    int set_num = nk * K / assoc / blocksize;

    Cache *c = create_cache(set_num, assoc);
    // debug("%d\n", c->sets[0].blocks[0].valid);
    // Seq *itr = c->sets[0].lru;
    // while(itr->block_index != -2) {
    //     debug("%d\n", itr->block_index);
    //     itr = itr->nxt;
    // }

    // char mode;
    u64 real_addr;
    char buffer[20];
    while (!feof(stdin)) {
        if (fgets(buffer, 20, stdin) != NULL) {
            // mode = buffer[0];
            real_addr = strtoull(buffer + 2, NULL, 16);
            // debug("%c %lld\n", mode, real_addr);
            Addr *addr = get_addr(real_addr, set_num);
            debug("set: %d, tag: %d\n", addr->index, addr->tag);
            bool is_hit = find_addr(c, addr);
            debug("%s\n", is_hit ? "HIT" : "MISS");
        } else {
            goto out;
        }
    }

out:
    destroy_cache(&c);
    return 0;
}

/*
 * Function: create_cache
 * ----------------------
 * Create a cache with "set_num" sets which has "block_num_per_set" blocks
 *
 * set_num: number of sets in the cache
 * block_number_per_set: number of blocks in each set
 *
 * returns: cache which is initialized valid and tag to 0
 */
Cache *create_cache(int set_num, int block_num_per_set)
{
    Cache *ret = malloc(sizeof(Cache));
    Set *tmp_sets = malloc(sizeof(Set) * set_num);
    for (int i = 0; i < set_num; i++) {
        Block *tmp_blocks = malloc(sizeof(Block) * block_num_per_set);
        for (int j = 0; j < block_num_per_set; j++) {
            tmp_blocks[j].valid = false;
            tmp_blocks[j].tag = 0;
        }
        tmp_sets[i].blocks = tmp_blocks;
        tmp_sets[i].lru = create_seq(block_num_per_set);
        tmp_sets[i].mru = (tmp_sets[i].lru)->pre;
    }
    ret->sets = tmp_sets;
    ret->set_num = set_num;
    ret->block_num_per_set = block_num_per_set;
    return ret;
}

/*
 * Function: destroy_cache
 * -----------------------
 * free all elements allocated memory
 *
 * cache: pointer to pointer to Cache (in order to assign NULL to the variable)
 */
void destroy_cache(Cache **cache)
{
    int set_num = (*cache)->set_num;
    for (int i = 0; i < set_num; i++) {
        free((*cache)->sets[i].blocks);
    }
    free((*cache)->sets);
    free(*cache);
    *cache = NULL;
}

/*
 * Function: create_seq_node
 * -------------------------
 * Create and return a node of sequence with setting block index
 *
 * block_index: the index used to point to specific block quickly
 *
 * returns: the seq node whit being initialized
 */
Seq *create_seq_node(int block_index)
{
    Seq *ret = malloc(sizeof(Seq));
    ret->block_index = block_index;
    ret->pre = NULL;
    ret->nxt = NULL;
    return ret;
}

/*
 * Function: create_seq
 * --------------------
 * Return a sequence (circular doubly linked list)
 *      ->[lru]<->[0]<->[1]<->[2]<->....<->[n]<->[mru]<-
 *
 * returns: the sequence
 */
Seq *create_seq(int num)
{
    Seq *lru = create_seq_node(-1);
    Seq *mru = create_seq_node(-2);
    Seq *itr = lru;
    for (int i = 0; i < num; i++) {
        Seq *tmp = create_seq_node(i);
        itr->nxt = tmp;
        tmp->pre = itr;
        itr = tmp;
    }
    itr->nxt = mru;
    mru->nxt = lru;
    lru->pre = mru;
    return lru;
}

/*
 * Function: get_addr
 * ------------------
 * Create and return the cache address by real address and the number of set
 *
 * real_addr: the real address
 * set_num: the number of sets
 *
 * returns: the Addr struct contains tag and index
 */
Addr *get_addr(u64 real_addr, int set_num)
{
    Addr *ret = malloc(sizeof(Addr));
    ret->tag = real_addr / set_num;
    ret->index = real_addr % set_num;
    return ret;
}

/*
 * Function: find_addr
 * -------------------
 * check addr if hit in cache
 *
 * cache: the cache
 * addr: the address for the cache
 *
 * returns: boolean, true if hit, false if miss
 */
bool find_addr(Cache *cache, Addr *addr)
{
    int index = addr->index;
    Set addr_set = cache->sets[index];
    int addr_tag = addr->tag;

    int n = cache->block_num_per_set;
    for (int i = 0; i < n; i++) {
        if (addr_set.blocks[i].valid == true &&
                addr_set.blocks[i].tag == addr_tag) {
            return true;
        }
    }
    return false;
}

void load_from_mem(Cache *cache, Addr *addr, ReplFunc repl)
{
}
