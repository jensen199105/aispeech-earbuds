/**
 * Project  : AIWakeup.v1.0.7
 * FileName : third/wtk/wakeup/dnn/wtk_wakeup_dnn.c
 *
 * COPYRIGHT (C) 2014, AISpeech Ltd.. All rights reserved.
 */
#ifdef WAKEUP_DUMP_SCORE
#include <stdio.h>
#include <math.h>
#endif

#include "wtk/wakeup/wtk_wakeup.h"
#include "wtk_wakeup_dnn_penalty.h"
#include "wtk_wakeup_dnn.h"

#ifdef USE_HIFI4_OPTIMISIZE
#include <xtensa/hal.h>
#endif

extern const unsigned AISPEECH_SPACE;

#define  DNN_SCORE_SHR(a,shift)      (int)((a + ((((int64_t)1)<<shift)>>1)) >>shift)
//TODO: remove below definination
#define  int64_t long long

#ifdef WAKEUP_DICT_WORD
// push data into circular queue and updata the sum
STATIC void wtk_update_smooth_win(wtk_smooth_score_t *pscore, U32 data)
{
    if (WTK_CQUEUE_FULL(pscore->scores))
    {
        pscore->sum -= wtk_cqueue_pop(pscore->scores);
    }

    wtk_cqueue_push(pscore->scores, data);
    pscore->sum += wtk_cqueue_back(pscore->scores);
}

STATIC void _wakeup_dnn_progressive_smooth_e2e(wtk_wakeup_dnn_t *w, wtk_frv_t *frv, float weight)
{
    wtk_wakeup_dnn_word_t *wrd;
    int avg_prob;
    int idx;
    wtk_smooth_score_t *ss;

    for (idx = 0; idx < w->wrds->n_wchars; idx++)
    {
        wtk_update_smooth_win(&(w->wrds->wchars[idx].rawp), frv->rv[idx + 1]);
    }

    for (idx = 0; idx < w->wrds->n_wrds; idx++)
    {
        wrd = &(w->wrds->wrds[idx]);
        ss  = &wrd->wchars[0]->rawp;
        avg_prob = ss->sum / ss->scores->used;
        wrd->conf = fxlog_q31(avg_prob);
    }

#if defined(WAKEUP_DUMP_SCORE)

    //printf("SCORE: ");
    for (idx = 0; idx < w->wrds->n_wrds; idx++)
    {
        wrd = &(w->wrds->wrds[idx]);
        printf("%7.20f ", exp((float)wrd->conf / (1 << 26)));
    }

    printf("\n");
#endif
}

STATIC int wakeup_dnn_score_apply_normal_post_e2e(wtk_wakeup_dnn_t *w, wtk_frv_t *frv)
{
    int i;
    wtk_wakeup_dnn_word_t *wrd;

    if (w->cur < w->cfg->wakeup_skip_win)
    {
        return 0;
    }

    for (i = 0; i < w->wrds->n_wrds; i++)
    {
        wrd     = &w->wrds->wrds[i];

        if (wrd->conf > wrd->threshold)
        {
            w->score = wrd->conf;
            w->result.conf = wrd->conf;
            w->result.llFrameIndex = w->parm->llAbsFrameIndex;
            w->result.waked_word = wrd->word;
            w->result.major = wrd->major;
            w->result.waked = 1;
            break;
        }
    }

    return 0;
}
#endif  // USE_E2E


#ifndef WKP_VAD_DISABLE
static int vad_post_state(wtk_wakeup_dnn_t *w, int frame_status, int ifrmIdx)
{
    if (w->dnn_vad_post != NULL)
    {
        w->dnn_vad_post(w->hook, frame_status, ifrmIdx);
    }

    return 0;
}
#endif

static int _wakeup_update_wrds(wtk_wakeup_dnn_t *w, wtk_wakeup_env_t *env)
{
    w->wrds = wtk_wakeup_dnn_wordlist_new(w->cfg, env->words, env->nwrd, env->thresholds, env->nthresh, env->majors,
                                          env->nmajor);

    if (w->wrds == NULL)
    {
        return -1;
    }

    return 0;
}

