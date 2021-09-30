/**
 * Project  : AIWakeup.v1.0.7
 * FileName : third/wtk/wakeup/dnn/wtk_wakeup_dnn_word.c
 *
 * COPYRIGHT (C) 2014, AISpeech Ltd.. All rights reserved.
 */

#include "AISP_TSL_str.h"
#include "wtk_wakeup_dnn_cfg.h"
#include "wtk_wakeup_dnn_word.h"

#define ARRAY_CNT(a) sizeof(a)/sizeof(a[0])

extern const unsigned int AISPEECH_ARRAY_CFG[];

#ifdef WAKEUP_XCHECK
extern const char *char_check_words[];
extern const short char_check_idx[];
#endif

#ifdef WAKEUP_XTERM
extern const char *max_conf_search_words[];
extern const char max_conf_search_win_array[];
extern const char max_conf_search_max_array[];
#endif

#ifdef WAKEUP_DELAY
extern const short delay_wakeup_word_count;
extern const char *delay_wakeup_words[];

void wtk_wakeup_dnn_delay_wakeup_reset(wtk_wakeup_dnn_delay_wakeup_t *pstDelayWakeup)
{
    pstDelayWakeup->iLastConf = PZERO;
    pstDelayWakeup->i1stPeakPos = 0;
    pstDelayWakeup->iPeakConf = PZERO;
    pstDelayWakeup->iMaxFrames = 0;
}
#endif

static int wtk_wakeup_dnn_indexof(const char *array[], int arr_count, const char *data)
{
    int i;

    for (i = 0; i < arr_count; i++)
    {
        if (AISP_TSL_strcmp(array[i], data) == 0)
        {
            return i;
        }
    }

    return -1;
}

static void wakeup_dnn_set_dict_bitsets(wtk_wakeup_dnn_wordlist_t *wl, int id, int *count)
{
    unsigned int *pbitset = &wl->dict_bitsets[id / 32];
    unsigned int bitmask = (unsigned int)1 << (id % 32);

    if (0 == (*pbitset & bitmask))
    {
        *pbitset |= bitmask;
        (*count)++;
    }
}

static int wakeup_dnn_get_dict_bitsets(wtk_wakeup_dnn_cfg_t *cfg, wtk_wakeup_dnn_wordlist_t *wl, wtk_string_t **words,
                                       int nwrd)
{
    char buff[64];
    char *pcToken;
    char *pcCtx;
#ifdef WAKEUP_DICT_PHONEME
    char buff2[64];
    char *pcToken2;
    char *pcCtx2;
#endif
    int id;
    int i;
    int count = 0;
    wl->dict_bitsets[0] |= 1;
    count++;

    for (i = 0; i < nwrd; i++)
    {
        AISP_TSL_memcpy(buff, words[i]->data, words[i]->len);
        buff[words[i]->len - 1] = '\0';
#ifdef WAKEUP_DICT_WORD

        if (cfg->use_e2e)
        {
            pcToken = buff;
            id = wtk_wakeup_dnn_indexof(cfg->dict, cfg->n_dict, pcToken);

            if (id < 0)
            {
                return -1;
            }

            wakeup_dnn_set_dict_bitsets(wl, id, &count);
        }
        else
#endif
        {
            for (pcToken = AISP_TSL_strtok_r(buff, " ", &pcCtx); NULL != pcToken; pcToken = AISP_TSL_strtok_r(NULL, " ", &pcCtx))
            {
#ifdef WAKEUP_DICT_PHONEME

                if (cfg->use_phoneme)
                {
                    id = wtk_wakeup_dnn_indexof(cfg->char_array, cfg->n_char, pcToken);

                    if (id < 0)
                    {
                        return -1;
                    }

                    AISP_TSL_strcpy(buff2, cfg->phones_array[id]);

                    for (pcToken2 = AISP_TSL_strtok_r(buff2, " ", &pcCtx2); NULL != pcToken2;
                            pcToken2 = AISP_TSL_strtok_r(NULL, " ", &pcCtx2))
                    {
                        id = wtk_wakeup_dnn_indexof(cfg->dict, cfg->n_dict, pcToken2);

                        if (id < 0)
                        {
                            return -1;
                        }

                        wakeup_dnn_set_dict_bitsets(wl, id, &count);
                    }
                }
                else
#endif
                {
#ifdef WAKEUP_DICT_CHAR
                    id = wtk_wakeup_dnn_indexof(cfg->dict, cfg->n_dict, pcToken);

                    if (id < 0)
                    {
                        return -1;
                    }

                    wakeup_dnn_set_dict_bitsets(wl, id, &count);
#else
                    return -1;
#endif
                }
            }
        }
    }

    return count;
}

