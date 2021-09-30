
/************************************************************
  Copyright (C), AISpeechTech. Co., Ltd.

  FileName   : gsc_core_func.h
  Project    :
  Module     :
  Version    :
  Date       : 2019/04/28
  Author     : Chao.Xu
  Document   :
  Description:

  Function List :

  History :
  <author>      <time>            <version >        <desc>
  Chao.Xu       2019/04/28        1.00              Create

************************************************************/
#ifndef __GSC_CORE_FUNC_H__
#define __GSC_CORE_FUNC_H__

#include "gsc_core_types.h"

/* Zone for fft functions */
S32 fftInit(AISP_TSL_FFTENG_S *pstFftEng, S32 fftSize);
S32 fftProcess(GSC_Api_S *pstGscApi, S8 *pcData);
S32 fftProcessParallel(GSC_Api_S *pstGscApi, S8 *pcChan1, S8 *pcChan2);
VOID fftDelete(AISP_TSL_FFTENG_S *pstFftEng);

/* Zone for gsc functions */
S32 gscUpdateVAD(GSC_Api_S *pstGscApi, GSC_COMPLEX_T *pcpxY);
S32 gscUpdateHalfVAD(GSC_Api_S *pstGscApi, GSC_COMPLEX_T *pcpxY);
VOID gscProcess(GSC_Api_S *pstGscApi, GSC_COMPLEX_T *pcpxY);
VOID dataPop(GSC_Api_S *pstGscApi, VOID *pvData);

#endif

