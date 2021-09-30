/**
 * Project  : AIWakeup.v1.0.7
 * FileName : third/wtk/wakeup/dnn/wtk_wakeup_dnn_cfg.c
 *
 * COPYRIGHT (C) 2014, AISpeech Ltd.. All rights reserved.
 */

#include "AISP_TSL_str.h"
#include "wtk_wakeup_dnn_cfg.h"


extern unsigned AISPEECH_ARRAY_CFG[];

extern const char *dict[];
extern const short dict_count;

#ifdef WAKEUP_DICT_PHONEME
extern const char *char_array[];
extern const short char_count;
extern const char *phones_array[];
extern const char use_phoneme;
extern char wmax_eword;
#endif

#ifdef WAKEUP_DICT_WORD
extern const char use_e2e;
extern const char wakeup_skip_win;
#endif

#ifdef WAKEUP_DUR_PENALTY
extern const char use_duration_penalty;
extern const char use_restart_easy_wakeup;
extern const int score_false_reduce_duration;
extern const int score_false_reduce_nowakeup_duration;
extern const int score_easy_wakeup_duration;
extern const int score_false_reduce_coef;
extern const int score_easy_wakeup_coef;
extern const int score_easy_wakeup_nowakeup_coef;
#endif

#ifdef WAKEUP_XCHECK
extern int char_score_max_win;
extern int char_score_min_win;
extern int char_score_thresh_array[];
#endif

#ifdef WAKEUP_DELAY
extern const char use_delay_wakeup;
extern const short delay_wakeup_search0;
extern const short delay_wakeup_search1;
extern const int delay_wakeup_thresh_times;
extern const short delay_wakeup_search_max;
#endif

int wtk_wakeup_dnn_cfg_init(wtk_wakeup_dnn_cfg_t *cfg)
{
    if (0 != wtk_parm_cfg_init(&cfg->parm))
    {
        return -1;
    }

    cfg->dict = dict;
    cfg->n_dict = dict_count;
    cfg->wsmooth        = AISPEECH_ARRAY_CFG[2];
#if defined(WAKEUP_DICT_PHONEME) || defined(WAKEUP_DICT_CHAR)
    cfg->wmax           = AISPEECH_ARRAY_CFG[3];
#endif
#ifdef WAKEUP_DICT_PHONEME
    cfg->use_phoneme = use_phoneme;
    cfg->char_array = char_array;
    cfg->n_char = char_count;
    cfg->phones_array = phones_array;
    cfg->wmax_eword     = wmax_eword;
#endif
#ifdef WAKEUP_DICT_WORD
    cfg->use_e2e            = use_e2e;
    cfg->wakeup_skip_win    = wakeup_skip_win;
#endif
#ifdef WAKEUP_DELAY
    cfg->use_delay_wakeup = use_delay_wakeup;
    cfg->delay_wakeup_search0 = delay_wakeup_search0;
    cfg->delay_wakeup_search1 = delay_wakeup_search1;
    cfg->delay_wakeup_thresh_times = delay_wakeup_thresh_times;
    cfg->delay_wakeup_search_max = delay_wakeup_search_max;
#endif
#ifndef WKP_VAD_DISABLE
    cfg->use_vad                = AISPEECH_ARRAY_CFG[0];

    if (cfg->use_vad == 1)
    {
        wtk_vad2_cfg_init(&(cfg->vad));
    }

#endif
#ifdef WAKEUP_DUR_PENALTY
    cfg->use_duration_penalty = use_duration_penalty;
    cfg->use_restart_easy_wakeup = use_restart_easy_wakeup;

    if (cfg->use_duration_penalty == 1)
    {
        cfg->score_false_reduce_duration = score_false_reduce_duration;
        cfg->score_false_reduce_nowakeup_duration = score_false_reduce_nowakeup_duration;
        cfg->score_easy_wakeup_duration = score_easy_wakeup_duration;
        cfg->score_false_reduce_coef = score_false_reduce_coef;//0.7f Q26 in ln domain
        cfg->score_easy_wakeup_coef = score_easy_wakeup_coef;//1.5f Q26 in ln domain
        cfg->score_easy_wakeup_nowakeup_coef = score_easy_wakeup_nowakeup_coef;//1.5f Q26 in ln domain
    }

#endif
#ifdef WAKEUP_XCHECK
    cfg->use_char_check = AISPEECH_ARRAY_CFG[5];
    cfg->char_score_min_win = char_score_min_win;
    cfg->char_score_max_win = char_score_max_win;
    cfg->char_score_thresh_array = char_score_thresh_array;
#endif
#ifdef WAKEUP_XTERM
    cfg->use_max_conf_search = AISPEECH_ARRAY_CFG[7];
#endif
    return 0;
}

int wtk_wakeup_dnn_cfg_clean(wtk_wakeup_dnn_cfg_t *cfg)
{
    return wtk_parm_cfg_clean(&cfg->parm);
}

int wtk_wakeup_dnn_cfg_update_local(wtk_wakeup_dnn_cfg_t *cfg)
{
    return wtk_parm_cfg_update_local(&cfg->parm);
}

int wtk_wakeup_dnn_cfg_update(wtk_wakeup_dnn_cfg_t *cfg)
{
    int ret = 0;
    ret = wtk_parm_cfg_update2(&cfg->parm);

    if (ret != 0)
    {
        wtk_debug("update parm failed.\n");
        goto end;
    }

    cfg->wrds = NULL;
end:
    return ret;
}