#ifdef WAKEUP_DICT_CHAR
static void _wakeup_dnn_progressive_smooth(wtk_wakeup_dnn_t *w, wtk_frv_t *frv, float weight)
{
    int64_t temp_dur;
    wtk_wakeup_dnn_word_t *wrd;
    wtk_wakeup_dnn_wchar_t *wchar;
    int prob_fix;
    int conf_fix;
    int cur, prev, pprev, dur;
    int i, j, idx;
    S16 startPos = 0;
    S16 endPos = 0;
#if !defined(WAKEUP_RESET)
    frv->index = w->cur + 1;
#endif
    dur   = AISP_TSL_MIN(frv->index, w->cfg->wsmooth);
    cur   = w->cur % w->nslot;
    prev  = (w->cur - 1) % w->nslot;
    pprev = (w->cur - dur) % w->nslot;

    for (idx = 0; idx < w->wrds->n_wchars; idx++)
    {
        wchar = &(w->wrds->wchars[idx]);
        wchar->priorities[cur] = frv->rv[idx + 1];
        wchar->in_used = 0;
    }

    for (idx = 0; idx < w->wrds->n_wrds; idx++)
    {
        wrd = &(w->wrds->wrds[idx]);

        for (i = 0; i < wrd->n_wchars; ++i)
        {
            wchar = wrd->wchars[i];

            if (0 == i)
            {
                wrd->p1[i][cur] = wchar->priorities[cur];
            }
            else
            {
                wrd->p1[i][cur] = DNN_SCORE_SHR((int64_t)wrd->p2[i - 1] * wchar->priorities[cur], 31);
            }

            if (w->cur > 0)
            {
                if (frv->index != dur && w->cur > 1)
                {
                    temp_dur = (int64_t)wrd->smthp[i][prev] * dur - wrd->p1[i][pprev] + wrd->p1[i][cur];
                    wrd->smthp[i][cur] = prob_fix = (int)((temp_dur + (dur >> 1)) / dur);

                    if (wchar->in_used == 0)
                    {
                        temp_dur = (int64_t)((int64_t)wchar->smooth_tendency[prev] * dur - wchar->priorities[pprev] + wchar->priorities[cur]);
                        wchar->smooth_tendency[cur] = (int)((temp_dur + (dur >> 1)) / dur);
                    }
                }
                else
                {
                    temp_dur = (int64_t)wrd->smthp[i][prev] * (dur - 1) + wrd->p1[i][cur];
                    wrd->smthp[i][cur] = prob_fix = (int)((temp_dur + (dur >> 1)) / dur);

                    if (wchar->in_used == 0)
                    {
                        temp_dur = (int64_t)wchar->smooth_tendency[prev] * (dur - 1) + wchar->priorities[cur];
                        wchar->smooth_tendency[cur] = (int)((temp_dur + (dur >> 1)) / dur);
                    }
                }
            }
            else
            {
                wrd->smthp[i][cur] = prob_fix = (wrd->p1[i][cur] + (dur >> 1)) / dur;

                if (wchar->in_used == 0)
                {
                    wchar->smooth_tendency[cur] = (wchar->priorities[cur] + (dur >> 1)) / dur;
                }
            }

            if (wchar->in_used == 0)
            {
                wchar->in_used = 1;
            }

#ifdef WAKEUP_XCHECK

            if (NULL != wrd->pstCharCheck)
            {
                wtk_wakeup_dnn_ringbuff_t *wscore = &wrd->pstCharCheck->wscores[i];

                if (i >= (wrd->n_wchars - 1))
                {
                    wscore->arr[0] = wchar->smooth_tendency[cur];
                }
                else
                {
                    wscore->arr[wscore->cur % wscore->length] = wchar->smooth_tendency[cur];
                }

                wscore->cur++;
            }

#endif

            //-------------------------------------------------------------
            // 更新唤醒词中第i个字的平滑联合概率p2, 取区间 [StartPos, endPos]内的最大值
            if (w->cur < w->cfg->wmax)
            {
                if (prob_fix > wrd->p2[i])
                {
                    wrd->pidx[i] = w->cur;
                    wrd->p2[i] = prob_fix;
                }
            }
            else
            {
                startPos = w->cur + 1 - w->cfg->wmax;
                endPos   = w->cur - AISPEECH_SPACE;

                if (wrd->pidx[i] < startPos)
                {
                    // 最大值被移出区间，重新计算整个区间的最大值
                    wrd->p2[i] = PZERO;

                    for (j = startPos; j <= endPos; ++j)
                    {
                        if (wrd->smthp[i][j % w->nslot] >= wrd->p2[i])
                        {
                            wrd->pidx[i] = j;
                            wrd->p2[i]   = wrd->smthp[i][j % w->nslot];
                        }
                    }
                }
                else
                {
                    if (wrd->smthp[i][endPos % w->nslot] >= wrd->smthp[i][wrd->pidx[i] % w->nslot])
                    {
                        wrd->pidx[i] = endPos;
                        wrd->p2[i]   = wrd->smthp[i][endPos % w->nslot];
                    }
                }
            }
        }
    }

    //-------------------------------------------------------------
    // 更新置信度
    for (idx = 0; idx < w->wrds->n_wrds; idx++)
    {
        wrd = &w->wrds->wrds[idx];
        prob_fix = AISP_TSL_ABS(wrd->smthp[wrd->n_wchars - 1][cur]);
        conf_fix = (fxlog_q31(prob_fix) + (int)(wrd->n_wchars >> 1)) / (int)wrd->n_wchars;
        wrd->conf = conf_fix;
#ifdef WAKEUP_DUR_PENALTY

        if (w->cfg->use_duration_penalty == 1)
        {
            wtk_wakeup_dnn_penalty_update(&w->duration_penalty, w->parm->cfg, w->cfg, wrd, idx);
        }

#endif
#if defined(WAKEUP_DUMP_SCORE)
        printf("%7.20f ", exp((float)wrd->conf / (1 << 26)));

        if (idx == w->wrds->n_wrds - 1)
        {
            printf("\n");
        }

#endif
    }
}
#endif