#if defined(WAKEUP_DICT_PHONEME) || defined(WAKEUP_DICT_CHAR)
static void wtk_wakeup_dnn_get_dict_item(wtk_wakeup_dnn_cfg_t *cfg, wtk_wakeup_dnn_wordlist_t *wl, const char *data,
        int *pid, int *pidx)
{
    unsigned int bitset;
    unsigned int bitmask;
    int i;
    *pid = wtk_wakeup_dnn_indexof(cfg->dict, cfg->n_dict, data);
    *pidx = -1;

    for (i = 0; i <= *pid; i++)
    {
        bitset = wl->dict_bitsets[i / 32];
        bitmask = (unsigned int)1 << (i % 32);

        if (bitset & bitmask)
        {
            (*pidx)++;
        }
    }
}

static int wakeup_dnn_get_count(char *str)
{
    char *pcToken;
    char *pcCtx;
    int count = 0;

    for (pcToken = AISP_TSL_strtok_r(str, " ", &pcCtx); NULL != pcToken; pcToken = AISP_TSL_strtok_r(NULL, " ", &pcCtx))
    {
        count++;
    }

    return count;
}
#endif

static int wakeup_dnn_get_wrd_nchars(wtk_wakeup_dnn_cfg_t *cfg, wtk_wakeup_dnn_word_t *wrd)
{
    char buff[64];
    AISP_TSL_memcpy(buff, wrd->word->data, wrd->word->len - 1);
    buff[wrd->word->len - 1] = '\0';
#ifdef WAKEUP_DICT_WORD

    if (cfg->use_e2e)
    {
        return 1;
    }
    else
#endif
#ifdef WAKEUP_DICT_PHONEME
        if (cfg->use_phoneme)
        {
            return wakeup_dnn_get_count(buff);
        }
        else
#endif
        {
#ifdef WAKEUP_DICT_CHAR
            return wakeup_dnn_get_count(buff);
#else
            return -1;
#endif
        }
}

#ifdef WAKEUP_DICT_PHONEME
static int wakeup_dnn_get_char_nphones(wtk_wakeup_dnn_cfg_t *cfg, const char *phoneme)
{
    char buff[64];
    AISP_TSL_strcpy(buff, phoneme);
    return wakeup_dnn_get_count(buff);
}
#endif

