#include "wtk_vad2.h"
void wtk_vad2_feed_frv(wtk_vad2_t *v, wtk_frv_t *frv);
void wtk_vad2_feed_speech_end(wtk_vad2_t *v, wtk_frv_t *frv);
void wtk_vad2_feed_min_speech(wtk_vad2_t *v, wtk_frv_t *frv);
void wtk_vad2_flush_cache(wtk_vad2_t *v);

wtk_vad2_t *wtk_vad2_new(wtk_vad2_cfg_t *cfg, wtk_parm_t *p, wtk_queue_t *output_queue)
{
    wtk_vad2_t *v;

    v = (wtk_vad2_t *)wtk_malloc(sizeof(*v));
    v->cfg = cfg;
    v->state = WTK_VAD2_SIL;
    v->parm = p;
    // 设置子模块的帧输出接口
    v->dnnvad = wtk_dnnvad_new(&(cfg->dnnvad), v, (wtk_vframe_raise_f)wtk_vad2_feed_frv);
    v->output_queue = output_queue;
    wtk_queue_init(&(v->cache_q));

    v->speech_end_sil_frames = 0;
    v->min_speech_count = 0;

    return v;
}

void wtk_vad2_delete(wtk_vad2_t *v)
{
    wtk_dnnvad_delete(v->dnnvad);
    wtk_free(v);
}

int wtk_vad2_start(wtk_vad2_t *v)
{
    int ret;

    v->state = WTK_VAD2_SIL;
    wtk_queue_init(&(v->cache_q));
    ret = 0;
    return ret;
}

/**
 * 重置函数
 */
int wtk_vad2_reset(wtk_vad2_t *v)
{
    int ret;

    v->speech_end_sil_frames = 0;
    v->min_speech_count = 0;
    wtk_dnnvad_reset(v->dnnvad);
    ret = 0;

    v->state = WTK_VAD2_SIL;
    return ret;
}

int wtk_vad2_restart(wtk_vad2_t *v)
{
    int ret;
    wtk_queue_node_t *n;
    wtk_frv_t *frv;
    wtk_dnnvad_feed_end(v->dnnvad);
    wtk_vad2_flush_cache(v);
    while(v->output_queue->length != 0)
    {
        n = wtk_queue_pop(v->output_queue);
        frv =data_offset(n,wtk_frv_t,vadout_n);
        wtk_parm_reuse_frv(v->parm,frv);
    }
    ret = wtk_vad2_reset(v);
    if (ret != 0) {
        goto end;
    }

    ret = wtk_vad2_start(v);

end:
    return ret;
}

void wtk_vad2_end(wtk_vad2_t *v)
{
    wtk_dnnvad_feed_end(v->dnnvad);
    wtk_vad2_flush_cache(v);
}

/**
 * 将指定的一帧特征保存到output_queue中
 */
void wtk_vad2_raise_frv(wtk_vad2_t *v, wtk_frv_t *frv)
{
    wtk_queue_push(v->output_queue, &(frv->vadout_n));
}

/**
 * 将cache_q中的特征, 除了最新的left_slot个，其他的全部推入output_queue
 */
void wtk_vad2_flush_queue(wtk_vad2_t *v, int left_slot)
{
    wtk_queue_t *q = &(v->cache_q);
    wtk_queue_node_t *n;
    wtk_frv_t *frv;

    while (q->length > left_slot) {
        n = wtk_queue_pop(q);
        if (!n) {
            break;
        }
        frv = data_offset(n, wtk_frv_t, vadpost_n);
        wtk_vad2_raise_frv(v, frv);
    }
}

/**
 * 设置cache_q中所有节点的状态
 */
void wtk_vad2_set_cache_state(wtk_vad2_t *v, wtk_vframe_state_t state)
{
    wtk_queue_node_t *n;
    wtk_queue_t *q = &(v->cache_q);
    wtk_frv_t *f;

    for (n = q->pop; n; n = n->next) {
        f = data_offset(n, wtk_frv_t, vadpost_n);
        f->state = state;
    }
}

/**
 * vad2状态为sil下处理输入的一帧
 *
 * 处理流程:
 *  - sil帧, 处理left_margin, 也就是保留cache_q中最右边的left_margin帧，　将之扩展为speech
 *  - speech_end_sil_frames
 *      - 如果min_speech有效，开始统计
 *      - 否则直接speech的处理
 */
void wtk_vad2_feed_sil(wtk_vad2_t *v, wtk_frv_t *frv)
{
    wtk_queue_t *q = &(v->cache_q);

    if (frv->state == wtk_vframe_sil) {
        // if vframe is silence, process left margin;
        wtk_queue_push(q, &(frv->vadpost_n));
        wtk_vad2_flush_queue(v, v->cfg->left_margin);

    } else {
        if (v->cfg->min_speech > 0) {
            v->state = WTK_VAD2_MIN_SPEECH;
            v->min_speech_count = 0;
            wtk_vad2_feed_min_speech(v, frv);
        } else {
            v->state = WTK_VAD2_SPEECH;
            wtk_vad2_feed_frv(v, frv);
        }
    }
}

/**
 *　统计最小帧数
 */
