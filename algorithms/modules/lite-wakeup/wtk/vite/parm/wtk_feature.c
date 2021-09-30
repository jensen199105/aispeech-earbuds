#include "wtk_parm_cfg.h"
#include "wtk_feature.h"
#include "wtk/vite/math/wtk_math.h"

wtk_feature_t *wtk_feature_new(int size, int xf_size)
{
    wtk_feature_t *f;
    f = (wtk_feature_t *)wtk_calloc(1, sizeof(*f));
    f->used = 0;
    f->index = 0;
    f->v = (int *)wtk_calloc(size+1,sizeof(int));
    f->v[0] = size;
#if defined(WAKEUP_NN_DNN)
    f->cmn = (int *)wtk_calloc(size/3,sizeof(int));
#elif defined(WAKEUP_NN_FSMN)
    f->cmn = (int*)wtk_calloc(size, sizeof(int));
#endif
    return f;
}

wtk_frv_t *wtk_frv_new(int dnn_out_cols)
{
    wtk_frv_t *frv;
    frv = (wtk_frv_t *)wtk_calloc(1, sizeof(*frv));
    frv->ref = 0;
    frv->index = 0;
    frv->dnn_v =(int *)wtk_calloc(dnn_out_cols+1,sizeof(int));
    frv->dnn_v[0]=dnn_out_cols;
    frv->rv = frv->dnn_v;
    frv->state = wtk_vframe_sil;
    return frv;
}

wtk_feature_t *wtk_feature_new2(struct wtk_parm_cfg *cfg, int xf_size)
{
	return wtk_feature_new(cfg->feature_cols, xf_size);
}

int wtk_feature_delete(wtk_feature_t *f)
{
    if (f->v) {
        wtk_vector_delete(f->v);
    }
    if(f->cmn){
        wtk_vector_delete(f->cmn);
    }
    wtk_free(f);
    return 0;
}

int wtk_frv_delete(wtk_frv_t *frv)
{

    if (frv->dnn_v) {
        wtk_free(frv->dnn_v);
        //wtk_vector_delete(f->dnn_v);
    }
    wtk_free(frv);
    return 0;
}

int wtk_feature_send(wtk_feature_t *f)
{
    return f->send(f->send_hook, f);
}

void wtk_feature_push_back(wtk_feature_t *f)
{
    --f->used;
    if (f->send) {
        f->send(f->send_hook, f);
    }
}

void wtk_frv_push_back(wtk_frv_t *frv)
{
    //--frv->used;
    if (frv->send) {
        //frv->state = wtk_vframe_sil;
        frv->send(frv->send_hook, frv);
    }
}

void wtk_feature_use_dec(wtk_feature_t *f)
{
    --f->used;
}
