#ifndef BTREESTORE_H
#define BTREESTORE_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdbool.h>
#include <math.h>

typedef struct
{
    struct info *data;
    uint32_t key;
} record;

#define VALTYPE  record

#ifndef BTREE_BSEARCH_MIN_SIZE
#define BTREE_BSEARCH_MIN_SIZE 16
#endif

#define BTREE_ITER_MAX_DEPTH    (sizeof(VALTYPE) * 8)

typedef VALTYPE val_t;


typedef struct bnode
{
    unsigned short size;        // 节点包含的元素数目
    unsigned short min_degree; // 最小度
    struct bnode **subs;        // 下级子树, 可以包含2t个节点; 如果是页节点为NULL
    val_t vals[];     // 节点元素
} node_t;

typedef struct bnode btree_node_t;

typedef struct
{
    unsigned short min_degree; // Btree的最小度数
    btree_node_t *root;       // Btree的根节点
} btree_t;

typedef struct
{
    int size;                // 迭代器保存节点的深度
    int idx[BTREE_ITER_MAX_DEPTH];    // 当前遍历节点的位置
    btree_node_t *sub[BTREE_ITER_MAX_DEPTH];    // 当前遍历的节点指针
} btree_iter_t;


struct info
{
    uint32_t size;
    uint32_t key[4];
    uint64_t nonce;
    void *data;
};

struct node
{
    uint16_t num_keys;
    uint32_t *keys;
};

int cmp_fun(val_t *data1, val_t *data2);

void *init_store(uint16_t branching, uint8_t n_processors);

btree_t *btree_create(unsigned short min_degree);

void close_store(void *helper);

val_t *btree_search(btree_t *btree, val_t *key);

int btree_insert_fun(btree_t *btree, val_t *key_val);

int btree_insert(uint32_t key, void *plaintext, size_t count, uint32_t encryption_key[4], uint64_t nonce, void *helper);

int btree_retrieve(uint32_t key, struct info *found, void *helper);

int btree_decrypt(uint32_t key, void *output, void *helper);

int btree_delete_fun(btree_t *btree, val_t *key);

int btree_delete(uint32_t key, void *helper);

uint64_t btree_export(void *helper, struct node **list);

uint32_t *encrypt_tea(uint32_t plain[2], uint32_t cipher[2], uint32_t key[4]);

uint32_t *decrypt_tea(uint32_t cipher[2], uint32_t plain[2], uint32_t key[4]);

uint64_t *encrypt_tea_ctr(uint64_t *plain, uint32_t key[4], uint64_t nonce, uint64_t *cipher, uint32_t num_blocks);

uint64_t *decrypt_tea_ctr(uint64_t *cipher, uint32_t key[4], uint64_t nonce, uint64_t *plain, uint32_t num_blocks);

val_t *btree_min(btree_t *btree);

val_t *btree_max(btree_t *btree);

val_t *btree_first(btree_t *btree, val_t *v, btree_iter_t *iter);

val_t *btree_last(btree_t *btree, val_t *v, btree_iter_t *iter);

val_t *btree_next(btree_t *btree, btree_iter_t *iter);

val_t *btree_prev(btree_t *btree, btree_iter_t *iter);

long my_pow(int x, int y);

#endif
