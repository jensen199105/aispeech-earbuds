#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "gsc_core_config.h"
#include "AISP_TSL_str.h"
#ifndef GSC_FIXED
#include "rbin.h"
#endif

/* Local macro declaration */
//#define USE_CONFIG_DUMP

#ifndef GSC_FIXED
/* Static function declaration */
static S32 gscCfgUpdateLocal(GSC_CoreCfg_S *pstGscCfg, rbin2_local_cfg_t *main);
#endif

static VOID gscCfgDump(GSC_CoreCfg_S *pstGscCfg);

/************************************************************
 *   Function   : gscDefaultCfgInit()
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
S32 gscDefaultCfgInit(GSC_CoreCfg_S *pstGscCfg)
{
    GSC_CoreCfg_S *pstCfg = pstGscCfg;

    if (pstCfg == NULL)
    {
        return -1;
    }

    /* Const configs */
    pstCfg->iGsc        = 3;
    pstCfg->iTaps       = 8;
    pstCfg->iDeltaGain  = 2;
#ifdef GSC_FIXED
    pstCfg->iMu         = 26214;     // Q15
    pstCfg->iGamma_pre  = 16384;     // Q15
    pstCfg->iMaxDelta   = 68719477;  // W32Q15
    pstCfg->iupStep     = 34359738;  // W32Q31
    pstCfg->idownStep   = 213030;    // Q31
#else
    pstCfg->iMu         = 0.8;
    pstCfg->iGamma_pre  = 0.5;
    pstCfg->iMinDelta   = 0.0073;
    pstCfg->iMaxDelta   = 2097.152;
    pstCfg->iupStep     = 0.016;
    pstCfg->idownStep   = 0.0000992;
#endif
    /* Variables */
    pstCfg->iInitFlag    = 0;
    pstCfg->iGscCurxIdx  = 0;
#ifdef AISP_TSL_INFO
    printf("Default config in function \'gscDefaultCfgInit\':\n");
#endif
    gscCfgDump(pstCfg);
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
#ifdef AISP_TSL_INFO
    printf("Updated config in function \'gscCfgUpdate\':\n");
#endif
    gscCfgDump(pstGscCfg);
end2:
    rbin2_cfg_file_delete(cfile);
end1:
    rbin2_delete(rbin);
    return iRet;
}

/************************************************************
 *   Function   : doaCfgUpdateLocal()
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
        rbin2_local_cfg_update_cfg_f(lc, pstGscCfg, iMu,        v);
        rbin2_local_cfg_update_cfg_f(lc, pstGscCfg, iGamma_pre, v);
        rbin2_local_cfg_update_cfg_f(lc, pstGscCfg, iMinDelta,  v);
        rbin2_local_cfg_update_cfg_f(lc, pstGscCfg, iMaxDelta,  v);
        rbin2_local_cfg_update_cfg_f(lc, pstGscCfg, iupStep,    v);
        rbin2_local_cfg_update_cfg_f(lc, pstGscCfg, idownStep,  v);
        rbin2_local_cfg_update_cfg_f(lc, pstGscCfg, iGsc,       v);
        rbin2_local_cfg_update_cfg_f(lc, pstGscCfg, iTaps,      v);
        rbin2_local_cfg_update_cfg_f(lc, pstGscCfg, iDeltaGain, v);
    }

    return 0;
}
#endif

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
#ifdef USE_CONFIG_DUMP
    printf("iMu           = %f\n", (float)pstGscCfg->iMu);
    printf("iGamma_pre    = %f\n", (float)pstGscCfg->iGamma_pre);
    printf("iMinDelta     = %f\n", (float)pstGscCfg->iMinDelta);
    printf("iMaxDelta     = %f\n", (float)pstGscCfg->iMaxDelta);
    printf("iupStep       = %f\n", (float)pstGscCfg->iupStep);
    printf("idownStep     = %f\n", (float)pstGscCfg->idownStep);
    printf("iGsc          = %d\n", pstGscCfg->iGsc);
    printf("iTaps         = %d\n", pstGscCfg->iTaps);
    printf("iDeltaGain    = %d\n", pstGscCfg->iDeltaGain);
#endif
}

