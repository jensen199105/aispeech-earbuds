#ifdef WAKEUP_DUR_PENALTY
#ifndef __WTK_WAKEUP_PENALTY_H__
#define __WTK_WAKEUP_PENALTY_H__

#include "wtk_wakeup_dnn_cfg.h"
#include "wtk_wakeup_dnn_word.h"

typedef struct wtk_wakeup_module_frame_robin
{
    unsigned char *frame;
    int cur;
    int length;
    int used;
    int speech_frames;
} wtk_wakeup_module_frame_robin_t;

typedef struct wtk_wakeup_dnn_word_coef
{
    int score_easy_wakeup_coef;
    int easy_wakeup_prev_score_frame;
    int easy_wakeup_cur_score_frame;
} wtk_wakeup_dnn_word_coef_t;

typedef struct wtk_wakeup_module_duration_penalty
{
    int false_reduce_all_frames;
    int false_reduce_vad_frames;
    int easy_wakeup_cur_frame;
    wtk_wakeup_module_frame_robin_t *frame_robin;
    char n_coefs;
    wtk_wakeup_dnn_word_coef_t *wrd_coefs;
} wtk_wakeup_dnn_duration_penalty_t;

void wtk_wakeup_dnn_penalty_init(wtk_wakeup_dnn_duration_penalty_t *penalty, int nrobin, int n_coefs);
void wtk_wakeup_dnn_penalty_reset(wtk_wakeup_dnn_duration_penalty_t *penalty);
void wtk_wakeup_dnn_penalty_set(wtk_wakeup_dnn_duration_penalty_t *penalty, int reduce_vframes, int reduce_aframes,
                                int cur_frame);
void wtk_wakeup_dnn_penalty_push(wtk_wakeup_dnn_duration_penalty_t *penalty, int state);
void wtk_wakeup_dnn_penalty_update(wtk_wakeup_dnn_duration_penalty_t *penalty, wtk_parm_cfg_t *parm_cfg,
                                   wtk_wakeup_dnn_cfg_t *cfg, wtk_wakeup_dnn_word_t *wrd, int idx);
wtk_wakeup_module_frame_robin_t *wtk_wakeup_module_frame_robin_new(int n);
void wtk_wakeup_module_frame_robin_push(wtk_wakeup_module_frame_robin_t *r, char value);
void wtk_wakeup_module_frame_robin_reset(wtk_wakeup_module_frame_robin_t *robin);
void wtk_wakeup_module_frame_robin_delete(wtk_wakeup_module_frame_robin_t *r);

#endif
#endif
