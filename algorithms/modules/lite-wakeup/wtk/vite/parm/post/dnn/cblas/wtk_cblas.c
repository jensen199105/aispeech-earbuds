#include "wtk_cblas.h"
#include "forward/wtk_cblas_i8.h"
#include "wtk/vite/parm/wtk_parm.h"
#include "wtk/vite/parm/post/dnn/wtk_dnn.h"

wtk_cblas_t *wtk_cblas_new(wtk_cblas_cfg_t *cfg, wtk_dnn_t *dnn)
{
    wtk_cblas_t *b;
    b = (wtk_cblas_t *)wtk_malloc(sizeof(*b));
    b->cfg = cfg;
    b->feat = wtk_cblas_matrix_new(cfg->cache_size, wtk_cblas_cfg_in_cols(cfg));
    b->feat->m = (int *)wtk_malloc(cfg->cache_size * wtk_cblas_cfg_in_cols(cfg) * sizeof(int));
    b->tmp_out = wtk_cblas_matrix_new(cfg->cache_size, cfg->max_col);
    //b->tmp_out->m = (float *)wtk_malloc(8*792*sizeof(float));
    b->tmp_out->m = b->feat->m;
    b->tmp_input_i8 = wtk_cblas_matrix_i8_new(cfg->cache_size, cfg->max_col);
    b->tmp0 = wtk_cblas_matrix_i32_new(cfg->cache_size, cfg->max_col);
    b->input_feature_robin = wtk_robin_new(cfg->cache_size);
    b->dnn = dnn;
    b->feature_hoard = dnn->parm->feature_hoard;
    b->frv_hoard = dnn->parm->frv_hoard;
    //b->last_feature = NULL;
    //b->last_frv = NULL;
    b->index = 0;
    return b;
}

void wtk_cblas_delete(wtk_cblas_t *b)
{
    wtk_cblas_matrix_delete(b->feat);
    wtk_free(b->tmp_out);
    wtk_cblas_matrix_i8_delete(b->tmp_input_i8);
    wtk_cblas_matrix_i32_delete(b->tmp0);
    wtk_robin_delete(b->input_feature_robin);
    wtk_free(b);
}

void wtk_cblas_reset(wtk_cblas_t *b)
{
    /*if (b->last_frv)
    {
        wtk_parm_reuse_frv(b->dnn->parm, b->last_frv);
        b->last_frv = NULL;
    }*/
    b->ifrmIdx = 0;
    wtk_robin_reset(b->input_feature_robin);
}

void wtk_cblas_process_matrix(wtk_cblas_t *d, wtk_cblas_matrix_t *m)
{
    wtk_cblas_process_matrix_i8(d, m);
}

void wtk_cblas_process_layer(wtk_cblas_t *d, wtk_feature_t **pv, int npv, wtk_feature_t *f)
{
    ++f->used;
    wtk_robin_push(d->input_feature_robin, f);

    if ((0 == d->dnn->cfg->skip_frame) || (f->index % d->dnn->cfg->skip_frame == 1))
    {
        ++d->index;
        wtk_cblas_feature_to_matirx(d->feat, d->index - 1, pv, npv, d->dnn->cfg->padding_frame, d->dnn->padding);
    }

    if (d->input_feature_robin->nslot == d->input_feature_robin->used)
    {
        d->feat->row = d->index;
        wtk_cblas_process_matrix(d, d->feat);
        d->index = 0;
    }
}

void wtk_cblas_flush_layer(wtk_cblas_t *d)
{
    wtk_robin_t *r = d->input_feature_robin;

    if (r->used > 0 && d->index > 0)
    {
        d->feat->row = d->index;
        wtk_cblas_process_matrix(d, d->feat);
        d->index = 0;
    }

    wtk_parm_flush_robin_all(d->dnn->parm, r);
}

void wtk_cblas_flush_layer_robin(wtk_cblas_t *d)
{
    wtk_robin_t *r = d->input_feature_robin;
    wtk_parm_flush_robin_all(d->dnn->parm, r);
}

void wtk_cblas_flush_end(wtk_cblas_t *d)
{
    /*if (d->last_frv) {
    --d->last_frv->used;
    wtk_parm_push_frv(d->dnn->parm, d->last_frv);
    d->last_frv = NULL;
    }*/
}
