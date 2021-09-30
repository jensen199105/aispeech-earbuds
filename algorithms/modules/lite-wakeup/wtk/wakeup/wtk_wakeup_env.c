/**
 * Project  : AIWakeup.v1.0.7
 * FileName : third/wtk/wakeup/wtk_wakeup_env.c
 *
 * COPYRIGHT (C) 2014, AISpeech Ltd.. All rights reserved.
 */

#include "AISP_TSL_str.h"
#include "wtk_wakeup_env.h"
#include "wtk_wakeup_cfg.h"
static int wtk_get_str_items(char *str, char skipChar)
{
    char *ptr = NULL;
    int iLoop = 0;

    for (iLoop = 0; iLoop < WAKEUP_MAX_WORDS; iLoop++)
    {
        ptr =  AISP_TSL_strchr(str, skipChar);

        if (NULL == ptr)
        {
            break;
        }

        str = ptr + 1;
    }

    return ++iLoop;
}

wtk_wakeup_env_t *wtk_wakeup_env_new(void)
{
    wtk_wakeup_env_t *env = NULL;
    env = (wtk_wakeup_env_t *)wtk_malloc(sizeof(wtk_wakeup_env_t));

    if (NULL == env)
    {
        return NULL;
    }

    env->words = NULL;
    env->majors = NULL;
    env->thresholds         = NULL;
    env->threshs_default    = NULL;
    env->threshs_wind       = NULL;
    env->thresh_type        = THRESH_TYPE_DEFAULT;
    env->nwrd = 0;
    env->nthresh = 0;
    env->nmajor = 0;
    return env;
}

void wtk_wakeup_env_delete(wtk_wakeup_env_t *env)
{
    if (env)
    {
        int i;

        for (i = 0; i < env->nwrd; i++)
        {
            wtk_string_delete(env->words[i]);
        }

        wtk_free(env->words);
        wtk_free(env->majors);
        wtk_free(env->thresholds);
        wtk_free(env);
    }
}

void wtk_wakeup_env_reset(wtk_wakeup_env_t *env)
{
    if (env)
    {
        env->words = NULL;
        env->nwrd = 0;
    }
}

int wtk_wakeup_env_set_threshs(wtk_wakeup_env_t *env, int **thresholds, char *val)
{
    int ret = -1;
    char *ptr = NULL;
    char *thresh = NULL;
    int i = 0;
    int iNums = 0;
    int temp_q31 = 0;
    float temp = 0.0f;

    if (AISP_TSL_strlen(val) == 0)
    {
        wtk_debug("empty thresh value.\n");
        goto end;
    }

    iNums = wtk_get_str_items(val, ',');

    // 配置的所有阈值项，阈值个数要相等
    if (0 != env->nthresh && (iNums != env->nthresh))
    {
        goto end;
    }

    env->nthresh = iNums;
    *thresholds = (int *)wtk_malloc(sizeof(int) * iNums);
    thresh = val;

    for (i = 0; i < iNums; ++i)
    {
        ptr =  AISP_TSL_strchr(thresh, ',');

        if (NULL != ptr)
        {
            *ptr = '\0';
        }

        temp = AISP_TSL_strtof(thresh);
        temp_q31 = (int)(temp * WTK_WAKEUP_Q31_ONE + 0.5f);
        (*thresholds)[i] = fxlog_q31(temp_q31);

        if (NULL == ptr)
        {
            break;
        }

        thresh = ptr + 1;
    }

    ret = 0;
end:
    return ret;
}

