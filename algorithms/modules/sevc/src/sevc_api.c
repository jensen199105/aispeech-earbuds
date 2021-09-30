/************************************************************
  Copyright (C), AISpeechTech. Co., Ltd.

  FileName   : sevc_api.c
  Project    :
  Module     :
  Version    :
  Date       : 2019/12/02
  Author     :
  Document   :
  Description:

  Function List :

  History :
  <author>          <time>            <version >        <desc>
  shizhang.tang     2019/12/02         v.00            Create

************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sevc_config.h"
#include "AISP_TSL_types.h"
#include "AISP_TSL_str.h"
#include "sevc_window.h"
#include "AISP_TSL_fft.h"
#include "sevc_version.h"
#include "sevc_config.h"
#include "sevc_api.h"
#include "sevc_types.h"
#include "sevc_func.h"
#include "sevc_util.h"
#include "sevc_version.h"
#include "AISP_TSL_base.h"
#include "fft_wrapper.h"

#ifndef SEVC_VISIBLE_ATTR
#define SEVC_VISIBLE_ATTR
#endif
/************************************************************
  Function   : sevcParamsShow()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/12/02, shizhang.tang create
************************************************************/
VOID sevcParamsShow(SEVC_CONFIG_S *pstSevcCfg)
{
#if 0
    MSG_DBG("----------------------------\n");
    MSG_DBG("SWITCH ----------------\n");
    MSG_DBG("WN_flag                : %d\n", pstSevcCfg->WN_flag);
    MSG_DBG("AEC_flag               : %d\n", pstSevcCfg->AEC_flag);
    MSG_DBG("BF_flag                : %d\n", pstSevcCfg->BF_flag);
    MSG_DBG("GSC_flag               : %d\n", pstSevcCfg->GSC_flag);
    MSG_DBG("BF_POST_flag           : %d\n", pstSevcCfg->BF_POST_flag);
    MSG_DBG("AUX_flag               : %d\n", pstSevcCfg->AUX_flag);
    MSG_DBG("NR_flag                : %d\n", pstSevcCfg->NR_flag);
    MSG_DBG("NN_flag                : %d\n", pstSevcCfg->NN_flag);
    MSG_DBG("CNG_flag               : %d\n", pstSevcCfg->CNG_flag);
    MSG_DBG("AES_flag               : %d\n", pstSevcCfg->AES_flag);
    MSG_DBG("EQ_flag                : %d\n", pstSevcCfg->EQ_flag);
    MSG_DBG("AGC_flag               : %d\n", pstSevcCfg->AGC_flag);
    MSG_DBG("NN_AES_flag            : %d\n", pstSevcCfg->NN_AES_flag);
    MSG_DBG("ucAecEmd               : %d\n", pstSevcCfg->ucAecEmd);
    MSG_DBG("ucBfSprFlag            : %d\n", pstSevcCfg->ucBfSprFlag);
    MSG_DBG("ucNnBarkSupport        : %d\n", pstSevcCfg->ucNnBarkSupport);
    //MSG_DBG("use_smooth_cmn_flag    : %d\n", pstSevcCfg->use_smooth_cmn_flag);
    MSG_DBG("Wav -------------------\n");
    MSG_DBG("wavChan                : %d\n", pstSevcCfg->wavChan);
    MSG_DBG("micChan                : %d\n", pstSevcCfg->micChan);
    MSG_DBG("refChan                : %d\n", pstSevcCfg->refChan);
    MSG_DBG("auxChanOffset          : %d\n", pstSevcCfg->auxChanOffset);
    MSG_DBG("hdr                    : %d\n", pstSevcCfg->hdr);
    MSG_DBG("fs                     : %d\n", pstSevcCfg->fs);
    MSG_DBG("Cfg -------------------\n");
    MSG_DBG("frameShift             : %d\n", pstSevcCfg->frameShift);
    MSG_DBG("micGain                : %d\n", pstSevcCfg->micGain);
    MSG_DBG("aecGain                : %d\n", pstSevcCfg->aecGain);
    MSG_DBG("outGain                : %d\n", pstSevcCfg->outGain);
    MSG_DBG("usFrameSize            : %d\n", pstSevcCfg->usFrameSize);
    MSG_DBG("usFftBin               : %d\n", pstSevcCfg->usFftBin);
    //wn
#if SEVC_SWITCH_WN
    MSG_DBG("WN --------------------\n");
    MSG_DBG("uiWnSil2Spe            : %d\n", pstSevcCfg->uiWnSil2Spe);
    MSG_DBG("uiWnSpe2Sil            : %d\n", pstSevcCfg->uiWnSpe2Sil);
    MSG_DBG("fWnPowerThresh1        : %d\n", pstSevcCfg->fWnPowerThresh1);
    MSG_DBG("fWnPowerThresh2        : %d\n", pstSevcCfg->fWnPowerThresh2);
    MSG_DBG("fWnPowerThresh3        : %d\n", pstSevcCfg->fWnPowerThresh3);
    MSG_DBG("fRefThresh             : %d\n", pstSevcCfg->fRefThresh);
    MSG_DBG("fWnRateThresh1         : %d\n", pstSevcCfg->fWnRateThresh1);
    MSG_DBG("fWnRateThresh2         : %d\n", pstSevcCfg->fWnRateThresh2);
    MSG_DBG("fWnRateThresh3         : %d\n", pstSevcCfg->fWnRateThresh3);
    MSG_DBG("fWnMicSmoothAlpha      : %d\n", pstSevcCfg->fWnMicSmoothAlpha);
    MSG_DBG("fWnRateSmoothAlpha     : %d\n", pstSevcCfg->fWnRateSmoothAlpha);
#endif
#if SEVC_SWITCH_AEC
    //aec
    MSG_DBG("AEC -------------------\n");
    MSG_DBG("usAecTaps              : %d\n", pstSevcCfg->usAecTaps);
    MSG_DBG("fAecPreemph            : %d\n", pstSevcCfg->fAecPreemph);
    MSG_DBG("fAecMu                 : %d\n", pstSevcCfg->fAecMu);
    MSG_DBG("fAecLamda              : %d\n", pstSevcCfg->fAecLamda);
    MSG_DBG("fAecLamdaUp            : %d\n", pstSevcCfg->fAecLamdaUp);
    MSG_DBG("fAecLamdaDown          : %d\n", pstSevcCfg->fAecLamdaDown);
    MSG_DBG("fAecDelta              : %d\n", pstSevcCfg->fAecDelta);
    MSG_DBG("fAecDeltaFloorMin      : %d\n", pstSevcCfg->fAecDeltaFloorMin);
#endif
#if SEVC_SWITCH_BF
    //bf
    MSG_DBG("BF --------------------\n");
    MSG_DBG("ucBfBeamCount          : %d\n", pstSevcCfg->ucBfBeamCount);
    MSG_DBG("uiBfTaps               : %d\n", pstSevcCfg->uiBfTaps);
    MSG_DBG("fBfGainFloor           : %d\n", pstSevcCfg->fBfGainFloor);
    MSG_DBG("fBfDelta               : %d\n", pstSevcCfg->fBfDelta);
    MSG_DBG("fBfRatioTrig1          : %d\n", pstSevcCfg->fBfRatioTrig1);
    MSG_DBG("fBfRatioTrig2          : %d\n", pstSevcCfg->fBfRatioTrig2);
    MSG_DBG("fBfRatioTrig3          : %d\n", pstSevcCfg->fBfRatioTrig3);
    MSG_DBG("fBfMu                  : %d\n", pstSevcCfg->fBfMu);
#endif
#if SEVC_SWITCH_GSC
    //gsc
    MSG_DBG("GSC -------------------\n");
    MSG_DBG("usGscTaps              : %d\n", pstSevcCfg->usGscTaps);
    MSG_DBG("usGscMu                : %d\n", pstSevcCfg->usGscMu);
    MSG_DBG("fGscDeltaGain          : %d\n", pstSevcCfg->fGscDeltaGain);
    MSG_DBG("fGscDeltaMin           : %d\n", pstSevcCfg->fGscDeltaMin);
    MSG_DBG("fGscDeltaMax           : %d\n", pstSevcCfg->fGscDeltaMax);
#endif
#if SEVC_SWITCH_AUX
    //aux
    MSG_DBG("AUX -------------------\n");
    MSG_DBG("uiAuxBin1              : %d\n", pstSevcCfg->uiAuxBin1);
    MSG_DBG("uiAuxBin2              : %d\n", pstSevcCfg->uiAuxBin2);
#endif
#if SEVC_SWITCH_NR
    //nr
    MSG_DBG("fNrMcraAd              : %d\n", pstSevcCfg->fNrMcraAd);
    MSG_DBG("fNrGainTrig            : %d\n", pstSevcCfg->fNrGainTrig);
    MSG_DBG("fNrGainFloor           : %d\n", pstSevcCfg->fNrGainFloor);
#endif
#if SEVC_SWITCH_NN
    //nn
    MSG_DBG("NN -------------------\n");
    MSG_DBG("ucNnFeatureDim         : %d\n", pstSevcCfg->ucNnFeatureDim);
    //MSG_DBG("cmn_win_len            : %d\r\n", pstSevcCfg->cmn_win_len);
    //MSG_DBG("nnVadGruSize         : %d\n",pstSevcCfg->nnVadGruSize);
    //MSG_DBG("nnNoiseGruSize       : %d\n",pstSevcCfg->nnNoiseGruSize);
    //MSG_DBG("nnDenoiseGruSize     : %d\n",pstSevcCfg->nnDenoiseGruSize);
    //MSG_DBG("feature_context_L      : %d\n", pstSevcCfg->feature_context_L);
    //MSG_DBG("feature_context_R      : %d\n", pstSevcCfg->feature_context_R);
    //MSG_DBG("nnRnnModelFile       : %s\n",pstSevcCfg->nnRnnModelFile);
    MSG_DBG("ucNnBandEnergySmooth   : %d\n", pstSevcCfg->ucNnBandEnergySmooth);
    MSG_DBG("fNnGainFloor           : %llu\n", pstSevcCfg->fNnGainFloor);
    MSG_DBG("fNnWindGainFloor       : %llu\n", pstSevcCfg->fNnWindGainFloor);
    MSG_DBG("fAesGainThresh         : %d\n", pstSevcCfg->fAesGainThresh);
#if SEVC_SWITCH_NN_AES
    MSG_DBG("NN_AES ----------------\n");
    MSG_DBG("usAesRefBinStart       : %d\n", pstSevcCfg->usAesRefBinStart);
    MSG_DBG("usAesRefBinCount       : %d\n", pstSevcCfg->usAesRefBinCount);
#endif
#endif
#if SEVC_SWITCH_AES
    MSG_DBG("AES -------------------\n");
    MSG_DBG("lamda_x                : %d\n", pstSevcCfg->lamda_x);
    MSG_DBG("scaft                  : %d\n", pstSevcCfg->scaft);
    MSG_DBG("gamma                  : %d\n", pstSevcCfg->gamma);
    MSG_DBG("gainfloor              : %d\n", pstSevcCfg->gainfloor);
    MSG_DBG("noise_floor            : %d\n", pstSevcCfg->noise_floor);
    MSG_DBG("usAesRefBinStart       : %d\n", pstSevcCfg->usAesRefBinStart);
    MSG_DBG("usAesRefBinCount       : %d\n", pstSevcCfg->usAesRefBinCount);
#endif
#if SEVC_SWITCH_AGC
    //agc
    MSG_DBG("AGC -------------------\n");
    //MSG_DBG("vad_trig               : %d\n", pstSevcCfg->vad_trig);
    //MSG_DBG("mcra_L                 : %d\n", pstSevcCfg->mcra_L);
    MSG_DBG("mcra_as                : %d\n", pstSevcCfg->mcra_as);
    MSG_DBG("mcra_as1               : %d\n", pstSevcCfg->mcra_as1);
    MSG_DBG("mcra_as2               : %d\n", pstSevcCfg->mcra_as2);
    MSG_DBG("G_low                  : %d\n", pstSevcCfg->G_low);
    MSG_DBG("ratio1                 : %d\n", pstSevcCfg->ratio1);
    MSG_DBG("ratio2                 : %d\n", pstSevcCfg->ratio2);
    MSG_DBG("T1                     : %d\n", pstSevcCfg->T1);
    MSG_DBG("T2                     : %d\n", pstSevcCfg->T2);
    MSG_DBG("T2_gain                : %d\n", pstSevcCfg->T2_gain);
    MSG_DBG("vad_smooth_short_len   : %d\n", pstSevcCfg->vad_smooth_short_len);
    MSG_DBG("vad_smooth_long_len    : %d\n", pstSevcCfg->vad_smooth_long_len);
    MSG_DBG("vad_smooth_short_thr   : %d\n", pstSevcCfg->vad_smooth_short_thr);
    MSG_DBG("vad_smooth_long_thr    : %d\n", pstSevcCfg->vad_smooth_long_thr);
    MSG_DBG("vad_thresh             : %d\n", pstSevcCfg->vad_thresh);
    MSG_DBG("agcAlphaABeginVAD      : %d\n", pstSevcCfg->agcAlphaABeginVAD);
    MSG_DBG("agcAlphaAInVAD         : %d\n", pstSevcCfg->agcAlphaAInVAD);
    MSG_DBG("agcAlphaRInVAD         : %d\n", pstSevcCfg->agcAlphaRInVAD);
    MSG_DBG("agcAlphaRSil           : %d\n", pstSevcCfg->agcAlphaRSil);
    //MSG_DBG("acResourseFile       : %s\n",pstSevcCfg->acResourseFile);
#endif
    MSG_DBG("----------------------------\r\n\r\n");
#endif
}

