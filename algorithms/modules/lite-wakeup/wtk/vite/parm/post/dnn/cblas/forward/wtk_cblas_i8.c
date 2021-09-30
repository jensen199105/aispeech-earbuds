#include "wtk_cblas_i8.h"
#include "wtk/vite/parm/wtk_parm.h"
#include "wtk/vite/parm/post/dnn/wtk_dnn.h"
#include "wtk/wakeup/dnn/wtk_wakeup_dnn_word.h"

void wtk_cblas_process_dnn_layer_i8(wtk_cblas_t *d, wtk_cblas_layer_i8_t *l,
                                    wtk_cblas_matrix_i8_t *input, wtk_cblas_matrix_i32_t *output, int index)
{
    // reset output row col
    output->row = input->row;
    output->col = l->w->row;
    cblas_sgemm_i8(input->row, l->w->row,
                   input->col, input->m, input->col, l->w->m, l->w->col,
                   (int *)output->m, output->col, l->b->v, (int *)input->row_maxs,
                   (int *)l->w->row_maxs);

    switch (l->type)
    {
        case wtk_dnn_sigmoid:
            cblas_sigmoid_fix_q24((int *)output->m, output->row * output->col);
            cblas_q24_to_q6_version2(input->m, (int *)input->row_maxs, (int *)output->m, output->row, output->col);
            break;

        case wtk_dnn_softmax:
            cblas_softmax_q24((int *)output->m, output->row, output->col);
            break;

        case wtk_dnn_relu:
            // cblas_relu(output->m, output->row * output->col);
            break;

        case wtk_dnn_linear:
            cblas_q24_to_q6_version2(input->m, (int *)input->row_maxs, (int *)output->m, output->row, output->col);
            break;

        default:
            wtk_debug("layer->type not in list. %d\n", l->type);
            break;
    }
}

void wtk_cblas_raise_dnn_fixed(wtk_cblas_t *d, wtk_cblas_matrix_i32_t *output_i)
{
    wtk_robin_t *r = d->input_feature_robin;
    wtk_feature_t *f;
    wtk_frv_t *frv;
    wtk_queue_node_t *n;
    wtk_wakeup_dnn_wordlist_t *wl;
    int n_wchars;
    int skip_frame;
    int idx = -1;
    int i = 0;
    int id_wchar = 0;
    wl  = (wtk_wakeup_dnn_wordlist_t *)d->hook;
    n_wchars = wl->n_wchars;
    skip_frame = d->dnn->cfg->skip_frame;

    while (r->used > 0)
    {
        f = wtk_robin_pop(r);

        if (!f)
        {
            break;
        }

        n = wtk_queue_pop(d->frv_hoard);

        if (!n)
        {
            wtk_parm_reuse_feature(d->dnn->parm, f);
            break;
        }

        frv = data_offset(n, wtk_frv_t, hoard_n);
        frv->index = f->index;
        wtk_parm_reuse_feature(d->dnn->parm, f);

        if (0 == skip_frame || (frv->index % (skip_frame) == 1))
        {
            ++idx;

            for (i = 0; i < n_wchars; i++)
            {
                id_wchar = wl->wchars[i].id;
                frv->dnn_v[1 + i] = output_i->m[output_i->col * idx + id_wchar];
            }
        }
        else
        {
        }

        wtk_dnn_raise_frv(d->dnn, frv);
    }
}

void wtk_cblas_process_matrix_i8(wtk_cblas_t *d, wtk_cblas_matrix_t *m)
{
    wtk_queue_node_t *n;
    wtk_cblas_layer_i8_t *l;
    wtk_cblas_matrix_i32_t *output_m = NULL;   /* middle matrix output Q16 */
    wtk_cblas_matrix_i8_t *input_m = NULL;
    int i;
#ifdef USE_CM4_OPTIMIZE
    arm_fill_q31(0, d->tmp0->m, d->tmp0->row * d->tmp0->col);
#else
    AISP_TSL_memset(d->tmp0->m, 0, sizeof(int) * d->tmp0->row * d->tmp0->col);
#endif
    output_m = d->tmp0;
    input_m = d->tmp_input_i8;
    cblas_trans_process(m->m, m->row, m->col, d->cfg->trans->w->v, d->cfg->trans->b->v);

    for (i = 0, n = d->cfg->layer_q.pop; n; n = n->next, ++i)
    {
        l = data_offset(n, wtk_cblas_layer_i8_t, q_n);

        if (i == 0)
        {
            input_m->row = m->row;
            input_m->col = m->col;
            cblas_fix32tofix8(input_m->m, m->m, m->row, m->col, (int *)input_m->row_maxs);
        }
        else
        {
            input_m->row = output_m->row;
            input_m->col = output_m->col;
            // cblas_floatto8bit(input_m->m, output_m->m, output_m->row, output_m->col, input_m->row_maxs);
        }

        wtk_cblas_process_dnn_layer_i8(d, l, input_m, output_m, i);
    }

    wtk_cblas_raise_dnn_fixed(d, output_m);
}
