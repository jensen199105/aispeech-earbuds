/**
 * Project  : AIWakeup.v1.0.7
 * FileName : third/wtk/wakeup/dnn/wtk_wakeup_dnn_cfg.h
 *
 * COPYRIGHT (C) 2014, AISpeech Ltd.. All rights reserved.
 */
#ifndef __AISPEECH__WTK_WAKEUP_DNN__WTK_WAKEUP_DNN_CFG_H__
#define __AISPEECH__WTK_WAKEUP_DNN__WTK_WAKEUP_DNN_CFG_H__

#include "wtk/vite/parm/wtk_parm.h"
#ifndef WKP_VAD_DISABLE
#include "wtk/vad/wtk_vad2_cfg.h"
#endif

typedef struct wtk_wakeup_dnn_wordlist    wtk_wakeup_dnn_wordlist_t;

typedef struct wtk_wakeup_dnn_cfg
{
    const char **dict;
    short n_dict;
    wtk_parm_cfg_t parm;
#ifndef WKP_VAD_DISABLE
    wtk_vad2_cfg_t vad;
#endif
    unsigned wsmooth;
#if defined(WAKEUP_DICT_PHONEME) || defined(WAKEUP_DICT_CHAR)
    unsigned wmax;
#endif
#ifdef WAKEUP_DICT_PHONEME
    char use_phoneme;
    const char **char_array;
    short n_char;
    const char **phones_array;
    unsigned wmax_eword;
#endif
    wtk_wakeup_dnn_wordlist_t *wrds; // 默认支持的唤醒词列表
#ifndef WKP_VAD_DISABLE
    unsigned use_vad;
#endif
#ifdef WAKEUP_DUR_PENALTY
    char use_duration_penalty;
    char use_restart_easy_wakeup;
    int score_false_reduce_duration;
    int score_false_reduce_nowakeup_duration;
    int score_easy_wakeup_duration;
    int score_false_reduce_coef; //Q16
    int score_easy_wakeup_coef;  //Q16
    int score_easy_wakeup_nowakeup_coef; //Q16
#endif
#ifdef WAKEUP_XCHECK
    unsigned use_char_check;
    unsigned char_score_min_win;
    unsigned char_score_max_win;
    int *char_score_thresh_array;
#endif
#ifdef WAKEUP_XTERM
    unsigned use_max_conf_search;
#endif
#ifdef WAKEUP_DELAY
    char use_delay_wakeup;
    short delay_wakeup_search0;
    short delay_wakeup_search1;
    int delay_wakeup_thresh_times;
    short delay_wakeup_search_max;
#endif
#ifdef WAKEUP_DICT_WORD
    S8 use_e2e;       // e2e: end-to-end
    S8 wakeup_skip_win;
#endif
} wtk_wakeup_dnn_cfg_t;

int wtk_wakeup_dnn_cfg_init(wtk_wakeup_dnn_cfg_t *cfg);
int wtk_wakeup_dnn_cfg_clean(wtk_wakeup_dnn_cfg_t *cfg);
int wtk_wakeup_dnn_cfg_update_local(wtk_wakeup_dnn_cfg_t *cfg);
int wtk_wakeup_dnn_cfg_update(wtk_wakeup_dnn_cfg_t *cfg);

#endif