VOID sevcDataEncode(U8 *pucData, U32 uiLen)
{
    U8 aucCode[4] = {0xAA, 0x55, 0xC0, 0x0C};
    U32 ii;

    for (ii = 0; ii < uiLen; ii++)
    {
        pucData[ii] = pucData[ii] ^ aucCode[ii % 4];
    }
}

/*
W32Q24:
  0.05: 838861
  0.08: 1342177
   0.1: 1677722
  0.15: 2516582
   0.2: 3355443
   0.9: 15602811
  0.95: 15938355
  0.98: 16441672
     2: 33554432
     9: 150994944
    10: 167772160
    25: 419430400
    35: 587202560
    50: 838860800
    55: 922746880
    80: 1342177280
    90: 1509949440
   127: 2130706432
*/

/************************************************************
  Function   : sevcDefaultCfgInit()

  Description: default config init
  Calls      :
  Called By  :
  Input      : sevc_api   struct pointer
  Output     :
  Return     :
  Others     :

  History    :
    2019/12/02, shizhang.tang create
************************************************************/
VOID sevcDefaultCfgInit(SEVC_CONFIG_S *pstSevcCfg)
{
    /*switch*/
    pstSevcCfg->WN_flag                  = SEVC_SWITCH_WN;
    pstSevcCfg->AEC_flag                 = SEVC_SWITCH_AEC;
    pstSevcCfg->BF_flag                  = SEVC_SWITCH_BF;
    pstSevcCfg->GSC_flag                 = SEVC_SWITCH_GSC;
    pstSevcCfg->BF_POST_flag             = SEVC_SWITCH_BF_POST;
    pstSevcCfg->AUX_flag                 = SEVC_SWITCH_AUX;
    pstSevcCfg->NR_flag                  = SEVC_SWITCH_NR;
    pstSevcCfg->NN_flag                  = SEVC_SWITCH_NN;
    pstSevcCfg->CNG_flag                 = SEVC_SWITCH_CNG;
    pstSevcCfg->AES_flag                 = SEVC_SWITCH_AES;
    pstSevcCfg->EQ_flag                  = SEVC_SWITCH_EQ;
    pstSevcCfg->AGC_flag                 = SEVC_SWITCH_AGC;
    pstSevcCfg->NN_AES_flag              = SEVC_SWITCH_NN_AES;
    pstSevcCfg->ucAecEmd                 = SEVC_SWITCH_AEC_EMD;
    pstSevcCfg->ucAecPreemphSupport      = SEVC_SWITCH_AEC_PREEMPH;
    pstSevcCfg->ucBfSprFlag              = SEVC_SWITCH_BF_SPR;
    pstSevcCfg->ucNnBarkSupport          = SEVC_SWITCH_NN_BARK;
    //pstSevcCfg->use_smooth_cmn_flag      = SEVC_SWITCH_NN_CMN;
    /*parameter*/
    pstSevcCfg->wavChan                  = SEVC_WAV_CHAN;
    pstSevcCfg->micChan                  = SEVC_MIC_CHAN;
    pstSevcCfg->refChan                  = SEVC_REF_CHAN;
#if SEVC_SWITCH_AUX
    pstSevcCfg->auxChanOffset            = SEVC_AUX_CHAN_OFFSET;
#endif
    //pstSevcCfg->cmn_win_len              = SEVC_NN_CMN_WIN_LEN;
    /*config*/
    pstSevcCfg->hdr                      = SEVC_HDR;
    pstSevcCfg->fs                       = SEVC_FS;
    pstSevcCfg->frameShift               = SEVC_FRAME_SHIFT;
    pstSevcCfg->micGain                  = SEVC_MIC_GAIN;
    pstSevcCfg->refGain                  = SEVC_REF_GAIN;
    pstSevcCfg->aecGain                  = SEVC_AEC_GAIN;
    pstSevcCfg->outGain                  = SEVC_OUT_GAIN;
    pstSevcCfg->postGain                 = SEVC_POST_GAIN;
    //wn
    pstSevcCfg->uiWnSil2Spe              = SEVC_WN_SIL2SPE;
    pstSevcCfg->uiWnSpe2Sil              = SEVC_WN_SPE2SIL;
    pstSevcCfg->fWnPowerThresh1          = SEVC_WN_POWER_THRESH1;
    pstSevcCfg->fWnPowerThresh2          = SEVC_WN_POWER_THRESH2;
    pstSevcCfg->fWnPowerThresh3          = SEVC_WN_POWER_THRESH3;
    pstSevcCfg->fRefThresh               = SEVC_WN_REF_POWER_THRESH;
    pstSevcCfg->fWnRateThresh1           = SEVC_WN_RATE_THRESH1;
    pstSevcCfg->fWnRateThresh2           = SEVC_WN_RATE_THRESH2;
    pstSevcCfg->fWnRateThresh3           = SEVC_WN_RATE_THRESH3;
    pstSevcCfg->fWnMicSmoothAlpha        = SEVC_WN_MIC_SMOOTH_ALPHA;
    pstSevcCfg->fWnRateSmoothAlpha       = SEVC_WN_RATE_SMOOTH_ALPHA;
    //aec
    pstSevcCfg->usAecTaps                = SEVC_AEC_TAPS;
    pstSevcCfg->fAecPreemph              = SEVC_AEC_PREEMPH;
    pstSevcCfg->fAecLamda                = SEVC_AEC_LAMDA;
    pstSevcCfg->fAecLamdaUp              = SEVC_AEC_LAMDA_UP;
    pstSevcCfg->fAecLamdaDown            = SEVC_AEC_LAMDA_DOWN;
#if SEVC_SWITCH_AEC_EMD
    pstSevcCfg->fAecMu                   = 262144 - SEVC_AEC_LAMDA; //W32Q18
#else
    pstSevcCfg->fAecMu                   = SEVC_AEC_MU;
#endif
    pstSevcCfg->fAecDelta                = SEVC_AEC_DALTA;
    pstSevcCfg->fAecDeltaFloorMin        = SEVC_AEC_DELTA_FLOOR_MIN;
    //bf
    pstSevcCfg->ucBfBeamCount            = SEVC_BF_BEAM_COUNT;
    pstSevcCfg->uiDistanceMic            = SEVC_BF_MIC_DISTANCE;
    pstSevcCfg->uiBfTaps                 = SEVC_BF_TAPS;
    pstSevcCfg->fBfGainFloor             = SEVC_BF_GAIN_FLOOR;
    pstSevcCfg->fBfDelta                 = SEVC_BF_DELTA;
    pstSevcCfg->fBfRatioTrig1            = SEVC_BF_RATIO_TRIG1;
    pstSevcCfg->fBfRatioTrig2            = SEVC_BF_RATIO_TRIG2;
    pstSevcCfg->fBfRatioTrig3            = SEVC_BF_RATIO_TRIG3;
    pstSevcCfg->fBfMu                    = SEVC_BF_MU;
    //gsc
    pstSevcCfg->usGscTaps                = SEVC_GSC_TAPS;
    pstSevcCfg->usGscMu                  = SEVC_GSC_MU;
    pstSevcCfg->fMuOffset                = PDIV(SEVC_GSC_MU, 20); // 0.05
    pstSevcCfg->fGscDeltaGain            = SEVC_GSC_DELTA_GAIN;
    pstSevcCfg->fGscDeltaMin             = SEVC_GSC_DELTA_MIN;
    pstSevcCfg->fGscDeltaMax             = SEVC_GSC_DELTA_MAX;
    // bfpost
    pstSevcCfg->fMcraAs                  = SEVC_BFPOST_MCRA_AS;
    pstSevcCfg->fMcraAp                  = SEVC_BFPOST_MCRA_AP;
    pstSevcCfg->usMcraDelta              = SEVC_BFPOST_MCRA_DELTA;
    pstSevcCfg->ucU                      = 4;
    pstSevcCfg->ucV                      = 15;
    pstSevcCfg->fBfpostMcraAd            = SEVC_BFPOST_MCRA_AD;
    pstSevcCfg->fBfpostAlpha             = SEVC_BFPOST_ALPHA;
    pstSevcCfg->fPriorMin                = SEVC_BFPOST_PRIOR_MIN;
    pstSevcCfg->fGainFloor               = SEVC_BFPOST_GAIN_FLOOR;
    pstSevcCfg->fRatioYuZhi              = SEVC_BFPOST_RATIO_YUZHI;
    pstSevcCfg->usK0                     = 8;
    pstSevcCfg->usK1                     = 63;
    pstSevcCfg->fPesai0                  = SEVC_BFPOST_PESAI_ZERO;
    pstSevcCfg->fBeta                    = SEVC_BFPOST_BETA;
    pstSevcCfg->fOmegaLow                = SEVC_BFPOST_OMEGA_LOW;
    pstSevcCfg->fOmegaHigh               = SEVC_BFPOST_OMEGA_HIGH;
    //aux
    pstSevcCfg->uiAuxBin1                = SEVC_AUX_BIN1;
    pstSevcCfg->uiAuxBin2                = SEVC_AUX_BIN2;
    pstSevcCfg->fAlphaGain               = SEVC_AUX_ALPHAGAIN;
    //nr
    pstSevcCfg->fNrMcraAd                = SEVC_NR_MCRA_AD;
    pstSevcCfg->fNrGainTrig              = SEVC_NR_GAIN_TRIG;
    pstSevcCfg->fNrGainFloor             = SEVC_NR_GAIN_FLOOR;
    //nn
    pstSevcCfg->ucNnBandEnergySmooth     = 0;
    pstSevcCfg->usFeatureContextR        = 0;
    pstSevcCfg->fNnGainFloor             = SEVC_NN_GAIN_FLOOR;
    pstSevcCfg->fNnWindGainFloor         = SEVC_NN_WIND_GAIN_FLOOR;
    //pstSevcCfg->nnRnnModelFile;
    // aes
    pstSevcCfg->fGamma                   = SEVC_AES_GAMMA;
    pstSevcCfg->alpha                    = SEVC_AES_ALPHA;
    pstSevcCfg->alphaMax                 = SEVC_AES_ALPHAMAX;
    pstSevcCfg->alphaMin                 = SEVC_AES_ALPHAMIN;
    pstSevcCfg->fAesGainFloor            = SEVC_AES_GAINFLOOR;
    // vad
    pstSevcCfg->fAmth                    = SEVC_VAD_AMTH;
    pstSevcCfg->sil2spe                  = SEVC_VAD_SILSPE;
    pstSevcCfg->spe2sil                  = SEVC_VAD_SPESIL;
    // sd
    pstSevcCfg->fSdAlpha                 = SEVC_SD_ALPHA;
    pstSevcCfg->usBinStart               = SEVC_SD_BINSTART;
    pstSevcCfg->usBinEnd                 = SEVC_SD_BINEND;
    pstSevcCfg->fWindTrig                = SEVC_SD_WINDTRIG;
    pstSevcCfg->fWearTrig                = SEVC_SD_WEARTRIG;
    pstSevcCfg->fWindSmooth              = SEVC_SD_WINDSMOOTH;
    pstSevcCfg->wind2quit                = SEVC_SD_WINDQUIT;
    pstSevcCfg->quit2wind                = SEVC_SD_QUITWIND;
    pstSevcCfg->tight2loose              = SEVC_SD_TIGHTLOOSE;
    pstSevcCfg->loose2tight              = SEVC_SD_LOOSETIGHT;
    pstSevcCfg->fAlphaWearLevelUp        = SEVC_SD_ALPHAWEARLEVELUP;
    pstSevcCfg->fAlphaWearLevelDown      = SEVC_SD_ALPHAWEARLEVELDOWN;
    pstSevcCfg->usWindSwitch             = SEVC_SD_WINDSWITCH;
    pstSevcCfg->usWearSwitch             = SEVC_SD_WEARSWITCH;
    //cng
    //pstSevcCfg->vad_trig                 = SEVC_CNG_VAD_TRIG;
    //pstSevcCfg->mcra_L                   = SEVC_CNG_MCRA_L;
    pstSevcCfg->mcra_as                  = SEVC_CNG_MCRA_AS;
    pstSevcCfg->mcra_as1                 = SEVC_CNG_MCRA_AS1;
    pstSevcCfg->mcra_as2                 = SEVC_CNG_MCRA_AS2;
    //agc
    //DRC_Gain_calc_parameters
    pstSevcCfg->vad_smooth_short_len     = SEVC_AGC_VAD_SMOOTH_SHORT_LEN;
    pstSevcCfg->vad_smooth_long_len      = SEVC_AGC_VAD_SMOOTH_LONG_LEN;
    pstSevcCfg->G_low                    = SEVC_AGC_GAIN_LOW;
    pstSevcCfg->ratio1                   = SEVC_AGC_RATIO1;
    pstSevcCfg->ratio2                   = SEVC_AGC_RATIO2;
    pstSevcCfg->T1                       = SEVC_AGC_T1;
    pstSevcCfg->T2                       = SEVC_AGC_T2;
    pstSevcCfg->T2_gain                  = SEVC_AGC_T2_GAIN;
    pstSevcCfg->vad_smooth_short_thr     = SEVC_AGC_VAD_SMOOTH_SHORT_THR;
    pstSevcCfg->vad_smooth_long_thr      = SEVC_AGC_VAD_SMOOTH_LONG_THR;
    pstSevcCfg->vad_thresh               = SEVC_AGC_VAD_THRESH;
    pstSevcCfg->gainThreshold            = SEVC_AGC_GAINTHR;
    //Gain_smooth_parameters
    pstSevcCfg->agcAlphaABeginVAD        = SEVC_AGC_ALPHA_ATTACK_BEGIN_VAD;
    pstSevcCfg->agcAlphaAInVAD           = SEVC_AGC_ALPHA_ATTACK_IN_VAD;
    pstSevcCfg->agcAlphaRInVAD           = SEVC_AGC_ALPHA_RELEASE_IN_VAD;
    pstSevcCfg->agcAlphaRSil             = SEVC_AGC_ALPHA_RELEASE_SIL; //W32Q24 fs=16000 frameShift=160 agcAlphaRSil=3
    AISP_TSL_memset(pstSevcCfg->scNnRnnModelFile, 0, sizeof(pstSevcCfg->scNnRnnModelFile));
    AISP_TSL_memset(pstSevcCfg->acResourseFile, 0, sizeof(pstSevcCfg->acResourseFile));
    pstSevcCfg->usFftFrameSize = pstSevcCfg->fs == 16000 ? 512 : 256;
    pstSevcCfg->usFrameSize = pstSevcCfg->usFftFrameSize;
    pstSevcCfg->usFftBin = (pstSevcCfg->usFftFrameSize >> 1) + 1;
}

