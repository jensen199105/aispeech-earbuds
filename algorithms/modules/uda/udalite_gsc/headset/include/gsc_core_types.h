/************************************************************
  Copyright (C), AISpeechTech. Co., Ltd.

  FileName   : gsc_core_types.h
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
#ifndef __GSC_CORE_TYPES_H__
#define __GSC_CORE_TYPES_H__

#include "AISP_TSL_types.h"
#include "fft_wrapper.h"

/* Macro */
#define GSC_MICS            (2)
#define GSC_IN_CHAN         (2)
#define GSC_FFTBINS         (257)
#define GSC_FFTLEN          (512)
#define GSC_FRMLEN          (512)
#define GSC_FRMINC          (256)

#ifndef GSC_FIXED
/* Float version */
#define GSC_S16_SCALAR      (32767)
#define GSC_Q15_SCALAR      (0.000030517578125)
#define GSC_ZERO_THRESH     (1e-10f)
#else
/* Fixed version */
#define GSC_S16_SCALAR      (1)
#define GSC_Q15_SCALAR      (1)
#define GSC_ZERO_THRESH     (1)
#endif

#ifdef GSC_EXPORT_EN
#define GSC_VISIBILITY_ATTR __attribute__((visibility("default")))
#else
#define GSC_VISIBILITY_ATTR
#endif

#ifdef GSC_FIXED
typedef long long           GSC_S64_T;
typedef int                 GSC_S32_T;
typedef short               GSC_S16_T;
#else
typedef float               GSC_S64_T;
typedef float               GSC_S32_T;
typedef float               GSC_S16_T;
#endif

typedef struct tagGSC_COMPLEX64
{
    GSC_S64_T real;
    GSC_S64_T imag;
} GSC_COMPLEX64_T;

typedef struct tagGSC_COMPLEX
{
    GSC_S32_T real;
    GSC_S32_T imag;
} GSC_COMPLEX_T;

typedef struct tagGSC_COMPLEX16
{
    GSC_S16_T real;
    GSC_S16_T imag;
} GSC_COMPLEX16_T;


/* Callback functions(hook) for gsc output. */
typedef void (*GSC_FUNCHOOK_T)(void *, int, char *, int);

typedef struct tagGSC_CoreCfg
{
    /* Const configs */
    GSC_S32_T iMu;

    S32 iGsc;
    S32 iWnorm_switch;
    S32 iTa_switch;
    S32 iWnorm_thre;
    S32 iWnorm_sqrtthre;
    S32 iTa_thre;
    S32 iTa_val;
    S32 iTaps;
    S32 iNoiseDec;     /* Wind noise detection */
    S32 iErrGain;      /* Wind noise detection */

    /* Variables */
    GSC_S64_T iPSD_Y;
    GSC_S32_T iSc_alpha;
    GSC_S32_T iSc_thr;
    S32       iEndBins;
    S32       iNoiseSta;

    S32       iGscCurxIdx;
    S32       iInitFlag;
} GSC_CoreCfg_S;

typedef struct tagGSC_Core
{
    /* Shared memory */
    GSC_COMPLEX_T     *pcpxXFFT;      //[RDMA_MICS][RDMA_FFTBINS];
    GSC_COMPLEX_T     *pcpxMic1;      //[RDMA_FFTBINS];
    GSC_COMPLEX_T     *pcpxMic2;      //[RDMA_FFTBINS];

    /* Unshared memory */
    GSC_COMPLEX_T     *pcpxGscX;      //[RDMA_GSC][RDMA_FFTBINS*RDMA_GSC_TAPS];
    GSC_COMPLEX_T     *pcpxGscWeight; //[RDMA_GSC][RDMA_FFTBINS*RDMA_GSC_TAPS];
    GSC_S64_T         *pGscXPowerSum; //[RDMA_GSC][RDMA_FFTBINS];
    GSC_S32_T         *piRaw;         //[RDMA_MICS][RDMA_FRMINC];
    GSC_S16_T         *psDataLeft;    //[RDMA_GSC][RDMA_FRMINC];

    /* Const tables */
    GSC_COMPLEX_T     *pcpxGscWq;
    GSC_COMPLEX_T     *pcpxGscB;
} GSC_Core_S;

typedef struct tagGSC_Api
{
    GSC_Core_S        *pstGscInstance;
    GSC_CoreCfg_S     *pstGscCfg;

    GSC_S32_T         *psHanningWind; /* Used for fft window. */
    VOID              *pvUsrData;
    GSC_FUNCHOOK_T     pfuncGscOutputHook;
    AISP_TSL_FFTENG_S  stFFTEng;

    GSC_FUNCHOOK_T     pfunWindCb;    /* Only used for wind noise detection. */
    VOID              *pParm;
} GSC_Api_S;

#endif
