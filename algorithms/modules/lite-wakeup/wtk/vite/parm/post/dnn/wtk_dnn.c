#include "wtk_dnn.h"
#include "wtk/vite/parm/wtk_parm.h"

wtk_dnn_t *wtk_dnn_new(wtk_dnn_cfg_t *cfg, wtk_parm_t *parm)
{
    wtk_dnn_t *d;
    int i;
    d = (wtk_dnn_t *)wtk_calloc(1, sizeof(*d));
    d->cfg = cfg;
    d->parm = parm;

    if (d->cfg->use_custom_win)
    {
        d->robin = wtk_robin_new(cfg->left_win + cfg->right_win + 1);
    }
    else
    {
        d->robin = wtk_robin_new(cfg->win * 2 + 1);
    }

    d->features = (wtk_feature_t **)wtk_calloc(d->robin->nslot, sizeof(wtk_feature_t *));

    if (cfg->padding_frame > 0)
    {
        float *f;
        d->padding = (float *)wtk_calloc(cfg->padding_frame, sizeof(float));
        f = cfg->expand_array;

        for (i = 0; i < cfg->padding_frame; i++)
        {
            d->padding[i] = f[i];
        }
    }

    if (cfg->use_cblas)
    {
        d->cblas = wtk_cblas_new(&(cfg->cblas), d);
    }

    return d;
}

void wtk_dnn_delete(wtk_dnn_t *d)
{
    if (d->cfg->padding_frame > 0)
    {
        wtk_free(d->padding);
    }

    if (d->cfg->use_cblas)
    {
        wtk_cblas_delete(d->cblas);
    }

    wtk_free(d->features);
    wtk_robin_delete(d->robin);
    wtk_free(d);
}

void wtk_dnn_reset(wtk_dnn_t *d)
{
    if (d->cfg->padding_frame)
    {
        float *f = d->cfg->expand_array;
        int i;

        for (i = 0; i < d->cfg->padding_frame; i++)
        {
            d->padding[i] = f[i];
        }
    }

    wtk_robin_reset(d->robin);

    if (d->cfg->use_cblas)
    {
        wtk_cblas_reset(d->cblas);
    }
}

void wtk_dnn_flush_robin(wtk_dnn_t *p, wtk_robin_t *r)
{
    wtk_feature_t *f;
    f = (wtk_feature_t *)wtk_robin_pop(r);
    --f->used;
    wtk_parm_push_feature(p->parm, f);
}


wtk_feature_t *wtk_dnn_flush_feature(wtk_dnn_t *d, wtk_parm_state_t s)
{
    wtk_feature_t **pv = d->features;
    wtk_robin_t *r = d->robin;
    wtk_feature_t *f;
    int win;
    int i, pad, j;
    int is_end;

    if (d->cfg->use_custom_win)
    {
        win = d->cfg->left_win + d->cfg->right_win;
    }
    else
    {
        win = d->cfg->win;
    }

    is_end = s == WTK_PARM_END;

    if (d->cfg->use_custom_win != 1 && r->used <= win)
    {
        return 0;
    }
    else
        if (d->cfg->use_custom_win == 1 && r->used <= (d->cfg->left_win + d->cfg->right_win - 1) / 2)
        {
            return 0;
        }

    pad = r->nslot - r->used;
    i = 0;

    if (pad > 0 && !is_end)
    {
        // if not end, add pad to front.
        // * |f0|f1|f2|0|0|  => |f0|f0|f0|f1|f2|
        // * |f0|f1|f2|f3|0| => |f0|f0|f1|f2|f3|
        f = (wtk_feature_t *)wtk_robin_at(r, 0);

        for (; i < pad; ++i)
        {
            pv[i] = f;
        }
    }

    for (j = 0; j < r->used; ++i, ++j)
    {
        f = ((wtk_feature_t *)wtk_robin_at(r, j));
        pv[i] = f;
    }

    if (pad > 0 && is_end)
    {
        // if is end and pad to the end.
        //|f0|f1|f2|f3|0| => |f0|f1|f2|f3|f3|
        //|f0|f1|f2|0|0| => |f0|f1|f2|f2|f2|
        f = (wtk_feature_t *)wtk_robin_at(r, r->used - 1);

        for (j = 0; j < pad; ++i, ++j)
        {
            pv[i] = f;
        }
    }

    if (d->cfg->use_custom_win)
    {
        f = pv[d->cfg->left_win];
    }
    else
    {
        f = pv[win];
    }

    if (d->cfg->use_cblas)
    {
        wtk_cblas_process_layer(d->cblas, pv, r->nslot, f);

        if (r->nslot == r->used || is_end)
        {
            // if robin is full or got end hint, remove the front feature in the robin.
            wtk_dnn_flush_robin(d, r);
        }

        f = 0;
    }

    return f;
}

void wtk_dnn_feed(wtk_dnn_t *d, wtk_feature_t *f)
{
    ++f->used;
    wtk_robin_push(d->robin, f);
    f = wtk_dnn_flush_feature(d, WTK_PARM_APPEND);

    if (f)
    {
        wtk_parm_output_feature_to_queue(d->parm, f);
    }
}

void wtk_dnn_raise_feature(wtk_dnn_t *d, wtk_feature_t *f)
{
    wtk_parm_output_feature_to_queue(d->parm, f);
}

void wtk_dnn_raise_frv(wtk_dnn_t *d, wtk_frv_t *frv)
{
    wtk_parm_output_frv_to_queue(d->parm, frv);
}

void wtk_dnn_skip_feature(wtk_dnn_t *d, wtk_feature_t *f)
{
    wtk_parm_push_feature(d->parm, f);
}

void wtk_dnn_flush_robin_all(wtk_dnn_t *d)
{
    wtk_robin_t *r = d->robin;

    if (d->cfg->use_cblas)
    {
        wtk_cblas_flush_layer_robin(d->cblas);
    }

    wtk_parm_flush_robin_all(d->parm, r);
}

void wtk_dnn_flush(wtk_dnn_t *d)
{
    wtk_robin_t *r = d->robin;
    wtk_feature_t *f;
    int win;
    int left_padding;

    if (d->cfg->use_custom_win)
    {
        win = d->cfg->left_win + d->cfg->right_win;
        left_padding = d->cfg->left_win;
    }
    else
    {
        left_padding = win = d->cfg->win;
    }

    while (r->used > left_padding)
    {
        f = wtk_dnn_flush_feature(d, WTK_PARM_END);

        if (!f)
        {
            // break;
        }

        if (f)
        {
            wtk_parm_output_feature_to_queue(d->parm, f);
        }
    }

    if (d->cfg->use_cblas)
    {
        wtk_cblas_flush_layer(d->cblas);
        wtk_cblas_flush_end(d->cblas);
    }

    wtk_parm_flush_robin_all(d->parm, r);
}