VOID sevcCfgReinit(SEVC_CONFIG_S *pstSevcCfg)
{
    pstSevcCfg->AEC_flag = (pstSevcCfg->refChan ? pstSevcCfg->AEC_flag : 0);
    pstSevcCfg->AES_flag = (pstSevcCfg->AEC_flag ? pstSevcCfg->AES_flag : 0);
    pstSevcCfg->ucAecEmd = (pstSevcCfg->AEC_flag ? pstSevcCfg->ucAecEmd : 0);
    pstSevcCfg->ucAecPreemphSupport = (pstSevcCfg->AEC_flag ? pstSevcCfg->ucAecPreemphSupport : 0);
    pstSevcCfg->ucNnBarkSupport = (pstSevcCfg->NN_flag ? pstSevcCfg->ucNnBarkSupport : 0);
    // pstSevcCfg->usFftFrameSize = sevcFftSizeConvert((pstSevcCfg->frameShift << 1));
    pstSevcCfg->usFftFrameSize = pstSevcCfg->fs == 16000 ? 512 : 256;
    pstSevcCfg->usFrameSize = pstSevcCfg->usFftFrameSize;
    pstSevcCfg->usFftBin = (pstSevcCfg->usFftFrameSize >> 1) + 1;
}

/************************************************************
  Function   : SEVC_API_ParamsShow()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/12/04, shizhang.tang create
************************************************************/
SEVC_VISIBLE_ATTR VOID SEVC_API_ParamsShow(VOID *pstSevcApi)
{
    //SEVC_API_S *pstSevcEng = (SEVC_API_S *)pstSevcApi;
    if (NULL == pstSevcApi)
    {
        return;
    }

    //sevcParamsShow(pstSevcEng->pstSevcCfg);
}

