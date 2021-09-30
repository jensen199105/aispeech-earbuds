#include "wtk_parm.h"
#include "wtk_sigp.h"
#ifdef USE_HIFI4_OPTIMISIZE
#include <xtensa/hal.h>
#endif
wtk_frv_t *wtk_parm_new_frv(wtk_parm_t *p, int nwchars);
wtk_feature_t *wtk_parm_new_feature(wtk_parm_t *p);

wtk_parm_t *wtk_parm_new(wtk_parm_cfg_t *cfg)
{
    return wtk_parm_new2(cfg);
}

wtk_parm_t *wtk_parm_new2(wtk_parm_cfg_t *cfg)
{
    wtk_parm_t *p;
    p = (wtk_parm_t *)wtk_malloc(sizeof(*p));
    wtk_parm_init(p, cfg);
    return p;
}

int wtk_parm_delete(wtk_parm_t *p)
{
    wtk_parm_clean(p);
    wtk_free(p);
    return 0;
}

int wtk_parm_frv_alloc(wtk_parm_t *p, int n_wchars)
{
    int t;
    wtk_frv_t *frv;

    for (t = 0; t < FRV_NODE_NUM; t++)
    {
        frv = wtk_parm_new_frv(p, n_wchars);
        wtk_queue_push(p->frv_hoard, &(frv->hoard_n));
    }

    return 0;
}

int wtk_parm_feature_alloc(wtk_parm_t *p)
{
    int t;
    wtk_feature_t *f;

    for (t = 0; t < p->zmean->cfg->win_size + 1; t++)
    {
        f = wtk_parm_new_feature(p);
        wtk_queue_push(p->feature_hoard, &(f->hoard_n));
    }

    return 0;
}


int wtk_parm_init(wtk_parm_t *p, wtk_parm_cfg_t *cfg)
{
    int t;
    int cache = cfg->cache_size;
    AISP_TSL_memset(p, 0, sizeof(*p));
    p->cfg = cfg;
    p->frame_buffer = wtk_short_vector_buffer_new(cfg->frame_size * cache);
    p->frame_vector = wtk_short_vector_new(cfg->frame_size);
    wtk_sigp_init(&(p->sigp), cfg);
    p->feature_hoard = wtk_queue_new();
    p->frv_hoard = wtk_queue_new();

    if (cfg->DELTA)
    {
        int del_win = cfg->del_win;
        int acc_win = cfg->acc_win;
        int third_win = cfg->third_win;
        p->feature_pos[0] = p->cfg->static_feature_cols + 1;
        p->feature_pos[1] = p->feature_pos[0] + p->cfg->static_feature_cols;
        p->feature_pos[2] = p->feature_pos[1] + p->cfg->static_feature_cols;
        p->robins[0] = wtk_robin_new(del_win * 2 + 1);
        p->win[0] = cfg->del_win;
        p->win[1] = cfg->acc_win;
        p->win[2] = cfg->third_win;
        p->diffs = 1;

        if (cfg->ACCS)
        {
            p->robins[1] = wtk_robin_new(acc_win * 2 + 1);
            ++p->diffs;
        }
        else
        {
            p->robins[1] = 0;
        }

        if (cfg->THIRD)
        {
            p->robins[2] = wtk_robin_new(third_win * 2 + 1);
            ++p->diffs;
        }
        else
        {
            p->robins[2] = 0;
        }

        t = del_win > acc_win ? del_win : acc_win;
        t = third_win > t ? third_win : t;
        t = t * 2 + 1;
        p->v_tmp_array = (aisp_s32_t **)wtk_malloc(t * sizeof(wtk_int_vector_t *));
        p->f_tmp_array = (wtk_feature_t **)wtk_malloc(t * sizeof(wtk_feature_t *));
    }

    p->n_frame_index = 0;

    if (cfg->ZMEAN)
    {
        if (cfg->use_cmn)
        {
            p->zmean = wtk_zmean_new(&(cfg->zmean), p);
            wtk_parm_set_static_post(p, (wtk_parm_static_post_f)wtk_zmean_static_post_feed, p->zmean);
            wtk_parm_set_post(p, (wtk_parm_post_f)wtk_zmean_post_feed, p->zmean);
        }
    }

#if defined(WAKEUP_NN_DNN)
    p->dnn = wtk_dnn_new(&cfg->dnn, p);
#elif defined(WAKEUP_NN_FSMN)
    p->fsmn = wtk_fsmn_new(&cfg->fsmn, p);
#endif
    p->output_queue = NULL;
    return 0;
}

