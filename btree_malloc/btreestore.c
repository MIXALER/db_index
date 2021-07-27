#include "btreestore.h"
#include <assert.h>
#include <pthread.h>

#define ARRAY_MOVE(a, i, b, l) memmove((a)+(i),(a)+(b),sizeof(*a)*((l)-(b)))
#define ARRAY_INSERT(a, b, l, n) do {ARRAY_MOVE(a,(b)+1,b,l); (a)[b]=(n);} while(0)
#define ISLEAF(n) ((n)->subs == NULL)

typedef btree_iter_t iter_t;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static inline node_t *btree_node_create(unsigned int min_degree, int is_leaf)
{
    node_t **subs = NULL;
    node_t *node = calloc(1, sizeof(node_t) + (2 * min_degree - 1) * sizeof(val_t));
    if (!is_leaf)
        subs = (node_t **) calloc(2 * min_degree, sizeof(node_t *));

    if (!node || !(is_leaf || subs))
    {
        free(node);
        free(subs);
        return NULL;
    }

    node->subs = subs;
    node->min_degree = min_degree;
    node->size = 0;

    return node;
}

static inline void btree_node_destroy(node_t *node)
{
    free(node->subs);
    free(node);
}

static int btree_node_search(val_t *arr, int len, val_t *n, int *ret)
{
    int m = 0, d = 0, cmp = 0;
    if (len > BTREE_BSEARCH_MIN_SIZE)
    {
        while (m < len)
        {
            d = (len + m) / 2;
            if ((cmp = cmp_fun(arr + d, n)) == 0)
            {
                *ret = d;
                return 1;
            }
            if (cmp < 0)
                m = d + 1;
            else
                len = d;
        }
    } else
    {
        for (m = 0; m < len; m++)
        {
            if ((cmp = cmp_fun(arr + m, n)) >= 0)
                break;
        }

        if (m < len && cmp == 0)
        {
            *ret = m;
            return 1;
        }
    }
    *ret = m;
    return 0;
}

static int btree_split_child(int t, node_t *parent, int idx, node_t *child)
{
    node_t *n = btree_node_create(t, ISLEAF(child));
    if (!n)
        return -1;

    // 将child的右半部分拷贝到新的节点
    memcpy(n->vals, child->vals + t, sizeof(*n->vals) * (t - 1));
    if (n->subs)
        memcpy(n->subs, child->subs + t, sizeof(*n->subs) * t);
    n->size = t - 1;

    // 直接修改现有的元素个数就可以了
    child->size = t - 1;

    // 将分裂的新节点插入到正确位置
    ARRAY_INSERT(parent->vals, idx, parent->size, child->vals[t - 1]);
    ARRAY_INSERT(parent->subs, idx + 1, parent->size + 1, n);
    parent->size++;

    return 0;
}

int btree_insert_fun(btree_t *btree, val_t *val)
{
    node_t *child = NULL, *node = btree->root;
    int t = btree->min_degree, idx;

    if (node->size == (2 * t - 1))
    {
        if ((child = btree_node_create(t, 0)) == NULL)
            return -1;

        child->subs[0] = node, node = btree->root = child;
        if (btree_split_child(t, child, 0, child->subs[0]) < 0)
            return -1;
    }

    for (;;)
    {
        if (btree_node_search(node->vals, node->size, val, &idx))
            return -1;
        if (ISLEAF(node))
        {
            ARRAY_INSERT(node->vals, idx, node->size, *val);
            node->size++;
            break;
        }

        if (node->subs[idx]->size == (2 * t - 1))
        {
            if (btree_split_child(t, node, idx, node->subs[idx]) < 0)
                return -1;
            if (cmp_fun(val, node->vals + idx) > 0)
                idx++;
        }

        node = node->subs[idx];
    }

    return 0;
}

static inline val_t *__btree_max(node_t *subtree)
{
    assert(subtree && subtree->size > 0);
    for (;;)
    {
        if (ISLEAF(subtree) || !subtree->subs[subtree->size])
            return &subtree->vals[subtree->size - 1];
        subtree = subtree->subs[subtree->size];
    }
}