/************************************************************
  Function   : SEVC_API_FrameSize()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/12/04, shizhang.tang create
************************************************************/
SEVC_VISIBLE_ATTR U32 SEVC_API_FrameSize(VOID *pstSevcApi)
{
    SEVC_API_S *pstSevcEng = (SEVC_API_S *)pstSevcApi;

    if (NULL == pstSevcApi)
    {
        return 0;
    }

    return pstSevcEng->pstSevcCfg->frameShift;
}

/************************************************************
  Function   : SEVC_API_Version()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/12/04, shizhang.tang create
************************************************************/
SEVC_VISIBLE_ATTR S8 *SEVC_API_Version(VOID)
{
    return SEVC_VERSION;
}
#if 0
SEVC_VISIBLE_ATTR VOID SEVC_API_InfoShow(VOID)
{
    S8 *pResourceInfo = sevcResourceInfoGet();
}
#endif
/************************************************************
  Function   : SEVC_API_MemSize()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/12/04, shizhang.tang create
************************************************************/
SEVC_VISIBLE_ATTR S16 SEVC_API_MemSize(VOID *pstSevcApi)
{
    SEVC_API_S *pstSevcEng = (SEVC_API_S *)pstSevcApi;

    if (NULL == pstSevcApi)
    {
        return 0;
    }

    return pstSevcEng->ucMemBlockSize;
}

