#ifdef WAKEUP_XTERM

#include "wtk/vite/parm/wtk_feature.h"
#include "wtk_wakeup_dnn_word.h"
#include "wtk_wakeup_dnn.h"
#include "wtk_wakeup_dnn_xterm.h"

void wtk_wakeup_dnn_max_conf_search_reset(wtk_wakeup_dnn_max_conf_search_t *pstMaxConfSearch)
{
    pstMaxConfSearch->cMaxSearching = 0;
    pstMaxConfSearch->uiSearchIndex = 0;
    pstMaxConfSearch->uiSearchTotalNum = 0;
    pstMaxConfSearch->conf = 0;
    pstMaxConfSearch->llFrameIndex = 0;
}

int wtk_wakeup_search_max_score(wtk_wakeup_dnn_t *w, wtk_wakeup_dnn_word_t *wrd, wtk_frv_t *frv, S64 *pllFrameIndex,
                                int *conf)
{
    wtk_wakeup_dnn_max_conf_search_t *pstMaxConfSearch = wrd->pstMaxConfSearch;
    pstMaxConfSearch->uiSearchTotalNum++;

    if (!pstMaxConfSearch->uiSearchIndex)
    {
        pstMaxConfSearch->cMaxSearching = 1;
        goto backup;
    }
    else
        if (pstMaxConfSearch->uiSearchIndex > pstMaxConfSearch->cSearchWin)
        {
            goto restore;
        }
        else
        {
            if (pstMaxConfSearch->conf >= wrd->conf)
            {
                pstMaxConfSearch->uiSearchIndex++;

                if (pstMaxConfSearch->uiSearchTotalNum >= pstMaxConfSearch->cSearchMax)
                {
                    goto restore;
                }

                return -1;
            }

            goto backup;
        }

    //restore values
restore:
    *pllFrameIndex = pstMaxConfSearch->llFrameIndex;
    *conf = pstMaxConfSearch->conf;
    wtk_wakeup_dnn_max_conf_search_reset(pstMaxConfSearch);
    return 0;
    //backup result
backup:
    pstMaxConfSearch->conf = wrd->conf;
    pstMaxConfSearch->llFrameIndex = w->parm->llAbsFrameIndex;
    pstMaxConfSearch->uiSearchIndex = 1;

    if (pstMaxConfSearch->uiSearchTotalNum >= pstMaxConfSearch->cSearchMax)
    {
        goto restore;
    }

    return -1;
}

#endif