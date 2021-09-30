#ifndef __NAES_H__
#define __NAES_H__

#include "AISP_TSL_base.h"
#include "AISP_TSL_types.h"
#include "AISP_TSL_complex.h"
#include "AISP_TSL_fft.h"

#define PI          AISP_FIXED16(3.1415926f)
#define AEC_TAPS    (4)             // length of DFT filter bank
#define TAPS_SHFT   (2)
#ifdef AISPEECH_FIXED
#define GAMMA       (655)
#define GAMMA_PRE   (16384)
#else
#define GAMMA       AISP_FIXED16(0.02)
#define GAMMA_PRE   AISP_FIXED16(0.5f)
#endif
#define DELTA_GAIN  (2)

typedef struct naes_core naes_core_T;
typedef struct tagAec_coreConfReset aec_coreConfReset_T;

struct naes_core
{
    unsigned int frameSize;
    unsigned int sampleRate;
    unsigned int fftBins;
    unsigned int xcoherenceInit;
    unsigned int xcoherenceInit2;


    void *memoryPool;

    /* power spectrum get from mdf, do not allocate memory for them */
    const aisp_s32_t *pErrPower;

    /* local arrays for leak estimation */
    aisp_s32_t *aPost;
    aisp_s32_t *residualEcho;
    aisp_s32_t *pPs_old;
    aisp_s32_t *LEAK2;
    aisp_cpx_s32_t *PPey;
    aisp_s32_t *PPyy ;
    aisp_s32_t *PPee ;

    aisp_s16_t Noise_floor;
};

typedef struct tagAec aec_core_T;
typedef struct tagAec_coreConf aec_coreConf_T;

struct tagAec_coreConf
{
    char  index;
    char  wavChan;
    char  micNum;
    char  refNum;
    char  vad;
    char  hdr;
    char  mute;
    char  nlp;
    char  emd;
    char  constrain;
    // char  nbBanks;
    char  freqOut;
    // char  deltafloorflag;
    char  xCoherence;
    char  DcRemoveFlag;
    char  gainFlag;

    char max_volume_detect;

    int   mute_time_ms;
    int   frameSize;
    int   fs;
    int   sil2spe;
    int   spe2sil;
    aisp_s16_t miu;
    aisp_s16_t lembda;
    aisp_s16_t deltamax;
    aisp_s16_t Mu;
    aisp_s16_t Noise_floor;
    aisp_s16_t amth;
    aisp_s16_t upStep;
    aisp_s16_t downStep;
    aisp_s16_t lembda_mu;
    aisp_s16_t lamda_x;
    aisp_s16_t scaft;
    aisp_s16_t sgmS2thr;
    aisp_s16_t outGain;
    aisp_s16_t preemph;
    aisp_s16_t lamda_xPower;

};

struct tagAec_coreConfReset
{
    char vad;
    char wavChan;
    char micNum;
    char refNum;
    char hdr;
    char nlp;
    char emd;
    char freqOut;
    char DcRemoveFlag;
    char gainFlag;
    aisp_s16_t outGain;
};

struct tagAec
{
    //determine whether an instance need to free memory for ref spectrum;
    char idx;
    char specnt;
    char silcnt;
    char vadResult;
    char  maxVolumeState;//max volume detect
    short clip;
    short confirmUpdate;
    int maxVal;
    // int deltafloorSwitch;
    unsigned int fftBins;
    unsigned int mute_Frames;
    unsigned int memoryPoolSize;
    short resetCount;

    aisp_s32_t MinDelta;
    aisp_s32_t MaxDelta;
    aisp_s16_t fftScalar;
    aisp_s16_t MIUQUOTE;
    aisp_s16_t memD[2];
    aisp_s16_t memDold[2];
    aisp_s16_t memX;
    aisp_s16_t memXold[4];
    aisp_s16_t memE;
    aisp_s16_t memEold[4];
    aisp_s32_t maxabsRef;
    // aisp_s32_t see;
    // aisp_s32_t syy;
    aisp_s64_t meanR11;
    aisp_s64_t meanR22;
    aisp_s64_t meanR33;
    aisp_s64_t meanDiag;
    aisp_fft_eng_t *fftEngine;
    void   *memoryPool;
    aisp_s16_t *hanningWindowSqrt;
    aisp_s16_t *aDeltaFloor;
    aisp_s32_t *aDelta;
    aisp_s32_t *aERRPower;

    aisp_s32_t *aXPower[AEC_TAPS];
    aisp_s16_t *aMicFrameOld;
    aisp_s16_t *aMicFrameWide;
    aisp_s16_t *aRefFrameOld[2];
    aisp_s16_t *aRefFrameWide[2];
    aisp_s32_t *aXPowerSum;
    aisp_s32_t *aXPowerSumSmooth;
    aisp_s16_t *aEstFrameWide;
    aisp_s16_t *aExtdw;
    aisp_s16_t *aErrPreOld;
    aisp_s16_t *aEstOld;
    aisp_s16_t *aErrFrameWide;
    aisp_s16_t *aErrOld;

    naes_core_T *pstNAes;

    aisp_cpx_s32_t *aW[AEC_TAPS];
    aisp_cpx_s32_t *aX[AEC_TAPS];
    aisp_cpx_s32_t *pX0;
    aisp_cpx_s32_t *pX1;
    aisp_cpx_s32_t *pX2;
    aisp_cpx_s32_t *puX_REF;

    aisp_cpx_s32_t *aMIC;
    // aisp_cpx_s32_t * aERRPre;
    aisp_cpx_s32_t *aERR;
    aisp_cpx_s32_t *aEST;

    /* EMD self-correlation */
    aisp_s32_t *aR11;
    aisp_s32_t *aR22;
    aisp_cpx_s32_t *aR12;

    unsigned char *aR_Qbase;

    aisp_cpx_s32_t *aR13;
    aisp_cpx_s32_t *aR23;
    aisp_s32_t      *aIR11;
    aisp_cpx_s32_t *aIR12;
    aisp_s32_t      *aIR22;

    /*
     * read only pointer used after rawprocess
     * it might point to the local data or
     * point to data of other aec instance that
     * compute the reference data.
     */
    const aisp_s32_t *pIR11;
    const aisp_cpx_s32_t *pIR12;
    const aisp_s32_t *pIR22;

    aisp_cpx_s32_t *aExtdW0;

    /* points to config */
    aec_coreConf_T *config;
};

/************************************************************
  Function   : naes_feed()

  Description: data feed to naes
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2016/08/06, ziyuan.Jiang create

************************************************************/
void naes_feed(aec_core_T *pAEC_State);



/************************************************************
  Function   : naes_reset()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2016/08/06, ziyuan.Jiang create

************************************************************/
void naes_reset(naes_core_T *pstNAes);



#endif