#if defined(WAKEUP_DICT_PHONEME) || defined(WAKEUP_DICT_CHAR)
static int wakeup_dnn_parse_wrd(wtk_wakeup_dnn_cfg_t *cfg, wtk_wakeup_dnn_wordlist_t *wl, wtk_wakeup_dnn_word_t *wrd)
{
    char buff[64];
    char *pcToken;
    char *pcCtx;
    int id;
    int j = 0;
#ifdef WAKEUP_DICT_PHONEME
    char buff2[64];
    char *pcToken2;
    char *pcCtx2;
    int jj;
#endif
    int idx;
    AISP_TSL_memcpy(buff, wrd->word->data, wrd->word->len - 1);
    buff[wrd->word->len - 1] = '\0';

    for (pcToken = AISP_TSL_strtok_r(buff, " ", &pcCtx); NULL != pcToken; pcToken = AISP_TSL_strtok_r(NULL, " ", &pcCtx))
    {
#ifdef WAKEUP_DICT_PHONEME

        if (cfg->use_phoneme)
        {
            id = wtk_wakeup_dnn_indexof(cfg->char_array, cfg->n_char, pcToken);
            //calc phoneme count in char
            wrd->ewchars[j].n_wchars = wakeup_dnn_get_char_nphones(cfg, cfg->phones_array[id]);
            wrd->ewchars[j].wchars = (wtk_wakeup_dnn_wchar_t **)wtk_calloc(wrd->ewchars[j].n_wchars,
                                     sizeof(wtk_wakeup_dnn_wchar_t *));
            AISP_TSL_strcpy(buff2, cfg->phones_array[id]);
            jj = 0;

            for (pcToken2 = AISP_TSL_strtok_r(buff2, " ", &pcCtx2); NULL != pcToken2;
                    pcToken2 = AISP_TSL_strtok_r(NULL, " ", &pcCtx2))
            {
                wtk_wakeup_dnn_get_dict_item(cfg, wl, pcToken2, &id, &idx);
                wl->wchars[idx].id = id;
                wl->wchars[idx].in_used = 1;
                wrd->ewchars[j].wchars[jj] = &wl->wchars[idx];
                jj++;
            }
        }
        else
#endif
        {
            wtk_wakeup_dnn_get_dict_item(cfg, wl, pcToken, &id, &idx);
            wl->wchars[idx].id = id;
            wl->wchars[idx].in_used = 1;
            wrd->wchars[j] = &wl->wchars[idx];
        }

        j++;
    }

    return 0;
}
#endif

#ifdef WAKEUP_DICT_WORD
int wakeup_dnn_parse_wrd_e2e(wtk_wakeup_dnn_cfg_t *cfg, wtk_wakeup_dnn_wordlist_t *wl, wtk_wakeup_dnn_word_t *wrd,
                             int wrd_index)
{
    int id = wtk_wakeup_dnn_indexof(cfg->dict, cfg->n_dict, wrd->word->data);

    if (id < 0)
    {
        return -1;
    }

    wl->wchars[wrd_index].id        = id;
    wl->wchars[wrd_index].in_used   = 1;
    wrd->wchars[0]  = &wl->wchars[wrd_index];
    return 0;
}

void wtk_wakeup_dnn_wordlist_delete_e2e(wtk_wakeup_dnn_wordlist_t *wl)
{
    int i;
    wtk_smooth_score_t *ss;

    for (i = 0; i < wl->n_wchars; i++)
    {
        ss = &(wl->wchars[i].rawp);
        wtk_cqueue_reset(ss->scores);
        ss->sum = 0;
    }
}
#endif