#ifdef WAKEUP_DICT_PHONEME
static void _wakeup_dnn_progressive_smooth_phoneme(wtk_wakeup_dnn_t *w, wtk_frv_t *frv, float weight)
{
    int64_t temp_dur;
    wtk_wakeup_dnn_word_t *wrd;
    wtk_wakeup_dnn_wchar_t *wchar;
    wtk_wakeup_dnn_eword_char_t *item;
    int prob_fix;
    int cur, prev, pprev, dur, cur_eword;
    int i, j, k, idx;
    int smooth_tendency_cur = 0;
    int smooth_tendency_prev = 0;
    int smooth_tendency_win = WORD_MAX_CHAR * CHAR_MAX_FRAME;
    S16 startPos, endPos;
#if !defined(WAKEUP_RESET)
    frv->index = w->cur + 1;
#endif
    dur = AISP_TSL_MIN(frv->index, w->cfg->wsmooth);
    cur = w->cur % w->nslot;
    prev = (w->cur - 1) % w->nslot;
    pprev = (w->cur - dur) % w->nslot;
    cur_eword = w->cur % (w->nslot_eword);

    // 更新单字的smooth score
    for (idx = 0; idx < w->wrds->n_wchars; idx++)
    {
        wchar = &(w->wrds->wchars[idx]);
        wchar->priorities[cur] = frv->rv[idx + 1];
    }

    // XXX: 概率乘积导致误差放大
    for (idx = 0; idx < w->wrds->n_wrds; idx++)
    {
        wrd = &w->wrds->wrds[idx];

        for (i = 0; i < wrd->n_wchars; i++)
        {
            smooth_tendency_cur = w->cur % smooth_tendency_win;
            smooth_tendency_prev = (w->cur - 1) % smooth_tendency_win;
            item = &wrd->ewchars[i];

            // phoneme的progressive 打分
            for (k = 0; k < item->n_wchars; k++)
            {
                wchar = item->wchars[k];

                if (k == 0)     // 第一个phone
                {
                    item->p1[k][cur] = wchar->priorities[cur];

                    if (w->cur > 0)
                    {
                        if (frv->index != dur && w->cur > 1)
                        {
                            temp_dur = (int64_t)item->smthp[k][prev] * dur - item->p1[k][pprev] + item->p1[k][cur];
                            item->smthp[k][cur] = prob_fix = (int)((temp_dur + (dur >> 1)) / dur);
                            temp_dur = (int64_t)wchar->smooth_tendency[smooth_tendency_prev] * dur - wchar->priorities[pprev] +
                                       wchar->priorities[cur];
                            wchar->smooth_tendency[smooth_tendency_cur] = (int)((temp_dur + (dur >> 1)) / dur);
                        }
                        else
                        {
                            temp_dur = (int64_t)item->smthp[k][prev] * (dur - 1) + item->p1[k][cur];
                            item->smthp[k][cur] = prob_fix = (int)((temp_dur + (dur >> 1)) / dur);
                            temp_dur = (int64_t)wchar->smooth_tendency[smooth_tendency_prev] * (dur - 1) + wchar->priorities[cur];
                            wchar->smooth_tendency[smooth_tendency_cur] = (int)((temp_dur + (dur >> 1)) / dur);
                        }
                    }
                    else
                    {
                        item->smthp[k][cur] = prob_fix = item->p1[k][cur] / dur;
                        wchar->smooth_tendency[smooth_tendency_cur] = wchar->priorities[cur] / dur;
                    }
                }
                else
                {
                    temp_dur = (int64_t)item->p2[k - 1] * wchar->priorities[cur];
                    item->p1[k][cur] = DNN_SCORE_SHR(temp_dur, 31);

                    if (w->cur > 0)
                    {
                        if (frv->index != dur && w->cur > 1)
                        {
                            temp_dur = (int64_t)item->smthp[k][prev] * dur - item->p1[k][pprev] + item->p1[k][cur];
                            item->smthp[k][cur] = prob_fix = (int)((temp_dur + (dur >> 1)) / dur);
                            temp_dur = (int64_t)wchar->smooth_tendency[smooth_tendency_prev] * dur - wchar->priorities[pprev] +
                                       wchar->priorities[cur];
                            wchar->smooth_tendency[smooth_tendency_cur] = (int)((temp_dur + (dur >> 1)) / dur);
                        }
                        else
                        {
                            temp_dur = (int64_t)item->smthp[k][prev] * (dur - 1) + item->p1[k][cur];
                            item->smthp[k][cur] = prob_fix = (int)((temp_dur + (dur >> 1)) / dur);
                            temp_dur = (int64_t)wchar->smooth_tendency[smooth_tendency_prev] * (dur - 1) + wchar->priorities[cur];
                            wchar->smooth_tendency[smooth_tendency_cur] = (int)((temp_dur + (dur >> 1)) / dur);
                        }
                    }
                    else
                    {
                        item->smthp[k][cur] = prob_fix = item->p1[k][cur] / dur;
                        wchar->smooth_tendency[smooth_tendency_cur] = wchar->priorities[cur] / dur;
                    }
                }

                //-------------------------------------------------------------
                // 更新唤醒词中第i个字的平滑联合概率p2, 取区间 [StartPos, endPos]内的最大值
                if (w->cur < w->cfg->wmax)
                {
                    if (prob_fix > item->p2[k])
                    {
                        item->pidx[k] = w->cur;
                        item->p2[k] = prob_fix;
                    }
                }
                else
                {
                    startPos = w->cur + 1 - w->cfg->wmax;
                    endPos   = w->cur - AISPEECH_SPACE;

                    if (item->pidx[k] < startPos)
                    {
                        // 最大值被移出区间，重新计算整个区间的最大值
                        item->p2[k] = PZERO;

                        for (j = startPos; j <= endPos; ++j)
                        {
                            if (item->smthp[k][j % w->nslot] >= item->p2[k])
                            {
                                item->pidx[k] = j;
                                item->p2[k] = item->smthp[k][j % w->nslot];
                            }
                        }
                    }
                    else
                    {
                        if (item->smthp[k][endPos % w->nslot] >= item->smthp[k][item->pidx[k] % w->nslot])
                        {
                            item->pidx[k] = endPos;
                            item->p2[k]   = item->smthp[k][endPos % w->nslot];
                        }
                    }
                }
            }

            // 对唤醒词进行打分
            if (i == 0)   // 第一个字
            {
                wrd->p1[i][cur_eword] = (fxlog_q31(item->smthp[item->n_wchars - 1][cur]) + (item->n_wchars >> 1)) / item->n_wchars;
                wrd->smthp[i][cur_eword] = prob_fix = wrd->p1[i][cur_eword];
            }
            else  // 唤醒词的后续字
            {
                wrd->p1[i][cur_eword] = wrd->p2[i - 1] + ((fxlog_q31(item->smthp[item->n_wchars - 1][cur]) +
                                        (item->n_wchars >> 1)) / item->n_wchars);
                wrd->smthp[i][cur_eword] = prob_fix = wrd->p1[i][cur_eword];
            }

            //-------------------------------------------------------------
            // 更新唤醒词中第i个字的平滑联合概率p2, 取区间 [StartPos, endPos]内的最大值
            if (w->cur < w->cfg->wmax_eword)
            {
                if (prob_fix > wrd->p2[i])
                {
                    wrd->pidx[i] = w->cur;
                    wrd->p2[i] = prob_fix;
                }
            }
            else
            {
                startPos = w->cur + 1 - w->cfg->wmax_eword;
                endPos   = w->cur - AISPEECH_SPACE;

                if (wrd->pidx[i] < startPos)
                {
                    // 最大值被移出区间，重新计算整个区间的最大值
                    wrd->p2[i] = PZERO;

                    for (j = startPos; j <= endPos; ++j)
                    {
                        if (wrd->smthp[i][j % w->nslot_eword] >= wrd->p2[i])
                        {
                            wrd->pidx[i] = j;
                            wrd->p2[i] = wrd->smthp[i][j % w->nslot_eword];
                        }
                    }
                }
                else
                {
                    if (wrd->smthp[i][endPos % w->nslot_eword] >= wrd->smthp[i][wrd->pidx[i] % w->nslot_eword])
                    {
                        wrd->pidx[i] = endPos;
                        wrd->p2[i]   = wrd->smthp[i][endPos % w->nslot_eword];
                    }
                }
            }
        }
    }

    // 更新每个唤醒词的置信度
    for (idx = 0; idx < w->wrds->n_wrds; idx++)
    {
        wrd = &w->wrds->wrds[idx];
        wrd->conf = (wrd->smthp[wrd->n_wchars - 1][cur_eword] + (wrd->n_wchars >> 1)) / wrd->n_wchars;
#ifdef WAKEUP_DUR_PENALTY

        if (w->cfg->use_duration_penalty == 1)
        {
            wtk_wakeup_dnn_penalty_update(&w->duration_penalty, w->parm->cfg, w->cfg, wrd, idx);
        }

#endif
#if defined(WAKEUP_DUMP_SCORE)
        printf("%7.20f ", exp((float)wrd->conf / (1 << 26)));

        if (idx == w->wrds->n_wrds - 1)
        {
            printf("\n");
        }

#endif
    }
}
#endif

