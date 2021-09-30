/**
 * Project  : AIWakeup.v1.0.7
 * FileName : third/wtk/wakeup/dnn/wtk_wakeup_dnn_word.h
 *
 * COPYRIGHT (C) 2014, AISpeech Ltd.. All rights reserved.
 */
#ifndef __AISPEECH__WTK_FST_WAKEUP_DNN__WTK_WAKEUP_DNN_WORD_H__
#define __AISPEECH__WTK_FST_WAKEUP_DNN__WTK_WAKEUP_DNN_WORD_H__

#include "wtk/core/wtk_str.h"
#include "wtk/core/wtk_circular_queue.h"
#include "wtk_wakeup_dnn_xcheck.h"
#include "wtk_wakeup_dnn_xterm.h"

#define CHAR_MAX_FRAME      (20)
#define WORD_MAX_CHAR       (10)

typedef struct wtk_wakeup_dnn_cfg wtk_wakeup_dnn_cfg_t;

#ifdef WAKEUP_DICT_WORD
typedef struct wtk_smooth_score
{
    S64 sum;
    wtk_cqueue_t *scores;
} wtk_smooth_score_t;
#endif

#ifdef WAKEUP_DELAY
typedef struct wtk_wakeup_dnn_delay_wakeup
{
    int iLastConf;
    int i1stPeakPos;
    int iPeakConf;
    int iMaxFrames;
} wtk_wakeup_dnn_delay_wakeup_t;
#endif

typedef struct wtk_wakeup_dnn_wchar
{
#if defined(WAKEUP_DICT_PHONEME) || defined(WAKEUP_DICT_CHAR)
    int *priorities;                                        // orig score [wsmooth]
    int *p1;                                                // smooth score [wsmooth]
    int *smooth_tendency;                                   // smooth
    int p2;
    int pidx;
#endif
    short id;                                               // output-idx of dnn output layer, index in dict
    char in_used;
#ifdef WAKEUP_DICT_WORD
    wtk_smooth_score_t rawp;                                // raw_probility.
#endif
} wtk_wakeup_dnn_wchar_t;

#ifdef WAKEUP_DICT_PHONEME
typedef struct wtk_wakeup_dnn_eword_char
{
    wtk_wakeup_dnn_wchar_t **wchars;
    int **smthp;
    int **p1;
    int n_wchars;
    int *p2;                                                // [n_wchars]
    int *pidx;                                              // [n_wchars]
} wtk_wakeup_dnn_eword_char_t;
#endif

typedef struct wtk_wakeup_dnn_word
{
    wtk_string_t *word;
#ifdef WAKEUP_DICT_PHONEME
    wtk_wakeup_dnn_eword_char_t *ewchars;
#endif
    wtk_wakeup_dnn_wchar_t **wchars;
#if defined(WAKEUP_DICT_PHONEME) || defined(WAKEUP_DICT_CHAR)
    int **smthp;                                            // [n_wchars][win]
    int **priorities;                                       // [n_wchars][win]
    int **p1;                                               // [n_wchars][win]
    int *p2;                                                // [n_wchars]
    int *pidx;                                              // [n_wchars]
#endif
    int threshold;
    char major;
    int conf;
    int n_wchars;

#ifdef WAKEUP_XTERM
    wtk_wakeup_dnn_max_conf_search_t *pstMaxConfSearch;
#endif
#ifdef WAKEUP_XCHECK
    wtk_wakeup_dnn_char_check_t *pstCharCheck;
#endif
#ifdef WAKEUP_DELAY
    wtk_wakeup_dnn_delay_wakeup_t *pstDelayWakeup;
#endif
} wtk_wakeup_dnn_word_t;

typedef struct wtk_wakeup_dnn_wordlist
{
    unsigned int *dict_bitsets;
    wtk_wakeup_dnn_wchar_t *wchars;
    wtk_wakeup_dnn_word_t *wrds;
    short n_wchars;                                         // word used in dict, include "sil"
    char n_wrds;
    unsigned char n_slot;
#ifdef WAKEUP_DICT_PHONEME
    unsigned char nslot_eword;
#endif
} wtk_wakeup_dnn_wordlist_t;

#ifdef WAKEUP_DELAY
void wtk_wakeup_dnn_delay_wakeup_reset(wtk_wakeup_dnn_delay_wakeup_t *pstDelayWakeup);
#endif

wtk_wakeup_dnn_wordlist_t *wtk_wakeup_dnn_wordlist_new(wtk_wakeup_dnn_cfg_t *cfg, wtk_string_t **words, int nwrd,
        int *thresholds, int nthresh, char *majors, int nmajor);
void wtk_wakeup_dnn_wordlist_reset(wtk_wakeup_dnn_wordlist_t *wl);

#if defined(WAKEUP_DICT_CHAR) || defined(WAKEUP_DICT_WORD)
int wtk_wakeup_dnn_wordlist_alloc(wtk_wakeup_dnn_wordlist_t *wl, int win);
#endif

#ifdef WAKEUP_DICT_PHONEME
int wtk_wakeup_dnn_wordlist_alloc_phoneme(wtk_wakeup_dnn_wordlist_t *wl, int win, int win_eword);
void wtk_wakeup_dnn_wordlist_delete_phoneme(wtk_wakeup_dnn_wordlist_t *wl);
#endif

#ifdef WAKEUP_DICT_CHAR
void wtk_wakeup_dnn_wordlist_delete(wtk_wakeup_dnn_wordlist_t *wl);
#endif

#ifdef WAKEUP_DICT_WORD
void wtk_wakeup_dnn_wordlist_delete_e2e(wtk_wakeup_dnn_wordlist_t *wl);
#endif

void wtk_wakeup_dnn_wordlist_update_thresh(wtk_wakeup_dnn_wordlist_t *wl, int *thresholds);

#endif
