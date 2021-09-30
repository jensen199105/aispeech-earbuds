#ifdef WAKEUP_XCHECK
#ifndef __WTK_WAKEUP_DNN_XCHECK_H__
#define __WTK_WAKEUP_DNN_XCHECK_H__

typedef struct wtk_wakeup_dnn       wtk_wakeup_dnn_t;
typedef struct wtk_wakeup_dnn_word  wtk_wakeup_dnn_word_t;

typedef struct wtk_wakeup_dnn_ringbuff
{
    int *arr;
    int length;
    int cur;
} wtk_wakeup_dnn_ringbuff_t;

typedef struct wtk_wakeup_dnn_char_check
{
    short sOffset;
    wtk_wakeup_dnn_ringbuff_t *wscores;
} wtk_wakeup_dnn_char_check_t;

void wtk_wakeup_dnn_xcheck_reset(wtk_wakeup_dnn_word_t *wrd);
int wtk_wakeup_dnn_xcheck(wtk_wakeup_dnn_t *w, wtk_wakeup_dnn_word_t *wrd, int offset);

#endif
#endif
