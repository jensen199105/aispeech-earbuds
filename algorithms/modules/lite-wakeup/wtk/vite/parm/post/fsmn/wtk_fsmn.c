/**
 * @file wtk_fsmn.c
 * @brief feedforward sequential memory networks
 * @author xtg
 * @version 1.0
 * @date 2018-11-19
 */

#include "AISP_TSL_common.h"
#include "AISP_TSL_str.h"
#include "wtk_fsmn.h"
#include "wtk/vite/parm/wtk_parm.h"
#include "wtk/wakeup/dnn/wtk_wakeup_dnn_word.h"

#ifdef JIELI_BR28
extern void vector_real_zs32_ys8_mul_xs8(volatile long *zptr, long *yptr, long *xptr, short len, char q);
extern void vector_real_zs32_ys32_add_xs32(volatile long *zptr, long *yptr, long *xptr, short len);
#endif
//TODO:移除优化===============================================================
static void vmsmul_fixed8(size_t vsize, const int8_t *a, const int8_t *b, int *out)
{
#ifdef JIELI_BR28
    vector_real_zs32_ys8_mul_xs8(out, (int *)a, (int *)b, vsize, 0);
#else
    int i;
    const int8_t *pa = a;
    const int8_t *pb = b;
#ifdef USE_HIFI4_OPTIMIZE
    ae_f16x4 h_pa;
    ae_f16x4 h_pb;
    ae_int16x4 h_pa1;
    ae_int16x4 h_pb1;
    ae_int32x2  h_tmp0;
    ae_int32x2  h_tmp1;
#endif
    int *pout = out;
    int nvsize = AISP_TSL_FLOOR(vsize, 4);
#ifdef USE_HIFI4_OPTIMIZE

    for (i = 0; i < nvsize; i += 4)
    {
        h_tmp0 = AE_ZERO32();
        h_tmp1 = AE_ZERO32();
        AE_L8X4F_IP(h_pa, pa, +4);
        AE_L8X4F_IP(h_pb, pb, +4);
        h_pa = AE_SRAA16RS(h_pa, 8);
        h_pb = AE_SRAA16RS(h_pb, 8);
        h_pa1 = ae_f16x4_rtor_ae_int16x4(h_pa);
        h_pb1 = ae_f16x4_rtor_ae_int16x4(h_pb);
        AE_MUL16X4(h_tmp0, h_tmp1, h_pa1, h_pb1);
        * pout++ = AE_MOVAD32_H(h_tmp0);
        * pout++ = AE_MOVAD32_L(h_tmp0);
        * pout++ = AE_MOVAD32_H(h_tmp1);
        * pout++ = AE_MOVAD32_L(h_tmp1);
    }

#else

    for (i = 0; i < nvsize; i += 4)
    {
        *pout++ = *(pa++) * (*(pb++));
        *pout++ = *(pa++) * (*(pb++));
        *pout++ = *(pa++) * (*(pb++));
        *pout++ = *(pa++) * (*(pb++));
    }

#endif

    for (; i < vsize; i++)
    {
        *pout++ = *(pa++) * (*(pb++));
    }

#endif
}

// out =  a + b
static void vmsadd_fixed32(size_t vsize, const int *a, const int *b, int *out)
{
#ifdef JIELI_BR28
    vector_real_zs32_ys32_add_xs32(out, a, b, vsize);
#else
    int i;
    const int *pa = a;
    const int *pb = b;
    int *pout = out;
    int nvsize = AISP_TSL_FLOOR(vsize, 4);

    for (i = 0; i < nvsize; i += 4)
    {
        *pout++ = *(pa++) + *(pb++);
        *pout++ = *(pa++) + *(pb++);
        *pout++ = *(pa++) + *(pb++);
        *pout++ = *(pa++) + *(pb++);
    }

    for (; i < vsize; i++)
    {
        *pout++ = *(pa++) + *(pb++);
    }

#endif
}

