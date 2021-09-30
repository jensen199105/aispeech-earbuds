/**
 * Project  : AIWakeup.v1.0.7
 * FileName : third/wtk/wakeup/wtk_wakeup.h
 *
 * COPYRIGHT (C) 2014, AISpeech Ltd.. All rights reserved.
 */
#ifndef __AISPEECH__WTK_FST_WAKEUP__WTK_WAKEUP_H__
#define __AISPEECH__WTK_FST_WAKEUP__WTK_WAKEUP_H__

#include "wtk/api/api_nwakeup.h"
#include "wtk/api/api_wakeup.h"
#include "dnn/wtk_wakeup_dnn.h"
#include "wtk_wakeup_cfg.h"
#include "wtk_wakeup_env.h"
#ifndef WKP_VAD_DISABLE
#include "wtk/vad/wtk_vad2.h"
#endif

#define WTK_WAKEUP_VER          "3.10.0.5"
#define WTK_WAKEUP_RES_VER      "002"

#define WTK_WAKEUP_VER_INFO     "wakeup_ver_prefix"WTK_WAKEUP_VER"_f"WTK_WAKEUP_RES_VER
#define WTK_WAKEUP_VER_OFFSET   (sizeof("wakeup_ver_prefix")-1)

#ifdef WKP_API_EXPORT_EN
#define WKP_API_PUBLIC __attribute__ ((visibility ("default")))
#else
#define WKP_API_PUBLIC
#endif

typedef enum wtk_wakeup_status
{
    WAKEUP_ERROR = -1,
    WAKEUP_WAIT,
    WAKEUP_WOKEN,
    WAKEUP_WOKEN_BOUNDARY,
    WAKEUP_RESTART,
} wtk_wakeup_status_t;

typedef struct wtk_wakeup
{
    wtk_wakeup_cfg_t *cfg;
    wtk_wakeup_env_t *env;

    wtk_wakeup_dnn_t *dnn;
#ifndef WKP_VAD_DISABLE
    wtk_vad2_t *vad;
    wtk_queue_t vad_q;
    wtk_wakeup_vad_status_t vad_status;
#endif
    wtk_wakeup_status_t status;
    void *hook;
    wakeup_handler_t post;
    char stopped;
#ifdef WAKEUP_NCHANNEL
    char index;
#endif
} wtk_wakeup_t;

int wtk_wakeup_register_handler(wtk_wakeup_t *w, void *hook, wakeup_handler_t func);
int wtk_wakeup_register_feature_handler(wtk_wakeup_t *w, void *hook, wakeup_feature_handler_t func);
#ifndef WKP_VAD_DISABLE
int wtk_vad_register_handler(wtk_wakeup_t *w, void *hook, vad_handler_t func);
#endif

wtk_wakeup_t *wtk_wakeup_new(wtk_wakeup_cfg_t *cfg);
void wtk_wakeup_delete(wtk_wakeup_t *w);
void wtk_wakeup_reset(wtk_wakeup_t *w);
void wtk_wakeup_restart(wtk_wakeup_t *w);
int wtk_wakeup_start(wtk_wakeup_t *w, char *env, int bytes);
int wtk_wakeup_end(wtk_wakeup_t *w);
int wtk_wakeup_feed(wtk_wakeup_t *w, char *data, int bytes, int is_end);
const char *wtk_wakeup_ver(void);

#endif //__AISPEECH__WTK_FST_WAKEUP__WTK_WAKEUP_H__
