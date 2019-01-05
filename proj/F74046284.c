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

typedef unsigned long long u64;
typedef struct _Addr {
    int addr;       /* store the origonal address (for victim cache) */
    int tag;        /* store the tag of the address*/
    int idx;        /* stroe the set index of the address */
    // int offset;
} Addr;

typedef struct _Block {
    bool valid;     /* for main cache entry*/
    int tag;        /* for main cache entry*/
    int addr;       /* for victim cache entry */
    // Data *data;
} Block;

typedef struct _Seq Seq;
struct _Seq {   /* the sequence of blocks be used */
    int block_idx;
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
    bool *v_bits;    /* for deciding if move to victim cache */
    int *loc_freq;
    int *glo_freq;
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

typedef struct _Set VCache;

typedef int (*ReplFunc)(Set *set);

CacheConfig read_cache_config(const char *filename);
void print_cacheconfig(CacheConfig cc);
Cache *create_cache(int set_num, int block_num_per_set);
void destroy_cache(Cache **cache);
void update_victim_bit(Cache *cache, bool *v_bits);
void print_victim_bit(FILE *fvic, bool *v_bits, int size);
bool *predictor(Cache *cache, int v_entry_num, int time_unit, int cycle_cnt);
void print_cache(Cache *cache);
void print_bits(size_t const size, void const *const ptr);
Seq *create_seq_node(int block_idx);
Seq *create_seq(int num);
void destroy_seq(Seq **seq);
VCache *create_v_cache(int block_num);
void destroy_v_cache(VCache **vcache);
void print_v_cache(VCache *vcache);
void move_block_to_v_cache(VCache *vcache, Block *block);
void move_to_mru(Seq *seq, int target_idx);
Addr *get_addr(u64 real_addr, int set_num, int block_size);
bool find_addr(Cache *cache, Addr *addr, ReplFunc repl);
bool find_addr_in_v(VCache *vcache, Addr *addr);
int repl_lru(Set *set);
void load_block_to_cache(Cache *cache, Addr *addr, ReplFunc repl, VCache *vcache);
double get_ms();
bool starts_with(const char *str, const char *pre);

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
    ReplFunc repl_func = repl_lru;
    int set_num = nk / assoc / blocksize;
    debug("%d %d %d %d\n", nk, assoc, blocksize, set_num);

    int m_hit_cnt = 0;    /* total Main-Cache hit */
    int v_hit_cnt = 0;    /* total Victim-Cache hit */
    int m_miss_cnt = 0;   /* total Main-Cache miss */
    int v_miss_cnt = 0;   /* total Victim-Cache miss */
    Cache *c = create_cache(set_num, assoc);

    /* Create victim cache with the number of blocks */
    VCache *vc = create_v_cache(cc.vcs / cc.es);

    u64 real_addr;
    char buffer[40];

    int cycle_cnt = 0;
    int vbit_change_cnt = 0;
    FILE *fsim = fopen("sim.txt", "w");
    fprintf(fsim, "# Student ID: F74046284\n");

    FILE *fvic = fopen("victim_bit.txt", "a");

