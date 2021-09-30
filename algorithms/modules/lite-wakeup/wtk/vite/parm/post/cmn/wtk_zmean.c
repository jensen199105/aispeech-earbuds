#include "wtk_zmean.h"
#include "wtk/vite/parm/wtk_parm.h"
#include "AISP_TSL_math.h"
#include "AISP_TSL_str.h"
wtk_zmean_t *wtk_zmean_new(wtk_zmean_cfg_t *cfg, struct wtk_parm *parm)
{
    wtk_zmean_t *z;
    //int index = 0;
    int cmn_robin_len = 1;
    z = (wtk_zmean_t *)wtk_malloc(sizeof(*z));
    z->cfg = cfg;
    z->frames = 0;
    z->parm = parm;
    z->past_feat_robin = wtk_robin_new(cfg->win_size + 1);
    cmn_robin_len = (cmn_robin_len < z->cfg->start_min_frame ? z->cfg->start_min_frame : cmn_robin_len);
    cmn_robin_len = (cmn_robin_len < z->cfg->left_seek_frame ? z->cfg->left_seek_frame : cmn_robin_len);
    cmn_robin_len = (cmn_robin_len < z->cfg->min_flush_frame ? z->cfg->min_flush_frame : cmn_robin_len);
    z->cmn_robin = wtk_robin_new(cmn_robin_len);
    z->vec_size = parm->cfg->static_feature_cols;
    z->buf = (int *)wtk_calloc(1, (z->vec_size + 1) * sizeof(int));
    z->cur = (int *)wtk_calloc(1, (z->vec_size + 1) * sizeof(int));
    z->buf_tmp = (long long *)wtk_calloc(1, (z->vec_size + 1) * sizeof(long long));
    wtk_zmean_reset(z);
    return z;
}

int wtk_zmean_delete(wtk_zmean_t *z)
{
    wtk_free(z->cur);
    wtk_free(z->buf);
    wtk_free(z->buf_tmp);
    wtk_robin_delete(z->past_feat_robin);
    wtk_robin_delete(z->cmn_robin);
    wtk_free(z);
    return 0;
}

int wtk_zmean_reset(wtk_zmean_t *z)
{
    wtk_queue_init(&(z->post_feature_q));
    z->frames = 0;
    AISP_TSL_memset(z->cur, 0, sizeof(int) * (z->vec_size + 1));
    AISP_TSL_memset(z->buf, 0, sizeof(int) * (z->vec_size + 1));

    if (z->past_feat_robin)
    {
        wtk_robin_reset(z->past_feat_robin);
    }

    if (z->cmn_robin)
    {
        AISP_TSL_memset(z->buf_tmp, 0, sizeof(long long) * (z->vec_size + 1));
        wtk_robin_reset(z->cmn_robin);
    }

    return 0;
}

/*
 * represents for 1, 1/2, 1/3, 1/4, ... in Q24
 */
const int denominator[] =
{
    16777216, 8388608, 5592405, 4194304, 3355443, 2796203, 2396745, 2097152,
    1864135, 1677722, 1525201, 1398101, 1290555, 1198373, 1118481, 1048576,
    986895,  932068,  883011,  838861,  798915,  762601,  729444,  699051,
    671089,  645278,  621378,  599186,  578525,  559241,  541201,  524288,
    508400,  493448,  479349,  466034,  453438,  441506,  430185,  419430,
    409200,  399458,  390168,  381300,  372827,  364722,  356962,  349525,
    342392,  335544,  328965,  322639,  316551,  310689,  305040,  299593,
    294337,  289262,  284360,  279620,  275036,  270600,  266305,  262144,
    258111,  254200,  250406,  246724,  243148,  239675,  236299,  233017,
    229825,  226719,  223696,  220753,  217886,  215093,  212370,  209715,
    207126,  204600,  202135,  199729,  197379,  195084,  192842,  190650,
    188508,  186414,  184365,  182361,  180400,  178481,  176602,  174763,
    172961,  171196,  169467,  167772
};