/*
 *
 */
static int _wakeup_dnn_score(wtk_wakeup_dnn_t *w, wtk_frv_t *frv)
{
    int iIdx = frv->index;
    float weight = 0.0;
#if defined(WAKEUP_NN_DNN)
    int iSkipFrms = w->parm->dnn->cfg->skip_frame;
#elif defined(WAKEUP_NN_FSMN)
    int iSkipFrms = w->parm->fsmn->cfg->skip_frame;
#endif

    if (iSkipFrms > 0 && (iIdx % iSkipFrms) != 1)
    {
        return -1;
    }

#ifdef WAKEUP_DICT_PHONEME

    if (w->cfg->use_phoneme)
    {
        _wakeup_dnn_progressive_smooth_phoneme(w, frv, weight);
    }
    else
#endif
#ifdef WAKEUP_DICT_WORD
        if (w->cfg->use_e2e)
        {
            _wakeup_dnn_progressive_smooth_e2e(w, frv, weight);
        }
        else
#endif
        {
#ifdef WAKEUP_DICT_CHAR
            _wakeup_dnn_progressive_smooth(w, frv, weight);
#else
            return -1;
#endif
        }

    w->cur++;
    return 0;
}

#ifndef _MSC_VER
#ifndef USE_CM4_OPTIMIZE
#pragma GCC push_options
#pragma GCC optimize("O0")
#endif
#endif

wtk_wakeup_dnn_t *wtk_wakeup_dnn_new(wtk_wakeup_dnn_cfg_t *cfg)
{
    wtk_wakeup_dnn_t *w;
    w = (wtk_wakeup_dnn_t *)wtk_calloc(1, sizeof(wtk_wakeup_dnn_t));

    if (NULL == w)
    {
        return NULL;
    }

    w->cfg  = cfg;
    w->parm = wtk_parm_new(&cfg->parm);
    w->parm->output_queue = &w->feature_q;
#ifdef WAKEUP_DICT_PHONEME

    if (cfg->use_phoneme)
    {
        w->nslot = AISP_TSL_MAX(w->cfg->wmax, w->cfg->wsmooth) + 1;
        //NOTE: no smooth support for eword, if it needs later, nslot_eword should be the result of expr "max(wmax_eword, wsmooth_eword) + 1"
        w->nslot_eword = w->cfg->wmax_eword + 1;
    }
    else
#endif
#ifdef WAKEUP_DICT_WORD
        if (cfg->use_e2e)
        {
            w->nslot = w->cfg->wsmooth;
        }
        else
#endif
        {
#ifdef WAKEUP_DICT_CHAR
            w->nslot = AISP_TSL_MAX(w->cfg->wmax, w->cfg->wsmooth) + 1;
#else
            return NULL;
#endif
        }

#ifndef WKP_VAD_DISABLE

    if (w->cfg->use_vad == 1)
    {
        w->vad_status = WAKEUP_VAD_INIT;
        wtk_queue_init(&w->vad_q);
        w->vad = wtk_vad2_new(&cfg->vad, w->parm, &w->vad_q);
    }

#endif
    wtk_wakeup_dnn_reset(w);
    return w;
}