static int wakeup_dnn_update_expwords(wtk_wakeup_dnn_cfg_t *cfg, wtk_wakeup_dnn_wordlist_t *wl,
                                      wtk_string_t **words, int nwrd, int *thresholds, int nthresh, char *majors, int nmajor)
{
    wtk_wakeup_dnn_word_t *wrd;
    int i;
    int ret;
    wl->n_wrds = nwrd;
    wl->wrds = (wtk_wakeup_dnn_word_t *)wtk_calloc(wl->n_wrds, sizeof(wtk_wakeup_dnn_word_t));

    for (i = 0; i < nwrd; i++)
    {
        wrd = &wl->wrds[i];
        wrd->word = words[i];
        wrd->n_wchars = 0;
        wrd->conf = PZERO;
        wrd->threshold = thresholds[i];
        wrd->major = majors ? majors[i] : 0;
        //calc char count in word
        wrd->n_wchars = wakeup_dnn_get_wrd_nchars(cfg, wrd);
        //alloc char space for word
#ifdef WAKEUP_DICT_PHONEME

        if (cfg->use_phoneme)
        {
            wrd->ewchars = (wtk_wakeup_dnn_eword_char_t *)wtk_calloc(wrd->n_wchars, sizeof(wtk_wakeup_dnn_eword_char_t));
        }
        else
#endif
        {
#if defined(WAKEUP_DICT_CHAR) || defined(WAKEUP_DICT_WORD)
            wrd->wchars = (wtk_wakeup_dnn_wchar_t **)wtk_calloc(wrd->n_wchars, sizeof(wtk_wakeup_dnn_wchar_t *));
#else
            return -1;
#endif
        }

#ifdef WAKEUP_DICT_WORD

        if (cfg->use_e2e)
        {
            ret = wakeup_dnn_parse_wrd_e2e(cfg, wl, wrd, i + 1);
        }
        else
#endif
        {
#if defined(WAKEUP_DICT_PHONEME) || defined(WAKEUP_DICT_CHAR)
            ret = wakeup_dnn_parse_wrd(cfg, wl, wrd);
#else
            return -1;
#endif
        }

        if (0 != ret)
        {
            return -1;
        }

#if defined(WAKEUP_XCHECK) || defined(WAKEUP_XTERM) || defined(WAKEUP_DELAY)
        {
            int j;
#ifdef WAKEUP_XCHECK
            //TODO: replace AISPEECH_ARRAY_CFG[6] with a var
            wrd->pstCharCheck = NULL;

            //char check not support for phoneme currently
            for (j = 0;
#ifdef WAKEUP_DICT_PHONEME
                    !cfg->use_phoneme &&
#endif
                    cfg->use_char_check && j < AISPEECH_ARRAY_CFG[6]; j++)
            {
                if (!AISP_TSL_strcmp(wrd->word->data, char_check_words[j]))
                {
                    int p;
                    wrd->pstCharCheck = (wtk_wakeup_dnn_char_check_t *)wtk_calloc(1, sizeof(wtk_wakeup_dnn_char_check_t));
                    wrd->pstCharCheck->sOffset = char_check_idx[j];
                    wrd->pstCharCheck->wscores = (wtk_wakeup_dnn_ringbuff_t *)wtk_calloc(wrd->n_wchars, sizeof(wtk_wakeup_dnn_ringbuff_t));

                    for (p = 0; p < wrd->n_wchars; p++)
                    {
                        wrd->pstCharCheck->wscores[p].length = (wrd->n_wchars - p - 1) * cfg->char_score_max_win > 0 ?
                                                               (wrd->n_wchars - p - 1) * cfg->char_score_max_win : 1;
                        wrd->pstCharCheck->wscores[p].cur = 0;
                        wrd->pstCharCheck->wscores[p].arr = (int *)wtk_malloc(wrd->pstCharCheck->wscores[p].length * sizeof(int));
                    }

                    break;
                }
            }

#endif
#ifdef WAKEUP_XTERM
            //TODO: replace AISPEECH_ARRAY_CFG[8] with a var
            wrd->pstMaxConfSearch = NULL;

            for (j = 0; cfg->use_max_conf_search && j < AISPEECH_ARRAY_CFG[8]; j++)
            {
                if (!AISP_TSL_strcmp(wrd->word->data, max_conf_search_words[j]))
                {
                    wrd->pstMaxConfSearch = (wtk_wakeup_dnn_max_conf_search_t *)wtk_calloc(1, sizeof(wtk_wakeup_dnn_max_conf_search_t));
                    wrd->pstMaxConfSearch->cSearchWin = max_conf_search_win_array[j];
                    wrd->pstMaxConfSearch->cSearchMax = max_conf_search_max_array[j];
                    break;
                }
            }

#endif
#ifdef WAKEUP_DELAY
            wrd->pstDelayWakeup = NULL;

            for (j = 0; cfg->use_delay_wakeup && j < delay_wakeup_word_count; j++)
            {
                if (!AISP_TSL_strcmp(wrd->word->data, delay_wakeup_words[j]))
                {
                    wrd->pstDelayWakeup = (wtk_wakeup_dnn_delay_wakeup_t *)wtk_malloc(sizeof(wtk_wakeup_dnn_delay_wakeup_t));
                    wtk_wakeup_dnn_delay_wakeup_reset(wrd->pstDelayWakeup);
                    break;
                }
            }

#endif
        }
#endif
    }

    return 0;
}