static inline val_t *__btree_min(node_t *subtree)
{
    assert(subtree && subtree->size > 0);
    for (;;)
    {
        if (ISLEAF(subtree) || !subtree->subs[0])
            return subtree->vals;
        subtree = subtree->subs[0];
    }
}

static inline node_t *btree_merge_siblings(btree_t *btree, node_t *parent, int idx)
{
    int t = btree->min_degree;
    node_t *n1, *n2;

    if (idx == parent->size)
        idx--;
    n1 = parent->subs[idx];
    n2 = parent->subs[idx + 1];

    assert(n1->size + n2->size + 1 < 2 * t);

    // 合并n1, parent->vals[idx], n2成一个大的节点
    memcpy(n1->vals + t, n2->vals, sizeof(*n1->vals) * (t - 1));
    if (n1->subs)
        memcpy(n1->subs + t, n2->subs, sizeof(*n1->subs) * t);
    n1->vals[t - 1] = parent->vals[idx];
    n1->size += n2->size + 1;

    // 将合并后的节点插入到parent的正确位置
    ARRAY_MOVE(parent->vals, idx, idx + 1, parent->size);
    ARRAY_MOVE(parent->subs, idx + 1, idx + 2, parent->size + 1);
    parent->subs[idx] = n1;
    parent->size--;

    // 如果root节点只有一个节点, 降低树高
    if (parent->size == 0 && btree->root == parent)
    {
        btree_node_destroy(parent);
        btree->root = n1;
    }

    btree_node_destroy(n2);
    return n1;
}

static void move_left_to_right(node_t *parent, int idx)
{
    // 将idx位置的数据移动到右节点最左边，从idx的左节点移动最大的key到idx位置，
    node_t *left = parent->subs[idx], *right = parent->subs[idx + 1];
    ARRAY_MOVE(right->vals, 1, 0, right->size);
    right->vals[0] = parent->vals[idx];
    parent->vals[idx] = left->vals[left->size - 1];

    if (right->subs)
    {
        ARRAY_MOVE(right->subs, 1, 0, right->size + 1);
        right->subs[0] = left->subs[left->size];
    }

    left->size--;
    right->size++;
}

static void move_right_to_left(node_t *parent, int idx)
{
    // 将idx位置的数据移动到左节点最右边，从idx的右节点移动最小的key到idx位置，
    node_t *left = parent->subs[idx], *right = parent->subs[idx + 1];
    left->vals[left->size] = parent->vals[idx];
    parent->vals[idx] = right->vals[0];
    ARRAY_MOVE(right->vals, 0, 1, right->size);

    if (right->subs)
    {
        left->subs[left->size + 1] = right->subs[0];
        ARRAY_MOVE(right->subs, 0, 1, right->size + 1);
    }

    right->size--;
    left->size++;
}