#ifndef _MSC_VER
#ifndef USE_CM4_OPTIMIZE
#pragma GCC pop_options
#endif
#endif

void wtk_wakeup_dnn_delete(wtk_wakeup_dnn_t *w)
{
    if (w)
    {
#ifndef WKP_VAD_DISABLE

        if (w->cfg->use_vad == 1)
        {
            wtk_vad2_delete(w->vad);
        }

#endif
        wtk_parm_delete(w->parm);
#ifdef WAKEUP_DUR_PENALTY
        wtk_wakeup_dnn_penalty_reset(&w->duration_penalty);
#endif

        if (w->wrds)
        {
            int i;
#if defined(WAKEUP_DICT_PHONEME) || defined(WAKEUP_DICT_CHAR)

            for (i = 0; i < w->wrds->n_wchars; i++)
            {
                wtk_wakeup_dnn_wchar_t *wchar = &w->wrds->wchars[i];
                wtk_free(wchar->smooth_tendency);
                wtk_free(wchar->priorities);
                wtk_free(wchar->p1);
            }

#endif
            wtk_free(w->wrds->wchars);

            for (i = 0; i < w->wrds->n_wrds; i++)
            {
                wtk_wakeup_dnn_word_t *wrd = &w->wrds->wrds[i];
#if defined(WAKEUP_DICT_PHONEME) || defined(WAKEUP_DICT_CHAR)
                int j;

                for (j = 0; j < w->wrds->wrds[i].n_wchars; j++)
                {
                    wtk_free(wrd->smthp[j]);
                    wtk_free(wrd->p1[j]);
                }

                wtk_free(wrd->p1);
                wtk_free(wrd->p2);
                wtk_free(wrd->pidx);
                wtk_free(wrd->smthp);
#endif
                wtk_free(wrd->wchars);
            }

            wtk_free(w->wrds->wrds);
            wtk_free(w->wrds);
        }

        wtk_free(w);
    }
}

void wtk_wakeup_dnn_reset(wtk_wakeup_dnn_t *w)
{
    if (w)
    {
#ifndef WKP_VAD_DISABLE

        if (w->cfg->use_vad == 1)
        {
            wtk_vad2_restart(w->vad);
        }

#endif
#ifdef WAKEUP_DUR_PENALTY

        if (w->cfg->use_duration_penalty == 1)
        {
            wtk_wakeup_dnn_penalty_reset(&w->duration_penalty);
        }

#endif
        wtk_parm_reset(w->parm);
#if defined(WAKEUP_RESET)
        wtk_queue_init2(&w->feature_q);
#endif
        w->cur = 0;
        w->score = PZERO;
        w->result.conf           = 0.0;
        w->result.waked          = 0;
        w->result.waked_word     = NULL;
        w->result.major          = 0;

        if (NULL != w->wrds)
        {
#ifdef WAKEUP_DICT_PHONEME

            if (w->cfg->use_phoneme)
            {
                if (w->use_env == 1)
                {
                    wtk_wakeup_dnn_wordlist_delete_phoneme(w->wrds);
                }
                else
                {
                    wtk_wakeup_dnn_wordlist_reset(w->wrds);
                }
            }
            else
#endif
#ifdef WAKEUP_DICT_WORD
                if (w->cfg->use_e2e)
                {
                    if (w->use_env == 1)
                    {
                        wtk_wakeup_dnn_wordlist_delete_e2e(w->wrds);
                    }
                    else
                    {
                        wtk_wakeup_dnn_wordlist_reset(w->wrds);
                    }
                }
                else
#endif
                {
#ifdef WAKEUP_DICT_CHAR

                    if (w->use_env == 1)
                    {
                        wtk_wakeup_dnn_wordlist_delete(w->wrds);
                    }
                    else
                    {
                        wtk_wakeup_dnn_wordlist_reset(w->wrds);
                    }

#endif
                }

#if defined(WAKEUP_XCHECK) || defined(WAKEUP_XTERM) || defined(WAKEUP_DELAY)
            {
                int i = 0;

                for (i = 0; i < w->wrds->n_wrds; i++)
                {
                    wtk_wakeup_dnn_word_t *wrd = &w->wrds->wrds[i];
#ifdef WAKEUP_XCHECK

                    if (NULL != wrd->pstCharCheck)
                    {
                        wtk_wakeup_dnn_xcheck_reset(wrd);
                    }

#endif
#ifdef WAKEUP_XTERM

                    if (NULL != wrd->pstMaxConfSearch)
                    {
                        wtk_wakeup_dnn_max_conf_search_reset(wrd->pstMaxConfSearch);
                    }

#endif
#ifdef WAKEUP_DELAY

                    if (NULL != wrd->pstDelayWakeup)
                    {
                        wtk_wakeup_dnn_delay_wakeup_reset(wrd->pstDelayWakeup);
                    }

#endif
                }
            }
#endif
        }

        w->use_env = 1;
    }
}