wtk_wakeup_dnn_wordlist_t *wtk_wakeup_dnn_wordlist_new(wtk_wakeup_dnn_cfg_t *cfg, wtk_string_t **words,
        int nwrd, int *thresholds, int nthresh, char *majors, int nmajor)
{
    wtk_wakeup_dnn_wordlist_t *wl;
    int ret = -1;
    wl = (wtk_wakeup_dnn_wordlist_t *)wtk_malloc(sizeof(wtk_wakeup_dnn_wordlist_t));

    if (wl == NULL)
    {
        goto end;
    }

    wl->dict_bitsets = wtk_calloc((cfg->n_dict + 31) / 32, sizeof(unsigned int));
    wl->n_wchars = wakeup_dnn_get_dict_bitsets(cfg, wl, words, nwrd);

    if (wl->n_wchars <= 0)
    {
        goto end;
    }

    wl->wchars = (wtk_wakeup_dnn_wchar_t *)wtk_calloc(wl->n_wchars, sizeof(wtk_wakeup_dnn_wchar_t));
    ret = wakeup_dnn_update_expwords(cfg, wl, words, nwrd, thresholds, nthresh, majors, nmajor);
end:

    if (ret != 0)
    {
        if (wl != NULL)
        {
            wtk_free(wl);
        }

        return NULL;
    }

    return wl;
}

#if defined(WAKEUP_DICT_CHAR) || defined(WAKEUP_DICT_WORD)
int wtk_wakeup_dnn_wordlist_alloc(wtk_wakeup_dnn_wordlist_t *wl, int win)
{
    if (wl && wl->n_wchars != 0)
    {
        int i;
        wl->n_slot = win;

        for (i = 0; i < wl->n_wchars; i++)
        {
            wtk_wakeup_dnn_wchar_t *wchar = &wl->wchars[i];
#ifdef WAKEUP_DICT_CHAR
            wchar->priorities = (int *)wtk_calloc(win, sizeof(int));
            wchar->smooth_tendency = (int *)wtk_calloc(win, sizeof(int));
            wchar->p1 = (int *)wtk_calloc(win, sizeof(int));
            wchar->p2 = 0;
            wchar->pidx = 0;
#endif
#ifdef WAKEUP_DICT_WORD
            wchar->rawp.sum = 0;
            wchar->rawp.scores = wtk_cqueue_new(wl->n_slot);
#endif
        }

#ifdef WAKEUP_DICT_CHAR

        for (i = 0; i < wl->n_wrds; i++)
        {
            wtk_wakeup_dnn_word_t *wrd = &wl->wrds[i];
            int j;
            wrd->smthp = (int **)wtk_calloc(wrd->n_wchars, sizeof(int *));
            wrd->p1 = (int **)wtk_calloc(wrd->n_wchars, sizeof(int *));
            wrd->p2 = (int *)wtk_calloc(wrd->n_wchars, sizeof(int));
            wrd->pidx = (int *)wtk_calloc(wrd->n_wchars, sizeof(int));

            for (j = 0; j < wrd->n_wchars; j++)
            {
                wrd->smthp[j] = (int *)wtk_calloc(win, sizeof(int));
                wrd->p1[j] = (int *)wtk_calloc(win, sizeof(int));
                wrd->p2[j] = PZERO;
            }
        }

#endif
        return 0;
    }
    else
    {
        return -1;
    }
}