/************************************************************
  Function   : SEVC_API_MicNum()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/12/04, shizhang.tang create
************************************************************/
SEVC_VISIBLE_ATTR S16 SEVC_API_MicNum(VOID *pstSevcApi)
{
    SEVC_API_S *pstSevcEng = (SEVC_API_S *)pstSevcApi;

    if (NULL == pstSevcApi)
    {
        return 0;
    }

    return pstSevcEng->pstSevcCfg->micChan;
}

/************************************************************
  Function   : SEVC_API_WavChan()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/12/04, shizhang.tang create
************************************************************/
SEVC_VISIBLE_ATTR S16 SEVC_API_WavChan(VOID *pstSevcApi)
{
    SEVC_API_S *pstSevcEng = (SEVC_API_S *)pstSevcApi;

    if (NULL == pstSevcApi)
    {
        return 0;
    }

    return pstSevcEng->pstSevcCfg->wavChan;
}

/************************************************************
  Function   : SEVC_API_Hdr()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/12/04, shizhang.tang create
************************************************************/
SEVC_VISIBLE_ATTR S16 SEVC_API_Hdr(VOID *pstSevcApi)
{
    SEVC_API_S *pstSevcEng = (SEVC_API_S *)pstSevcApi;

    if (NULL == pstSevcApi)
    {
        return 0;
    }

    return pstSevcEng->pstSevcCfg->hdr;
}

/************************************************************
  Function   : sevcApiLocMemSizeGet()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/12/04, shizhang.tang create
************************************************************/
U32 sevcApiLocMemSizeGet(SEVC_CONFIG_S *pstSevcCfg)
{
    U32 uiSize = 0;
    U32 uiTotalMemSize = 0;
    U16 usFrmShft      = 0;
    U16 usMicChan      = 0;
    U16 usRefNum       = 0;
    usFrmShft = pstSevcCfg->frameShift;
    usMicChan = pstSevcCfg->micChan;
    usRefNum  = pstSevcCfg->refChan;
    // sevc_memLocator engine
    uiTotalMemSize += SIZE_ALIGN(sizeof(MEM_LOCATOR_S));
    // sevc config pointer
    uiTotalMemSize += SIZE_ALIGN(sizeof(SEVC_CONFIG_S));
    // sevc_api engine
    uiTotalMemSize += SIZE_ALIGN(sizeof(SEVC_API_S));
    //pfFrameBuffer
    uiSize = (usMicChan + usRefNum) * usFrmShft * sizeof(aisp_s16_t);
    uiTotalMemSize += SIZE_ALIGN(uiSize);
    uiTotalMemSize += SEVC_LocMemSizeGet(pstSevcCfg);
    uiTotalMemSize += ALIGN_SIZE;
#if MEM_BUFF_DBG
    MSG_DBG("%s uiTotalMemSize=%d\r\n", __func__, uiTotalMemSize);
#endif
    return uiTotalMemSize;
}
#if 0
/************************************************************
  Function   : SEVC_API_LocMemSizeGet()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/12/04, shizhang.tang create
************************************************************/
SEVC_VISIBLE_ATTR U32 SEVC_API_LocMemSizeGet(const S8 *pcResouce)
{
    SEVC_CONFIG_S sevcConfig;
    SEVC_CONFIG_S *pstSevcCfg;
#ifndef AISPEECH_FIXED
    S32 iRet = -1;
#endif
    pstSevcCfg = &sevcConfig;
    sevcDefaultCfgInit(pstSevcCfg);
    sevcCfgReinit(pstSevcCfg);
    return sevcApiLocMemSizeGet(pstSevcCfg);
}
#endif
U32 sevcApiShareMemSizeGet(SEVC_CONFIG_S *pstSevcCfg)
{
#ifdef USE_SHARE_MEM
    return SEVC_ShMemSizeGet(pstSevcCfg);
#else
    return 0;
#endif
}
#if 0
/************************************************************
  Function   : SEVC_API_ShareMemSizeGet()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/12/04, shizhang.tang create
************************************************************/
SEVC_VISIBLE_ATTR U32 SEVC_API_ShareMemSizeGet(const S8 *pcResouce)
{
    SEVC_CONFIG_S sevcConfig;
    SEVC_CONFIG_S *pstSevcCfg;
#ifndef AISPEECH_FIXED
    S32 iRet = -1;
#endif
    pstSevcCfg = &sevcConfig;
    sevcDefaultCfgInit(pstSevcCfg);
#ifndef AISPEECH_FIXED

    if (NULL == pcResouce)
    {
        MSG_DBG("%s [%d] params error\r\n", __func__, __LINE__);
        return 0;
    }

    iRet = SEVC_CFG_Update(pstSevcCfg, pcResouce);

    if (0 != iRet)
    {
        MSG_DBG("%s resouce can not parse\r\n", __func__);
        return 0;
    }

#endif
    return sevcApiShareMemSizeGet(pstSevcCfg);
}
#endif
/************************************************************
  Function   : SEVC_API_MemSizeGet()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/12/04, shizhang.tang create
************************************************************/
SEVC_VISIBLE_ATTR U32 SEVC_API_MemSizeGet(const S8 *pcResouce, U32 uiSampleRate)
{
    U32 uiTotalMemSize   = 0;
    SEVC_CONFIG_S sevcConfig;
    SEVC_CONFIG_S *pstSevcCfg;
#ifndef AISPEECH_FIXED
    S32 iRet = -1;
#endif
    // frameshift and samplerate is updated by default configure
    pstSevcCfg = &sevcConfig;
    sevcDefaultCfgInit(pstSevcCfg);
    // pstSevcCfg->frameShift = (uiSampleRate == 16000 ? 256 : 128);
    // pstSevcCfg->fs = uiSampleRate;
#ifndef AISPEECH_FIXED

    if (NULL == pcResouce)
    {
        MSG_DBG("%s [%d] params error\r\n", __func__, __LINE__);
        return 0;
    }

    iRet = SEVC_CFG_Update(pstSevcCfg, pcResouce);

    if (0 != iRet)
    {
        MSG_DBG("%s resouce can not parse\r\n", __func__);
        return 0;
    }

#endif
    //sevcParamsShow(pstSevcCfg);
    uiTotalMemSize += sevcApiLocMemSizeGet(pstSevcCfg);
    uiTotalMemSize += sevcApiShareMemSizeGet(pstSevcCfg);
    uiTotalMemSize += ALIGN_SIZE;
    return uiTotalMemSize;
}

