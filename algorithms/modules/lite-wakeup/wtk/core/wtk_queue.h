#ifndef _WTK_CORE_WTK_QUEUE_H_
#define _WTK_CORE_WTK_QUEUE_H_

#include "wtk_type.h"
#include "wtk_str.h"
#include "AISP_TSL_str.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct wtk_queue_node wtk_queue_node_t;
typedef struct wtk_queue      wtk_queue_t;

#define wtk_queue_node_data(q, type, link) (type*)((q)!=NULL ? (void*)((char*)q-offsetof(type,link))  : NULL)
#define wtk_queue_init(q)                  AISP_TSL_memset(q, 0, sizeof(*q))

typedef void (*queue_push_listener)(void *data);

struct wtk_queue_node
{
    wtk_queue_node_t *next;
    wtk_queue_node_t *prev;
};

#define WTK_QUEUE \
    wtk_queue_node_t *pop; \
    wtk_queue_node_t *push; \
    queue_push_listener listener; \
    void *data; \
    int length;

struct wtk_queue
{
    WTK_QUEUE
};

/**
 * @brief new queue, just malloc wtk_queue_t memory and init;
 */
wtk_queue_t *wtk_queue_new(void);

/**
 * @brief free memory of wtk_queue_t;
 */
int wtk_queue_delete(wtk_queue_t *q);

/**
 * @brief init queue struct;
 */
int wtk_queue_init2(wtk_queue_t *q);

/**
 * @brief push node to the tail of the queue;
 */
int wtk_queue_push(wtk_queue_t *q, wtk_queue_node_t *n);

/**
 *  @brief pop the head node from queue;
 */
wtk_queue_node_t *wtk_queue_pop(wtk_queue_t *q);

#ifdef __cplusplus
};
#endif

#endif

