#include <ons/ons.h>
#include <memoria/memoria.h>
#include <stdio.h>

int main() {
    unsigned long i, ii, COUNT = 10000;
    mem_rbtree_t tree;
    mem_rbnode_t *nodes[COUNT];
    char buffer[100];

    for(i = 0; i < COUNT; ++i) nodes[i] = NULL;

    mem_rbt_init(&tree, NULL);

    for(ii = 0; ii < 100; ++ii) {
        for(i = 0; i < COUNT; ++i) {
            sprintf(buffer, "%u", rand());
            if(!mem_rbt_add(&tree, MEM_STR(buffer), mem_malloc(100), &nodes[i])) ONS_ABORT("mem_rbt_add failed (collision in RNG)");
        }

        for(i = 0; i < COUNT; ++i) {
            mem_free(mem_rbt_del(&tree, nodes[i]));
            nodes[i] = NULL;
        }
    }

    return 0;
}
