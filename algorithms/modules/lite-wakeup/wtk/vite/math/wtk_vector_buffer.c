#include "wtk_vector_buffer.h"
#include "AISP_TSL_str.h"
wtk_short_vector_buffer_t *wtk_short_vector_buffer_new(int size)
{
    wtk_short_vector_buffer_t *b;
    int t;
    t = wtk_round_8(sizeof(*b)) + size * sizeof(short);
    b = (wtk_short_vector_buffer_t *)wtk_malloc(t);
    b->odd = 0;
    b->rstart = b->cur = b->start = (short *)(((char *)b) + wtk_round_8((sizeof(*b))));
    b->end = b->rstart + size;
    return b;
}

int wtk_short_vector_buffer_delete(wtk_short_vector_buffer_t *b)
{
    wtk_free(b);
    return 0;
}

int wtk_short_vector_buffer_reset(wtk_short_vector_buffer_t *b)
{
    b->odd = 0;
    b->rstart = b->cur = b->start = (short *)(((char *)b) + wtk_round_8((sizeof(*b))));
    return 0;
}

// 尽可能多的将data的数据拷贝到v中
// 返回成功拷贝的 sample 个数
// 注：一个sample点, 占2个字节
static int wtk_short_vector_buffer_push(wtk_short_vector_buffer_t *v, short *data, int samples)
{
    short *start  = data;
    short *end    = start + samples;
    short *dst    = v->cur;
    short *dstEnd = v->end;

    while ((start < end) && (dst < dstEnd))
    {
        *(dst++) = *(start++);
    }

    /* update v->cur */
    v->cur = dst;
    return start - data;
}

// 返回拷贝到 v 中的字节数
int wtk_short_vector_buffer_push_c(wtk_short_vector_buffer_t *v, char *data, int bytes)
{
    short odd;
    char  *p;
    int   cpy = 0, left, samples;
    left = wtk_vector_buffer_left_samples(v);

    if (left <= 0 || bytes <= 0)
    {
        goto end;
    }

    if (v->odd)
    {
        p      = (char *)&odd;
        p[0]   = v->odd_char;
        p[1]   = data[0];
        bytes -= 1;
        data  += 1;
        v->odd = 0;
        wtk_short_vector_buffer_push(v, &odd, 1);
        cpy   += 1;
    }

    // Jiangyh modified 2019-4-29 samples = bytes/2;
    samples = bytes >> 1;
    left    = wtk_short_vector_buffer_push(v, (short *)data, samples);
    cpy    += left << 1;

    if ((left == samples) && (bytes & 1))
    {
        //pad odd data.
        v->odd_char = data[cpy];
        v->odd      = 1;
        cpy        += 1;
    }

end:
    return cpy;
}

// is_end = 0, 当 b 中的数据达到 frame_size 个时, 拷贝到 v
// is_end = 1, 将 b 中的数据全部拷贝到 v, 并补0到 frame_size
int wtk_short_vector_buffer_peek(wtk_short_vector_buffer_t *b, wtk_short_vector_t *v, int is_end)
{
    int samples;
    int valid_len;
    int i;
    samples   = wtk_short_vector_size(v);
    //samples   = 480;
    valid_len = wtk_vector_buffer_valid_len(b);

    if (!is_end)
    {
        if (valid_len < samples)
        {
            return -1;
        }

        AISP_TSL_memcpy(&(v[1]), b->start, samples * sizeof(short));
        return 0;
    }
    else
    {
        AISP_TSL_memcpy(&(v[1]), b->start, valid_len * sizeof(short));

        for (i = valid_len + 1; i <= samples; ++i)
        {
            v[i] = 0;
        }

        return 0;
    }
}

void wtk_short_vector_buffer_skip(wtk_short_vector_buffer_t *b, int samples, int left_enough)
{
    int size;
    b->start += samples;

    if ((b->end - b->start) < left_enough)
    {
        //in this way,there is no overlap,so use memcpy not memmove.
        size = b->cur - b->start;
        AISP_TSL_memmove(b->rstart, b->start, size * sizeof(short));
        b->start = b->rstart;
        b->cur = b->start + size;
    }
}
