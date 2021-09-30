/************************************************************
  Copyright (C), AISpeechTech. Co., Ltd.

  FileName   : aec_api_config.c
  Project    :
  Module     :
  Version    :
  Date       : 2017/06/19
  Author     : Youhai.Jiang
  Document   :
  Description:

  Function List :

  History :
  <author>      <time>            <version >        <desc>
  Youhai.Jiang  2017/06/19         1.00              Create

************************************************************/
#include <stdlib.h>
#include <string.h>
#include "sevc_config.h"

#define USE_CONFIG  0
#ifndef AISPEECH_FIXED
#include "sevc_types.h"
#include <rbin.h>

/************************************************************
  Function   : aec_api_cfg_update_local()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2017/06/19, Youhai.Jiang create

************************************************************/
int sevcApiCfgUpdateLocal(SEVC_CONFIG_S *cfg, rbin2_local_cfg_t *main)
{
    rbin2_string_t *v;
    rbin2_local_cfg_t *lc = NULL;
    /* switches*/
    lc = rbin2_local_cfg_find_lc_s(main, "switch");

    if (lc != NULL)
    {
        rbin2_local_cfg_update_cfg_i(lc, cfg, AEC_flag, v);
        rbin2_local_cfg_update_cfg_b(lc, cfg, BF_flag, v);
        rbin2_local_cfg_update_cfg_b(lc, cfg, NN_flag, v);
        rbin2_local_cfg_update_cfg_b(lc, cfg, AGC_flag, v);
        rbin2_local_cfg_update_cfg_b(lc, cfg, aecPreemphSupport, v);
        rbin2_local_cfg_update_cfg_b(lc, cfg, nnBarkSupport, v);
    }

    /* input configuration*/
    lc = rbin2_local_cfg_find_lc_s(main, "params");

    if (lc != NULL)
    {
        rbin2_local_cfg_update_cfg_b(lc, cfg, hdr, v);
        rbin2_local_cfg_update_cfg_i(lc, cfg, fs, v);
        rbin2_local_cfg_update_cfg_i(lc, cfg, wavChan, v);
        rbin2_local_cfg_update_cfg_i(lc, cfg, micChan, v);
        rbin2_local_cfg_update_cfg_i(lc, cfg, refChan, v);
    }

    /* parameters*/
#if 1
    lc = rbin2_local_cfg_find_lc_s(main, "config");

    if (lc != NULL)
    {
        rbin2_local_cfg_update_cfg_i(lc, cfg, frameShift, v);
        //rbin2_local_cfg_update_cfg_i(lc, cfg, scalarShift, v);
    }

#endif

    /* aec_config*/
    if (cfg->AEC_flag)
    {
        lc = rbin2_local_cfg_find_lc_s(main, "aec_config");

        if (lc != NULL)
        {
            rbin2_local_cfg_update_cfg_i(lc, cfg, Taps, v);
            rbin2_local_cfg_update_cfg_f(lc, cfg, preemph, v);
            rbin2_local_cfg_update_cfg_f(lc, cfg, Mu, v);
            rbin2_local_cfg_update_cfg_f(lc, cfg, aecOutGain, v);
        }
    }

    /* bf_config*/
    if (cfg->BF_flag)
    {
        lc = rbin2_local_cfg_find_lc_s(main, "bf_config");

        if (lc != NULL)
        {
        }
    }

    /* nn_config*/
    if (cfg->NN_flag)
    {
        lc = rbin2_local_cfg_find_lc_s(main, "nn_config");

        if (lc != NULL)
        {
#ifdef SEVC_NN_FSMN
            rbin2_local_cfg_update_cfg_i(lc, cfg, feature_context_L, v);
            rbin2_local_cfg_update_cfg_i(lc, cfg, feature_context_R, v);
#endif
            v = rbin2_local_cfg_find_string_s(lc, "nnRnnModelFile");
            strncpy(cfg->nnRnnModelFile, v->data, sizeof(cfg->nnRnnModelFile) - 1);
        }
    }

    /* agc_config*/
    if (cfg->AEC_flag)
    {
        lc = rbin2_local_cfg_find_lc_s(main, "agc_config");

        if (lc != NULL)
        {
            rbin2_local_cfg_update_cfg_f(lc, cfg, G_low, v);
            rbin2_local_cfg_update_cfg_f(lc, cfg, ratio1, v);
            rbin2_local_cfg_update_cfg_f(lc, cfg, ratio2, v);
            rbin2_local_cfg_update_cfg_f(lc, cfg, T1, v);
            rbin2_local_cfg_update_cfg_f(lc, cfg, T2, v);
            rbin2_local_cfg_update_cfg_f(lc, cfg, T2_gain, v);
            rbin2_local_cfg_update_cfg_i(lc, cfg, vad_smooth_short_len, v);
            rbin2_local_cfg_update_cfg_i(lc, cfg, vad_smooth_long_len, v);
            rbin2_local_cfg_update_cfg_f(lc, cfg, vad_smooth_short_thr, v);
            rbin2_local_cfg_update_cfg_f(lc, cfg, vad_smooth_long_thr, v);
            rbin2_local_cfg_update_cfg_f(lc, cfg, vad_thresh, v);
            rbin2_local_cfg_update_cfg_f(lc, cfg, AttackTime_beginVAD, v);
            rbin2_local_cfg_update_cfg_f(lc, cfg, AttackTime_inVAD, v);
            rbin2_local_cfg_update_cfg_f(lc, cfg, ReleaseTime_inVAD, v);
            rbin2_local_cfg_update_cfg_f(lc, cfg, ReleaseTime_sil, v);
        }
    }

    return 0;
}

/************************************************************
  Function   : SEVC_CFG_Update()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2017/06/19, Youhai.Jiang create

************************************************************/
S32 _SEVC_CFG_Update_(SEVC_CONFIG_S *cfg, const char *bin_fn)
{
#if USE_CONFIG
    rbin2_st_t *rbin;
    rbin2_cfg_file_t *cfile;
    rbin2_item_t *item;
    char *cfg_fn = "./sevc.cfg.r";
    int ret;
    rbin = rbin2_new();
    // printf("rbin new sucess\n");
    ret = rbin2_read(rbin, (char *)bin_fn);

    if (ret != 0)
    {
        rbin2_debug("read failed\n");
        goto end1;
    }

    item = rbin2_get2(rbin, cfg_fn, strlen(cfg_fn));

    if (!item)
    {
        rbin2_debug("%s not found %s\n", cfg_fn, bin_fn);
        ret = -1;
        goto end1;
    }

    cfile = rbin2_cfg_file_new();
    rbin2_cfg_file_add_var_ks(cfile, "pwd", ".", 1);
    ret = rbin2_cfg_file_feed(cfile, item->data->data, item->data->len);

    if (ret != 0)
    {
        goto end2;
    }

    ret = sevcApiCfgUpdateLocal(cfg, cfile->main);
end2:
    rbin2_cfg_file_delete(cfile);
end1:
    rbin2_delete(rbin);
    return ret;
#else
    return 0;
#endif
}

#endif