int wtk_wakeup_dnn_start(wtk_wakeup_dnn_t *w, wtk_wakeup_env_t *env)
{
    w->cur   = 0;

    if (_wakeup_update_wrds(w, env) == -1)
    {
        return -1;
    }

    wtk_queue_init2(&w->feature_q);
#ifdef WAKEUP_DICT_PHONEME

    if (w->cfg->use_phoneme)
    {
        wtk_wakeup_dnn_wordlist_alloc_phoneme(w->wrds, w->nslot, w->nslot_eword);
    }
    else
#endif
    {
#if defined(WAKEUP_DICT_CHAR) || defined(WAKEUP_DICT_WORD)
        wtk_wakeup_dnn_wordlist_alloc(w->wrds, w->nslot);
#else
        return -1;
#endif
    }

#if defined(WAKEUP_NN_DNN)
    w->parm->dnn->cblas->hook = w->wrds;
#elif defined(WAKEUP_NN_FSMN)
    w->parm->fsmn->hook = w->wrds;
#endif
    wtk_parm_feature_alloc(w->parm);
    wtk_parm_frv_alloc(w->parm, w->wrds->n_wchars);
#ifdef WAKEUP_DUR_PENALTY

    if (w->cfg->use_duration_penalty)
    {
        wtk_wakeup_dnn_penalty_init(&w->duration_penalty, w->cfg->score_false_reduce_duration, w->wrds->n_wrds);
    }

#endif
#if defined(WAKEUP_XCHECK) || defined(WAKEUP_XTERM) || defined(WAKEUP_DELAY)
    {
        int i;

        for (i = 0; i < w->wrds->n_wrds; i++)
        {
            wtk_wakeup_dnn_word_t *wrd = &w->wrds->wrds[i];
#ifdef WAKEUP_XCHECK

            if (NULL != wrd->pstCharCheck)
            {
                wtk_wakeup_dnn_xcheck_reset(wrd);
            }

#endif
#ifdef WAKEUP_XTERM

            if (NULL != wrd->pstMaxConfSearch)
            {
                wtk_wakeup_dnn_max_conf_search_reset(wrd->pstMaxConfSearch);
            }

#endif
#ifdef WAKEUP_DELAY

            if (NULL != wrd->pstDelayWakeup)
            {
                wtk_wakeup_dnn_delay_wakeup_reset(wrd->pstDelayWakeup);
            }

#endif
        }
    }
#endif
    return 0;
}

int wtk_wakeup_dnn_end(wtk_wakeup_dnn_t *w)
{
    return wtk_wakeup_dnn_feed(w, NULL, 0, 1);
}

wtk_wakeup_dnn_result_t *wtk_wakeup_dnn_get_result(wtk_wakeup_dnn_t *w)
{
    return &w->result;
}

#if defined(WAKEUP_DICT_PHONEME) || defined(WAKEUP_DICT_CHAR)
static int wakeup_dnn_score_apply_normal_post(wtk_wakeup_dnn_t *w, wtk_frv_t *frv)
{
    int i;
    int thresh;
    wtk_wakeup_dnn_word_t *wrd;
#ifdef WAKEUP_XCHECK
    wtk_wakeup_dnn_char_check_t *pstCharCheck;
#endif
#ifdef WAKEUP_XTERM
    wtk_wakeup_dnn_max_conf_search_t *pstMaxConfSearch;
#endif
    char cMaxSearching;

    for (i = 0; i < w->wrds->n_wrds; i++)
    {
        wrd = &w->wrds->wrds[i];
        thresh = wrd->threshold;
#ifdef WAKEUP_XTERM
        pstMaxConfSearch = wrd->pstMaxConfSearch;
#endif
#ifdef WAKEUP_DELAY

        /*
         * if the word requires wakeup delay, skip
         */
        if (NULL != wrd->pstDelayWakeup)
        {
            continue;
        }

#endif
#ifdef WAKEUP_XTERM
        cMaxSearching = NULL != pstMaxConfSearch && pstMaxConfSearch->cMaxSearching;
#else
        cMaxSearching = 0;
#endif

        if (cMaxSearching || wrd->conf > thresh)
        {
#ifdef WAKEUP_XCHECK
            pstCharCheck = wrd->pstCharCheck;

            if (NULL != pstCharCheck && !cMaxSearching)
            {
                // failed to pass char check
                if (0 != wtk_wakeup_dnn_xcheck(w, wrd, pstCharCheck->sOffset))
                {
                    continue;
                }
            }

#endif

            if (cMaxSearching || w->score < wrd->conf)
            {
#ifdef WAKEUP_XTERM
                pstMaxConfSearch = wrd->pstMaxConfSearch;

                if (w->cfg->use_max_conf_search && NULL != pstMaxConfSearch)
                {
                    S64 llFrameIndex;
                    int conf;

                    /* search max conf for the last char in wakeup word */
                    if (wtk_wakeup_search_max_score(w, wrd, frv, &llFrameIndex, &conf) < 0)
                    {
                        continue;
                    }

                    w->score = conf;
                    w->result.conf = conf;
                    w->result.llFrameIndex = llFrameIndex;
                }
                else
#endif
                {
                    w->score = wrd->conf;
                    w->result.conf = wrd->conf;
                    w->result.llFrameIndex = w->parm->llAbsFrameIndex;
                }

                w->result.waked_word = wrd->word;
                w->result.major = wrd->major;
                w->result.waked = 1;
            }
        }
    }

    return 0;
}