    /* Open ReferenceList.txt and simulate */
    FILE *fin = fopen(argv[2], "r");
    double t_start = get_ms();
    while (!feof(fin)) {
        if (fgets(buffer, 40, fin) != NULL) {
            if (buffer[0] == '#') { // comment
                continue;
            }

            if (starts_with(buffer, ".benchmark")) { // start
                continue;
            }

            if (starts_with(buffer, ".end")) { // end
                continue;
            }

            if (cycle_cnt % ((cc.tu + 1) / cc.mctu) == 0) {
                bool *v_bits = predictor(c, vc->block_num, cc.tu, cycle_cnt);
                update_victim_bit(c, v_bits);
                vbit_change_cnt++;
                free(v_bits);
            }
            cycle_cnt++;

            real_addr = strtoull(buffer, NULL, 2);
            debug("%lld\n", real_addr);

            Addr *addr = get_addr(real_addr, set_num, blocksize);
            debug("set: %d, tag: %d\n", addr->idx, addr->tag);
            debug("addr: %d\n", addr->addr);

            bool is_hit = find_addr(c, addr, repl_func);
            debug("m: %s\n", is_hit ? "HIT" : "MISS");

            if (!is_hit) {
                m_miss_cnt++;
                bool is_hit_in_v = find_addr_in_v(vc, addr);
                debug("v: %s\n", is_hit_in_v ? "HIT" : "MISS");

                if (!is_hit_in_v) {
                    v_miss_cnt++;
                } else {
                    v_hit_cnt++;
                }
                /*
                 * Whether hit in the victim or not, main cache need to load
                 * the block from the address. In addition, if load from
                 * victim cache and the set is full, it is possible that need
                 * to move the other block to the victim cache again.
                 */
                load_block_to_cache(c, addr, repl_func, vc);
            } else {
                m_hit_cnt++;
            }
            free(addr);
            // print_cache(c);
            // print_v_cache(vc);
            // printf("\n\n");
        } else {
            goto out;
        }
    }

out:;
    double t_end = get_ms();
    fprintf(fsim, "# total Main-Cache miss\n");
    fprintf(fsim, "MCM %d\n", m_miss_cnt);
    fprintf(fsim, "# total Victim-Cache miss\n");
    fprintf(fsim, "VCM %d\n", v_miss_cnt);
    fprintf(fsim, "# Total Cycle Time\n");
    fprintf(fsim, "TCT %d\n", m_hit_cnt * cc.abg[0] +
                        v_hit_cnt * cc.abg[1] +
                        v_miss_cnt * cc.abg[2]);
    fprintf(fsim, "# CPU Time(ms)\n");
    fprintf(fsim, "T %.0lf\n", t_end - t_start);
    fprintf(fvic, "# Configuration of Victim-Bits: total change count\n");
    fprintf(fvic, "CVB %d\n", vbit_change_cnt);
    fprintf(fvic, "# Cycle Time Configuration of Victim-Bits\n");
    destroy_v_cache(&vc);
    destroy_cache(&c);

    /* Need to simulate twice because of the order of writing */
    m_hit_cnt = 0;    /* total Main-Cache hit */
    v_hit_cnt = 0;    /* total Victim-Cache hit */
    m_miss_cnt = 0;   /* total Main-Cache miss */
    v_miss_cnt = 0;   /* total Victim-Cache miss */
    c = create_cache(set_num, assoc);

    /* Create victim cache with the number of blocks */
    vc = create_v_cache(cc.vcs / cc.es);

    cycle_cnt = 0;

    /* Open ReferenceList.txt and simulate */
    fin = fopen(argv[2], "r");
    while (!feof(fin)) {
        if (fgets(buffer, 40, fin) != NULL) {
            if (buffer[0] == '#') { // comment
                continue;
            }

            if (starts_with(buffer, ".benchmark")) { // start
                fprintf(fsim, "%s", buffer);
                fprintf(fsim, "#input_address\tmain_cache status\tvictim_cache status\n");
                continue;
            }

            if (starts_with(buffer, ".end")) { // end
                fprintf(fsim, "%s", buffer);
                continue;
            }

            if (cycle_cnt % ((cc.tu + 1) / cc.mctu) == 0) {
                bool *v_bits = predictor(c, vc->block_num, cc.tu, cycle_cnt);
                update_victim_bit(c, v_bits);
                free(v_bits);
                fprintf(fvic, "%d ", cycle_cnt + 1);
                print_victim_bit(fvic, c->v_bits, c->set_num);
            }
            cycle_cnt++;

            for (int i = 0; i < cc.m; i++) {
                fprintf(fsim, "%c", buffer[i]);
            }
            real_addr = strtoull(buffer, NULL, 2);

            Addr *addr = get_addr(real_addr, set_num, blocksize);

            bool is_hit = find_addr(c, addr, repl_func);
            fprintf(fsim, "\t%s", is_hit ? "h\tn\n" : "m");

            if (!is_hit) {
                m_miss_cnt++;
                bool is_hit_in_v = find_addr_in_v(vc, addr);
                fprintf(fsim, "\t%s\n", is_hit_in_v ? "h" : "m");

                if (!is_hit_in_v) {
                    v_miss_cnt++;
                } else {
                    v_hit_cnt++;
                }
                load_block_to_cache(c, addr, repl_func, vc);
            } else {
                m_hit_cnt++;
            }
            free(addr);
        } else {
            goto finish;
        }
    }

finish:;
    destroy_v_cache(&vc);
    destroy_cache(&c);
    fclose(fvic);
    fclose(fsim);
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
            tmp_blocks[j].addr = 0;
        }
        tmp_sets[i].block_count = 0;
        tmp_sets[i].block_num = block_num_per_set;
        tmp_sets[i].blocks = tmp_blocks;
        tmp_sets[i].block_seq = create_seq(block_num_per_set);
    }
    ret->sets = tmp_sets;
    ret->set_num = set_num;
    ret->v_bits = calloc(set_num, sizeof(bool));
    ret->loc_freq = calloc(set_num, sizeof(int));
    ret->glo_freq = calloc(set_num, sizeof(int));
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
    free((*cache)->v_bits);
    free((*cache)->loc_freq);
    free((*cache)->glo_freq);
    free(*cache);
    *cache = NULL;
}


