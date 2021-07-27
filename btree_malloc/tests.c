#include "btreestore.h"
#include <assert.h>


int main()
{
    setbuf(stdout, NULL);
    // Your own testing code here
    void *helper = init_store(4, 4);

    for (uint32_t i = 0; i < 10000; ++i)
    {
        uint32_t en_key[4] = {1, 2, 3, 4};
        uint64_t tmp[3] = {i - 1, i, i + 1};
        assert(btree_insert(i + 1, tmp, 3, en_key, i + 1, helper) == 0);
    }

    for (uint32_t i = 0; i < 10000; ++i)
    {
        struct info *found = malloc(sizeof(struct info));
        assert(btree_retrieve(i + 1, found, helper) == 0);
        free(found);
    }

    for (uint32_t i = 0; i < 10000; i++)
    {
        struct info *output = malloc(sizeof(struct info));
        uint64_t tmp[3];
        output->data = tmp;
        assert(btree_decrypt(i + 1, output, helper) == 0);
        free(output);
    }

    struct node **list = NULL;

    uint64_t num = btree_export(helper, list);

    printf("%ld\n", num);

    for (uint32_t i = 0; i < 10000; ++i)
    {
        assert(btree_delete(i + 1, helper) == 0);
    }
    uint64_t num_2 = btree_export(helper, list);

    printf("%ld\n", num_2);

    close_store(helper);
    return 0;
}
