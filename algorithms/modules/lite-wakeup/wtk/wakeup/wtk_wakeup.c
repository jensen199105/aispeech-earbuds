/**
 * Project  : AIWakeup.v1.0.7
 * FileName : third/wtk/wakeup/wtk_wakeup.c
 *
 * COPYRIGHT (C) 2014, AISpeech Ltd.. All rights reserved.
 */
#include "wtk/wakeup/dnn/wtk_wakeup_dnn_penalty.h"
#include "wtk_wakeup.h"

#ifdef USE_HIFI4_OPTIMISIZE
#include <xtensa/hal.h>
#endif

#define STRWKP_START          "{\"wakeupWord\":\""
#define STRWKP_MAJOR          "\",\"major\":"
#define STRWKP_FRAME          ",\"frame\":"
#define STRWKP_END            "}"

static int _wakeup_post_wakeup_status(wtk_wakeup_t *w, wtk_wakeup_dnn_result_t *result)
{
    if (result && result->waked && w->post)
    {
        char strMajor[2];
        char strFrameIndex[16];
        char buf[128];
        AISP_TSL_memset(buf, 0, sizeof(buf));
        AISP_TSL_itoa(result->major, strMajor);
        AISP_TSL_itoa(result->llFrameIndex, strFrameIndex);
        AISP_TSL_strncat(buf, STRWKP_START, AISP_TSL_strlen(STRWKP_START));
        AISP_TSL_strncat(buf, result->waked_word->data, AISP_TSL_strlen(result->waked_word->data));
        AISP_TSL_strncat(buf, STRWKP_MAJOR, AISP_TSL_strlen(STRWKP_MAJOR));
        AISP_TSL_strncat(buf, strMajor, AISP_TSL_strlen(strMajor));
        AISP_TSL_strncat(buf, STRWKP_FRAME, AISP_TSL_strlen(STRWKP_FRAME));
        AISP_TSL_strncat(buf, strFrameIndex, AISP_TSL_strlen(strFrameIndex));
        AISP_TSL_strncat(buf, STRWKP_END, AISP_TSL_strlen(STRWKP_END));
        w->post(w->hook, WAKEUP_STATUS_WOKEN, buf, AISP_TSL_strlen(buf));
    }

    return 0;
}

static wtk_wakeup_status_t _wakeup_feed_data(wtk_wakeup_t *w, char *data, int bytes, int is_end)
{
    int ret = -1;
    wtk_wakeup_dnn_result_t *result;
    ret = wtk_wakeup_dnn_feed(w->dnn, data, bytes, is_end);

    if (ret == -1)
    {
        goto end;
    }

    result = wtk_wakeup_dnn_get_result(w->dnn);

    if (result && result->waked == 1)
    {
        w->status = WAKEUP_WOKEN;
        _wakeup_post_wakeup_status(w, result);
        w->stopped = 1;
    }

    ret = 0;
end:
    return w->status;
}

static void _wakeup_restart_all(wtk_wakeup_t *w, unsigned forced)
{
    if (!w)
    {
        return ;
    }

    w->stopped = 0;
    w->status  = WAKEUP_WAIT;
#ifdef WAKEUP_DUR_PENALTY

    if (w->dnn->cfg->use_duration_penalty)
    {
        if (WAKEUP_WOKEN == w->status || forced)
        {
            int i;
            wtk_wakeup_dnn_penalty_reset(&w->dnn->duration_penalty);

            for (i = 0; i < w->dnn->wrds->n_wrds; i++)
            {
                w->dnn->duration_penalty.wrd_coefs[i].easy_wakeup_cur_score_frame = 0;
                w->dnn->duration_penalty.wrd_coefs[i].easy_wakeup_prev_score_frame = 0;
                w->dnn->duration_penalty.wrd_coefs[i].score_easy_wakeup_coef = 1;
            }
        }
    }

#endif
    wtk_wakeup_dnn_reset(w->dnn);
}

wtk_wakeup_t *wtk_wakeup_new(wtk_wakeup_cfg_t *cfg)
{
    wtk_wakeup_t *w;
    w = (wtk_wakeup_t *)wtk_malloc(sizeof(wtk_wakeup_t));

    if (w == NULL)
    {
        goto error;
    }

    w->cfg  = cfg;
    w->env = wtk_wakeup_env_new();
    w->dnn = wtk_wakeup_dnn_new(w->cfg->dnn);
    w->post = NULL;
    w->hook = NULL;
    w->stopped = 0;
    w->status = WAKEUP_WAIT;

    if (NULL == w->env || NULL == w->dnn)
    {
        goto error;
    }

    wtk_wakeup_reset(w);
    return w;
error:

    if (NULL != w)
    {
        if (NULL != w->env)
        {
            wtk_free(w->env);
        }

        if (NULL == w->dnn)
        {
            wtk_free(w->dnn);
        }

        wtk_free(w);
    }

    return NULL;
}

void wtk_wakeup_delete(wtk_wakeup_t *w)
{
    if (w)
    {
        wtk_wakeup_reset(w);
        wtk_wakeup_dnn_delete(w->dnn);
        wtk_wakeup_env_delete(w->env);
        wtk_free(w);
    }
}

void wtk_wakeup_reset(wtk_wakeup_t *w)
{
    if (w)
    {
        wtk_wakeup_dnn_reset(w->dnn);
        w->status = WAKEUP_WAIT;
    }
}

int wtk_wakeup_start(wtk_wakeup_t *w, char *env, int bytes)
{
    int ret = -1;
    w->stopped = 0;

    if (bytes > 0)
    {
        ret = wtk_wakeup_env_parse(w->env, env, bytes);

        if (ret != 0)
        {
            return ret;
        }
    }

    w->dnn->use_env = 1;
    ret = wtk_wakeup_dnn_start(w->dnn, w->env);
    return ret;
}

int wtk_wakeup_end(wtk_wakeup_t *w)
{
    return wtk_wakeup_feed(w, NULL, 0, 1);
}

int wtk_wakeup_feed(wtk_wakeup_t *w, char *data, int bytes, int is_end)
{
    _wakeup_feed_data(w, data, bytes, is_end);
    w->stopped |= is_end;

    if (w->stopped)
    {
        if (w->status != WAKEUP_WOKEN && w->status != WAKEUP_WOKEN_BOUNDARY)
        {
            _wakeup_feed_data(w, NULL, 0, 1);
        }

        _wakeup_restart_all(w, 0);
    }

    return w->status;
}

int wtk_wakeup_register_handler(wtk_wakeup_t *w, void *hook, wakeup_handler_t func)
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

int wtk_wakeup_register_feature_handler(wtk_wakeup_t *w, void *hook, wakeup_feature_handler_t func)
{
    if (!w)
    {
        return -1;
    }

    w->dnn->parm->feature_hook = hook;
    w->dnn->parm->feature_hook_f = func;
    return 0;
}

#ifndef WKP_VAD_DISABLE
int wtk_vad_register_handler(wtk_wakeup_t *w, void *hook, vad_handler_t func)
{
    if (w)
    {
        w->dnn->hook = hook;
        w->dnn->dnn_vad_post = func;
        return 0;
    }
    else
    {
        return -1;
    }
}
#endif

void wtk_wakeup_restart(wtk_wakeup_t *w)
{
    _wakeup_restart_all(w, 1);
}

const char *wtk_wakeup_ver(void)
{
    const char *ver = WTK_WAKEUP_VER_INFO;
    return &ver[WTK_WAKEUP_VER_OFFSET];
}