static int __btree_delete(btree_t *btree, node_t *sub, val_t *key)
{
    int idx, t = btree->min_degree;

    for (;;)
    {
        node_t *parent;
        if (btree_node_search(sub->vals, sub->size, key, &idx))
            break;
        if (ISLEAF(sub))
            return -1;

        parent = sub, sub = sub->subs[idx];
        assert(sub != NULL);
        if (sub->size > t - 1)
            continue;

        // 3: key不在内部节点中，必须保证其查询路径上的父节点的元素都大于t-1个
        // 3.a: 如果兄弟节点的元素个数大于t-1, 从兄弟节点提升一个节点到父节点
        // 在将父节点idx上的元素下降到这个节点上保证大于t-1个元素
        // 3.b: 兄弟节点的元素个数都是t-1, 合并成一个新的节点b:
        if (idx < parent->size && parent->subs[idx + 1]->size > t - 1)
            move_right_to_left(parent, idx);
        else if (idx > 0 && parent->subs[idx - 1]->size > t - 1)
            move_left_to_right(parent, idx - 1);
        else
            sub = btree_merge_siblings(btree, parent, idx);
    }
    LOOP:
    if (ISLEAF(sub))
    { // 1: 为页节点; a)在根节点上 b)元素个数大于t - 1
        assert(sub == btree->root || sub->size > t - 1);
        ARRAY_MOVE(sub->vals, idx, idx + 1, sub->size); // 将idx之后的数据往前移
        sub->size--;
    } else
    { // 2: 在内部节点上
        if (sub->subs[idx]->size > t - 1)
        {
            // 2.a: 小于Key的子树的根节点包含了大于t - 1个元素,找到子树的
            // 最大元素替换当前元素，并从子树中删除
            sub->vals[idx] = *__btree_max(sub->subs[idx]);
            __btree_delete(btree, sub->subs[idx], sub->vals + idx);
        } else if (sub->subs[idx + 1]->size > t - 1)
        {
            // 2.b: 大于Key的子树的根节点包含了大于t - 1个元素,找到子树的
            // 最小元素替换当前元素，并从子树中删除
            sub->vals[idx] = *__btree_min(sub->subs[idx + 1]);
            __btree_delete(btree, sub->subs[idx + 1], sub->vals + idx);
        } else
        {
            // 2.c: 小于和大于Key的子树都只包含了t - 1个元素, 那么将lt, key
            // 和gt都合并到一个节点上, 然后递归删除
            assert(sub->subs[idx]->size == t - 1 && sub->subs[idx + 1]->size == t - 1);
            sub = btree_merge_siblings(btree, sub, idx);
            idx = t - 1;
            goto LOOP;
        }
    }

    return 0;
}

int btree_delete_fun(btree_t *btree, val_t *key)
{
    return __btree_delete(btree, btree->root, key);
}

static void __btree_destroy(node_t *n)
{
    if (n)
    {
        if (!ISLEAF(n))
        {
            int i = 0;
            for (i = 0; i < n->size + 1; i++)
                __btree_destroy(n->subs[i]);
        }
        btree_node_destroy(n);
    }
}

void btree_destroy(btree_t *btree)
{
    if (btree)
    {
        __btree_destroy(btree->root);
        free(btree);
    }
}

btree_t *btree_create(unsigned short min_degree)
{
    btree_t *btree = calloc(1, sizeof(*btree));
    if (btree)
    {
        btree->min_degree = min_degree;
        btree->root = btree_node_create(min_degree, 1);
    }
    return btree;
}

val_t *btree_search(btree_t *btree, val_t *key)
{
    node_t *node = btree->root;
    for (;;)
    {
        int i = 0;
        if (btree_node_search(node->vals, node->size, key, &i))
            return node->vals + i;
        if (ISLEAF(node))
            return NULL;
        node = node->subs[i];
    }

    return NULL;
}

val_t *btree_max(btree_t *btree)
{
    return __btree_max(btree->root);
}

val_t *btree_min(btree_t *btree)
{
    return __btree_min(btree->root);
}

static inline void btree_iter_init(btree_t *btree, val_t *v, iter_t *iter, int n)
{
    node_t *node = btree->root;
    iter->size = -1;
    for (;;)
    {
        int idx = 0;
        int r = btree_node_search(node->vals, node->size, v, &idx);

        iter->size++;
        assert(iter->size < BTREE_ITER_MAX_DEPTH);
        iter->sub[iter->size] = node;
        iter->idx[iter->size] = idx + !!(r && !n);

        if (ISLEAF(node) || r)
            return;
        node = node->subs[idx];
    }
}

static inline val_t *_btree_next(iter_t *iter)
{
    node_t *node = iter->sub[iter->size];
    int idx = iter->idx[iter->size];

    iter->idx[iter->size]++;
    if (!ISLEAF(node) && idx < node->size)
    {
        node_t *sub = node->subs[idx + 1];
        for (;;)
        {
            iter->size++;
            assert(iter->size < BTREE_ITER_MAX_DEPTH);
            iter->sub[iter->size] = sub;
            iter->idx[iter->size] = 0;
            if (ISLEAF(sub))
                break;
            sub = sub->subs[0];
        }
    }

    return idx < node->size ? node->vals + idx : NULL;
}