/************************************************************
  Function   : SEVC_API_New()

  Description: create sevc_api engine
  Calls      :
  Called By  :
  Input      : fileName: sevc config file
  Output     : sevc_api   struct pointer
  Return     :
  Others     :

  History    :
    2019/12/04, shizhang.tang create
************************************************************/
SEVC_VISIBLE_ATTR VOID *SEVC_API_New(const S8 *pcResouce, S8 *pcBuff, U32 uiBuffSize, U32 uiSampleRate)
{
    U8 ucIsUseLocalMem = 0;
    U32 uiSize = 0;
    U16 usMicNum = 0;
    U16 usRefNum = 0;
    U16 usFrameShift = 0;
    U32 uiTotalMemSize = 0;
    U32 uiLocMemSize = 0;
    U32 uiShMemSize = 0;
    S8 *pcMemBase;
    S8 *pcMem;
    SEVC_API_S *pstSevcApi  = NULL;
    MEM_LOCATOR_S *pstMemLocator = NULL;
    SEVC_CONFIG_S sevcConfigTmp;
    SEVC_CONFIG_S *pstSevcCfg = &sevcConfigTmp;
    sevcDefaultCfgInit(pstSevcCfg);
    // configure frameshift and sample rate according to sample rate parameters
    pstSevcCfg->frameShift = (uiSampleRate == 16000 ? pstSevcCfg->frameShift : 128);
    pstSevcCfg->fs = uiSampleRate;
    sevcCfgReinit(pstSevcCfg);
    uiLocMemSize = sevcApiLocMemSizeGet(pstSevcCfg);
    uiShMemSize = sevcApiShareMemSizeGet(pstSevcCfg);
    uiTotalMemSize = uiLocMemSize + uiShMemSize;
    MSG_DBG("SEVC_WAV %d %d %d %d\r\n", pstSevcCfg->wavChan, pstSevcCfg->micChan,
            pstSevcCfg->refChan, pstSevcCfg->fs);
    MSG_DBG("SEVC_G %d %d %d\r\n", pstSevcCfg->micGain, pstSevcCfg->aecGain, pstSevcCfg->outGain);
    MSG_DBG("SEVC_T %d %d\r\n", pstSevcCfg->usAecTaps, pstSevcCfg->uiBfTaps);
    // MSG_DBG("%s uiTotalMemSize=%d uiLocMemSize=%d uiShMemSize=%d\r\n", __func__, uiTotalMemSize, uiLocMemSize, uiShMemSize);

    if (0 == uiTotalMemSize)
    {
        return NULL;
    }

    if (NULL == pcBuff || 0 == uiBuffSize)
    {
        ucIsUseLocalMem = 1;
        uiTotalMemSize += ALIGN_SIZE;
        pcMemBase   = (S8 *)SEVC_CALLOC(uiTotalMemSize);

        if (NULL == pcMemBase)
        {
            return NULL;
        }

        AISP_TSL_memset(pcMemBase, 0, uiTotalMemSize);
    }
    else
    {
        if (uiTotalMemSize > uiBuffSize)
        {
            MSG_DBG("%s [%d] memory size is too small %d(%d + %d)\r\n", __func__, __LINE__,
                    uiBuffSize, uiLocMemSize, uiShMemSize);
            return NULL;
        }
        else
        {
            pcMemBase = pcBuff;
            AISP_TSL_memset(pcMemBase, 0, uiBuffSize);
        }
    }

    pcMem = AISP_MEM_ALIGN(pcMemBase, 8);
    /*mem init*/
    pstMemLocator = (MEM_LOCATOR_S *)pcMem;
    uiSize = SIZE_ALIGN(sizeof(MEM_LOCATOR_S));
    memLocatorInit(pstMemLocator, pcMem + uiSize, uiLocMemSize - uiSize, uiShMemSize);
    MEM_LOCATOR_SHOW(pstMemLocator);
    /* config init*/
    uiSize = sizeof(SEVC_CONFIG_S);
    BUFF_POINT_SET_ALIGN(pstSevcCfg, uiSize, pstMemLocator, FAIL);
    sevcDefaultCfgInit(pstSevcCfg);

    if (uiSampleRate == 8000)
    {
        pstSevcCfg->fAecDeltaFloorMin = pstSevcCfg->fAecDeltaFloorMin / 4;
    }

#ifndef AISPEECH_FIXED

    if (NULL == pcResouce)
    {
        MSG_DBG("%s [%d] params error\r\n", __func__, __LINE__);
        goto FAIL;
    }

    //strncpy(pstSevcCfg->acResourseFile, pcFileName, sizeof(pstSevcCfg->acResourseFile));
    if (0 != SEVC_CFG_Update(pstSevcCfg, pcResouce))
    {
        MSG_DBG("%s resouce can not parse\r\n", __func__);
        goto FAIL;
    }

#endif
    // configure frameshift and sample rate according to sample rate parameters
    pstSevcCfg->frameShift = (uiSampleRate == 16000 ? pstSevcCfg->frameShift : 128);
    pstSevcCfg->fs = uiSampleRate;
    sevcCfgReinit(pstSevcCfg);
    pstSevcCfg->usFftScalar = AISP_TSL_FFT_SCALAR(pstSevcCfg->usFftFrameSize);
    pstSevcCfg->pfHanningWindowSqrt =
        AISP_TSL_hanningWindowWithSqrt(pstSevcCfg->usFrameSize);
    pstSevcCfg->pfHanningWindow =
        AISP_TSL_hanningWindow(pstSevcCfg->usFrameSize);
    //pstSevcCfg->pstFftEng = AISP_TSL_fft32_init(pstSevcCfg->usFftFrameSize);
#ifndef JIELI_BR28
    fftInit(&pstSevcCfg->stFFTEng, pstSevcCfg->usFftFrameSize);
#endif
    //sevcParamsShow(pstSevcCfg);
    usMicNum = pstSevcCfg->micChan;
    usRefNum = pstSevcCfg->refChan;
    usFrameShift = pstSevcCfg->frameShift;
    /*sevcApi engine init*/
    uiSize = sizeof(SEVC_API_S);
    BUFF_POINT_SET_ALIGN(pstSevcApi, uiSize, pstMemLocator, FAIL);
    pstSevcApi->ucMemBlockSize = (pstSevcCfg->hdr ?
                                  pstSevcCfg->wavChan * 3
                                  : pstSevcCfg->wavChan * 2);
    pstSevcApi->pcMemBase = (ucIsUseLocalMem ? pcMemBase : NULL);
    pstSevcApi->pstSevcCfg  = pstSevcCfg;
    //pfFrameBuffer
    uiSize = SIZE_ALIGN((usMicNum + usRefNum) * usFrameShift * sizeof(aisp_s16_t));
    BUFF_POINT_SET_ALIGN(pstSevcApi->pfFrameBuffer, uiSize, pstMemLocator, FAIL);
    /*sevc instance new*/
    pstSevcApi->pstSevcEng = SEVC_New(pstSevcCfg, pstMemLocator);
    AISP_TSL_PTR_CHECK(pstSevcApi->pstSevcEng, FAIL);
    //MEM_LOCATOR_SHOW(pstMemLocator);
    /* sevcApi engine config regist*/
    pstSevcApi->pstMemLocator = pstMemLocator;
    MEM_LOCATOR_SHOW(pstMemLocator);
    sevcParamsShow(pstSevcCfg);
    return (VOID *)pstSevcApi;
FAIL:
    MSG_DBG("%s [%d] fail\r\n", __func__, __LINE__);
    SEVC_API_Delete(pstSevcApi);
    return NULL;
}

