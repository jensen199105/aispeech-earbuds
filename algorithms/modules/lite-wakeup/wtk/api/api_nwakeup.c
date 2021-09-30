/*
 * =====================================================================================
 *
 *       Filename:  api_nwakeup.c
 *
 *    Description:  api source file for multi-wakeup
 *
 *        Version:  1.0
 *        Created:  2017年12月21日 09时10分08秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Shengwei.Bai
 *   Organization:  AISPEECH Co.,Ltd.
 *
 * =====================================================================================
 */

#ifdef WAKEUP_NCHANNEL

#include <stdlib.h>
#ifdef AISP_TSL_INFO
#include <stdio.h>
#endif
#include "wtk/wakeup/wtk_wakeup.h"
#include "api_nwakeup.h"
#define CHANS_NUM   (8)
#ifdef LUDA_SUPPORT_FREQOUT_DIRECT
#define FEED_BYTES  (257*8)
#else
#define FEED_BYTES  (640)
#endif

typedef struct wakeup_hook_info
{
    wakeup_Nchans_t *pwakeup_Nchans;
    wtk_wakeup_t *pwakeup;
} wakeup_hook_info_t;

struct wakeup_Nchans
{
    wtk_wakeup_t *w[CHANS_NUM];
    wtk_wakeup_cfg_t *wakeup_cfg;
    unsigned char chans_num;

    void *hook;
    wakeup_Nchans_handler post;
    void *vad_hook;
    wakeup_Nchans_vad_handler vad_post;
    wakeup_hook_info_t *info[CHANS_NUM];
    char *frame_buffer;
    short last_length[CHANS_NUM];
    char *vspAlgMembase;
    int vspAlgMemSize;
};

static int _wakeup_Nchans_handler(void *obj, wakeup_status_t status, char *json, int bytes)
{
    wakeup_hook_info_t *info = (wakeup_hook_info_t *)obj;
    wtk_wakeup_t *wakeup = info->pwakeup;

    if (status == 0)
    {
        //not wakeup
    }
    else
    {
        wakeup_Nchans_t *wakeup_Nchans = info->pwakeup_Nchans;

        if (wakeup_Nchans->post != NULL)
        {
            wakeup_Nchans->post(wakeup_Nchans->hook, wakeup->index + 1, json);
        }
        else
        {
#ifdef AISP_TSL_INFO
            printf("%.*s\n", bytes, json);
#endif
        }
    }

    return 0;
}

#ifndef WKP_VAD_DISABLE
static int _vad_Nchans_handler(void *obj, int frame_state, int ifrmIdx)
{
    wakeup_hook_info_t *info = (wakeup_hook_info_t *)obj;
    wtk_wakeup_t *wakeup = info->pwakeup;
    wakeup_Nchans_t *wakeup_Nchans = info->pwakeup_Nchans;

    if (wakeup_Nchans->post != NULL)
    {
        wakeup_Nchans->vad_post(wakeup_Nchans->vad_hook, wakeup->index + 1, frame_state, ifrmIdx);
    }

    return 0;
}
#endif

WKP_API_PUBLIC int wakeup_Nchans_register_handler(wakeup_Nchans_t *w, void *hook, wakeup_Nchans_handler func)
{
    if (w)
    {
        w->hook = hook;
        w->post = func;
        return 0;
    }
    else
    {
        return -1;
    }
}

#ifndef WKP_VAD_DISABLE
WKP_API_PUBLIC int wakeup_Nchans_register_vad_handler(wakeup_Nchans_t *w, void *hook, wakeup_Nchans_vad_handler func)
{
    if (w)
    {
        w->vad_hook = hook;
        w->vad_post = func;
        return 0;
    }
    else
    {
        return -1;
    }
}
#endif

WKP_API_PUBLIC wakeup_Nchans_t *wakeup_Nchans_new(char *mem_ptr, int n, int size, void *hook,
        wakeup_Nchans_handler func, wakeup_Nchans_vad_handler vad_func)
{
    int i, j;
    wtk_wakeup_t *wakeup;
    wakeup_Nchans_t *w;

    if (n > CHANS_NUM)
    {
        goto error;
    }

    wtk_memInit(mem_ptr, size);
    w = (wakeup_Nchans_t *)wtk_malloc(sizeof(wakeup_Nchans_t));
    w->frame_buffer = (char *)wtk_calloc(FEED_BYTES * n, 1);
    w->vspAlgMembase = mem_ptr;
    w->vspAlgMemSize = size;

    if (w == NULL)
    {
        goto error;
    }

    w->wakeup_cfg = (wtk_wakeup_cfg_t *)wtk_malloc(sizeof(wtk_wakeup_cfg_t));
    wtk_wakeup_cfg_init(w->wakeup_cfg);

    if (0 != wtk_wakeup_cfg_update_local(w->wakeup_cfg))
    {
        goto error;
    }

    wtk_wakeup_cfg_update(w->wakeup_cfg);
    w->chans_num = n;

    for (i = 0; i < n; i++)
    {
        w->last_length[i] = 0;
        wakeup = wtk_wakeup_new(w->wakeup_cfg);

        if (wakeup == NULL)
        {
            wtk_wakeup_cfg_delete_bin(w->wakeup_cfg);

            for (j = 0; j < 1; j++)
            {
                wtk_free(w->w[j]);
            }

            wtk_free(w);
            return NULL;
        }

        wakeup->index = i;
        wtk_wakeup_reset(wakeup);
        w->info[i] = (wakeup_hook_info_t *)wtk_malloc(sizeof(wakeup_hook_info_t));
        w->info[i]->pwakeup = wakeup;
        w->info[i]->pwakeup_Nchans = w;
        wtk_wakeup_register_handler(wakeup, w->info[i], (wakeup_handler_t)_wakeup_Nchans_handler);
#ifndef WKP_VAD_DISABLE
        wtk_vad_register_handler(wakeup, w->info[i], (vad_handler_t)_vad_Nchans_handler);
#endif
        w->w[i] = wakeup;
    }

    wakeup_Nchans_register_handler(w, hook, func);
#ifndef WKP_VAD_DISABLE
    wakeup_Nchans_register_vad_handler(w, hook, vad_func);
#endif
    return w;
error:

    if (NULL != w)
    {
        wtk_free(w);
    }

    return NULL;
}