// out =  a + b
static void vmsadd_fixed8(size_t vsize, const int8_t *a, const int *b, int *out, int qfmta, int qfmtb)
{
    int i;
    const int8_t *pa = a;
    const int *pb = b;
    int *pout = out;
    int lShift = qfmta - qfmtb;
    int nvsize = AISP_TSL_FLOOR(vsize, 4);

    for (i = 0; i < nvsize; i += 4)
    {
        *pout++ = (*(pa++) << lShift) + *(pb++);
        *pout++ = (*(pa++) << lShift) + *(pb++);
        *pout++ = (*(pa++) << lShift) + *(pb++);
        *pout++ = (*(pa++) << lShift) + *(pb++);
    }

    for (; i < vsize; i++)
    {
        *pout++ = (*(pa++) << lShift) + *(pb++);
    }
}
//TODO:移除优化===============================================================

static void _forward_int8(wtk_fsmn_t *fsmn, wtk_feature_t *f)
{
    wtk_fsmn_layer_t *layer = NULL;
    wtk_queue_node_t *node = NULL;
    wtk_frv_t *frv;
    int layer_index;
    int *output32;
    wtk_wakeup_dnn_wordlist_t *wl = (wtk_wakeup_dnn_wordlist_t *)fsmn->hook;
    int i;
    int j = 0;
    fsmn->input_col = fsmn->cfg->in_col;

    if (fsmn->cfg->linear.m)
    {
        const int16_t *linear = fsmn->cfg->linear.m;
        aisp_s32_t *feat;
        S64 llSum;
        // global transform
        cblas_trans_process(fsmn->feat, 1, fsmn->feat_col, fsmn->cfg->trans_window, fsmn->cfg->trans_bias);

        for (i = 0; i < fsmn->cfg->in_col; i++)
        {
            feat = fsmn->feat;
            llSum = 0;

            for (j = 0; j < fsmn->cfg->linear.col; j++)
            {
                llSum += (S64)(*(feat++)) * (*(linear++));
            }

            fsmn->input[i] = (S8)PSHR(llSum, 27);
        }
    }
    else
    {
        // global transform
        cblas_trans_process(fsmn->feat, 1, fsmn->feat_col, fsmn->cfg->trans_window, fsmn->cfg->trans_bias);
        cblas_fix32tofix8(fsmn->input, fsmn->feat, fsmn->feat_col, 20, 5);
    }

    node = fsmn->cfg->layers.pop;
    layer_index = 0;

    while (node)
    {
        layer = data_offset(node, wtk_fsmn_layer_t, qn);
        // output<q6> = input<q5> * dense1<q7> + bias<q12>
        fsmn->input_col = layer->dense1.col;
        fsmn->output_col = layer->dense1.row;
        cblas_sgemm_i8(1, layer->dense1.row,
                       layer->dense1.col, fsmn->input, layer->dense1.col,
                       layer->dense1.m, layer->dense1.col, fsmn->output,
                       layer->dense1.row, layer->bias, 5, 7, 6);
        // output<q6> = relu2(output<q6>)
        cblas_relu2_fixed8(fsmn->output, fsmn->output_col, 6);
        // input<q5> = output<q6> * dense1<q7>
        fsmn->input_col = layer->dense2.row;
        cblas_sgemm_i8(1, layer->dense2.row,
                       layer->dense2.col, fsmn->output, layer->dense2.col,
                       layer->dense2.m, layer->dense2.col, fsmn->input,
                       layer->dense2.row, NULL, 6, 7, 5);
        // update history
        // tmp<q12> = input<q5> * filter<q7>
        vmsmul_fixed8(layer->out_dim, fsmn->input,
                      &layer->filter.m[layer->history_frame * layer->out_dim], fsmn->tmp);

        // tmp<q12> = tmp<q12> + tmp_history<q12>[j]
        for (j = 0; j < layer->history_frame; j++)
        {
            // tmp_history<q12> = filter<q7> * history<q5>
            vmsmul_fixed8(layer->out_dim, &fsmn->history[layer_index]->m[j * layer->out_dim],
                          &layer->filter.m[j * layer->out_dim], fsmn->tmp_history);
            vmsadd_fixed32(layer->out_dim, fsmn->tmp_history, fsmn->tmp, fsmn->tmp);
        }

        AISP_TSL_memmove(fsmn->history[layer_index]->m, &fsmn->history[layer_index]->m[layer->out_dim],
                         layer->out_dim * (layer->history_frame - 1) * sizeof(int8_t));
        AISP_TSL_memcpy(&fsmn->history[layer_index]->m[(layer->history_frame - 1) * layer->out_dim],
                        fsmn->input, layer->out_dim * sizeof(int8_t));
        // tmp<q12> = input<q5> + tmp<q12>
        vmsadd_fixed8(layer->out_dim, fsmn->input, fsmn->tmp, fsmn->tmp, 12, 5);
        // input<q5> = tmp<q12>
        cblas_fix32tofix8(fsmn->input, fsmn->tmp, layer->out_dim, 12, 5);
        node = node->next;
        layer_index++;
    }

    // dense2 output = input * dense2 + bias2
    fsmn->output_col = fsmn->cfg->dense2.row;
    cblas_sgemm_i32i16i32(1, fsmn->cfg->dense2.row,
                          fsmn->cfg->dense2.col, fsmn->tmp, fsmn->cfg->dense2.col,
                          fsmn->cfg->dense2.m, fsmn->cfg->dense2.col, fsmn->output32,
                          fsmn->cfg->dense2.row, fsmn->cfg->bias2, 12, 12, 24);
    output32 = fsmn->output32;
    node = wtk_queue_pop(fsmn->parm->frv_hoard);

    if (NULL == node)
    {
        wtk_parm_reuse_feature(fsmn->parm, f);
        return;
    }

    frv = data_offset(node, wtk_frv_t, hoard_n);
    frv->index = f->index;
    wtk_parm_reuse_feature(fsmn->parm, f);
    cblas_softmax_q24(output32, 1, fsmn->cfg->out_col);

    for (j = 0; j < wl->n_wchars; j++)
    {
        int char_id = wl->wchars[j].id;
        frv->dnn_v[j + 1] = output32[char_id];
    }

    wtk_parm_output_frv_to_queue(fsmn->parm, frv);
}

