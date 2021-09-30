#include <stdio.h>
#include "wtk/core/wtk_circular_queue.h"

wtk_cqueue_t *wtk_cqueue_new(U16 data_num)
{
    wtk_cqueue_t *q;
    q = (wtk_cqueue_t *)wtk_calloc(1, sizeof(wtk_cqueue_t) + data_num * sizeof(S32));
    q->size = data_num;
    wtk_cqueue_reset(q);
    q->data = (S32 *)((char *)q + sizeof(wtk_cqueue_t));
    return q;
}

void wtk_cqueue_free(wtk_cqueue_t *q)
{
    wtk_free(q);
}

void wtk_cqueue_reset(wtk_cqueue_t *q)
{
    q->head = 0;
    q->tail = 0;
    q->used = 0;
}

void wtk_cqueue_push(wtk_cqueue_t *q, S32 data)
{
    if (WTK_CQUEUE_FULL(q))
    {
        //printf("%s. cqueue is full, drop the olddest data\n", __FUNCTION__);
        // 丢弃最旧的数据
        (void)wtk_cqueue_pop(q);
    }

    q->data[q->tail] = data;
    q->tail++;

    if (q->tail == q->size)
    {
        q->tail = 0;
    }

    q->used++;
}

S32 wtk_cqueue_pop(wtk_cqueue_t *q)
{
    if (WTK_CQUEUE_EMPTY(q))
    {
        //printf("ERROR, queue is empty. pop failed.\n");
        return 0;
    }

    int num = q->data[q->head];
    q->head++;

    if (q->head == q->size)
    {
        q->head = 0;
    }

    q->used--;
    return num;
}

S32 wtk_cqueue_front(wtk_cqueue_t *q)
{
    if (WTK_CQUEUE_EMPTY(q))
    {
        //printf("ERROR, queue is empty. get front failed.\n");
        return 0;
    }

    return q->data[q->head];
}

S32 wtk_cqueue_back(wtk_cqueue_t *q)
{
    if (WTK_CQUEUE_EMPTY(q))
    {
        //printf("ERROR, queue is empty. get back failed.\n");
        return 0;
    }

    if (q->tail != 0)
    {
        return q->data[q->tail - 1];
    }
    else
    {
        return q->data[q->size - 1];
    }
}

