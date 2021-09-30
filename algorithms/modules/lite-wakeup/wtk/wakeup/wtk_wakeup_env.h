/**
 * Project  : AIWakeup.v1.0.7
 * FileName : third/wtk/wakeup/wtk_wakeup_env.h
 *
 * COPYRIGHT (C) 2014, AISpeech Ltd.. All rights reserved.
 */
#ifndef __AISPEECH__WTK_WAKEUP__WTK_WAKEUP_ENV_H__
#define __AISPEECH__WTK_WAKEUP__WTK_WAKEUP_ENV_H__

#include "wtk/core/wtk_str.h"
#include "api_wakeup.h"

typedef struct wtk_wakeup_env wtk_wakeup_env_t;

struct wtk_wakeup_env
{
    wtk_string_t **words;
    int *thresholds;        // 根据 thresh_type 指向下方的某个thresh数组
    int *threshs_default;   // [nthresh]
    int *threshs_wind;      // [nthresh]
    char *majors;

    char nwrd;
    char nthresh;
    char nmajor;

    thresh_type_e thresh_type;
};

wtk_wakeup_env_t *wtk_wakeup_env_new(void);
void wtk_wakeup_env_delete(wtk_wakeup_env_t *env);
void wtk_wakeup_env_reset(wtk_wakeup_env_t *env);

int wtk_wakeup_env_parse(wtk_wakeup_env_t *env, char *str, int bytes);
int wtk_wakeup_set_thresh(wtk_wakeup_env_t *env, int thresh_type);

#endif //__AISPEECH__WTK_WAKEUP__WTK_WAKEUP_ENV_H__