static int _gen_win_inputs(wtk_fsmn_t *fsmn, wtk_parm_state_t s)
{
    wtk_feature_t **pv = fsmn->features;
    wtk_robin_t *r = fsmn->feature_win;
    wtk_feature_t *f;
    int win;
    int i, pad, j;
    int is_end;
    win = fsmn->cfg->win;
    is_end = s == WTK_PARM_END;

    if (r->used <= win)
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

    f = pv[win];

    if ((fsmn->cfg->skip_frame && 1 == (f->index % fsmn->cfg->skip_frame)) || !fsmn->cfg->skip_frame)
    {
        //++ for feature_cache
        f->used++;
        wtk_cblas_feature_to_matirx(fsmn->feat, pv, r->nslot);
        _forward_int8(fsmn, f);
    }

    if (r->nslot == r->used || is_end)
    {
        f = (wtk_feature_t *)wtk_robin_pop(r);
        // -- for feature_win
        --f->used;
        wtk_parm_push_feature(fsmn->parm, f);
    }

    return 0;
}

wtk_fsmn_t *wtk_fsmn_new(wtk_fsmn_cfg_t *cfg, struct wtk_parm *parm)
{
    int i = 0;
    wtk_fsmn_t *fsmn = NULL;
    wtk_fsmn_layer_t *layer = NULL;
    wtk_queue_node_t *node = NULL;
    fsmn = (wtk_fsmn_t *)wtk_malloc(sizeof(wtk_fsmn_t));
    fsmn->cfg = cfg;
    fsmn->parm = parm;

    if (cfg->linear.m)
    {
        fsmn->feat = (int32_t *)wtk_calloc(cfg->linear.col, sizeof(fsmn->feat[0]));
        fsmn->feat_col = cfg->linear.col;
    }
    else
    {
        fsmn->feat = (int32_t *)wtk_calloc(cfg->in_col, sizeof(fsmn->feat[0]));
        fsmn->feat_col = cfg->in_col;
    }

    fsmn->feature_win = wtk_robin_new(cfg->win * 2 + 1);
    fsmn->features = (wtk_feature_t **)wtk_calloc(fsmn->feature_win->nslot, sizeof(fsmn->features[0]));
    fsmn->history = (wtk_cblas_matrix_i8_t **)wtk_calloc(cfg->layers.length, sizeof(fsmn->history[0]));
    fsmn->input = wtk_calloc(cfg->max_col, sizeof(fsmn->input[0]));
    fsmn->input_col = cfg->max_col;
    fsmn->tmp_history = (int32_t *)wtk_calloc(cfg->max_history_col, sizeof(fsmn->tmp_history[0]));
    fsmn->tmp = (int32_t *)wtk_calloc(cfg->max_col, sizeof(fsmn->tmp[0]));
    fsmn->output = wtk_calloc(cfg->max_col, sizeof(fsmn->output[0]));
    fsmn->output_col = cfg->max_col;
    fsmn->output32 = (int32_t *)wtk_calloc(cfg->out_col, sizeof(fsmn->output32[0]));
    // new history
    node = cfg->layers.pop;

    while (node)
    {
        layer = data_offset(node, wtk_fsmn_layer_t, qn);
        fsmn->history[i] = wtk_cblas_matrix_i8_new(layer->out_dim, layer->history_frame);
        node = node->next;
        i++;
    }

    return fsmn;
}

