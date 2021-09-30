#include "wtk_dnnvad_post.h"
#include "wtk_dnnvad.h"

wtk_dnnvad_post_t *wtk_dnnvad_post_new(wtk_dnnvad_t *vad)
{
    wtk_dnnvad_post_t *p;

    p = (wtk_dnnvad_post_t *)wtk_malloc(sizeof(*p));
    p->vad = vad;
    p->sil_robin = wtk_robin_new(vad->cfg->siltrap);
    p->speech_robin = wtk_robin_new(vad->cfg->speechtrap);
    wtk_dnnvad_post_reset(p);
    return p;
}

void wtk_dnnvad_post_delete(wtk_dnnvad_post_t *p)
{
    wtk_robin_delete(p->sil_robin);
    wtk_robin_delete(p->speech_robin);
    wtk_free(p);
}

void wtk_dnnvad_post_reset(wtk_dnnvad_post_t *p)
{
    p->state = WTK_DNNVAD_SIL;
    wtk_robin_reset(p->sil_robin);
    wtk_robin_reset(p->speech_robin);
}

void wtk_dnnvad_post_flush_feature_robin(wtk_dnnvad_post_t *v, wtk_robin_t *r, int is_sil)
{
    wtk_frv_t *frv;

    while (r->used > 0) {
        frv = (wtk_frv_t *)wtk_robin_pop(r);
        if (!frv) {
            break;
        }
        wtk_dnnvad_raise_frv(v->vad, frv, is_sil);
    }
}

void wtk_dnnvad_post_feed(wtk_dnnvad_post_t *v, wtk_frv_t *frv)
{
    int is_sil;
    is_sil = frv->state == wtk_vframe_sil;

    switch (v->state) {
    case WTK_DNNVAD_SIL:
        if (is_sil) {
            if (v->sil_robin->used > 0) {
                wtk_dnnvad_post_flush_feature_robin(v, v->sil_robin, is_sil);
            }
            wtk_dnnvad_raise_frv(v->vad, frv, is_sil);
        } else {
            ++frv->ref;
            wtk_robin_push(v->sil_robin, frv);
            if (v->sil_robin->used >= v->vad->cfg->siltrap) {
                wtk_dnnvad_post_flush_feature_robin(v, v->sil_robin, is_sil);
                v->state = WTK_DNNVAD_SPEECH;
            }
        }
        break;
    case WTK_DNNVAD_SPEECH:
        if (is_sil) {
            ++frv->ref;
            wtk_robin_push(v->speech_robin, frv);
            if (v->speech_robin->used >= v->vad->cfg->speechtrap) {
                wtk_dnnvad_post_flush_feature_robin(v, v->speech_robin, is_sil);
                v->state = WTK_DNNVAD_SIL;
            }
        } else {
            if (v->speech_robin->used > 0) {
                wtk_dnnvad_post_flush_feature_robin(v, v->speech_robin, is_sil);
            }
            wtk_dnnvad_raise_frv(v->vad, frv, is_sil);
        }
        break;
    }
}

void wtk_dnnvad_flush_end(wtk_dnnvad_post_t *v)
{

    switch (v->state) {
    case WTK_DNNVAD_SIL:
        if (v->sil_robin->used > 0) {
            wtk_dnnvad_post_flush_feature_robin(v, v->sil_robin, 1);
        }
        break;
    case WTK_DNNVAD_SPEECH:
        if (v->speech_robin->used > 0) {
            wtk_dnnvad_post_flush_feature_robin(v, v->speech_robin, 0);
        }
        break;
    }
}