static inline val_t *_btree_prev(iter_t *iter)
{
    node_t *node = iter->sub[iter->size];
    int idx = iter->idx[iter->size];

    iter->idx[iter->size]--;
    if (!ISLEAF(node) && idx > 0)
    {
        node_t *sub = node->subs[idx - 1];
        for (;;)
        {
            iter->size++;
            assert(iter->size < BTREE_ITER_MAX_DEPTH);
            iter->sub[iter->size] = sub;
            iter->idx[iter->size] = sub->size;
            if (ISLEAF(sub))
                break;
            sub = sub->subs[sub->size];
        }
    }

    return idx > 0 ? node->vals + (idx - 1) : NULL;
}

static inline val_t *_btree_iter(btree_t *btree, iter_t *iter, int lt)
{
    for (; iter->size >= 0; iter->size--)
    {
        val_t *r = lt ? _btree_next(iter) : _btree_prev(iter);
        if (r)
            return r;
    }

    return NULL;
}

val_t *btree_first(btree_t *btree, val_t *v, iter_t *iter)
{
    btree_iter_init(btree, v, iter, 1);
    return btree_next(btree, iter);
}

val_t *btree_last(btree_t *btree, val_t *v, iter_t *iter)
{
    btree_iter_init(btree, v, iter, 0);
    return btree_prev(btree, iter);
}

val_t *btree_next(btree_t *btree, iter_t *iter)
{
    return _btree_iter(btree, iter, 1);
}

val_t *btree_prev(btree_t *btree, iter_t *iter)
{
    return _btree_iter(btree, iter, 0);
}

int cmp_fun(val_t *data1, val_t *data2)
{
    uint32_t k1 = data1->key;
    uint32_t k2 = data2->key;
    if (k1 < k2)
        return -1;
    else if (k1 == k2)
        return 0;
    else
        return 1;
}

void *init_store(uint16_t branching, uint8_t n_processors)
{
    // Your code here
//    todo: 多线程初始化
    uint16_t min_degree = (branching + 1) / 2; // 节点的最小容量
    btree_t *btree;
    btree = btree_create(min_degree);
    return (void *) btree;
}

void close_store(void *helper)
{
    // Your code here;
    btree_t *btree = (btree_t *) helper;
    btree_destroy(btree);
}

int btree_insert(uint32_t key, void *plaintext, size_t count, uint32_t encryption_key[4], uint64_t nonce, void *helper)
{
    // Your code here
    btree_t *btree = (btree_t *) helper;

    val_t *for_insert = malloc(sizeof(val_t));
    struct info *for_insert_data = malloc(sizeof(struct info));
    uint64_t *cipher = malloc(count * sizeof(uint64_t));
    cipher = encrypt_tea_ctr(plaintext, encryption_key, nonce, cipher, count);
    for_insert_data->data = cipher;

    for (int i = 0; i < 4; ++i)
    {
        for_insert_data->key[i] = encryption_key[i];
    }
    for_insert_data->size = count;
    for_insert_data->nonce = nonce;
    for_insert->data = for_insert_data;
    for_insert->key = key;
    pthread_mutex_lock(&mutex);
    int flag;
    flag = (btree_insert_fun(btree, for_insert) == 0);
    pthread_mutex_unlock(&mutex);
    if (flag)
        return 0;
    else
        return 1;
}

