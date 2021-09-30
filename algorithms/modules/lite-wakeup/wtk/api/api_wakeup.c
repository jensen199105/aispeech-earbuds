/**
 * Project  : AIWakeup.v1.0.7
 * FileName : api/api_wakeup.c
 *
 * COPYRIGHT (C) 2015-2016, AISpeech Ltd.. All rights reserved.
 */

#ifdef AISP_TSL_INFO
#include <stdio.h>
#endif

#include "api_wakeup.h"
#include "wtk/wakeup/wtk_wakeup.h"
#include "wtk/core/wtk_alloc.h"


struct wakeup
{
    wtk_wakeup_cfg_t *wakeup_cfg;
    wtk_wakeup_t *wakeup_inst;
    char *pmembase;
    unsigned int memsize;
};

WKP_API_PUBLIC wakeup_t *wakeup_new(char *mem_ptr, unsigned int size)
{
    wakeup_t *w = NULL;
    wtk_memInit(mem_ptr, size);
    w = (wakeup_t *)wtk_malloc(sizeof(wakeup_t));

    if (NULL == w)
    {
        goto error;
    }

    w->pmembase = mem_ptr;
    w->memsize = size;
    w->wakeup_cfg = (wtk_wakeup_cfg_t *)wtk_malloc(sizeof(wtk_wakeup_cfg_t));
    wtk_wakeup_cfg_init(w->wakeup_cfg);

    if (0 != wtk_wakeup_cfg_update_local(w->wakeup_cfg))
    {
        goto error;
    }

    if (0 != wtk_wakeup_cfg_update(w->wakeup_cfg))
    {
        goto error;
    }

    w->wakeup_inst = wtk_wakeup_new(w->wakeup_cfg);
    return w;
error:

    if (NULL != w)
    {
        wtk_free(w);
    }

    return NULL;
}

WKP_API_PUBLIC void wakeup_delete(wakeup_t *w)
{
#ifdef PC_PLATFORM

    if (w)
    {
        wtk_wakeup_delete(w->wakeup_inst);
        wtk_wakeup_cfg_delete_bin(w->wakeup_cfg);
        wtk_free(w);
    }

#elif defined DSP_PLATFORM

    if (w)
    {
        extern _m_malloc_dev_t malloc_dev;
        //wtk_mem_memset(membase,0,MEM_MAX_SIZE);
        AISP_TSL_memset(w->pmembase, 0, w->memsize);
        malloc_dev.cur = NULL;
        malloc_dev.end = NULL;
        malloc_dev.start = NULL;
        malloc_dev.memrdy = 0;
    }

#endif
}

WKP_API_PUBLIC void wakeup_reset(wakeup_t *w)
{
    if (w && w->wakeup_inst)
    {
        wtk_wakeup_reset(w->wakeup_inst);
    }
}

WKP_API_PUBLIC int wakeup_start(wakeup_t *w, char *env, int bytes)
{
    int ret = -1;

    if (w && w->wakeup_inst)
    {
        ret = wtk_wakeup_start(w->wakeup_inst, env, bytes);
    }

    return ret;
}

WKP_API_PUBLIC wakeup_status_t wakeup_feed(wakeup_t *w, char *data, int bytes)
{
    wakeup_status_t status = WAKEUP_STATUS_ERROR;
#ifdef WKP_RELEASE_FOR_DEBUG
#define WKP_DEBUG_ACTIVE_FRAMES (1350000)
    static int iCalcVal = 0;

    if (iCalcVal++ >= WKP_DEBUG_ACTIVE_FRAMES)
    {
        return -1;
    }

#endif

    if (w && w->wakeup_inst)
    {
        status = (wakeup_status_t)wtk_wakeup_feed(w->wakeup_inst, data, bytes, 0);
    }

    return status;
}

WKP_API_PUBLIC wakeup_status_t wakeup_end(wakeup_t *w)
{
    wakeup_status_t status = WAKEUP_STATUS_ERROR;

    if (w && w->wakeup_inst)
    {
        int ret = -1;
        ret = wtk_wakeup_end(w->wakeup_inst);

        if (ret == 0)
        {
            status = WAKEUP_STATUS_WAIT;
        }
    }

    return status;
}

WKP_API_PUBLIC void wakeup_register_handler(wakeup_t *w, void *user_data, wakeup_handler_t func)
{
    if (w && w->wakeup_inst)
    {
        wtk_wakeup_register_handler(w->wakeup_inst, user_data, func);
    }
}

WKP_API_PUBLIC void wakeup_register_feature_handler(wakeup_t *w, void *user_data, wakeup_feature_handler_t func)
{
    if (w && w->wakeup_inst)
    {
        wtk_wakeup_register_feature_handler(w->wakeup_inst, user_data, func);
    }
}

WKP_API_PUBLIC const char *wakeup_version(void)
{
    return wtk_wakeup_ver();
}

#ifndef WKP_VAD_DISABLE
WKP_API_PUBLIC void wakeup_register_vad_handler(wakeup_t *w, void *user_data, vad_handler_t func)
{
    if (w && w->wakeup_inst)
    {
        wtk_vad_register_handler(w->wakeup_inst, user_data, func);
    }
}
#endif

WKP_API_PUBLIC int wakeup_set_thresh(wakeup_t *w, int thresh_type)
{
    if (w && (thresh_type != w->wakeup_inst->env->thresh_type))
    {
        wtk_wakeup_set_thresh(w->wakeup_inst->env, thresh_type);
        wtk_wakeup_dnn_wordlist_update_thresh(w->wakeup_inst->dnn->wrds, w->wakeup_inst->env->thresholds);
        return 0;
    }
    else
    {
        return -1;
    }
}

//重置唤醒词和阈值, 该接口在早期的dnn项目中提供，存在风险，对有需要的客户开放

// WKP_API_PUBLIC void wakeup_reset_wkpwords(wakeup_t *w, unsigned char *mem_ptr, wakeup_handler_t func,char *env)
// {
//   int memsize = w->memsize;
//   wakeup_delete(w);
//   wakeup_new((char*)mem_ptr, memsize);
//   wakeup_start(w,env,strlen(env));
//   wakeup_register_handler(w, w, func);
// }