int wtk_wakeup_env_parse(wtk_wakeup_env_t *env, char *str, int bytes)
{
    char _buf[256];
    char *buf = NULL;
    int ret = -1;
    char *tok;
    char *key, *val;
    int i;
    char *save_ptr;
    buf = bytes >= sizeof(_buf) ? wtk_malloc(bytes + 1) : _buf;

    if (NULL == buf)
    {
        goto end;
    }

    AISP_TSL_memcpy(buf, str, bytes);
    buf[bytes] = '\0';
    tok = AISP_TSL_strtok_r(buf, ";", &save_ptr);

    while (tok != NULL)
    {
        key = val = tok;

        while (*val)
        {
            if (*val == '=')
            {
                *(val++) = '\0';
                break;
            }

            val++;
        }

        switch (key[0])
        {
            case 'w':

                /* words */
                if (AISP_TSL_strcmp(key, "words") == 0)
                {
                    char *ptr = NULL;
                    char *wrd = NULL;
                    int   iNWords = 1;
                    int   iLen    = 0;
                    int num;
                    iLen = AISP_TSL_strlen(val);

                    if (iLen == 0)
                    {
                        wtk_debug("empty words.\n");
                        goto end;
                    }

                    for (num = 0; num < iLen; num++)
                    {
                        if ((val[num]) == ',')
                        {
                            iNWords++;
                        }
                    }

                    env->nwrd = env->nthresh = iNWords;
                    env->words = (wtk_string_t **)wtk_malloc(sizeof(wtk_string_t *) * iNWords);
                    wrd = val;

                    for (i = 0; i < iNWords; ++i)
                    {
                        ptr =  AISP_TSL_strchr(wrd, ',');

                        if (NULL == ptr)
                        {
                            iLen = AISP_TSL_strlen(wrd);
                            env->words[i] = wtk_string_dup_data2(wrd, iLen);
                            break;
                        }
                        else
                        {
                            iLen = (ptr - wrd);
                            env->words[i] = wtk_string_dup_data2(wrd, iLen);
                        }

                        wrd = ptr + 1;
                    }
                }

                break;

            case 't':
                if (!AISP_TSL_strcmp(key, "thresh"))
                {
                    ret = wtk_wakeup_env_set_threshs(env, &(env->threshs_default), val);
                }
                else
                    if (!AISP_TSL_strcmp(key, "thresh_wind"))
                    {
                        ret = wtk_wakeup_env_set_threshs(env, &(env->threshs_wind), val);
                    }

                if (ret)
                {
#ifdef AISP_TSL_INFO
                    printf("parse thresh failed, key(%s)\n", key);
#endif
                    goto end;
                }

                break;

            case 'm':
                if (AISP_TSL_strcmp(key, "major") == 0)
                {
                    /* major */
                    char *ptr = NULL;
                    char *major = NULL;
                    int i;
                    int iNums = 0;

                    if (AISP_TSL_strlen(val) == 0)
                    {
                        wtk_debug("empty majors.\n");
                        goto end;
                    }

                    env->nmajor = iNums = wtk_get_str_items(val, ',');
                    env->majors = (char *)wtk_malloc(sizeof(char) * iNums);
                    major = val;

                    for (i = 0; i < iNums; ++i)
                    {
                        ptr = AISP_TSL_strchr(major, ',');
                        env->majors[i] = (AISP_TSL_atoi(major) != 0) ? 1 : 0;

                        if (NULL == ptr)
                        {
                            break;
                        }

                        major = ptr + 1;
                    }
                }

                break;
        }

        tok = AISP_TSL_strtok_r(NULL, ";", &save_ptr);
    }

    // 阈值初始化为默认阈值
    env->thresh_type    = THRESH_TYPE_DEFAULT;
    env->thresholds     = env->threshs_default;

    if (NULL == env->thresholds)
    {
        // 必须配置默认的阈值
        goto end;
    }

    ret = 0;
end:

    if (NULL != buf && buf != _buf)
    {
        wtk_free(buf);
    }

    return ret;
}

int wtk_wakeup_set_thresh(wtk_wakeup_env_t *env, int thresh_type)
{
    int ret = 0;

    switch (thresh_type)
    {
        case THRESH_TYPE_DEFAULT:
            env->thresholds = env->threshs_default;
            break;

        case THRESH_TYPE_WIND:
            if (env->threshs_wind)
            {
                env->thresholds = env->threshs_wind;
            }

            break;

        default:
            // 参数非法的情况下，使用默认值
            env->thresholds = env->threshs_default;
            ret = -1;
            break;
    }

    env->thresh_type = thresh_type;
    return ret;
}
