/**
 * Project  : AIWakeup.v1.0.7
 * FileName : third/wtk/wakeup/dnn/wtk_wakeup_dnn.h
 *
 * COPYRIGHT (C) 2014, AISpeech Ltd.. All rights reserved.
 */
#ifndef __AISPEECH__WTK_WAKEUP_DNN__WTK_WAKEUP_DNN_H__
#define __AISPEECH__WTK_WAKEUP_DNN__WTK_WAKEUP_DNN_H__

#include "wtk/wakeup/wtk_wakeup_env.h"
#include "wtk_wakeup_dnn_cfg.h"
#include "wtk_wakeup_dnn_word.h"
#include "wtk_wakeup_dnn_penalty.h"
#ifndef WKP_VAD_DISABLE
#include "wtk/vad/wtk_vad2.h"
#endif

#ifndef WKP_VAD_DISABLE
typedef int (*dnn_vad_handler_t)(void *user_data, int frame_status, int ifrmIdx);
#endif

typedef enum wtk_wakeup_vad_status
{
    WAKEUP_VAD_INIT,
    WAKEUP_VAD_START,
    WAKEUP_VAD_END,
} wtk_wakeup_vad_status_t;

typedef struct wtk_wakeup_dnn_result
{
    wtk_string_t *waked_word;
    S64 llFrameIndex;
    char major;
    int conf;
    char waked;
} wtk_wakeup_dnn_result_t;

typedef struct wtk_wakeup_dnn
{
    char use_env;
    wtk_wakeup_dnn_cfg_t *cfg;
    wtk_wakeup_dnn_wordlist_t *wrds;
    wtk_parm_t *parm;
    wtk_queue_t feature_q;
    wtk_wakeup_env_t *env;
    int cur;
    unsigned nslot;
#ifdef WAKEUP_DICT_PHONEME
    unsigned nslot_eword;
#endif
#ifndef WKP_VAD_DISABLE
    wtk_vad2_t *vad;
    wtk_queue_t vad_q;
    wtk_wakeup_vad_status_t vad_status;
    vad_handler_t dnn_vad_post;
#endif
#ifdef WAKEUP_DUR_PENALTY
    wtk_wakeup_dnn_duration_penalty_t duration_penalty;
#endif
    int score;
    void *hook;
    wtk_wakeup_dnn_result_t result;
} wtk_wakeup_dnn_t;

wtk_wakeup_dnn_t *wtk_wakeup_dnn_new(wtk_wakeup_dnn_cfg_t *cfg);
void wtk_wakeup_dnn_delete(wtk_wakeup_dnn_t *w);
void wtk_wakeup_dnn_reset(wtk_wakeup_dnn_t *w);
int wtk_wakeup_dnn_start(wtk_wakeup_dnn_t *w, wtk_wakeup_env_t *env);
int wtk_wakeup_dnn_end(wtk_wakeup_dnn_t *w);
int wtk_wakeup_dnn_feed(wtk_wakeup_dnn_t *w, char *data, int bytes, int is_end);
wtk_wakeup_dnn_result_t *wtk_wakeup_dnn_get_result(wtk_wakeup_dnn_t *w);

#endif