int btree_retrieve(uint32_t key, struct info *found, void *helper)
{
    // Your code here
    btree_t *btree = (btree_t *) helper;
    val_t *for_search = malloc(sizeof(val_t));
    for_search->key = key;
    for_search->data = NULL;

    pthread_mutex_lock(&mutex);
    val_t *search_ans = btree_search(btree, for_search);
    pthread_mutex_unlock(&mutex);

    if (search_ans == NULL || search_ans->key != for_search->key)
        return -1;
    else
    {
        *(found) = *(search_ans->data);
    }

    free(for_search);
    if (found != NULL)
        return 0;
    else
        return -1;
}

int btree_decrypt(uint32_t key, void *output, void *helper)
{
    // Your code here
    btree_t *btree = (btree_t *) helper;
    val_t *for_search = malloc(sizeof(val_t));
    for_search->key = key;
    for_search->data = NULL;
    pthread_mutex_lock(&mutex);
    val_t *search_ans = btree_search(btree, for_search);
    pthread_mutex_unlock(&mutex);
    free(for_search);
    if (search_ans == NULL || search_ans->key != key || search_ans->data == NULL)
        return -1;
    else
    {
        struct info *de_ans = (struct info *) output;
        de_ans->data = decrypt_tea_ctr(search_ans->data->data, search_ans->data->key, search_ans->data->nonce,
                                       de_ans->data,
                                       search_ans->data->size);
        de_ans->size = search_ans->data->size;
        de_ans->nonce = search_ans->data->nonce;
        for (int i = 0; i < 4; ++i)
        {
            de_ans->key[i] = search_ans->data->key[i];
        }
        return 0;
    }
}

int btree_delete(uint32_t key, void *helper)
{
    // Your code here
    btree_t *btree = (btree_t *) helper;

    val_t *for_delete = malloc(sizeof(val_t));
    for_delete->data = NULL;
    for_delete->key = key;

    pthread_mutex_lock(&mutex);//atomic opreation through mutex lock
    int flag = (btree_delete_fun(btree, for_delete) == 0);
    pthread_mutex_unlock(&mutex);

    if (flag)
        return 0;
    else
        return 1;
}

void get_node_num(node_t *btree_node, uint64_t *node_num)
{
    if (btree_node == NULL)
    {
        return;
    } else
    {
        *node_num = *node_num + 1;
        int i = 0;
        if (btree_node->subs != NULL)
        {
            while (btree_node->subs[i] != NULL && i < btree_node->size)
            {
                get_node_num(btree_node->subs[i], node_num);
                i++;
            }
        }
        return;
    }
}

void pre_order_traverse(node_t *btree_node, struct node **list, uint64_t *count)
{
    if (btree_node == NULL)
        return;
    else
    {
        list[*count]->num_keys = btree_node->size;
        uint32_t *keys = malloc(btree_node->size * sizeof(uint32_t));
        for (int i = 0; i < btree_node->size; ++i)
        {
            keys[i] = btree_node->vals[i].key;
        }
        list[*count]->keys = keys;
        (*count)++;
        uint16_t i = 0;
        if (btree_node->subs != NULL)
        {
            while (btree_node->subs[i] != NULL && i < btree_node->size)
            {
                pre_order_traverse(btree_node->subs[i], list, count);
                i++;
            }
        }
        return;
    }
}

uint64_t btree_export(void *helper, struct node **list)
{
    // Your code here
    btree_t *btree = (btree_t *) helper;

    uint64_t node_num = 0;

    pthread_mutex_lock(&mutex);
    get_node_num(btree->root, &node_num);
    pthread_mutex_unlock(&mutex);

    list = malloc(node_num * sizeof(struct node *));
    for (int i = 0; i < node_num; ++i)
    {
        list[i] = malloc(sizeof(struct node));
    }

    uint64_t count = 0;

    pthread_mutex_lock(&mutex);
    pre_order_traverse(btree->root, list, &count);
    pthread_mutex_unlock(&mutex);

    return node_num;
}

