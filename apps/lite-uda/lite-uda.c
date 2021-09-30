/************************************************************
  Copyright (C), AISpeechTech. Co., Ltd.

  FileName   : lite-wakeup.c
  Project    :
  Module     :
  Version    :
  Date       : 2019/10/10
  Author     : Chao.Xu
  Document   :
  Description:

  Function List :

  History :
  <author>      <time>            <version >        <desc>
  Chao.Xu       2019/10/10         1.00              Create

************************************************************/
#include "gsc_api.h"

#define LUDA_API_ATTR   __attribute__ ((visibility ("default")))

/************************************************************
  Function   : LUDA_CfgInit()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/10/10, Chao.Xu create

************************************************************/
LUDA_API_ATTR int LUDA_CfgInit(GSC_CoreCfg_S *pstGscCfg, const char *pcBinFn)
{
    return GSC_CfgInit(pstGscCfg, pcBinFn);
}

/************************************************************
  Function   : LUDA_CfgMemSizeGet()

  Description: Get the size of cfg struct
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/10/10, Chao.Xu create

************************************************************/
LUDA_API_ATTR int LUDA_CfgMemSizeGet(void)
{
    return GSC_CfgMemSizeGet();
}

/************************************************************
  Function   : LUDA_SharedMemSizeGet()

  Description: Get the size of the shared memory.
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/10/10, Chao.Xu create

************************************************************/
LUDA_API_ATTR int LUDA_SharedMemSizeGet(GSC_CoreCfg_S *pstGscCfg)
{
    return GSC_SharedMemSizeGet(pstGscCfg);
}

/************************************************************
  Function   : LUDA_UnsharedMemSizeGet()

  Description: Get the size of the unshared memory.
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/10/10, Chao.Xu create

************************************************************/
LUDA_API_ATTR int LUDA_UnsharedMemSizeGet(GSC_CoreCfg_S *pstGscCfg)
{
    return GSC_UnsharedMemSizeGet(pstGscCfg);
}

/************************************************************
  Function   : LUDA_New()

  Description: Initialize the engine struct.
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/10/10, Chao.Xu create

************************************************************/
LUDA_API_ATTR void *LUDA_New(GSC_CoreCfg_S *pstGscCfg, S8 *pcSharedBuf, S8 *pcUnsharedBuf, GSC_FUNCHOOK_T pGscCallback,
                             void *pvUsrData)
{
    return GSC_New(pstGscCfg, pcSharedBuf, pcUnsharedBuf, pGscCallback, pvUsrData);
}

/************************************************************
  Function   : LUDA_LenPerSnd()

  Description: Get the size of input data, calculated as bytes.
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/10/10, Chao.Xu create

************************************************************/
LUDA_API_ATTR int LUDA_LenPerSnd(GSC_Api_S *pstGscApi)
{
    return GSC_LenPerSnd(pstGscApi);
}

/************************************************************
  Function   : LUDA_Feed()

  Description: Feed audio wav into engine, the data is interleaved.
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/10/10, Chao.Xu create

************************************************************/
LUDA_API_ATTR int LUDA_Feed(GSC_Api_S *pstGscApi, S8 *pcData, S32 iLen)
{
    return GSC_Feed(pstGscApi, pcData, iLen);
}

/************************************************************
  Function   : LUDA_FeedParallel()

  Description: Feed audio wav into engine in parallel.
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/10/10, Chao.Xu create

************************************************************/
LUDA_API_ATTR int LUDA_FeedParallel(GSC_Api_S *pstGscApi, S8 *pcChan1, S8 *pcChan2, S32 iLen)
{
    return GSC_FeedParallel(pstGscApi, pcChan1, pcChan2, iLen);
}

/************************************************************
  Function   : LUDA_Reset()

  Description: Reset all the shared/unshared memory except configs.
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/10/10, Chao.Xu create

************************************************************/
LUDA_API_ATTR void LUDA_Reset(GSC_Api_S *pstGscApi)
{
    GSC_Reset(pstGscApi);
}

/************************************************************
  Function   : LUDA_CfgDelete()

  Description: Free cfg struct if calloced.
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/10/10, Chao.Xu create

************************************************************/
LUDA_API_ATTR void LUDA_CfgDelete(GSC_CoreCfg_S *pstGscCfg)
{
    GSC_CfgDelete(pstGscCfg);
}

/************************************************************
  Function   : LUDA_Delete()

  Description: Free the necessary memory.
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/10/10, Chao.Xu create

************************************************************/
LUDA_API_ATTR void LUDA_Delete(GSC_Api_S *pstGscApi)
{
    GSC_Delete(pstGscApi);
}

/************************************************************
  Function   : LUDA_Version()

  Description: Get the version info.
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/10/10, Chao.Xu create

************************************************************/
LUDA_API_ATTR char *LUDA_Version(void)
{
    return GSC_Version();
}

#ifdef ENABLE_LUDA_FOR_HEADSET
/************************************************************
  Function   : LUDA_WindCbRegister()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/10/15, Chao.Xu create

************************************************************/
LUDA_API_ATTR int LUDA_WindCbRegister(GSC_Api_S *pstGscApi, void *pvHandler, void *pvData)
{
    return GSC_WindCbRegister(pstGscApi, pvHandler, pvData);
}
#endif