void wtk_wakeup_dnn_wordlist_delete(wtk_wakeup_dnn_wordlist_t *wl)
{
#ifdef WAKEUP_DICT_CHAR
    int iNWords = 0;
    int iNChars = 0;
    int iLen    = 0;
    int i;

    if (NULL == wl || NULL == wl->wrds)
    {
        return;
    }

    iLen = wl->n_slot * sizeof(int);
    iNWords = wl->n_wrds;

    for (i = 0; i < iNWords; ++i)
    {
        int j;
        iNChars = wl->wrds[i].n_wchars;

        for (j = 0; j < iNChars; ++j)
        {
            AISP_TSL_memset(wl->wrds[i].smthp[j], 0, iLen);
            AISP_TSL_memset(wl->wrds[i].p1[j], 0, iLen);
            wl->wrds[i].p2[j] = PZERO;
        }

        AISP_TSL_memset(wl->wrds[i].pidx, 0, iNChars * sizeof(int));
    }

    iNChars = wl->n_wchars;

    for (i = 0; i < iNChars; i++)
    {
        AISP_TSL_memset(wl->wchars[i].priorities, 0, iLen);
        AISP_TSL_memset(wl->wchars[i].p1, 0, iLen);
        AISP_TSL_memset(wl->wchars[i].smooth_tendency, 0, iLen);
        wl->wchars[i].p2 = 0;
        wl->wchars[i].pidx = 0;
    }

#endif
}
#endif

#ifdef WAKEUP_DICT_PHONEME
int wtk_wakeup_dnn_wordlist_alloc_phoneme(wtk_wakeup_dnn_wordlist_t *wl, int win, int win_eword)
{
    wtk_wakeup_dnn_word_t *wrd;
    wtk_wakeup_dnn_wchar_t *wchar;
    int i;
    int k = 0;
    int j = 0;
    wl->n_slot = win;
    wl->nslot_eword = win_eword;

    if (wl && wl->n_wchars != 0)
    {
        for (i = 0; i < wl->n_wchars; i++)
        {
            wchar = &wl->wchars[i];
            wchar->priorities = (int *)wtk_calloc(win, sizeof(int));
            wchar->smooth_tendency = (int *)wtk_calloc(WORD_MAX_CHAR * CHAR_MAX_FRAME, sizeof(int));
            wchar->p1 = (int *)wtk_calloc(win, sizeof(int));
            wchar->p2 = 0;
            wchar->pidx = 0;
        }

        for (i = 0; i < wl->n_wrds; i++)
        {
            wrd = &wl->wrds[i];
            wrd->smthp = (int **)wtk_calloc(wrd->n_wchars, sizeof(int *));
            wrd->p1 = (int **)wtk_calloc(wrd->n_wchars, sizeof(int *));
            wrd->p2 = (int *)wtk_calloc(wrd->n_wchars, sizeof(int));
            wrd->pidx = (int *)wtk_calloc(wrd->n_wchars, sizeof(int));

            for (j = 0; j < wrd->n_wchars; j++)
            {
                wrd->smthp[j] = (int *)wtk_calloc(win_eword, sizeof(int));
                wrd->p1[j] = (int *)wtk_calloc(win_eword, sizeof(int));
                wrd->p2[j] = PZERO;
                wrd->ewchars[j].smthp = (int **)wtk_calloc(wrd->ewchars[j].n_wchars, sizeof(int *));
                wrd->ewchars[j].p1 = (int **)wtk_calloc(wrd->ewchars[j].n_wchars, sizeof(int *));
                wrd->ewchars[j].p2 = (int *)wtk_calloc(wrd->ewchars[j].n_wchars, sizeof(int));
                wrd->ewchars[j].pidx = (int *)wtk_calloc(wrd->ewchars[j].n_wchars, sizeof(int));

                for (k = 0; k < wrd->ewchars[j].n_wchars; k++)
                {
                    wrd->ewchars[j].smthp[k] = (int *)wtk_calloc(win, sizeof(int));
                    wrd->ewchars[j].p1[k] = (int *)wtk_calloc(win, sizeof(int));
                }
            }
        }
    }

    return 0;
}

