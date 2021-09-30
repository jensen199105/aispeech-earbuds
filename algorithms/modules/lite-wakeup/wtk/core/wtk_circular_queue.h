#ifndef WTK_CORE_WTK_CIRCULAR_QUEUE_H_
#define WTK_CORE_WTK_CIRCULAR_QUEUE_H_

#include "wtk/core/wtk_type.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_cqueue wtk_cqueue_t;

// circular queue
struct wtk_cqueue
{
    U16 size;       // 最大容量, 一旦初始化, 不可更改
    U16 used;       // 实际存储的数据个数
    U16 head;       // position of the first valid data
    U16 tail;       // position after the last valid data
    S32 *data;      // 数组头指针, 实际数据紧跟其后
};

wtk_cqueue_t *wtk_cqueue_new(U16 n);

/* 所有 wtk_cqueue_xx(wtk_cqueue_t* q, ...) 函数
   由调用函数保证q非空,          函数内部不做非空判断 */
void          wtk_cqueue_free(wtk_cqueue_t *q);
void          wtk_cqueue_push(wtk_cqueue_t *q, S32 data);
S32           wtk_cqueue_pop(wtk_cqueue_t *q);
S32           wtk_cqueue_front(wtk_cqueue_t *q);
S32           wtk_cqueue_back(wtk_cqueue_t *q);
void          wtk_cqueue_reset(wtk_cqueue_t *q);

#define WTK_CQUEUE_FULL(pointer) (((wtk_cqueue_t *)(pointer))->used == ((wtk_cqueue_t *)(pointer))->size)
#define WTK_CQUEUE_EMPTY(pointer) (0 == ((wtk_cqueue_t *)(pointer))->used)

#ifdef __cplusplus
};
#endif
#endif
