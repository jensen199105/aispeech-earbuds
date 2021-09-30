#ifdef WAKEUP_DUR_PENALTY

#include "AISP_TSL_str.h"
#include "wtk/core/wtk_alloc.h"
#include "wtk_wakeup_dnn_penalty.h"

#define  bit(b)   (1 << (b))
#define  bitSet(value, bit)   ((value) |= (1UL << (bit)))
#define  bitClear(value, bit)   ((value) &= ~(1UL << (bit)))
#define  bitWrite(value, bit, bitvalue)   (bitvalue ? bitSet(value, bit) : bitClear(value, bit))
#define  bitRead(value, bit)   (((value) >> (bit)) & 0x01)

void wtk_wakeup_dnn_penalty_init(wtk_wakeup_dnn_duration_penalty_t *penalty, int nrobin, int n_coefs)
{
    if (NULL == penalty->frame_robin)
    {
        penalty->frame_robin = wtk_wakeup_module_frame_robin_new(nrobin);
    }

    if (NULL == penalty->wrd_coefs)
    {
        penalty->wrd_coefs = (wtk_wakeup_dnn_word_coef_t *)wtk_calloc(n_coefs, sizeof(wtk_wakeup_dnn_word_coef_t));
        penalty->n_coefs = n_coefs;
    }

    wtk_wakeup_dnn_penalty_reset(penalty);
}

void wtk_wakeup_dnn_penalty_reset(wtk_wakeup_dnn_duration_penalty_t *penalty)
{
    int i;
    penalty->false_reduce_all_frames = 0;
    penalty->false_reduce_vad_frames = 0;
    penalty->easy_wakeup_cur_frame = 0;

    if (NULL != penalty->frame_robin)
    {
        wtk_wakeup_module_frame_robin_reset(penalty->frame_robin);
    }

    if (NULL != penalty->wrd_coefs)
    {
        for (i = 0; i < penalty->n_coefs; i++)
        {
            AISP_TSL_memset(&penalty->wrd_coefs[i], 0, sizeof(penalty->wrd_coefs[i]));
        }
    }
}

void wtk_wakeup_dnn_penalty_set(wtk_wakeup_dnn_duration_penalty_t *penalty, int reduce_vframes, int reduce_aframes,
                                int cur_frame)
{
    penalty->false_reduce_vad_frames = reduce_vframes;
    penalty->false_reduce_all_frames = reduce_aframes;
    penalty->easy_wakeup_cur_frame = cur_frame;
}

void wtk_wakeup_dnn_penalty_push(wtk_wakeup_dnn_duration_penalty_t *penalty, int state)
{
    wtk_wakeup_module_frame_robin_push(penalty->frame_robin, state);
}

void wtk_wakeup_dnn_penalty_update(wtk_wakeup_dnn_duration_penalty_t *penalty, wtk_parm_cfg_t *parm_cfg,
                                   wtk_wakeup_dnn_cfg_t *cfg, wtk_wakeup_dnn_word_t *wrd, int idx)
{
    int score_false_reduce_coef = 0;
    int easy_wakeup_nowakeup_interval = 16000 / parm_cfg->frame_step; // 1s间隔
    wtk_wakeup_dnn_word_coef_t *coef = &penalty->wrd_coefs[idx];

    if (penalty->false_reduce_all_frames > cfg->score_false_reduce_duration &&
            penalty->false_reduce_vad_frames > cfg->score_false_reduce_nowakeup_duration)
    {
        score_false_reduce_coef = cfg->score_false_reduce_coef;//Q26
    }
    else
    {
        score_false_reduce_coef = 0;
    }

    wrd->conf += score_false_reduce_coef; //Q26

    if ((coef->easy_wakeup_prev_score_frame <= 0) ||
            (penalty->easy_wakeup_cur_frame - coef->easy_wakeup_prev_score_frame) > cfg->score_easy_wakeup_duration)
    {
        coef->score_easy_wakeup_coef = 0;
        coef->easy_wakeup_prev_score_frame = coef->easy_wakeup_cur_score_frame;

        if ((coef->easy_wakeup_prev_score_frame > 0)
                && (penalty->easy_wakeup_cur_frame - coef->easy_wakeup_prev_score_frame) > easy_wakeup_nowakeup_interval)
        {
            coef->score_easy_wakeup_coef = cfg->score_easy_wakeup_coef;
        }

        coef->easy_wakeup_cur_score_frame = 0;
    }
    else
        if ((penalty->easy_wakeup_cur_frame - coef->easy_wakeup_prev_score_frame) > easy_wakeup_nowakeup_interval)
        {
            coef->score_easy_wakeup_coef = cfg->score_easy_wakeup_coef;
        }
        else
        {
            coef->score_easy_wakeup_coef = 0;
        }

    // restart后,score_easy_wakeup_duration时长内,更容易唤醒
    if (cfg->use_restart_easy_wakeup == 1 &&
            penalty->easy_wakeup_cur_frame <= cfg->score_easy_wakeup_duration)
    {
        coef->score_easy_wakeup_coef = cfg->score_easy_wakeup_coef;
    }

    wrd->conf += coef->score_easy_wakeup_coef;//Q26

    if (wrd->conf + cfg->score_easy_wakeup_nowakeup_coef > wrd->threshold)
    {
        coef->easy_wakeup_cur_score_frame = penalty->easy_wakeup_cur_frame;
    }
    else
        if (penalty->easy_wakeup_cur_frame - coef->easy_wakeup_cur_score_frame > cfg->score_easy_wakeup_duration)
        {
            coef->easy_wakeup_cur_score_frame = 0;
        }
}

wtk_wakeup_module_frame_robin_t *wtk_wakeup_module_frame_robin_new(int n)
{
    int frame_size = 0;
    wtk_wakeup_module_frame_robin_t *robin = (wtk_wakeup_module_frame_robin_t *)wtk_malloc(sizeof(
                wtk_wakeup_module_frame_robin_t));
    frame_size = wtk_round_8(n) >> 3;   // wtk_round_8(n) / 8
    robin->frame = (unsigned char *)wtk_malloc(frame_size * sizeof(char));
    robin->speech_frames = 0;
    robin->length = n;
    robin->used = 0;
    robin->cur = 0;
    return robin;
}

void wtk_wakeup_module_frame_robin_reset(wtk_wakeup_module_frame_robin_t *robin)
{
    robin->speech_frames = 0;
    robin->used = 0;
    robin->cur = 0;
}

void wtk_wakeup_module_frame_robin_push(wtk_wakeup_module_frame_robin_t *r, char value)
{
    int byte_num = 0;
    int odds = 0;
    byte_num = r->cur / 8;
    odds = r->cur % 8;

    if (r->used != r->length)
    {
        r->used++;
    }
    else
    {
        if (bitRead(r->frame[byte_num], odds) == wtk_vframe_speech)
        {
            r->speech_frames--;
        }
    }

    bitWrite(r->frame[byte_num], odds, value);
    //r->cur = (++r->cur) % r->length;
    r->cur %= r->length;
    r->cur++;

    if (value == wtk_vframe_speech)
    {
        r->speech_frames++;
    }
}

void wtk_wakeup_module_frame_robin_delete(wtk_wakeup_module_frame_robin_t *r)
{
    if (r)
    {
        if (r->frame)
        {
            wtk_free(r->frame);
        }

        wtk_free(r);
    }
}

#endif
