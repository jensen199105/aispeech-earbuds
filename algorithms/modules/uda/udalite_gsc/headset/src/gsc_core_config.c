#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "gsc_core_config.h"
#include "AISP_TSL_str.h"
#ifndef GSC_FIXED
#include "rbin.h"
#endif

/* Local macro declaration */
#define USE_CONFIG_DUMP

#ifndef GSC_FIXED
/* Static function declaration */
static S32 gscCfgUpdateLocal(GSC_CoreCfg_S *pstGscCfg, rbin2_local_cfg_t *main);
#endif

#ifdef USE_CONFIG_DUMP
static VOID gscCfgDump(GSC_CoreCfg_S *pstGscCfg);
#endif


S32 gscDefaultCfgInit(GSC_CoreCfg_S *pstGscCfg)
{
    GSC_CoreCfg_S *pstCfg = pstGscCfg;

    if (pstCfg == NULL)
    {
        return -1;
    }

    /* Const configs */
#ifdef GSC_FIXED
    pstCfg->iMu            = 13421773; /* Fixed: Q24 */
    pstCfg->iWnorm_thre    = 67108864; /* Fixed: Q24 */
    pstCfg->iWnorm_sqrtthre = 33554432; /* Fixed: Q24 */
    pstCfg->iSc_alpha      = 26214;    /* Fixed: Q15 */
    pstCfg->iSc_thr        = 327680;   /* Fixed: Q15 */
    pstCfg->iPSD_Y         = 0;        /* Fixed: Q15 */
#else
    pstCfg->iMu            = 0.8;
    pstCfg->iWnorm_thre    = 4;
    pstCfg->iWnorm_sqrtthre = 2;
    pstCfg->iSc_alpha      = 0.8;
    pstCfg->iSc_thr        = 10.0;
    pstCfg->iPSD_Y         = 0.0;
#endif
    pstCfg->iGsc          = 1;
    pstCfg->iWnorm_switch = 0;
    pstCfg->iTa_switch    = 1;
    pstCfg->iTa_thre      = 2;
    pstCfg->iTa_val       = 0;
    pstCfg->iTaps         = 2;
    pstCfg->iNoiseDec     = 1;
    pstCfg->iEndBins      = 4;
    pstCfg->iNoiseSta     = 0;
    /* Variables */
    pstCfg->iGscCurxIdx = 0;
    pstCfg->iInitFlag   = 0;
    pstCfg->iErrGain    = pstCfg->iTaps;
    //pstCfg->iErrGain    = 16;
#ifdef USE_CONFIG_DUMP
    printf("Default config in function \'gscDefaultCfgInit\':\n");
    gscCfgDump(pstCfg);
#endif
    return 0;
}



#ifndef GSC_FIXED
/************************************************************
 *   Function   : gscCfgUpdate()
 *   Description:
 *   Calls      :
 *   Called By  :
 *   Input      :
 *   Output     :
 *   Return     :
 *   Others     :
 *   History    :
 *       2019/04/28, chao.xu create
 *
 *************************************************************/
S32 gscCfgUpdate(GSC_CoreCfg_S *pstGscCfg, const char *pcBinFn)
{
    S32 iRet;
    rbin2_st_t       *rbin;
    rbin2_item_t     *item;
    rbin2_cfg_file_t *cfile;
    char *cfg_fn = "./udalite.cfg.r";
    rbin = rbin2_new();
    iRet = rbin2_read(rbin, (char *)pcBinFn);

    if (iRet != 0)
    {
        rbin2_debug("read failed\n");
        goto end1;
    }

    item = rbin2_get2(rbin, cfg_fn, AISP_TSL_strlen(cfg_fn));

    if (!item)
    {
        rbin2_debug("%s not found %s\n", cfg_fn, pcBinFn);
        iRet = -1;
        goto end1;
    }

    cfile = rbin2_cfg_file_new();
    rbin2_cfg_file_add_var_ks(cfile, "pwd", ".", 1);
    iRet = rbin2_cfg_file_feed(cfile, item->data->data, item->data->len);

    if (iRet != 0)
    {
        goto end2;
    }

    iRet = gscCfgUpdateLocal(pstGscCfg, cfile->main);
#ifdef USE_CONFIG_DUMP
    printf("Updated config in function \'gscCfgUpdate\':\n");
    gscCfgDump(pstGscCfg);
#endif
end2:
    rbin2_cfg_file_delete(cfile);
end1:
    rbin2_delete(rbin);
    return iRet;
}