void update_victim_bit(Cache *cache, bool *v_bits)
{
    for (int i = 0; i < cache->set_num; i++) {
        cache->v_bits[i] = v_bits[i];

        /* clear local frequency history */
        cache->loc_freq[i] = 0;
    }
}

void print_victim_bit(FILE *fvic, bool *v_bits, int size)
{
    for (int i = 0; i < size; i++) {
        if (v_bits[i]) {
            fprintf(fvic, "1");
        } else {
            fprintf(fvic, "0");
        }
    }
    fprintf(fvic, "\n");
}

bool *predictor(Cache *cache, int v_entry_num, int time_unit, int cycle_cnt)
{
    int set_num = cache->set_num;
    bool *v_bits = calloc(set_num, sizeof(bool));
    int size_cnt = 0; // count the number of bits which are set
    int size_threshold = v_entry_num * 3 / 2;
    int loc_threshold = (time_unit / v_entry_num) / 2;
    int glo_threshold = (cycle_cnt / v_entry_num) / 8;
    loc_threshold = loc_threshold ? loc_threshold : 1;
    glo_threshold = glo_threshold ? glo_threshold : 1;
    debug("l_t: %d, g_t: %d\n", loc_threshold, glo_threshold);
    /* clear victim bits */
    for (int i = 0; i < set_num; i++) {
        v_bits[i] = false;
    }
    for (int i = 0; i < set_num; i++) {
        if (cache->loc_freq[i] >= loc_threshold) {
            v_bits[i] = true;
            size_cnt++;
        }
    }
    if (size_cnt > size_threshold) {
        goto enought;
    }
    for (int i = 0; i < set_num; i++) {
        if (!v_bits[i] && cache->glo_freq[i] >= glo_threshold) {
            v_bits[i] = true;
            size_cnt++;
        }
    }
    if (size_cnt > size_threshold) {
        goto enought;
    }
    for (int i = 0; i < set_num; i++) {
        if (!v_bits[i] && cache->glo_freq[i] > 0) {
            v_bits[i] = true;
            size_cnt++;
        }
    }
    if (size_cnt > size_threshold) {
        goto enought;
    }
    for (int i = 0; i < set_num; i++) {
        if (!v_bits[i]) {
            v_bits[i] = true;
            size_cnt++;
        }
        if (size_cnt > size_threshold) {
            goto enought;
        }
    }
enought:;
    debug("set: %d\n", size_cnt);
    return v_bits;
}

/*
 *
 */
void print_cache(Cache *cache)
{
    printf("+-- Main cache --+\n");
    int set_num = cache->set_num;
    for (int i = 0; i < set_num; i++) {
        Set set = cache->sets[i];
        printf("+ set%02d ---------+, g(%d), l(%d)\n", i,
                cache->glo_freq[i],
                cache->loc_freq[i]);

        Seq *itr = set.block_seq->nxt;
        Seq *mru = set.block_seq->pre;
        int b_cnt = 0;
        while (itr != mru) {
            int idx = itr->block_idx;
            int addr = set.blocks[idx].addr;
            printf("|  b%d: ", b_cnt++);
            print_bits(1, &addr);
            printf("  |\n");
            itr = itr->nxt;
        }
    }
    printf("+----------------+\n");
}

void print_bits(size_t const size, void const *const ptr)
{
    unsigned char *b = (unsigned char*) ptr;
    for (int i = size - 1; i >= 0; i--){
        for (int j = 7; j >= 0; j--){
            printf("%u", (b[i] >> j) & 1);
        }
    }
}

/*
 * Function: create_seq_node
 * -------------------------
 * Create and return a node of sequence with setting block index
 *
 * block_idx: the index used to point to specific block quickly
 *
 * returns: the seq node whit being initialized
 */