void wtk_fsmn_delete(wtk_fsmn_t *fsmn)
{
    int i = 0;

    if (fsmn)
    {
        wtk_robin_delete(fsmn->feature_win);
        wtk_free(fsmn->features);
        wtk_free(fsmn->input);
        wtk_free(fsmn->tmp_history);
        wtk_free(fsmn->tmp);
        wtk_free(fsmn->output);

        if (fsmn->history)
        {
            for (i = 0; i < fsmn->cfg->layers.length; i++)
            {
                if (fsmn->history[i])
                {
                    wtk_cblas_matrix_i8_delete(fsmn->history[i]);
                }
            }

            wtk_free(fsmn->history);
        }

        wtk_free(fsmn);
    }
}

void wtk_fsmn_reset(wtk_fsmn_t *fsmn)
{
    int i;
    wtk_cblas_matrix_i8_t *mat;

    for (i = 0; i < fsmn->cfg->layers.length; i++)
    {
        mat = fsmn->history[i];

        if (mat)
        {
            AISP_TSL_memset(mat->m, 0, sizeof(mat->m[0]) * mat->row * mat->col);
        }
    }
}

void wtk_fsmn_feed(wtk_fsmn_t *fsmn, wtk_feature_t *f)
{
    // ++ for feature_win
    f->used++;
    wtk_robin_push(fsmn->feature_win, f);
    _gen_win_inputs(fsmn, WTK_PARM_APPEND);
}

void wtk_fsmn_flush(wtk_fsmn_t *fsmn)
{
    wtk_feature_t *f = NULL;

    // flush feature_win
    while (fsmn->feature_win->used > fsmn->cfg->win)
    {
        _gen_win_inputs(fsmn, WTK_PARM_END);
    }

    while (fsmn->feature_win->used > 0)
    {
        f = (wtk_feature_t *)wtk_robin_pop(fsmn->feature_win);
        --f->used;
        wtk_parm_push_feature(fsmn->parm, f);
    }
}
