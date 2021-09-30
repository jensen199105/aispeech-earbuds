#include "wtk_cblas_cfg.h"

//#define ROW_MAX_QVALUE 24    /* Accuracy and range are met */

extern short wkp_info[];
extern short bias[];
extern short window[];
extern int *AISPEECH_PWM[];
extern int *AISPEECH_PB[];
extern signed char *AISPEECH_PW[];
int wtk_cblas_cfg_init(wtk_cblas_cfg_t *cfg) {
  cfg->trans = 0;
  cfg->cache_size = 1;
  cfg->in_cols = 0;
  cfg->out_cols = 0;
  cfg->max_col = 0;
  wtk_queue_init(&(cfg->layer_q));
  return 0;
}

int wtk_cblas_cfg_clean(wtk_cblas_cfg_t *cfg) {
  wtk_cblas_layer_delete(cfg);
  if (cfg->trans) {
    wtk_cblas_trans_delete(cfg->trans);
  }
  return 0;
}

int wtk_cblas_cfg_load_net(wtk_cblas_cfg_t *cfg) {
    int i,ret = 0;
    wtk_cblas_matrix_i8_t *m;
    wtk_cblas_layer_i8_t *l;
    int layer_cnt =wkp_info[0];
    cfg->in_cols = wkp_info[1];
    cfg->out_cols =wkp_info[2];
    cfg->max_col = wkp_info[3];

    for(i = 0; i < layer_cnt; i++)
    {
        l =(wtk_cblas_layer_i8_t *)wtk_cblas_layer_new(cfg);
        l->w =wtk_malloc(sizeof(*m));
        l->w->row =wkp_info[2*i+4];
        l->w->col =wkp_info[2*i+5];
        l->w->m =AISPEECH_PW[i];
        l->w->row_maxs = (int32_t *)AISPEECH_PWM[i];
        l->b = wtk_cblas_vector_i32_new(l->w->row);
        l->b->v =AISPEECH_PB[i];
        if(i == layer_cnt - 1)
        {
            l->type = wtk_dnn_softmax;
        }
        else
        {
            if(i & 1)//奇数项
            {
                l->type = wtk_dnn_sigmoid;
            }
            else//偶数项
            {
                l->type = wtk_dnn_linear;
            }
        }
        wtk_queue_push(&(cfg->layer_q),&(l->q_n));
    }

    return ret;
 }

int wtk_cblas_cfg_load_trans(wtk_cblas_cfg_t *cfg) {
    wtk_cblas_trans_t *trans;
    trans    = wtk_cblas_trans_new();
    trans->b = wtk_cblas_vector_new(792);
    trans->b->v = bias;
    //trans->b->v = window;
    trans->w    = wtk_cblas_vector_new(792);
    trans->w->v =window;
    //trans->w->v = bias;
    cfg->trans  = trans;
    return 0;
}

int wtk_cblas_cfg_update(wtk_cblas_cfg_t *cfg)
{
    int ret = 0;
    if(cfg->cache_size <= 0)
    {
        cfg->cache_size =1;
    }
    wtk_cblas_cfg_load_trans(cfg);
    wtk_cblas_cfg_load_net(cfg);
    cfg->in_cols = wtk_cblas_cfg_in_rows(cfg);
    cfg->in_col_bytes = cfg->in_cols * sizeof(float);
    return ret;
}


int wtk_cblas_cfg_out_cols(wtk_cblas_cfg_t *cfg) {
  wtk_cblas_layer_t *l;

  if (cfg->layer_q.length <= 0) {
    return 0;
  }
  l = data_offset(cfg->layer_q.push, wtk_cblas_layer_t, q_n);
  return l->b->len;
}

int wtk_cblas_cfg_in_rows(wtk_cblas_cfg_t *cfg) {
  wtk_cblas_layer_t *l;

  if (cfg->layer_q.length <= 0) {
    return 0;
  }
  l = data_offset(cfg->layer_q.pop, wtk_cblas_layer_t, q_n);
  return l->w->row;
}

int wtk_cblas_cfg_in_cols(wtk_cblas_cfg_t *cfg) {
  wtk_cblas_layer_t *l;

  if (cfg->layer_q.length <= 0) {
    return 0;
  }
  l = data_offset(cfg->layer_q.pop, wtk_cblas_layer_t, q_n);
  return l->w->col;
}