/************************************************************
  Function   : sevcApiChannDataPartition()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/12/04, shizhang.tang create
************************************************************/
static S8 sevcApiChanDataPartition(VOID *pstSevcApi, aisp_s16_t *pfFrame,
                                   aisp_s16_t *pfAuxFrame, U8 *pucBuff, U32 uiBuffSize)
{
    SEVC_API_S *pstSevcApiEng = (SEVC_API_S *)pstSevcApi;
    U16 usMicIdx, usLoop;
    U16 usMicNum = pstSevcApiEng->pstSevcCfg->micChan;
    U16 usFrameShift = pstSevcApiEng->pstSevcCfg->frameShift;
    U32 uiBlockSize = pstSevcApiEng->ucMemBlockSize;
    U32 uiBlockCnt = uiBuffSize / uiBlockSize;
    U32 uiOffset = 0;
    U8 *pucLine = NULL;
    aisp_s16_t *psPcmBuffer = NULL;
#if SEVC_SWITCH_AUX
    U16 usAuxChanOffset = pstSevcApiEng->pstSevcCfg->auxChanOffset;
#endif
#if SEVC_SWITCH_AEC
    U16 usRefIdx = 0;
    U16 usRefNum = pstSevcApiEng->pstSevcCfg->refChan;
    U32 uiRefOffset = pstSevcApiEng->pstSevcCfg->wavChan - usRefNum;
#endif

    if (NULL == pstSevcApi || NULL == pfFrame || NULL == pucBuff
            || 0 == uiBuffSize || 0 != uiBuffSize % pstSevcApiEng->ucMemBlockSize)
    {
        MSG_DBG("%s [%d] params error\r\n", __func__, __LINE__);
        return -1;
    }

    if (pstSevcApiEng->pstSevcCfg->hdr == 0)
    {
        for (usMicIdx = 0; usMicIdx < usMicNum; usMicIdx++)
        {
            pucLine = pucBuff;
            psPcmBuffer = pfFrame + uiOffset;

            for (usLoop = 0; usLoop < uiBlockCnt; usLoop++)
            {
                *psPcmBuffer++ = sevcCharToFloat(pucLine, usMicIdx);
                pucLine += uiBlockSize;
            }

            uiOffset += usFrameShift;
        }

#if SEVC_SWITCH_AUX
        pucLine = pucBuff;
        psPcmBuffer = pfAuxFrame;

        for (usLoop = 0; usLoop < uiBlockCnt; usLoop++)
        {
            /* digital loopback needs a little value 1E-6 */
            *psPcmBuffer++ = sevcCharToFloat(pucLine, usAuxChanOffset);
            pucLine += uiBlockSize;
        }

#endif
#if SEVC_SWITCH_AEC

        for (usRefIdx = 0; usRefIdx < usRefNum; usRefIdx++)
        {
            pucLine = pucBuff;
            psPcmBuffer = pfFrame + uiOffset;

            for (usLoop = 0; usLoop < uiBlockCnt; usLoop++)
            {
                /* digital loopback needs a little value 1E-6 */
                *psPcmBuffer++ = sevcCharToFloat(pucLine, uiRefOffset + usRefIdx);
                pucLine += uiBlockSize;
            }

            uiOffset += usFrameShift;
        }

#endif
    }
    else
    {
        for (usMicIdx = 0; usMicIdx < usMicNum; usMicIdx++)
        {
            pucLine = pucBuff;
            psPcmBuffer = pfFrame + uiOffset;

            for (usLoop = 0; usLoop < uiBlockCnt; usLoop++)
            {
                *psPcmBuffer++ = sevc24bitToFloat(pucLine, usMicIdx);
                pucLine += uiBlockSize;
            }

            uiOffset += usFrameShift;
        }

#if SEVC_SWITCH_AUX
        pucLine = pucBuff;
        psPcmBuffer = pfAuxFrame;

        for (usLoop = 0; usLoop < uiBlockCnt; usLoop++)
        {
            /* digital loopback needs a little value 1E-6 */
            *psPcmBuffer++ = sevc24bitToFloat(pucLine, usAuxChanOffset);
            pucLine += uiBlockSize;
        }

#endif
#if SEVC_SWITCH_AEC

        for (usRefIdx = 0; usRefIdx < usRefNum; usRefIdx++)
        {
            pucLine = pucBuff;
            psPcmBuffer = pfFrame + uiOffset;

            for (usLoop = 0; usLoop < uiBlockCnt; usLoop++)
            {
                *psPcmBuffer++ = sevc24bitToFloat(pucLine, uiRefOffset + usRefIdx);
                pucLine += uiBlockSize;
            }

            uiOffset += usFrameShift;
        }

#endif
    }

    return 0;
}

/************************************************************
  Function   : SEVC_API_Feed()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/12/04, shizhang.tang create
************************************************************/
SEVC_VISIBLE_ATTR S8 SEVC_API_Feed(VOID *pstSevcApi, U8 *pucBuff, U32 uiBuffSize)
{
    SEVC_API_S *pstSevcApiEng = (SEVC_API_S *)pstSevcApi;
    aisp_s16_t *pfFrame = pstSevcApiEng->pfFrameBuffer;
    aisp_s16_t *pfAuxFrame = pstSevcApiEng->pstSevcEng->pfAuxFrame;

    if (NULL == pstSevcApi || NULL == pucBuff
            || 0 == uiBuffSize || 0 != uiBuffSize % pstSevcApiEng->ucMemBlockSize)
    {
        return -1;
    }

    if (0 != sevcApiChanDataPartition(pstSevcApi, pfFrame, pfAuxFrame, pucBuff, uiBuffSize))
    {
        return -1;
    }

    SEVC_Feed(pstSevcApiEng->pstSevcEng, pfFrame);
    return 0;
}

/************************************************************
  Function   : SEVC_API_CallbackRegister()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/12/04, shizhang.tang create
************************************************************/
SEVC_VISIBLE_ATTR VOID SEVC_API_CallbackRegister(VOID *pstSevcApi,
        VOID *pCBFunc, VOID *pUsrData)
{
    SEVC_API_S *pstSevcApiEng = (SEVC_API_S *)pstSevcApi;

    if (NULL == pstSevcApi || NULL == pCBFunc)
    {
        MSG_DBG("%s [%d] params error\r\n", __func__, __LINE__);
        return ;
    }

    SEVC_CbFuncRegister(pstSevcApiEng->pstSevcEng, pCBFunc, pUsrData);
}