Seq *create_seq_node(int block_idx)
{
    Seq *ret = malloc(sizeof(Seq));
    ret->block_idx = block_idx;
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
    while(itr->block_idx != -2) {
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
 * target_idx: the index need to move to the last,
 *               if the index is illegal seq will not change
 */
void move_to_mru(Seq *seq, int target_idx)
{
    Seq *itr = seq->nxt;
    Seq *mru = seq->pre;
    while (itr->block_idx != -2) {
        if (itr->block_idx == target_idx) {
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
 */
VCache *create_v_cache(int block_num)
{
    VCache *ret = malloc(sizeof(VCache));
    Block *tmp_blocks = malloc(sizeof(Block) * block_num);
    for (int i = 0; i < block_num; i++) {
        tmp_blocks[i].valid = false;
        tmp_blocks[i].tag = 0;
        tmp_blocks[i].addr = 0;
    }
    ret->block_count = 0;
    ret->block_num = block_num;
    ret->blocks = tmp_blocks;
    ret->block_seq = create_seq(block_num);
    return ret;
}

/*
 */
void destroy_v_cache(VCache **vcache)
{
    destroy_seq(&((*vcache)->block_seq));
    free((*vcache)->blocks);
    free(*vcache);
    *vcache = NULL;
}

/*
 *
 */
void print_v_cache(VCache *vcache)
{
    printf("+- Victim cache -+\n");
    Seq *itr = vcache->block_seq->nxt;
    Seq *mru = vcache->block_seq->pre;
    int b_cnt = 0;
    while (itr != mru) {
        int idx = itr->block_idx;
            printf("|  b%d: ", b_cnt++);
        if (vcache->blocks[idx].valid) {
            int addr = vcache->blocks[idx].addr;
            print_bits(1, &addr);
        } else {
            printf("X       ");
        }
        printf("  |\n");
        itr = itr->nxt;
    }
    printf("+----------------+\n");
}

/*
 *
 */
void move_block_to_v_cache(VCache *vcache, Block *block)
{
    int place_idx = vcache->block_seq->nxt->block_idx;
    vcache->blocks[place_idx].valid = true;
    vcache->blocks[place_idx].tag = block->tag;
    vcache->blocks[place_idx].addr = block->addr;
    move_to_mru(vcache->block_seq, place_idx);
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
    int idx_mask = set_num - 1;
    Addr *ret = malloc(sizeof(Addr));
    ret->addr = real_addr;
    ret->idx = real_addr & idx_mask;
    ret->tag = real_addr & (~idx_mask);
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
    int idx = addr->idx;
    Set *addr_set = &(cache->sets[idx]);
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

bool find_addr_in_v(VCache *vcache, Addr *addr)
{
    int addr_addr = addr->addr;

    int n = vcache->block_num;
    for (int i = 0; i < n; i++) {
        if (vcache->blocks[i].valid == true &&
                vcache->blocks[i].addr == addr_addr) {
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
    return set->block_seq->nxt->block_idx;
}

/*
 * Function: load_block_to_cache
 * -----------------------
 * load block from memory to cache, if set is full then replace one block
 *
 * cache: the cache
 * addr: the address of block need to be loaded
 * repl: the replacement algorithm for getting the replace index
 */
void load_block_to_cache(Cache *cache, Addr *addr, ReplFunc repl, VCache *vcache)
{
    int idx = addr->idx;
    Set *addr_set = &(cache->sets[idx]);
    bool is_victim = cache->v_bits[idx];

    cache->loc_freq[idx]++;
    cache->glo_freq[idx]++;

    int load_idx = 0;
    if (addr_set->block_count < addr_set->block_num) {
        load_idx = repl_lru(addr_set);
        addr_set->block_count++;
    } else { /* find a index to replace with the new block */
        load_idx = repl(addr_set);
        if (is_victim) {
            move_block_to_v_cache(vcache, &(addr_set->blocks[load_idx]));
        }
    }
    addr_set->blocks[load_idx].valid = true;
    addr_set->blocks[load_idx].tag = addr->tag;
    addr_set->blocks[load_idx].addr = addr->addr;
    move_to_mru(addr_set->block_seq, load_idx);
}

double get_ms()
{
    struct timespec ts;
    double sec;
    clock_gettime(CLOCK_REALTIME, &ts);
    sec = ts.tv_nsec;
    sec /= 1e9;
    sec += ts.tv_sec;
    return sec * 1000;
}

bool starts_with(const char *str, const char *pre)
{
    size_t lenpre = strlen(pre);
    size_t lenstr = strlen(str);
    return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
}
