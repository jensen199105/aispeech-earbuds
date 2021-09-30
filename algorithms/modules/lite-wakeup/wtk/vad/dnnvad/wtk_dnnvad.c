#include "wtk_dnnvad.h"

void wtk_dnnvad_push_frv(wtk_dnnvad_t *v, wtk_frv_t *frv)
{
    if(frv->ref == 0)
    {
        //wtk_parm_reuse_frv(v->parm,frv);        // 重新压入缓冲
    }
}

wtk_dnnvad_t *wtk_dnnvad_new(wtk_dnnvad_cfg_t *cfg, void *raise_ths, wtk_vframe_raise_f raise)
{
    wtk_dnnvad_t *v;
    v = (wtk_dnnvad_t *)wtk_malloc(sizeof(*v));
    v->cfg = cfg;
    v->raise = raise;
    v->raise_ths = raise_ths;
    v->feat_robin = wtk_robin_new(cfg->win * 2 + 1);
    v->post = wtk_dnnvad_post_new(v);
    wtk_dnnvad_reset(v);
    return v;
}

void wtk_dnnvad_delete(wtk_dnnvad_t *v)
{
    wtk_dnnvad_post_delete(v->post);
    wtk_robin_delete(v->feat_robin);
    wtk_free(v);
}

void wtk_dnnvad_reset(wtk_dnnvad_t *v)
{
    wtk_dnnvad_post_reset(v->post);
    wtk_robin_reset(v->feat_robin);
}

void wtk_dnnvad_raise_frv(wtk_dnnvad_t *v, wtk_frv_t *frv, int is_sil)
{
    if (is_sil) {
        frv->state = wtk_vframe_sil;
    } else {
        frv->state = wtk_vframe_speech;
    }
    v->raise(v->raise_ths,frv);
}

void wtk_dnnvad_flush_frv(wtk_dnnvad_t *v, wtk_parm_state_t state)
{
    wtk_robin_t *r = v->feat_robin;
    wtk_frv_t *frv;
    int i, pad;
    int is_end;
    int value;

    if (r->used <= v->cfg->win) {
        return;
    }
    value = 0;
    for (i = 0; i < r->used; ++i) {
        frv = (wtk_frv_t *)wtk_robin_at(r, i);
        value += frv->is_sil;
    }
    value = value << 1; 
    is_end = state == WTK_PARM_END;

    if (r->used == r->nslot) {
        frv = (wtk_frv_t *)wtk_robin_at(r, v->cfg->win);
    } else {
        pad = r->nslot - r->used;
        if (is_end) {
            frv = (wtk_frv_t *)wtk_robin_at(r, v->cfg->win);
        } else {
            frv = (wtk_frv_t *)wtk_robin_at(r, v->cfg->win - pad);
        }
    }

    if (value <= r->nslot && frv->index > v->cfg->startfrm) {
        frv->state = wtk_vframe_speech;
    } else {
        frv->state = wtk_vframe_sil;
    }

    wtk_dnnvad_post_feed(v->post, frv);
    if (r->nslot == r->used || is_end) {//尚未修改
        frv = (wtk_frv_t *)wtk_robin_pop(r);
        --frv->ref;
    }
    return;
}

void wtk_dnnvad_feed_frv(wtk_dnnvad_t *v, wtk_frv_t *frv)
{
    wtk_robin_t *r = v->feat_robin;

    ++frv->ref;
    wtk_robin_push(r, frv);
    if (r->used <= v->cfg->win) {
        return;
    }
    wtk_dnnvad_flush_frv(v, WTK_PARM_APPEND);
}

void wtk_dnnvad_feed_parm_frv(wtk_dnnvad_t *v, wtk_frv_t *frv)
{
    int is_sil;
    wtk_int_vector_t *vec;
    vec = frv->rv;
    is_sil = vec[1] >= v->cfg->sil_thresh;
    frv->is_sil = is_sil;
    wtk_dnnvad_feed_frv(v, frv);
}

void wtk_dnnvad_feed_end(wtk_dnnvad_t *v)
{
    wtk_robin_t *r = v->feat_robin;

    while (r->used > v->cfg->win) {
        wtk_dnnvad_flush_frv(v, WTK_PARM_END);
    }
    wtk_dnnvad_flush_end(v->post);
}

int wtk_dnnvad_feed(wtk_dnnvad_t *v, wtk_parm_state_t state, wtk_frv_t *frv)
{
    wtk_dnnvad_feed_parm_frv(v,frv);//end　还没
    return 0;
}
