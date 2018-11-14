// Author: 黃柏瑄 Huang Po-Hsuan <aben20807@gmail.com>
// Filename: F74046284.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

// #define DEBUG
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
    int block_count;
    int block_num;
    Seq *block_seq;
    Block *blocks;
} Set;

typedef struct _Cache {
    int set_num;
    int block_num_per_set;
    Set *sets;
} Cache;

typedef int (*ReplFunc)(Set *set);

Cache *create_cache(int set_num, int block_num_per_set);
void destroy_cache(Cache **cache);
Seq *create_seq_node(int block_index);
Seq *create_seq(int num);
void destroy_seq(Seq **seq);
void move_to_mru(Seq *seq, int target_index);
Addr *get_addr(u64 real_addr, int set_num);
bool find_addr(Cache *cache, Addr *addr, ReplFunc repl);
int repl_lru(Set *set);
int repl_random(Set *set);
void load_from_mem(Cache *cache, Addr *addr, ReplFunc repl);

int main(int argc, char *argv[])
{
    if (argc != 5) {
        fprintf(stderr, "Need 4 arguments but get %d\n", argc - 1);
        exit(1);
    }

    srand(time(NULL));

    /* Get the values of arguments */
    int nk = atoi(argv[1]);
    int assoc = atoi(argv[2]);
    int blocksize = atoi(argv[3]);
    char repl = argv[4][0];
    debug("%d %d %d %c\n", nk, assoc, blocksize, repl);
    ReplFunc repl_func;
    if (repl == 'l') {
        repl_func = repl_lru;
    } else if (repl == 'r') {
        repl_func = repl_random;
    }

    int set_num = nk * K / assoc / blocksize;

    int r_miss_count = 0;
    int r_access_count = 0;
    int w_miss_count = 0;
    int w_access_count = 0;
    Cache *c = create_cache(set_num, assoc);

    char mode;
    u64 real_addr;
    char buffer[20];
    while (!feof(stdin)) {
        if (fgets(buffer, 20, stdin) != NULL) {
            mode = buffer[0];
            r_access_count += (mode == 'r') ? 1 : 0;
            w_access_count += (mode == 'w') ? 1 : 0;

            real_addr = strtoull(buffer + 2, NULL, 16);
            debug("%c %lld\n", mode, real_addr);
            Addr *addr = get_addr(real_addr, set_num);
            debug("set: %d, tag: %d\n", addr->index, addr->tag);
            bool is_hit = find_addr(c, addr, repl_func);
            debug("%s\n", is_hit ? "HIT" : "MISS");
            if (!is_hit) {
                r_miss_count += (mode == 'r') ? 1 : 0;
                w_miss_count += (mode == 'w') ? 1 : 0;
                load_from_mem(c, addr, repl_func);
            }
        } else {
            goto out;
        }
    }

out:;
    int total_miss_count = r_miss_count + w_miss_count;
    int total_access_count = r_access_count + w_access_count;
    printf("%d %lf%% %d %lf%% %d %lf%%\n",
            total_miss_count,
            (double) total_miss_count / total_access_count * 100,
            r_miss_count,
            (double) r_miss_count / r_access_count * 100,
            w_miss_count,
            (double) w_miss_count / w_access_count * 100
        );
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
        tmp_sets[i].block_count = 0;
        tmp_sets[i].block_num = block_num_per_set;
        tmp_sets[i].blocks = tmp_blocks;
        tmp_sets[i].block_seq = create_seq(block_num_per_set);
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
        destroy_seq(&((*cache)->sets[i].block_seq));
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
    mru->pre = itr;
    mru->nxt = lru;
    lru->pre = mru;
    return lru;
}

/*
 * Function: destroy_seq
 * ---------------------
 * Destroy the block sequence
 *
 * seq: the sequence that need to be destroied
 */
void destroy_seq(Seq **seq)
{
    Seq *itr = *seq;
    while(itr->block_index != -2) {
        Seq *nxt = itr->nxt;
        free(itr);
        itr = nxt;
    }
    free(itr);
    *seq = NULL;
}

/*
 * Function: move_to_mru
 * ---------------------
 * Move the specific seq node to to the last (mru)
 *
 * seq: the sequence that need to do move operation
 * target_index: the index need to move to the last,
 *               if the index is illegal seq will not change
 */
void move_to_mru(Seq *seq, int target_index)
{
    Seq *itr = seq->nxt;
    Seq *mru = seq->pre;
    while (itr->block_index != -2) {
        if (itr->block_index == target_index) {
            itr->nxt->pre = itr->pre;
            itr->pre->nxt = itr->nxt;
            mru->pre->nxt = itr;
            itr->pre = mru->pre;
            mru->pre = itr;
            itr->nxt = mru;
            return;
        }
        itr = itr->nxt;
    }
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
 * check addr if hit in cache, move hit block to mru when replacement policy
 * is lru
 *
 * cache: the cache
 * addr: the address for the cache
 *
 * returns: boolean, true if hit, false if miss
 */
bool find_addr(Cache *cache, Addr *addr, ReplFunc repl)
{
    int index = addr->index;
    Set *addr_set = &(cache->sets[index]);
    int addr_tag = addr->tag;
    Seq *seq = addr_set->block_seq;

    int n = cache->block_num_per_set;
    for (int i = 0; i < n; i++) {
        if (addr_set->blocks[i].valid == true &&
                addr_set->blocks[i].tag == addr_tag) {
            if (repl == repl_lru) {
                move_to_mru(seq, i);
            }
            return true;
        }
    }
    return false;
}

int repl_lru(Set *set)
{
    Seq *seq = set->block_seq;
    int lru_index = seq->nxt->block_index;
    move_to_mru(seq, lru_index);
    return lru_index;
}

int repl_random(Set *set)
{
    return rand() % (set->block_num);
}

/*
 * Function: load_from_mem
 * -----------------------
 * load block from memory to cache, if set is full then replace one block
 *
 * cache: the cache
 * addr: the address of block need to be loaded
 * repl: the replacement algorithm
 */
void load_from_mem(Cache *cache, Addr *addr, ReplFunc repl)
{
    int index = addr->index;
    Set *addr_set = &(cache->sets[index]);
    int addr_tag = addr->tag;

    int load_index = 0;
    if (addr_set->block_count < addr_set->block_num) {
        load_index = addr_set->block_seq->nxt->block_index;
        move_to_mru(addr_set->block_seq, load_index);
        addr_set->block_count++;
    } else {
        load_index = repl(addr_set);
    }
    addr_set->blocks[load_index].valid = true;
    addr_set->blocks[load_index].tag = addr_tag;
}