WKP_API_PUBLIC int wakeup_Nchans_start(wakeup_Nchans_t *w, char *env, int bytes)
{
    int ret = -1;

    if (w)
    {
        int i;
        int chans_num = w->chans_num;

        for (i = 0; i < chans_num; i++)
        {
            ret = wtk_wakeup_start(w->w[i], env, bytes);

            if (ret == -1)
            {
                return ret;
            }
        }

        ret = 0;
    }

    return ret;
}

WKP_API_PUBLIC int wakeup_Nchans_feed(wakeup_Nchans_t *w, int chan_index, char *data, int bytes, unsigned is_end)
{
    int i   = 0;
    int j   = 0;
    int off = 0;
    int ret = -1;
    char *frame_buffer = NULL;
    frame_buffer = w->frame_buffer + chan_index * FEED_BYTES;

    if (w)
    {
        if (w->last_length[chan_index] + bytes >= FEED_BYTES)
        {
            for (i = w->last_length[chan_index], j = 0; i < FEED_BYTES; i++, j++)
            {
                frame_buffer[i] = data[j];
            }

            off = FEED_BYTES - w->last_length[chan_index];
            ret = wtk_wakeup_feed(w->w[chan_index], frame_buffer, FEED_BYTES, is_end);
            w->last_length[chan_index] = 0;
        }

        while (bytes - off >= FEED_BYTES)
        {
            ret  = wtk_wakeup_feed(w->w[chan_index], data + off, FEED_BYTES, is_end);
            off += FEED_BYTES;
        }

        for (i = w->last_length[chan_index], j = off; j < bytes; i++, j++)
        {
            frame_buffer[i] = data[j];
        }

        w->last_length[chan_index] += bytes - off;
    }

    return ret;
}

WKP_API_PUBLIC int wakeup_Nchans_end(wakeup_Nchans_t *w)
{
    int ret = -1;

    if (w)
    {
        int i;
        int chans_num =  w->chans_num;

        for (i = 0; i < chans_num; i++)
        {
            ret = wtk_wakeup_end(w->w[i]);
        }
    }

    return ret;
}

WKP_API_PUBLIC void wakeup_Nchans_delete(wakeup_Nchans_t *w)
{
    if (w)
    {
#ifdef PC_PLATFORM
        int i;
        int chans_num = w->chans_num;

        for (i = 0; i < chans_num; i++)
        {
            wtk_wakeup_delete(w->w[i]);
            wtk_free(w->info[i]);
        }

        wtk_wakeup_cfg_delete_bin(w->wakeup_cfg);
        wtk_free(w->frame_buffer);
        wtk_free(w);
#elif defined DSP_PLATFORM
        extern _m_malloc_dev_t malloc_dev;
        AISP_TSL_memset(w->vspAlgMembase, 0, w->vspAlgMemSize);
        malloc_dev.cur = NULL;
        malloc_dev.end = NULL;
        malloc_dev.start = NULL;
        malloc_dev.memrdy = 0;
#endif
    }
}

WKP_API_PUBLIC void wakeup_Nchans_reset(wakeup_Nchans_t *w)
{
    if (w)
    {
        int i;
        int chans_num = w->chans_num;

        for (i = 0; i < chans_num; i++)
        {
            wtk_wakeup_reset(w->w[i]);
        }
    }
}

WKP_API_PUBLIC void wakeup_Nchans_restart(wakeup_Nchans_t *w)
{
    if (w)
    {
        int i;
        int chans_num = w->chans_num;

        for (i = 0; i < chans_num; i++)
        {
            wtk_wakeup_restart(w->w[i]);
        }
    }
}

WKP_API_PUBLIC const char *wakeup_Nchans_version(void)
{
    return wtk_wakeup_ver();
}

#endif