int wtk_parm_clean(wtk_parm_t *p)
{
    int i;
    wtk_queue_node_t *n;
    wtk_feature_t *f;
    wtk_frv_t *frv;

    for (i = 0; i < 3; ++i)
    {
        if (p->robins[i])
        {
            wtk_robin_delete(p->robins[i]);
        }
    }

    if (p->v_tmp_array)
    {
        wtk_free(p->v_tmp_array);
    }

    if (p->f_tmp_array)
    {
        wtk_free(p->f_tmp_array);
    }

    wtk_sigp_clean(&(p->sigp));
    wtk_short_vector_buffer_delete(p->frame_buffer);
    wtk_vector_delete(p->frame_vector);

    while (p->feature_hoard->length != 0)
    {
        n = wtk_queue_pop(p->feature_hoard);
        f = data_offset(n, wtk_feature_t, hoard_n);

        if (!f)
        {
            break;
        }

        wtk_feature_delete(f);
    }

    while (p->frv_hoard->length != 0)
    {
        n = wtk_queue_pop(p->frv_hoard);
        frv = data_offset(n, wtk_frv_t, hoard_n);
        wtk_frv_delete(frv);
    }

    if (p->cfg->ZMEAN)
    {
        if (p->cfg->use_cmn)
        {
            wtk_zmean_delete(p->zmean);
        }
    }

    wtk_queue_delete(p->feature_hoard);
    wtk_queue_delete(p->frv_hoard);
#if defined(WAKEUP_NN_DNN)
    wtk_dnn_delete(p->dnn);
#elif defined(WAKEUP_NN_FSMN)
    wtk_fsmn_delete(p->fsmn);
#endif
    return 0;
}

void wtk_parm_set_output_queue(wtk_parm_t *p, wtk_queue_t *q)
{
    p->output_queue = q;
}

void wtk_parm_set_post(wtk_parm_t *p, wtk_parm_post_f post, void *hook)
{
    p->post_f = post;
    p->post_hook = hook;
}

void wtk_parm_set_static_post(wtk_parm_t *p, wtk_parm_static_post_f static_post, void *hook)
{
    p->static_post_f = static_post;
    p->static_post_hook = hook;
}

wtk_feature_t *wtk_parm_new_feature(wtk_parm_t *p)
{
    wtk_feature_t *f;
    f = wtk_feature_new2(p->cfg, p->xform_rows);
    f->send_hook = p;
    f->send = (wtk_feature_sender_t)wtk_parm_push_feature;
    return f;
}

wtk_frv_t *wtk_parm_new_frv(wtk_parm_t *p, int n_wchars)
{
    wtk_frv_t *frv;
    frv = wtk_frv_new(n_wchars);
    frv->send_hook = p;
    frv->send = (wtk_frv_sender_t)wtk_parm_push_frv;
    return frv;
}

int wtk_parm_push_feature(wtk_parm_t *p, wtk_feature_t *f)
{
    if (f->used == 0)
    {
        wtk_queue_push(p->feature_hoard, &(f->hoard_n));
    }

    return 0;
}

int wtk_parm_push_frv(wtk_parm_t *p, wtk_frv_t *frv)
{
    wtk_queue_push(p->frv_hoard, &(frv->hoard_n));
    return 0;
}