void wtk_zmean_update_buf(wtk_zmean_t *z, int *v)
{
    int *buf = z->buf;
    int i, n = z->vec_size;
    wtk_feature_t *pre_f;

    if (z->cfg->win_size > 0 && z->past_feat_robin->used == z->past_feat_robin->nslot)
    {
        pre_f = (wtk_feature_t *)wtk_robin_pop(z->past_feat_robin);

        for (i = 1; i <= n; ++i)
        {
            z->buf_tmp[i] += ((v[i] << 4) - (pre_f->cmn[i - 1] << 4));
            buf[i] = (z->buf_tmp[i] * denominator[z->cfg->win_size - 1]) >> 24;
        }

        --pre_f->used;
        wtk_parm_push_feature(z->parm, pre_f);
        ++z->frames;
    }
    else
    {
        ++z->frames;

        for (i = 1; i <= n; ++i)
        {
            z->buf_tmp[i] += (v[i] << 4);
            buf[i] = ((z->buf_tmp[i]) * denominator[z->frames - 1]) >> 24;
        }
    }
}

void wtk_zmean_update_cmn(wtk_zmean_t *z)
{
    int *buf = z->buf;
    int i, n = z->vec_size;
    int *cur = z->cur;

    for (i = 1; i <= n; ++i)
    {
        cur[i] = buf[i];
    }
}

void wtk_zmean_process_cmn(wtk_zmean_t *z, int *feat)
{
    int *cmn;
    int i, tmp, n = z->vec_size;
    cmn = z->cur;

    for (i = 1; i <= n; ++i)
    {
        tmp = feat[i] << 4;
        tmp -= cmn[i];
        feat[i] = tmp >> 4;
    }
}

void wtk_zmean_static_post_feed(wtk_zmean_t *z, wtk_feature_t *f)
{
    AISP_TSL_copy_q31(&(f->v[1]), f->cmn, z->vec_size);

    if (z->cfg->win_size > 0)
    {
        ++f->used;
        wtk_robin_push(z->past_feat_robin, f);
    }

    wtk_zmean_update_buf(z, f->v);

    if (z->frames >= z->cfg->start_min_frame)
    {
        wtk_zmean_update_cmn(z);

        if (z->cfg->left_seek_frame > 0 && z->frames >= z->cfg->left_seek_frame)
        {
            if (z->cfg->min_flush_frame <= 0 || z->cmn_robin->used >= z->cfg->min_flush_frame)
            {
                wtk_zmean_process_cmn(z, f->v);

                while (z->cmn_robin->used > 0)
                {
                    f = (wtk_feature_t *)wtk_robin_pop(z->cmn_robin);
                    wtk_zmean_process_cmn(z, f->v);
                    --f->used;
                }
            }
            else
            {
                ++f->used;
                wtk_robin_push(z->cmn_robin, f);
            }
        }
        else
        {
            ++f->used;
            wtk_robin_push(z->cmn_robin, f);

            if (((z->cfg->post_update_frame <= 0) || (z->cmn_robin->used >= z->cfg->post_update_frame)))
            {
                if (z->cfg->smooth)
                {
                    f = (wtk_feature_t *)wtk_robin_pop(z->cmn_robin);
                    wtk_zmean_process_cmn(z, f->v);
                    --f->used;
                }
                else
                {
                    while (z->cmn_robin->used > 0)
                    {
                        f = (wtk_feature_t *)wtk_robin_pop(z->cmn_robin);
                        wtk_zmean_process_cmn(z, f->v);
                        --f->used;
                    }
                }
            }
        }
    }
    else
    {
        ++f->used;
        wtk_robin_push(z->cmn_robin, f);
    }
}

static void wtk_zmean_flush_queue(wtk_zmean_t *z, wtk_queue_t *q)
{
    wtk_queue_node_t *n;
    wtk_feature_t *f;

    while (1)
    {
        n = wtk_queue_pop(q);

        if (!n)
        {
            break;
        }

        f = data_offset(n, wtk_feature_t, queue_n);
        --f->used;
        wtk_parm_output_feature(z->parm, f);
    }
}

