// Author: 黃柏瑄 Huang Po-Hsuan <aben20807@gmail.com>
// Filename: F74046284.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#define DEBUG
#ifdef DEBUG
#define debug(...) do { printf(__VA_ARGS__); } while(0)
#else
#define debug(...)
#endif

// #define K 1024

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
    Set *sets;
} Cache;

typedef struct _CacheConfig {
    int m;          /* bits of addressing bus */
    int mcs;        /* main cache size */
    int es;         /* entry(block) size*/
    int a;          /* n-way associative */
    int ib[128];    /* index bit */
    int ib_cnt;     /* count of index bit*/
    int vcs;        /* victim-cache size */
    int tu;         /* time unit */
    int mctu;       /* maximum changes per time unit */
    int abg[3];     /* alpha beta gamma */
} CacheConfig;

typedef int (*ReplFunc)(Set *set);

CacheConfig read_cache_config(const char *filename);
void print_cacheconfig(CacheConfig cc);
Cache *create_cache(int set_num, int block_num_per_set);
void destroy_cache(Cache **cache);
Seq *create_seq_node(int block_index);
Seq *create_seq(int num);
void destroy_seq(Seq **seq);
void move_to_mru(Seq *seq, int target_index);
Addr *get_addr(u64 real_addr, int set_num, int block_size);
bool find_addr(Cache *cache, Addr *addr, ReplFunc repl);
int repl_lru(Set *set);
int repl_random(Set *set);
void load_from_mem(Cache *cache, Addr *addr, ReplFunc repl);

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Need 2 arguments but get %d.\nFormat:\n", argc - 1);
        fprintf(stderr, "./cache <Cache.txt> <ReferenceList.txt>\n");
        exit(1);
    }

    CacheConfig cc = read_cache_config(argv[1]);
    // print_cacheconfig(cc);

    /* Get the values of arguments */
    int nk = cc.mcs;
    int assoc = cc.a;
    int blocksize = cc.es;
    char repl = 'l';
    ReplFunc repl_func = (repl == 'l') ? repl_lru : repl_random;
    int set_num = nk / assoc / blocksize;
    debug("%d %d %d %d %c\n", nk, assoc, blocksize, set_num, repl);

    int m_miss_count = 0;   /* total Main-Cache miss */
    int m_access_count = 0;
    Cache *c = create_cache(set_num, assoc);

    u64 real_addr;
    char buffer[40];

    /* Open ReferenceList.txt and simulate */
    FILE *fin = fopen(argv[2], "r");
    while (!feof(fin)) {
        if (fgets(buffer, 40, fin) != NULL) {
            if (buffer[0] == '#') { // comment
                continue;
            }

            if (buffer[0] == '.') { // start and end
                continue;
            }

            real_addr = strtoull(buffer, NULL, 2);
            debug("%lld\n", real_addr);
            m_access_count++;

            Addr *addr = get_addr(real_addr, set_num, blocksize);
            debug("set: %d, tag: %d\n", addr->index, addr->tag);

            bool is_hit = find_addr(c, addr, repl_func);
            debug("%s\n", is_hit ? "HIT" : "MISS");
            if (!is_hit) {
                m_miss_count++;
                load_from_mem(c, addr, repl_func);
            }
        } else {
            goto out;
        }
    }

    out:;
    printf("%d %lf%%\n",
            m_miss_count,
            (double) m_miss_count / m_access_count * 100
        );
    destroy_cache(&c);
    return 0;
}