int wtk_parm_reuse_feature(wtk_parm_t *p, wtk_feature_t *f)
{
    --f->used;
    return wtk_parm_push_feature(p, f);
}

int wtk_parm_reuse_frv(wtk_parm_t *p, wtk_frv_t *frv)
{
    return wtk_parm_push_frv(p, frv);
}

wtk_feature_t *wtk_parm_pop_feature(wtk_parm_t *p)
{
    wtk_feature_t *f;
    f = (wtk_feature_t *)wtk_queue_pop(p->feature_hoard);
    f->used = 0;
    f->index = ++p->n_frame_index;
    return f;
}
/*
 *remove the front feature in robin.
 */
static void wtk_parm_flush_robin(wtk_parm_t *p, wtk_robin_t *r)
{
    wtk_feature_t *f;
    f = (wtk_feature_t *)wtk_robin_pop(r);
    --f->used;
    wtk_parm_push_feature(p, f);
}

/*
 *remove all the feature in robin.
 */
void wtk_parm_flush_robin_all(wtk_parm_t *p, wtk_robin_t *r)
{
    wtk_feature_t *f;

    while (r->used > 0)
    {
        f = (wtk_feature_t *)wtk_robin_pop(r);
        --f->used;
        wtk_parm_push_feature(p, f);
    }
}

static wtk_feature_t *wtk_parm_flush_feature(wtk_parm_t *p, wtk_parm_state_t s, wtk_robin_t *r, int win,
        aisp_s16_t diff_sigma, int startv)
{
    int i              = 0;
    int pad            = 0;
    int j              = 0;
    int is_end         = 0;
    aisp_s32_t    **pv = p->v_tmp_array;
    wtk_feature_t **fv = p->f_tmp_array;
    wtk_feature_t  *f;
    is_end = s == WTK_PARM_END;

    if (r->used <= win)
    {
        return 0;
    }

    pad = r->nslot - r->used;
    i   = 0;

    if (pad > 0 && !is_end)
    {
        // if not end, add pad to front.
        // * |f0|f1|f2|0|0|  => |f0|f0|f0|f1|f2|
        // * |f0|f1|f2|f3|0| => |f0|f0|f1|f2|f3|
        f = (wtk_feature_t *)wtk_robin_at(r, 0);

        for (; i < pad; ++i)
        {
            pv[i] = f->v;
            fv[i] = f;
        }
    }

    for (j = 0; j < r->used; ++i, ++j)
    {
        f = ((wtk_feature_t *)wtk_robin_at(r, j));
        pv[i] = f->v;
        fv[i] = f;
    }

    if (pad > 0 && is_end)
    {
        // if is end and pad to the end.
        //|f0|f1|f2|f3|0| => |f0|f1|f2|f3|f3|
        //|f0|f1|f2|0|0| => |f0|f1|f2|f2|f2|
        f = (wtk_feature_t *)wtk_robin_at(r, r->used - 1);

        for (j = 0; j < pad; ++i, ++j)
        {
            pv[i] = f->v;
            fv[i] = f;
        }
    }

    if (p->cfg->SIMPLEDIFFS)
    {
    }
    else
    {
        wtk_math_do_diff(pv, win, diff_sigma, startv, p->cfg->static_feature_cols);
    }

    f = fv[win];

    // f=wtk_feature_v_to_f(pv[win]);
    if (r->nslot == r->used || is_end)
    {
        // if robin is full or got end hint, remove the front feature in the robin.
        wtk_parm_flush_robin(p, r);
    }

    return f;
}

void wtk_parm_output_feature_to_queue(wtk_parm_t *p, wtk_feature_t *f)
{
    ++f->used;

    if (p->output_queue)
    {
        wtk_queue_push(p->output_queue, &(f->hoard_n));
    }
}