void wtk_wakeup_dnn_wordlist_delete_phoneme(wtk_wakeup_dnn_wordlist_t *wl)
{
    wtk_wakeup_dnn_word_t *wrd;
    int i;
    int j;
    int k;

    if (NULL == wl || NULL == wl->wrds)
    {
        return;
    }

    for (i = 0; i < wl->n_wrds; i++)
    {
        wrd = &wl->wrds[i];

        for (j = 0; j < wrd->n_wchars; j++)
        {
            for (k = 0; k < wrd->ewchars[j].n_wchars; k++)
            {
                AISP_TSL_memset(wrd->ewchars[j].smthp[k], 0, wl->n_slot * sizeof(int));
                AISP_TSL_memset(wrd->ewchars[j].p1[k], 0, wl->n_slot * sizeof(int));
            }

            AISP_TSL_memset(wrd->ewchars[j].p2, 0, wrd->ewchars[j].n_wchars * sizeof(int));
            AISP_TSL_memset(wrd->ewchars[j].pidx, 0, wrd->ewchars[j].n_wchars * sizeof(int));
            wrd->p2[j] = PZERO;
            AISP_TSL_memset(wrd->p1[j], 0, wl->nslot_eword * sizeof(int));
            AISP_TSL_memset(wrd->smthp[j], 0, wl->nslot_eword * sizeof(int));
        }

        AISP_TSL_memset(wrd->pidx, 0, wrd->n_wchars * sizeof(int));
    }

    for (i = 0; i < wl->n_wchars; i++)
    {
        AISP_TSL_memset(wl->wchars[i].priorities, 0, wl->n_slot * sizeof(int));
        AISP_TSL_memset(wl->wchars[i].p1, 0, wl->n_slot * sizeof(int));
        AISP_TSL_memset(wl->wchars[i].smooth_tendency, 0, WORD_MAX_CHAR * CHAR_MAX_FRAME * sizeof(int));
        wl->wchars[i].p2 = 0;
        wl->wchars[i].pidx = 0;
    }
}
#endif

void wtk_wakeup_dnn_wordlist_reset(wtk_wakeup_dnn_wordlist_t *wl)
{
#if defined(WAKEUP_DICT_PHONEME) || defined(WAKEUP_DICT_CHAR)
    wtk_wakeup_dnn_wchar_t *wchar;
    int i;
#ifdef WAKEUP_DICT_PHONEME
    int j;
#endif

    if (NULL == wl)
    {
        return;
    }

    for (i = 0; i < wl->n_wchars; i++)
    {
        wchar = &wl->wchars[i];
        wchar->priorities = NULL;
        wchar->smooth_tendency = NULL;
        wchar->p1 = NULL;
    }

    for (i = 0; i < wl->n_wrds; i++)
    {
        wtk_wakeup_dnn_word_t *wrd = &wl->wrds[i];
#ifdef WAKEUP_DICT_PHONEME

        for (j = 0; j < wrd->n_wchars; j++)
        {
            wrd->ewchars[j].p1 = 0;
            wrd->ewchars[j].p2 = 0;
            wrd->ewchars[j].pidx = 0;
            wrd->ewchars[j].n_wchars = 0;
        }

#endif
        wrd->smthp = NULL;
        wrd->p2 = NULL;
        wrd->pidx = NULL;
    }

#endif
}

void wtk_wakeup_dnn_wordlist_update_thresh(wtk_wakeup_dnn_wordlist_t *wl, int *thresholds)
{
    int i = 0;

    for (i = 0; i < wl->n_wrds; i++)
    {
        wl->wrds[i].threshold = thresholds[i];
    }
}