CacheConfig read_cache_config(const char *filename)
{
    CacheConfig ret = {};
    FILE *in = fopen(filename, "r");
    if (!in) {
        fprintf(stderr, "Fail to open Cache.txt at %s\n", filename);
        goto read_fail;
    }
    FILE *out = fopen("victim_bit.txt", "w");
    if (!out) {
        fprintf(stderr, "Fail to open victim_bit.txt\n");
        goto write_fail;
    }
    fprintf(out, "# Student ID: P74046284\n");
    fprintf(out, "# Cache Organization\n");

    /* Parse config file */
    char buffer[80];
    while (!feof(in)) {
        if (fgets(buffer, 80, in) != NULL) {
            fprintf(out, "%s", buffer); // write victim_bit.txt

            if (buffer[0] == '#') { // comment
                continue;
            }
            /* Split first first token */
            char *ptr = strtok(buffer, " \n\t\r");
            ptr = strtok(NULL, " \n\t\r");

            if (!strcmp(buffer, "M")) {
                ret.m = (int) strtol(ptr, NULL, 10);
            } else if (!strcmp(buffer, "MCS")) {
                ret.mcs = (int) strtol(ptr, NULL, 10);
            } else if (!strcmp(buffer, "ES")) {
                ret.es = (int) strtol(ptr, NULL, 10);
            } else if (!strcmp(buffer, "A")) {
                ret.a = (int) strtol(ptr, NULL, 10);
            } else if (!strcmp(buffer, "IB")) {
                for (int i = 0; ptr != NULL; i++) {
                    ret.ib[i] = (int) strtol(ptr, NULL, 10);
                    ptr = strtok(NULL, " \n\t\r");
                    ret.ib_cnt++;
                }
            } else if (!strcmp(buffer, "VCS")) {
                ret.vcs = (int) strtol(ptr, NULL, 10);
            } else if (!strcmp(buffer, "TU")) {
                ret.tu = (int) strtol(ptr, NULL, 10);
            } else if (!strcmp(buffer, "MCTU")) {
                ret.mctu = (int) strtol(ptr, NULL, 10);
            } else if (!strcmp(buffer, "ABG")) {
                for (int i = 0; ptr != NULL; i++) {
                    ret.abg[i] = (int) strtol(ptr, NULL, 10);
                    ptr = strtok(NULL, " \n\t\r");
                }
            } else {
                fprintf(stderr, "Unknown cache config field: %s\n", buffer);
            }
        }
    }
    fclose(out);

write_fail:
    fclose(in);

read_fail:
    return ret;
}

void print_cacheconfig(CacheConfig cc)
{
    printf("%s: %d\n", "M",      cc.m);          /* bits of addressing bus */
    printf("%s: %d\n", "MCS",    cc.mcs);        /* main cache size */
    printf("%s: %d\n", "ES",     cc.es);         /* entry(block) size*/
    printf("%s: %d\n", "A",      cc.a);          /* n-way associative */
    printf("%s:", "IB");                         /* index bit */
    for (int i = 0; i < cc.ib_cnt; i++)
        printf(" %d", cc.ib[i]);
    printf("\n%s: %d\n", "IB_CNT", cc.ib_cnt);   /* count of index bit*/
    printf("%s: %d\n", "VCS",    cc.vcs);        /* victim-cache size */
    printf("%s: %d\n", "TU",     cc.tu);         /* time unit */
    printf("%s: %d\n", "MCTU",   cc.mctu);       /* maximum changes per time unit */
    printf("%s:", "ABG");                        /* alpha beta gamma */
    for (int i = 0; i < 3; i++)
        printf(" %d", cc.abg[i]);
    printf("\n");
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
Addr *get_addr(u64 real_addr, int set_num, int block_size)
{ // TODO use CacheConfig index bit
    int index_mask = set_num - 1;
    Addr *ret = malloc(sizeof(Addr));
    ret->index = real_addr & index_mask;
    ret->tag = real_addr & (~index_mask);
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

    int n = addr_set->block_num;
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

/*
 * Function: repl_lru
 * ------------------
 * Return the index of lru side block
 *
 * set: the set which contain the sequence infomation
 *
 * returns: the lru index
 */
int repl_lru(Set *set)
{
    return set->block_seq->nxt->block_index;
}

/*
 * Function: repl_random
 * ------------------
 * return the random index amount the set size
 *
 * set: the set which contain the size infomation
 *
 * returns: the random index
 */
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
 * repl: the replacement algorithm for getting the replace index
 */
void load_from_mem(Cache *cache, Addr *addr, ReplFunc repl)
{
    int index = addr->index;
    Set *addr_set = &(cache->sets[index]);
    int addr_tag = addr->tag;

    int load_index = 0;
    if (addr_set->block_count < addr_set->block_num) {
        load_index = repl_lru(addr_set);
        addr_set->block_count++;
    } else {
        load_index = repl(addr_set);
    }
    addr_set->blocks[load_index].valid = true;
    addr_set->blocks[load_index].tag = addr_tag;
    move_to_mru(addr_set->block_seq, load_index);
}