#ifdef WAKEUP_DELAY
static int wakeup_dnn_score_apply_delay_post(wtk_wakeup_dnn_t *w, wtk_frv_t *frv)
{
    int i;
    int j;
    int thresh;
    int iSearchRatio = 0;
    int iInW32Q24    = 0;
    wtk_wakeup_dnn_word_t *wrd;
    wtk_wakeup_dnn_cfg_t *cfg = w->cfg;
    int iMaxDelayWakeupConf = PZERO;
#ifdef WAKEUP_XCHECK
    wtk_wakeup_dnn_char_check_t *pstCharCheck;
#endif
    wtk_wakeup_dnn_delay_wakeup_t *pstDelayWakeup;

    /*
     * get max conf of current frame for the wakeup words applied with wakeup delay feature
     */
    for (i = 0; i < w->wrds->n_wrds; i++)
    {
        wrd = &w->wrds->wrds[i];

        /*
         * if the word does not require wakeup delay, skip
         */
        if (NULL == wrd->pstDelayWakeup)
        {
            continue;
        }

        if (wrd->conf > iMaxDelayWakeupConf)
        {
            iMaxDelayWakeupConf = wrd->conf;
        }
    }

    /*
     * wakeup delay feature
     */
    for (i = 0; i < w->wrds->n_wrds; i++)
    {
        wrd = &w->wrds->wrds[i];
        thresh = wrd->threshold;
        pstDelayWakeup = wrd->pstDelayWakeup;
#ifdef WAKEUP_XCHECK
        pstCharCheck = wrd->pstCharCheck;
#endif

        /*
         * if the word does not require wakeup delay, skip
         */
        if (NULL == pstDelayWakeup)
        {
            continue;
        }

        /*
         * if the first peak position hasn't been found
         */
        if (pstDelayWakeup->i1stPeakPos <= 0)
        {
            /*
             * current frame's conf is larger than the last one's
             */
            if (wrd->conf > pstDelayWakeup->iLastConf)
            {
                if (wrd->conf < thresh)
                {
                    wtk_wakeup_dnn_delay_wakeup_reset(pstDelayWakeup);
                    continue;
                }

#ifdef WAKEUP_XCHECK

                if (NULL != pstCharCheck)
                {
                    // failed to pass char check
                    if (0 != wtk_wakeup_dnn_xcheck(w, wrd, pstCharCheck->sOffset))
                    {
                        wtk_wakeup_dnn_delay_wakeup_reset(pstDelayWakeup);
                        continue;
                    }
                }

#endif
                pstDelayWakeup->iLastConf = wrd->conf;

                if (wrd->conf > cfg->delay_wakeup_thresh_times + thresh || wrd->conf > -46516320/*ln(0.5) in Q26*/)
                {
                    goto wakeup;
                }

                continue;
            }
        }

        if (wrd->conf > pstDelayWakeup->iPeakConf)
        {
            if (pstDelayWakeup->i1stPeakPos <= 0)
            {
                pstDelayWakeup->i1stPeakPos = frv->index - 1;
                pstDelayWakeup->iPeakConf = pstDelayWakeup->iLastConf;
            }
            else
            {
                pstDelayWakeup->iPeakConf = wrd->conf;
            }

            iInW32Q24    = PSHR_POSITIVE(pstDelayWakeup->iPeakConf - thresh, 2); /* Q24 = Q26 >> 2 */
            iSearchRatio = AISP_TSL_sigmoid_xW32Q24_yW32Q24(iInW32Q24);
            iSearchRatio = (((long long)1 << 48) + (iSearchRatio >> 1)) / iSearchRatio - 16777216; /* exp(-x) = 1/sigmoid - 1 */
            /* Make sure that delay_wakeup_search1 is less than 127 */
            pstDelayWakeup->iMaxFrames = cfg->delay_wakeup_search0 + PSHR_POSITIVE(cfg->delay_wakeup_search1 * iSearchRatio, 24);

            if (pstDelayWakeup->iMaxFrames > cfg->delay_wakeup_search_max)
            {
                pstDelayWakeup->iMaxFrames = cfg->delay_wakeup_search_max;
            }
        }

        if (pstDelayWakeup->i1stPeakPos > 0 && pstDelayWakeup->iPeakConf < iMaxDelayWakeupConf)
        {
            wtk_wakeup_dnn_delay_wakeup_reset(pstDelayWakeup);
        }

        if (pstDelayWakeup->i1stPeakPos > 0 && frv->index - pstDelayWakeup->i1stPeakPos >= pstDelayWakeup->iMaxFrames)
        {
wakeup:
            w->result.conf = wrd->conf;
            w->result.llFrameIndex = w->parm->llAbsFrameIndex;
            w->result.waked_word = wrd->word;
            w->result.major = wrd->major;
            w->result.waked = 1;

            for (j = 0; j < w->wrds->n_wrds; j++)
            {
                if (NULL != w->wrds->wrds[i].pstDelayWakeup)
                {
                    wtk_wakeup_dnn_delay_wakeup_reset(w->wrds->wrds[i].pstDelayWakeup);
                }
            }

            break;
        }
    }

    return 0;
}
#endif
#endif

static int _wakeup_dnn_score_post_process(wtk_wakeup_dnn_t *w, wtk_frv_t *frv)
{
#ifdef WAKEUP_DICT_WORD

    if (w->cfg->use_e2e)
    {
        wakeup_dnn_score_apply_normal_post_e2e(w, frv);
    }
    else
#endif
    {
#if defined(WAKEUP_DICT_PHONEME) || defined(WAKEUP_DICT_CHAR)
        wakeup_dnn_score_apply_normal_post(w, frv);
#ifdef WAKEUP_DELAY
        wakeup_dnn_score_apply_delay_post(w, frv);
#endif
#else
        return -1;
#endif
    }

    return 0;
}

