#ifdef WAKEUP_XTERM
#ifndef __WTK_WAKEUP_DNN_XTERM_H__
#define __WTK_WAKEUP_DNN_XTERM_H__

typedef struct wtk_frv              wtk_frv_t;
typedef struct wtk_wakeup_dnn       wtk_wakeup_dnn_t;
typedef struct wtk_wakeup_dnn_word  wtk_wakeup_dnn_word_t;

typedef struct wtk_wakeup_dnn_max_conf_search
{
    char cMaxSearching;
    char cSearchWin;
    char cSearchMax;
    unsigned int uiSearchIndex;
    unsigned int uiSearchTotalNum;
    int conf;
    S64 llFrameIndex;
} wtk_wakeup_dnn_max_conf_search_t;

void wtk_wakeup_dnn_max_conf_search_reset(wtk_wakeup_dnn_max_conf_search_t *pstMaxConfSearch);
int wtk_wakeup_search_max_score(wtk_wakeup_dnn_t *w, wtk_wakeup_dnn_word_t *wrd, wtk_frv_t *frv, S64 *pllFrameIndex,
                                int *conf);

#endif
#endif