void wtk_zmean_flush_parm_post_queue(wtk_zmean_t *z)
{
    wtk_feature_t *f;

    if (z->frames < z->cfg->start_min_frame)
    {
        // cmn not update for min frame is long enough;
        wtk_zmean_update_cmn(z);
    }

    while (z->cmn_robin->used > 0)
    {
        f = (wtk_feature_t *)wtk_robin_pop(z->cmn_robin);
        wtk_zmean_process_cmn(z, f->v);
        --f->used;
    }

    wtk_zmean_flush_queue(z, &(z->post_feature_q));
}

void wtk_zmean_flush_robin(wtk_zmean_t *z)
{
    wtk_robin_t *r;
    wtk_feature_t *f;
    r = z->past_feat_robin;

    while (r->used > 0)
    {
        f = (wtk_feature_t *)wtk_robin_pop(r);
        --f->used;
        wtk_parm_push_feature(z->parm, f);
    }
}

static void wtk_zmean_flush_one_feature(wtk_zmean_t *z)
{
    wtk_queue_t *q = &(z->post_feature_q);
    wtk_queue_node_t *n;
    wtk_feature_t *f;

    while (1)
    {
        n = wtk_queue_pop(q);

        if (!n)
        {
            break;
        }

        f = data_offset(n, wtk_feature_t, queue_n);
        --f->used;
        wtk_parm_output_feature(z->parm, f);
        break;
    }
}

void wtk_zmean_post_feed(wtk_zmean_t *z, wtk_feature_t *f)
{
    if (z->frames >= z->cfg->start_min_frame)
    {
        if (z->cfg->left_seek_frame > 0 && z->frames >= z->cfg->left_seek_frame)
        {
            if (z->cfg->min_flush_frame <= 0 || z->post_feature_q.length >= z->cfg->min_flush_frame)
            {
                if (z->post_feature_q.length > 0)
                {
                    wtk_zmean_flush_queue(z, &(z->post_feature_q));
                }

                wtk_parm_output_feature(z->parm, f);
            }
            else
            {
                ++f->used;
                wtk_queue_push(&(z->post_feature_q), &(f->queue_n));
            }
        }
        else
        {
            ++f->used;
            wtk_queue_push(&(z->post_feature_q), &(f->queue_n));

            if (((z->cfg->post_update_frame <= 0) || (z->post_feature_q.length >= z->cfg->post_update_frame)))
            {
                if (z->cfg->smooth)
                {
                    wtk_zmean_flush_one_feature(z);
                }
                else
                {
                    wtk_zmean_flush_queue(z, &(z->post_feature_q));
                }
            }
        }
    }
    else
    {
        ++f->used;
        wtk_queue_push(&(z->post_feature_q), &(f->queue_n));
    }
}

void wtk_zmean_flush(wtk_zmean_t *z, int force)
{
    wtk_feature_t *f;
    int min_frame;
    int b;

    if (force)
    {
        b = 1;
    }
    else
    {
        min_frame = z->cfg->min_flush_frame;

        if (min_frame > 0 && z->frames >= z->cfg->start_min_frame && z->cfg->left_seek_frame > 0 &&
                z->frames >= z->cfg->left_seek_frame && z->post_feature_q.length >= min_frame)
        {
            b = 1;
        }
        else
        {
            b = 0;
        }
    }

    if (b)
    {
        while (z->cmn_robin->used > 0)
        {
            f = (wtk_feature_t *)wtk_robin_pop(z->cmn_robin);
            wtk_zmean_process_cmn(z, f->v);
            --f->used;
        }

        wtk_zmean_flush_queue(z, &(z->post_feature_q));
    }
}

int wtk_zmean_can_flush_all(wtk_zmean_t *z)
{
    int b;

    if (z->cfg->left_seek_frame > 0 && z->frames >= z->cfg->left_seek_frame)
    {
        b = 1;
    }
    else
    {
        b = 0;
    }

    return b;
}