uint32_t *encrypt_tea(uint32_t plain[2], uint32_t cipher[2], uint32_t key[4])
{
    // Your code here
    uint32_t sum = 0;
    uint32_t delta = 0x9E3779B9;
    cipher[0] = plain[0];
    cipher[1] = plain[1];
    for (int i = 0; i < 1024; ++i)
    {
        sum = ((sum + delta) % my_pow(2, 32));
        uint32_t tmp1 = ((cipher[1] << 4) + key[0]) % my_pow(2, 32);
        uint32_t tmp2 = (cipher[1] + sum) % my_pow(2, 32);
        uint32_t tmp3 = ((cipher[1] >> 5) + key[1]) % my_pow(2, 32);
        cipher[0] = (cipher[0] + (tmp1 ^ tmp2 ^ tmp3)) % my_pow(2, 32);
        uint32_t tmp4 = ((cipher[0] << 4) + key[2]) % my_pow(2, 32);
        uint32_t tmp5 = (cipher[0] + sum) % my_pow(2, 32);
        uint32_t tmp6 = ((cipher[0] >> 5) + key[3]) % my_pow(2, 32);
        cipher[1] = (cipher[1] + (tmp4 ^ tmp5 ^ tmp6)) % my_pow(2, 32);
    }
    return cipher;
}

uint32_t *decrypt_tea(uint32_t cipher[2], uint32_t plain[2], uint32_t key[4])
{
    // Your code here
    uint32_t sum = 0xDDE6E400;
    uint32_t delta = 0X9E3779B9;
    for (int i = 0; i < 1024; ++i)
    {
        uint32_t tmp4 = ((cipher[0] << 4) + key[2]) % my_pow(2, 32);
        uint32_t tmp5 = (cipher[0] + sum) % my_pow(2, 32);
        uint32_t tmp6 = ((cipher[0] >> 5) + key[3]) % my_pow(2, 32);
        cipher[1] = (cipher[1] - (tmp4 ^ tmp5 ^ tmp6)) % my_pow(2, 32);
        uint32_t tmp1 = ((cipher[1] << 4) + key[0]) % my_pow(2, 32);
        uint32_t tmp2 = (cipher[1] + sum) % my_pow(2, 32);
        uint32_t tmp3 = ((cipher[1] >> 5) + key[1]) % my_pow(2, 32);
        cipher[0] = (cipher[0] - (tmp1 ^ tmp2 ^ tmp3)) % my_pow(2, 32);
        sum = (sum - delta) % my_pow(2, 32);
    }
    plain[0] = cipher[0];
    plain[1] = cipher[1];

    return plain;
}

uint64_t *encrypt_tea_ctr(uint64_t *plain, uint32_t key[4], uint64_t nonce, uint64_t *cipher, uint32_t num_blocks)
{
    // Your code here
    for (int i = 0; i < num_blocks; ++i)
    {
        uint32_t plain_tmp_32[2];
        uint32_t cipher_tmp_32[2];
        plain_tmp_32[0] = plain[i] >> 32;
        plain_tmp_32[1] = plain[i];

        uint32_t *en_res = encrypt_tea(plain_tmp_32, cipher_tmp_32, key);
        cipher[i] = en_res[0];
        cipher[i] = (cipher[i] << 32);
        cipher[i] |= en_res[1];
    }

    return cipher;
}

uint64_t *decrypt_tea_ctr(uint64_t *cipher, uint32_t key[4], uint64_t nonce, uint64_t *plain, uint32_t num_blocks)
{
    // Your code here
    for (int i = 0; i < num_blocks; ++i)
    {
        uint32_t plain_tmp_32[2];
        uint32_t cipher_tmp_32[2];
        cipher_tmp_32[0] = cipher[i] >> 32;
        cipher_tmp_32[1] = cipher[i];

        uint32_t *de_res = decrypt_tea(cipher_tmp_32, plain_tmp_32, key);
        plain[i] = de_res[0];
        plain[i] = (plain[i] << 32);
        plain[i] |= de_res[1];
    }
    return plain;
}

long my_pow(int x, int y)
{
    long res = 1;
    for (int i = 0; i < y; ++i)
    {
        res = res * x;
    }
    return res;
}
