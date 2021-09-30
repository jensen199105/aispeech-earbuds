#ifndef WTK_MATH_WTK_VECTOR_BUFFER_H_
#define WTK_MATH_WTK_VECTOR_BUFFER_H_
#include "wtk_vector.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_short_vector_buffer wtk_short_vector_buffer_t;
#define wtk_vector_buffer_valid_len(b) ((b->cur)-(b->start))
#define wtk_vector_buffer_left_samples(b) ((b->end)-(b->cur))

struct wtk_short_vector_buffer
{
    short *rstart;      // 缓冲区数据头, 一旦初始化,保持不变
    short *start;
    short *cur;         // 下一个数据的存放位置, 空位
    short *end;         // 一旦初始化,保持不变
    char odd_char;
    unsigned odd: 1;
};


wtk_short_vector_buffer_t *wtk_short_vector_buffer_new(int size);
int wtk_short_vector_buffer_push_c(wtk_short_vector_buffer_t *v, char *data, int bytes);
int wtk_short_vector_buffer_peek(wtk_short_vector_buffer_t *b, wtk_short_vector_t *v, int is_end);
void wtk_short_vector_buffer_skip(wtk_short_vector_buffer_t *b, int samples, int left_enough);
int wtk_short_vector_buffer_reset(wtk_short_vector_buffer_t *b);
int wtk_short_vector_buffer_delete(wtk_short_vector_buffer_t *b);
/*
 * @brief push sample data to vector.
 * @return samples that saved into buffer.
 */
#ifdef __cplusplus
};
#endif
#endif