/************************************************************
 *   Function   : gscCfgUpdateLocal()
 *   Description:
 *   Calls      :
 *   Called By  :
 *   Input      :
 *   Output     :
 *   Return     :
 *   Others     :
 *   History    :
 *       2019/04/28, chao.xu create
 *
 *************************************************************/
static S32 gscCfgUpdateLocal(GSC_CoreCfg_S *pstGscCfg, rbin2_local_cfg_t *main)
{
    rbin2_string_t    *v  = NULL;
    rbin2_local_cfg_t *lc = NULL;
    /* confParm */
    lc = rbin2_local_cfg_find_lc_s(main, "confParm");

    if (lc != NULL)
    {
        rbin2_local_cfg_update_cfg_f(lc, pstGscCfg, iMu,             v);
        rbin2_local_cfg_update_cfg_f(lc, pstGscCfg, iSc_alpha,       v);
        rbin2_local_cfg_update_cfg_f(lc, pstGscCfg, iSc_thr,         v);
        rbin2_local_cfg_update_cfg_i(lc, pstGscCfg, iWnorm_thre,     v);
        rbin2_local_cfg_update_cfg_i(lc, pstGscCfg, iWnorm_sqrtthre, v);
        rbin2_local_cfg_update_cfg_i(lc, pstGscCfg, iGsc,            v);
        rbin2_local_cfg_update_cfg_i(lc, pstGscCfg, iWnorm_switch,   v);
        rbin2_local_cfg_update_cfg_i(lc, pstGscCfg, iTa_switch,      v);
        rbin2_local_cfg_update_cfg_i(lc, pstGscCfg, iTa_thre,        v);
        rbin2_local_cfg_update_cfg_i(lc, pstGscCfg, iTa_val,         v);
        rbin2_local_cfg_update_cfg_i(lc, pstGscCfg, iTaps,           v);
        rbin2_local_cfg_update_cfg_i(lc, pstGscCfg, iNoiseDec,       v);
        rbin2_local_cfg_update_cfg_i(lc, pstGscCfg, iEndBins,        v);
        rbin2_local_cfg_update_cfg_i(lc, pstGscCfg, iNoiseSta,       v);
    }

    return 0;
}
#endif

#ifdef USE_CONFIG_DUMP
/************************************************************
 *   Function   : gscCfgDump()
 *   Description: Dump the total config for debug.
 *   Calls      :
 *   Called By  :
 *   Input      :
 *   Output     :
 *   Return     :
 *   Others     :
 *   History    :
 *       2019/04/28, chao.xu create
 *
 *************************************************************/
static VOID gscCfgDump(GSC_CoreCfg_S *pstGscCfg)
{
    printf("iMu             = %f\n", (float)pstGscCfg->iMu);
    printf("iSc_alpha       = %f\n", (float)pstGscCfg->iSc_alpha);
    printf("iSc_thr         = %f\n", (float)pstGscCfg->iSc_thr);
    printf("iWnorm_thre     = %d\n", pstGscCfg->iWnorm_thre);
    printf("iWnorm_sqrtthre = %d\n", pstGscCfg->iWnorm_sqrtthre);
    printf("iGsc            = %d\n", pstGscCfg->iGsc);
    printf("iWnorm_switch   = %d\n", pstGscCfg->iWnorm_switch);
    printf("iTa_switch      = %d\n", pstGscCfg->iTa_switch);
    printf("iTa_thre        = %d\n", pstGscCfg->iTa_thre);
    printf("iTa_val         = %d\n", pstGscCfg->iTa_val);
    printf("iTaps           = %d\n", pstGscCfg->iTaps);
    printf("iNoiseDec       = %d\n", pstGscCfg->iNoiseDec);
    printf("iEndBins        = %d\n", pstGscCfg->iEndBins);
    printf("iNoiseSta       = %d\n", pstGscCfg->iNoiseSta);
}
#endif /* USE_CONFIG_DUMP Macro */