void wtk_parm_output_frv_to_queue(wtk_parm_t *p, wtk_frv_t *frv)
{
    //++frv->used;
    if (p->output_queue)
    {
        wtk_queue_push(p->output_queue, &(frv->hoard_n));
    }
}

static void wtk_parm_feed_feature(wtk_parm_t *p, wtk_feature_t *f, int index)
{
    wtk_robin_t *r = p->robins[index];
    int win        = p->win[index];
    int startv     = p->feature_pos[index];
    ++f->used;
    wtk_robin_push(r, f);

    if (r->used <= win)
    {
        goto end;
    }

    //|f0|f1|f2|f3|f4| => calc the dynamic feature of f2 and return it/
    f = wtk_parm_flush_feature(p, WTK_PARM_APPEND, r, win, p->cfg->sigma[index], startv);
    ++index;

    if (index < 3 && p->robins[index])
    {
        // if there is D T A want to do, raise the feature to the higher robin.
        wtk_parm_feed_feature(p, f, index);
    }
    else
    {
        // if feature all get, output the feature.
        if (p->post_f)
        {
            p->post_f(p->post_hook, f);
        }
        else
        {
#if defined(WAKEUP_NN_DNN)
            wtk_dnn_feed(p->dnn, f);
#elif defined(WAKEUP_NN_FSMN)
            wtk_fsmn_feed(p->fsmn, f);
#endif
        }
    }

end:
    return;
}

void wtk_parm_output_feature(wtk_parm_t *p, wtk_feature_t *f)
{
    if (p->feature_hook_f)
    {
        /*
         * static_hook_f return 1 means we no need handle the feautres any more
         */
#if defined(WAKEUP_NN_DNN)
        if (p->feature_hook_f(p->feature_hook, &f->v[1], f->v[0] / 3))
#elif defined(WAKEUP_NN_FSMN)
        if (p->feature_hook_f(p->feature_hook, &f->v[1], f->v[0]))
#endif
        {
            return;
        }
    }

#if defined(WAKEUP_NN_DNN)
    wtk_dnn_feed(p->dnn, f);
#elif defined(WAKEUP_NN_FSMN)
    wtk_fsmn_feed(p->fsmn, f);
#endif
}

static void wtk_parm_process_post_feature(wtk_parm_t *p, wtk_feature_t *f)
{
    if (p->post_f)
    {
        p->post_f(p->post_hook, f);
    }
    else
    {
        wtk_parm_output_feature(p, f);
    }
}

void wtk_parm_flush_robins(wtk_parm_t *p)
{
    int i;

    for (i = 0; i < 3; ++i)
    {
        if (p->robins[i])
        {
            //wtk_parm_flush_robin_feature(p, i); // edit by bsw
            wtk_parm_flush_robin_all(p, p->robins[i]);
        }
    }
}

void wtk_parm_flush_feature_queue(wtk_parm_t *p, wtk_queue_t *q)
{
    wtk_feature_t *f;
    wtk_queue_node_t *n;

    while (1)
    {
        n = wtk_queue_pop(q);

        if (!n)
        {
            break;
        }

        f = wtk_queue_node_data(n, wtk_feature_t, hoard_n);
        --f->used;
        wtk_parm_push_feature(p, f);
    }
}

#if defined (WAKEUP_RESET)
static void wtk_parm_flush_frv_queue(wtk_parm_t *p, wtk_queue_t *q)
{
    wtk_frv_t *frv;
    wtk_queue_node_t *n;

    while (1)
    {
        n = wtk_queue_pop(q);

        if (!n)
        {
            break;
        }

        frv = data_offset(n, wtk_frv_t, hoard_n);
        wtk_parm_push_frv(p, frv);
    }
}
#endif

