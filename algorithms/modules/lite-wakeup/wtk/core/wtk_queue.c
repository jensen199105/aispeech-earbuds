#include "wtk_queue.h"

wtk_queue_t *wtk_queue_new(void)
{
    wtk_queue_t *q;
    q = (wtk_queue_t *)wtk_malloc(sizeof(*q));
    wtk_queue_init(q);
    return q;
}

int wtk_queue_delete(wtk_queue_t *q)
{
    wtk_free(q);
    return 0;
}

int wtk_queue_init2(wtk_queue_t *q)
{
    q->pop = q->push = 0;
    q->listener = 0;
    q->data = 0;
    q->length = 0;
    return 0;
}

int wtk_queue_push(wtk_queue_t *q, wtk_queue_node_t *n)
{
    n->prev = q->push;

    if (q->push)
    {
        q->push->next = n;
    }

    n->next = 0;
    q->push = n;

    if (!q->pop)
    {
        q->pop = n;
    }

    if (q->listener)
    {
        q->listener(q->data);
    }

    ++q->length;
    return 0;
}

wtk_queue_node_t *wtk_queue_pop(wtk_queue_t *q)
{
    wtk_queue_node_t *n = NULL;

    if (q->length <= 0)
    {
        return 0;
    }

    if (NULL == q->pop)
    {
        return NULL;
    }

    n = q->pop;
    q->pop = q->pop->next;

    if (q->pop)
    {
        q->pop->prev = NULL;
    }
    else
    {
        q->push = NULL;
    }

    --q->length;
    return n;
}