int wtk_wakeup_dnn_feed(wtk_wakeup_dnn_t *w, char *data, int bytes, int is_end)
{
    wtk_frv_t *frv;
#ifndef WKP_VAD_DISABLE
    wtk_frv_t *frv_vad;
    wtk_queue_node_t *n_vad;
#endif
    wtk_queue_node_t *n;
#ifdef WAKEUP_DUR_PENALTY

    if (w->cfg->use_duration_penalty)
    {
        wtk_wakeup_dnn_penalty_set(&w->duration_penalty, w->duration_penalty.frame_robin->speech_frames,
                                   (w->duration_penalty.false_reduce_all_frames * (w->parm->cfg->frame_step << 1) + bytes) /
                                   (w->parm->cfg->frame_step << 1),
                                   w->duration_penalty.false_reduce_all_frames);
    }

#endif
    wtk_parm_feed2(w->parm, (is_end ? WTK_PARM_END : WTK_PARM_APPEND), data, bytes);
#ifndef WKP_VAD_DISABLE

    if (w->cfg->use_vad) //vad routine
    {
        if (is_end) //wakeup end
        {
            while (w->feature_q.length != 0)
            {
                n_vad = wtk_queue_pop(&w->feature_q);//wakeup flush end

                if (n_vad == NULL)
                {
                    break;
                }

                frv_vad = data_offset(n_vad, wtk_frv_t, hoard_n);
                _wakeup_dnn_score(w, frv_vad);
                wtk_frv_push_back(frv_vad);
                _wakeup_dnn_score_post_process(w, frv_vad, 0);
                wtk_vad2_feed(w->vad, WTK_PARM_APPEND, frv_vad);

                while (w->vad_q.length != 0)
                {
                    n_vad = wtk_queue_pop(&w->vad_q);
                    frv_vad = data_offset(n_vad, wtk_frv_t, vadout_n);
                    vad_post_state(w, frv_vad->state, frv_vad->index);
#ifdef WAKEUP_DUR_PENALTY

                    if (w->cfg->use_duration_penalty == 1)
                    {
                        wtk_wakeup_dnn_penalty_push(&w->duration_penalty, frv_vad->state);
                    }

#endif

                    if (frv_vad->state == wtk_vframe_speech)
                    {
                        if (w->result.waked == 1)
                        {
                            break;
                        }
                    }
                }

                if (w->result.waked == 1)
                {
                    break;
                }
            }

            wtk_dnnvad_feed_end(w->vad->dnnvad);//vad flush end
            wtk_vad2_flush_cache(w->vad);

            while (w->vad_q.length != 0)
            {
                n_vad = wtk_queue_pop(&w->vad_q);
                frv_vad = data_offset(n_vad, wtk_frv_t, vadout_n);
                vad_post_state(w, frv_vad->state, frv_vad->index);
#ifdef WAKEUP_DUR_PENALTY

                if (w->cfg->use_duration_penalty == 1)
                {
                    wtk_wakeup_dnn_penalty_push(&w->duration_penalty, frv_vad->state);
                }

#endif

                if (frv_vad->state == wtk_vframe_speech)
                {
                    if (w->result.waked == 1)
                    {
                        break;
                    }
                }
            }
        }
        else
        {
            while (w->feature_q.length != 0)
            {
                n_vad = wtk_queue_pop(&w->feature_q);

                if (n_vad == NULL)
                {
                    break;
                }

                frv_vad = data_offset(n_vad, wtk_frv_t, hoard_n);
                _wakeup_dnn_score(w, frv_vad);
                wtk_frv_push_back(frv_vad);
                _wakeup_dnn_score_post_process(w, frv_vad, 0);
                wtk_vad2_feed(w->vad, WTK_PARM_APPEND, frv_vad);

                while (w->vad_q.length != 0)
                {
                    n_vad = wtk_queue_pop(&w->vad_q);
                    frv_vad = data_offset(n_vad, wtk_frv_t, vadout_n);
                    vad_post_state(w, frv_vad->state, frv_vad->index);
#ifdef WAKEUP_DUR_PENALTY

                    if (w->cfg->use_duration_penalty == 1)
                    {
                        wtk_wakeup_dnn_penalty_push(&w->duration_penalty, frv_vad->state);
                    }

#endif

                    if (frv_vad->state == wtk_vframe_speech)
                    {
                        if (w->result.waked == 1)
                        {
                            break;
                        }
                    }
                }

                if (w->result.waked == 1)
                {
                    break;
                }
            }
        }
    }
    else  // no vad routine
#endif
    {
        while (w->feature_q.length)
        {
            n = wtk_queue_pop(&w->feature_q);

            if (NULL == n)
            {
                break;
            }

            frv = data_offset(n, wtk_frv_t, hoard_n);
            _wakeup_dnn_score(w, frv);
#ifdef WAKEUP_DUR_PENALTY

            if (w->cfg->use_duration_penalty)
            {
                // consider as silence if sil's probability >= 0.5
                int state = frv->dnn_v[1] >= 1073741824/*0.5 in Q31*/ ? wtk_vframe_sil : wtk_vframe_speech;
                wtk_wakeup_dnn_penalty_push(&w->duration_penalty, state);
            }

#endif
            _wakeup_dnn_score_post_process(w, frv);
            wtk_frv_push_back(frv);

            if (w->result.waked == 1)
            {
                break;
            }
        }
    }

    return 0;
}
