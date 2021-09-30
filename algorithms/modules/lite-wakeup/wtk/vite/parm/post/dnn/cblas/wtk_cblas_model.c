#include "wtk_cblas_model.h"
#include "wtk_cblas_cfg.h"
#include <stdlib.h>
void *wtk_cblas_layer_new(wtk_cblas_cfg_t *cfg) {
      wtk_cblas_layer_i8_t *l;
      l = (wtk_cblas_layer_i8_t *)wtk_malloc(sizeof(*l));
      l->w = 0;
      l->b = 0;
      return (void *)l;
}

void wtk_cblas_layer_delete(wtk_cblas_cfg_t *cfg) {
  wtk_queue_node_t *n;
  wtk_cblas_layer_i8_t *layer;
  while (1) {
    n = wtk_queue_pop(&(cfg->layer_q));
    if (!n) {
        break;
        }
        layer = data_offset(n, wtk_cblas_layer_i8_t, q_n);
        if (layer->w) {
         layer->w->m = NULL;
         layer->w->row_maxs = NULL;
         wtk_cblas_matrix_i8_delete(layer->w);
        }
        if (layer->b) {
          layer->b->v = NULL;
          wtk_cblas_vector_i32_delete(layer->b);
        }
        wtk_free(layer);
    }
}

wtk_cblas_trans_t *wtk_cblas_trans_new() {
  wtk_cblas_trans_t *t;

  t = wtk_malloc(sizeof(*t));
  t->b = 0;
  t->w = 0;
  return t;
}

void wtk_cblas_trans_delete(wtk_cblas_trans_t *t) {
  if (t->b) {
    t->b->v= NULL;
    wtk_cblas_vector_delete(t->b);
  }
  if (t->w) {
    t->w->v = NULL;
    wtk_cblas_vector_delete(t->w);
  }
  wtk_free(t);
}