SEVC_VISIBLE_ATTR VOID SEVC_API_CallbackRegisterMiddle(VOID *pstSevcApi,
        VOID *pCBFunc, VOID *pUsrData)
{
#ifdef SEVC_MULTI_CORE
    SEVC_API_S *pstSevcApiEng = (SEVC_API_S *)pstSevcApi;

    if (NULL == pstSevcApi || NULL == pCBFunc)
    {
        MSG_DBG("%s [%d] params error\r\n", __func__, __LINE__);
        return ;
    }

    SEVC_MiddleCbFuncRegister(pstSevcApiEng->pstSevcEng, pCBFunc, pUsrData);
#else
    MSG_DBG("%s [%d] not support\r\n", __func__, __LINE__);
#endif
}

/************************************************************
  Function   : SEVC_API_Reset()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/12/04, shizhang.tang create
************************************************************/
SEVC_VISIBLE_ATTR VOID SEVC_API_Reset(VOID *pstSevcApi)
{
    SEVC_API_S *pstSevcEng = (SEVC_API_S *)pstSevcApi;

    if (NULL == pstSevcApi)
    {
        MSG_DBG("%s [%d] params error\r\n", __func__, __LINE__);
        return ;
    }

    SEVC_Reset(pstSevcEng->pstSevcEng);
}

/************************************************************
  Function   : SEVC_API_Delete()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/12/04, shizhang.tang create
************************************************************/
SEVC_VISIBLE_ATTR VOID SEVC_API_Delete(VOID *pstSevcApi)
{
    SEVC_API_S *pstSevcApiEng = (SEVC_API_S *)pstSevcApi;
    SEVC_CONFIG_S *pstSevcCfg;

    if (NULL == pstSevcApi)
    {
        MSG_DBG("%s [%d] params error\r\n", __func__, __LINE__);
        return ;
    }

    SEVC_Delete(pstSevcApiEng->pstSevcEng);
    pstSevcCfg = pstSevcApiEng->pstSevcCfg;

    if (NULL != pstSevcCfg)
    {
#ifndef JIELI_BR28
        fftDelete(&pstSevcCfg->stFFTEng);
#endif
    }

    if (pstSevcApiEng->pcMemBase)
    {
        SEVC_FREE(pstSevcApiEng->pcMemBase);
    }

    return ;
}

#if SEVC_SWITCH_NN
S32 nnProcessorQValueDataGet(NN_STATE_S *pstNn);
#endif
/************************************************************
  Function   : SEVC_API_Delete()

  Description:
  Calls      :
  Called By  :
  Input      :
  Output     :
  Return     :
  Others     :

  History    :
    2019/04/28, chengfei.song create
************************************************************/
SEVC_VISIBLE_ATTR S32 SEVC_API_IoCtrl(VOID *pvArg, S32 iType, VOID *pvBuf, S32 *len)
{
    SEVC_API_S *pstSevcApiEng = (SEVC_API_S *)pvArg;
    S32 iRes = 0;

    switch (iType)
    {
        default:
            {
                MSG_DBG("cmd not support\r\n");
                iRes = -1;
                break;
            }

        case IOCTL_GET_INPUT_LEN:
            {
                S32 *data = (S32 *)pvBuf;
                *data = pstSevcApiEng->pstSevcCfg->micChan * pstSevcApiEng->pstSevcCfg->frameShift
                        * sizeof(S16);
                break;
            }

#if SEVC_SWITCH_NN

        case IOCTL_GET_GAINS:
            {
                F32 *data = (F32 *)pvBuf;
                S32 i, bins;
                S32 Q = nnProcessorQValueDataGet(pstSevcApiEng->pstSevcEng->pstSevcNnEng->pstNn);
                S32 *pfGain = pstSevcApiEng->pstSevcEng->pstSevcNnEng->pfGains;
                bins = pstSevcApiEng->pstSevcCfg->usFftBin;

                if (*len < bins)
                {
                    return -1;
                }

                for (i = 0; i < bins; i++)
                {
                    data[i] = (F32)pfGain[i] / (F32)(1 << Q);
                }

                *len = bins;
                break;
            }

#endif

        case IOCTL_GET_FRAME_R_NUM:
            {
                S32 *data = (S32 *)pvBuf;
                *data = pstSevcApiEng->pstSevcEng->pstSevcNnEng->pstNn->usFeatureContextR;
                break;
            }

        case IOCTL_GET_MODEL_INFO:
            {
                S8 *pModelInfo = SEVC_ModelInfoGet(pstSevcApiEng->pstSevcEng);
                AISP_TSL_strcpy((S8 *)pvBuf, (const S8 *)pModelInfo);
                *len = AISP_TSL_strlen((const S8 *)pModelInfo);
                break;
            }

        case IOCTL_GET_SEVC_INFO:
            {
                S32 iDataLen;
                S8 *pModelInfo = SEVC_ModelInfoGet(pstSevcApiEng->pstSevcEng);
                iDataLen = AISP_TSL_strlen((const S8 *)pModelInfo);

                if (iDataLen > sizeof(pstSevcApiEng->pstSevcCfg->scNnRnnModelFile) - 1)
                {
                    iDataLen = sizeof(pstSevcApiEng->pstSevcCfg->scNnRnnModelFile) - 1;
                }

                AISP_TSL_memcpy(pstSevcApiEng->pstSevcCfg->scNnRnnModelFile, pModelInfo, iDataLen);
                AISP_TSL_memcpy(pvBuf, pstSevcApiEng->pstSevcCfg, sizeof(SEVC_CONFIG_S));
                sevcDataEncode((U8 *)pvBuf, sizeof(SEVC_CONFIG_S));
                *len = sizeof(SEVC_CONFIG_S);
                break;
            }

#ifdef SEVC_MULTI_CORE
#if 0

        case IOCTL_GET_WN_STATUS:
            {
                U8 *pucData = (U8 *)pvBuf;
                *pucData = (pstSevcApiEng->pstSevcCfg->WN_flag ?
                            pstSevcApiEng->pstSevcEng->pstSevcWnEng->uiWindNoiseState : 0);
                break;
            }

        case IOCTL_SET_WN_STATUS:
            {
                pstSevcApiEng->pstSevcEng->ucWnStatus = *((U8 *)pvBuf);
                break;
            }

#endif

        case IOCTL_SET_MIDDLE_FREQ_BINS:
            {
                //U32 uiMicChan = pstSevcApiEng->pstSevcCfg->micChan;
                U32 uiFftBin = pstSevcApiEng->pstSevcCfg->usFftBin;
                U32 uiCxpSize = sizeof(aisp_cpx_s32_t);
                U8 *pucData = (U8 *)pvBuf;
                U32 uiSize = *len;

                if (uiSize == uiCxpSize * uiFftBin)
                {
                    SEVC_MiddleFreqBinsFeed(pstSevcApiEng->pstSevcEng, pucData, uiSize);
                }
                else
                {
                    iRes = -1;
                }

                break;
            }

#endif
    }

    return iRes;
}

SEVC_VISIBLE_ATTR VOID SEVC_API_GetWindStatus(VOID *pstSevcApi, U32 *windStatus, S32 *windSpeed)
{
    SEVC_API_S *pstSevcApiEng = (SEVC_API_S *)pstSevcApi;

    if (pstSevcApiEng != NULL && pstSevcApiEng->pstSevcEng != NULL)
    {
#if SEVC_SWITCH_WN

        if (windStatus != NULL && windSpeed != NULL)
        {
            *windStatus = pstSevcApiEng->pstSevcEng->uiWindStatus;
            *windSpeed = pstSevcApiEng->pstSevcEng->pstSevcStateEng->windSpeed;
        }

#endif
    }
}