void wtk_vad2_feed_min_speech(wtk_vad2_t *v, wtk_frv_t *frv)
{
    wtk_queue_t *q = &(v->cache_q);

    wtk_queue_push(q, &(frv->hoard_n));

    if (frv->state == wtk_vframe_sil) {
        // if it is silence, goto sil state and leave left margin;
        wtk_vad2_set_cache_state(v, wtk_vframe_sil);
        wtk_vad2_flush_queue(v, v->cfg->left_margin);
        v->state = WTK_VAD2_SIL;
    } else {
        ++v->min_speech_count;
        if (v->min_speech_count >= v->cfg->min_speech) {
            wtk_vad2_set_cache_state(v, wtk_vframe_speech);
            wtk_vad2_flush_queue(v, 0);
            v->state = WTK_VAD2_SPEECH;
        }
    }
}

void wtk_vad2_feed_speech(wtk_vad2_t *v, wtk_frv_t *f)
{
     if (f->state == wtk_vframe_sil) {
        v->state = WTK_VAD2_SPEECH_END;
        v->speech_end_sil_frames = 0;
        wtk_vad2_feed_speech_end(v, f);
    } else {
        // if there is some silence, this will not be happen,but logical will be;
        if (v->cache_q.length > 0) {
            wtk_vad2_set_cache_state(v, wtk_vframe_speech);
            wtk_vad2_flush_queue(v, 0);
        }
        wtk_vad2_raise_frv(v, f);
    }
}

void wtk_vad2_flush_speech_end(wtk_vad2_t *v)
{
    wtk_queue_t *q = &(v->cache_q);

    if (q->length <= 0) {
        return;
    }

    // if right margin satisfied, flush speech and goto sil;
    wtk_vad2_set_cache_state(v, wtk_vframe_speech);

    // the last frame is speech end;
    wtk_vad2_flush_queue(v, 0);
}

/**
 * @brief 处理右边界
 */
void wtk_vad2_feed_speech_end(wtk_vad2_t *v, wtk_frv_t *frv)
{
    wtk_queue_t *q = &(v->cache_q);

    if (frv->state == wtk_vframe_sil) {
        if (v->cfg->right_margin <= 0) {
            wtk_vad2_raise_frv(v, frv);
            v->state = WTK_VAD2_SIL;
        } else {
            ++v->speech_end_sil_frames;
            if (q->length > 0) {
                wtk_vad2_flush_speech_end(v);
            }

            frv->state = wtk_vframe_speech;
            wtk_vad2_raise_frv(v, frv);
            if (v->speech_end_sil_frames >= v->cfg->right_margin) {
                v->state = WTK_VAD2_SIL;
            }
        }
    } else {
        if (q->length > 0) {
            wtk_vad2_flush_speech_end(v);
        }

        // check for min speech;
        v->state = WTK_VAD2_MIN_SPEECH;
        v->min_speech_count = 0;
        wtk_vad2_feed_min_speech(v, frv);
    }
}

/**
 * 子模块处理完之后的帧输出接口
 */
void wtk_vad2_feed_frv(wtk_vad2_t *v, wtk_frv_t *frv)
{
    switch (v->state) {
    case WTK_VAD2_SIL:
         wtk_vad2_feed_sil(v, frv);
         break;
    case WTK_VAD2_MIN_SPEECH:
         break;
    case WTK_VAD2_SPEECH:
         wtk_vad2_feed_speech(v, frv);
         break;

    case WTK_VAD2_SPEECH_END:
         wtk_vad2_feed_speech_end(v, frv);
         break;
    }
}

/**
 * @brief　将最后残留的帧全部输入到下一个模块
 */
void wtk_vad2_flush_cache(wtk_vad2_t *v)
{
    switch (v->state) {
    case WTK_VAD2_SIL:
        //　sil状态下，cache_q中会保留一些帧来处理左边界
        wtk_vad2_flush_queue(v, 0);
        break;

    case WTK_VAD2_MIN_SPEECH:
        //　统计min_speech的时候,cache_q中会保留一些帧来统计最小音频长度
        wtk_vad2_set_cache_state(v, wtk_vframe_sil);
        wtk_vad2_flush_queue(v, 0);
        break;

    case WTK_VAD2_SPEECH:
        wtk_vad2_set_cache_state(v, wtk_vframe_speech);
        wtk_vad2_flush_queue(v, 0);
        break;

    case WTK_VAD2_SPEECH_END:
        //　cache_q中会残留一些帧用作右边界扩展
        wtk_vad2_flush_speech_end(v);
        break;
    }
}

/**
 * VAD模块数据输入接口
 */
int wtk_vad2_feed(wtk_vad2_t *v, wtk_parm_state_t s, wtk_frv_t *frv)
{
    int ret;
    ret = wtk_dnnvad_feed(v->dnnvad, s, frv);

    if(s == WTK_PARM_END)
    {
        wtk_dnnvad_feed_end(v->dnnvad);
        //wtk_vad2_flush_cache(v);
    }

    return ret;
}

void wtk_vad2_push_frv(wtk_vad2_t *v, wtk_frv_t *f)
{
    //wtk_dnnvad_push_vframe(v->dnnvad, f);
}

void wtk_vad2_flush(wtk_vad2_t *v)
{
    // wtk_dnnvad_flush(v->dnnvad);
}