int wtk_parm_reset(wtk_parm_t *p)
{
#if defined(WAKEUP_RESET)
    int i;

    if (p->feature_hoard->length < p->zmean->cfg->win_size + 1)
    {
        wtk_parm_feed(p, WTK_PARM_END, 0, 0);
    }

    if (p->output_queue)
    {
        wtk_parm_flush_frv_queue(p, p->output_queue);
        wtk_queue_init(p->output_queue);
    }

    for (i = 0; i < 3; ++i)
    {
        if (p->robins[i])
        {
            wtk_robin_reset(p->robins[i]);
        }
    }

#endif
    p->n_frame_index = 0;
#if defined(WAKEUP_RESET)

    if (p->cfg->ZMEAN)
    {
        if (p->cfg->use_cmn)
        {
            wtk_zmean_reset(p->zmean);
        }
    }

#if defined(WAKEUP_NN_DNN)

    if (p->dnn)
    {
        wtk_dnn_reset(p->dnn);
    }

#elif defined(WAKEUP_NN_FSMN)

    if (p->fsmn)
    {
        wtk_fsmn_reset(p->fsmn);
    }

#endif
#endif
    return 0;
}

int wtk_parm_feed(wtk_parm_t *p, wtk_parm_state_t state, short *data, int samples)
{
    return wtk_parm_feed2(p, state, (char *)data, samples * 2);
}

void wtk_parm_feed_input_feature(wtk_parm_t *p, wtk_feature_t *f)
{
    wtk_parm_cfg_t *cfg = p->cfg;

    if (p->static_post_f)
    {
        p->static_post_f(p->static_post_hook, f);
    }

    if (cfg->DELTA)
    {
        wtk_parm_feed_feature(p, f, 0);
    }
    else
    {
        wtk_parm_process_post_feature(p, f);
    }
}

void wtk_parm_feed_end(wtk_parm_t *p)
{
    wtk_parm_flush_robins(p);

    if (p->cfg->ZMEAN)
    {
        if (p->cfg->use_cmn)
        {
            wtk_zmean_flush_parm_post_queue(p->zmean);
            wtk_zmean_flush_robin(p->zmean);
        }
    }

#if defined(WAKEUP_NN_DNN)
    wtk_dnn_flush(p->dnn);
    //wtk_dnn_flush_robin_all(p->dnn);
#elif defined(WAKEUP_NN_FSMN)
    wtk_fsmn_flush(p->fsmn);
    //wtk_fsmn_flush_robin_all(p->fsmn);
#endif
}

void wtk_parm_feed_plp(wtk_parm_t *p, wtk_feature_t *f)
{
    wtk_parm_feed_input_feature(p, f);
}

static void wtk_parm_feed_sample(wtk_parm_t *p, wtk_short_vector_t *v)
{
    wtk_feature_t  *feature;
    p->llAbsFrameIndex++;
    feature = wtk_parm_pop_feature(p);
    wtk_sigp_procss(&(p->sigp), v, &(feature->v[1]));
    wtk_parm_feed_plp(p, feature);
}

void wtk_parm_flush_end(wtk_parm_t *p)
{
    // flush robin, pad end feature and process D T A if need
#if defined(WAKEUP_RESET)
    wtk_parm_feed_end(p);
#endif
}

int wtk_parm_feed2(wtk_parm_t *p, wtk_parm_state_t state, char *data, int bytes)
{
    char *end = data + bytes;
    wtk_short_vector_t *v = p->frame_vector;
    wtk_short_vector_buffer_t *b = p->frame_buffer;
    wtk_parm_cfg_t *cfg = p->cfg;

    while (data < end)
    {
        data += wtk_short_vector_buffer_push_c(b, data, end - data);

        while (wtk_short_vector_buffer_peek(b, v, 0) == 0)
        {
            wtk_parm_feed_sample(p, v);
            wtk_short_vector_buffer_skip(p->frame_buffer, cfg->frame_step, cfg->frame_size);
        }
    }

    if (state == WTK_PARM_END)
    {
        wtk_parm_flush_end(p);
    }

    return 0;
}

