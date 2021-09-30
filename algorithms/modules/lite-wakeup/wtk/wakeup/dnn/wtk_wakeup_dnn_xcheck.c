#ifdef WAKEUP_XCHECK

#include "AISP_TSL_str.h"
#include "wtk_wakeup_dnn_word.h"
#include "wtk_wakeup_dnn.h"
#include "wtk_wakeup_dnn_xcheck.h"

void wtk_wakeup_dnn_xcheck_reset(wtk_wakeup_dnn_word_t *wrd)
{
    int i;

    for (i = 0; i < wrd->n_wchars; i++)
    {
        wrd->pstCharCheck->wscores[i].cur = 0;
    }
}

int wtk_wakeup_dnn_xcheck(wtk_wakeup_dnn_t *w, wtk_wakeup_dnn_word_t *wrd, int offset)
{
    int i = 0;
    int j = 0;
    int cur = 0;
    int len = 0;
    int cur_max = 0;
    int iRealChars = wrd->n_wchars - 1;
    unsigned int uiCharScoreMinWin = w->cfg->char_score_min_win;
    int iArr = 0;
    int *threshs = NULL;
    wtk_wakeup_dnn_ringbuff_t *rbuff = NULL;
    threshs = w->cfg->char_score_thresh_array + offset;
    /* the last char of wakeuped word */
    rbuff = &wrd->pstCharCheck->wscores[iRealChars];
    cur = rbuff->cur;

    if (rbuff->arr[0] < threshs[iRealChars])
    {
        return -1;
    }

    for (i = iRealChars; i > 0; i--)
    {
        cur_max = 0;

        if (cur < uiCharScoreMinWin)
        {
            return -1;
        }

        rbuff--;

        if (cur < rbuff->length)
        {
            len = cur - uiCharScoreMinWin;
        }
        else
        {
            len = w->cfg->char_score_max_win - uiCharScoreMinWin;
        }

        j = cur - uiCharScoreMinWin;

        while (len-- > 0)
        {
            --j;
            iArr = rbuff->arr[j % rbuff->length];

            if (iArr > cur_max)
            {
                cur_max = iArr;
                cur = j;
            }
        }

        if (cur_max < threshs[i - 1])
        {
            return -1;
        }
    }

    return 0;
}

#endif