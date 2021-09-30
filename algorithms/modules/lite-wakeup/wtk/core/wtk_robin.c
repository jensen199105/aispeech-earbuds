#include "wtk/core/wtk_robin.h"

wtk_robin_t *wtk_robin_new(int n)
{
    wtk_robin_t *r;
    r = (wtk_robin_t *)wtk_malloc(sizeof(*r) + n * sizeof(void **));
    r->nslot = n;
    r->pop = 0;
    r->used = 0;
    r->r = (void **)(r + 1);
    return r;
}

void wtk_robin_reset(wtk_robin_t *r)
{
    r->pop = r->used = 0;
}

int wtk_robin_delete(wtk_robin_t *r)
{
    wtk_free(r);
    return 0;
}

void wtk_robin_push(wtk_robin_t *r, void *d)
{
    int index;
    index = (r->pop + r->used) % r->nslot;
    r->r[index] = d;
    ++r->used;
}

void *wtk_robin_pop(wtk_robin_t *r)
{
    void *d;
    d = r->r[r->pop];
    r->pop = (r->pop + 1) % r->nslot;
    --r->used;
    return d;
}

