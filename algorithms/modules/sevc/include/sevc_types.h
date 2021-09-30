#ifndef SEVC_TYPES_H_
#define SEVC_TYPES_H_

#include "AISP_TSL_complex.h"
#include "AISP_TSL_common.h"
#include "AISP_TSL_fft.h"
#include "fft_wrapper.h"
#include "sevc_config.h"
#ifdef NN_FSMN
#include "nn_fsmn_type.h"
#else
#include "nn_gru_type.h"
#endif
//#include "AISP_TSL_circular_buffer.h"
#define FREQ_BIN_Q21

#define MEM_BUFF_DBG            0
#define SEVC_TRUE               1
#define SEVC_FALSE              0

#define ERR_QUEUE_DEEP          2
#define FRAME_QUEUE_DEEP        2

typedef VOID (*CallBackFunc)(U8 *pucBuff, U32 uiSize, VOID *pUsrData);
typedef aisp_s16_t (*ActFunc)(aisp_s16_t x);

typedef enum
{
    ACTIVATION_SIGMOID    = 0,
    ACTIVATION_TANH       = 1,
    ACTIVATION_RELU       = 2,
    ACTIVATION_NONE       = 3, // HBZ add
} activation_type_e;

typedef struct
{
    S32 iInputs;
    S32 iOutputs;
    U8 ucWQ;
    U8 ucDataQ;
    activation_type_e iActivation;
} LAYER_INFO_S;

typedef struct linear_layer_info
{
    U16 usInputSize;
    U16 usOutputSize;
    U8 ucAction;
    U8 ucWQ;
    U8 ucDataQ;
    U8 reserved;
} LINEAR_LAYER_s;

typedef struct fsmn_layer_info
{
    U16 usInputSize;
    U16 usHiddenSize;
    U16 usOutputSize;
    U16 usMemBlock;
    U8 ucAction;
    U8 ucWQ;
    U8 ucDataQ;
    U8 reserved;
} FSMN_LAYER_s;

typedef struct gru_layer_info
{
    U16 usInputSize;
    U16 usOutputSize;
    U8 ucAction;
    U8 reserved[3];
} GRU_LAYER_s;


typedef struct nn_model_info
{
    U8 ucWQ;
    U8 ucDataQ;
    U16 usFeatureTotalDim;
#ifdef NN_FSMN
    U8 ucNBBands;
    U8 ucInLinears;
    U8 ucNetLayers;
    U8 ucOutLinears;
    LINEAR_LAYER_s *pInLinears;
    FSMN_LAYER_s *pNets;
    LINEAR_LAYER_s *pOutLinears;
#else
    U16 usDenseInInput;
    U16 usDenseInNeurons;

    U16 usGruLayerInput[3];
    U16 usGruLayerNeurons[3];
    U16 usDenseOutInput;
    U16 usDenseOutNeurons;

    U8 ucNBBands;
    U8 ucDenseInActoin;
    U8 ucGruLayerActoin[3];
    U8 ucDenseOutActoin;
#endif
} NN_MODEL_INFO_S;


typedef struct RNNModel
{
#ifdef NN_FSMN
    S32 iInDenseLayers;
    S32 iFsmnLayers;
    S32 iOutDenseLayers;
    S8 *pcSharedMem;
    FC_LAYER_S **ppInFcLayer;
    FSMN_LAYER_S **ppFsmnLayer;
    FC_LAYER_S **ppOutFcLayer;
    aisp_s32_t **ppfInFcOutput;
    aisp_s32_t **ppfOutFcOutput;
#else
    S32 iDenseInSize;
    FC_LAYER_S *pFcInLayer;
    S32 iGruLayerSize[3];
    GRU_LAYER_S **ppGruLayer;
    S32 iDenseOutSize;
    FC_LAYER_S *pFcOutLayer;
    aisp_s32_t *pfOutFcOutput;
#endif
    S32 iOutLen;
    aisp_s32_t *pOutput; //just a point
} NN_MODEL_S;

typedef struct NNState
{
    NN_MODEL_S          *pstModel;
    NN_MODEL_INFO_S     modelInfo;
    U8                  ucCmnSupport;
    U8                  ucCmnLen;
    U16                 usNbBands;
    U16                 usFeatureContextL;
    U16                 usFeatureContextR;
    U16                 usFeatureContext;
    U16                 *psFull2BarkMapping;
    aisp_s64_t          *pfNbBandsSum;
    aisp_s64_t          *pfNbBandsSumTmp;
    aisp_s32_t          *pfFeaturesHistory;
    aisp_s32_t          *pfFeatures;
    aisp_s64_t          *pCmnFeatureSum;
    aisp_s32_t          **ppCmnBuffer;
    aisp_s32_t          *pfRnnOut;

#ifdef NN_FSMN
    aisp_s32_t          *piFeatureMean;
    aisp_s32_t          *piFeatureVar;
#endif

#ifdef NN_GRU
    aisp_s32_t          *pfVadGruState;
    aisp_s32_t          *pfNoiseGruState;
    aisp_s32_t          *pfDenoiseGruState;
    aisp_s32_t          *pfDenseVadOut;
    aisp_s32_t          *pfNoiseInput;
    aisp_s32_t          *pfDenoiseInput;
    aisp_s32_t          *pfMiu;
#endif
} NN_STATE_S;

// quiet detection module is for bfpost and nn
typedef struct QuietDetect
{
    U32 uiSpeCnt;
    U32 uiSilenceCnt;
    U32 uiQuietState;
    U32 highNoiseDetect;
    S32 noiseLevel;
    U32 highNoise;
    U32 noisePow;
    U32 pMin;
    U32 pTmp;
    S32 pkMcra;
    U32 jj;
    U32 u1;
    U32 minGsc[4];
    U32 highCnt;
    U32 lowCnt;
    U32 U;
    U32 V;
} QuietDetect_S;

typedef struct tag_sevc_config
{
    /*switch*/
    U8 WN_flag;
    U8 AEC_flag;
    U8 BF_flag;
    U8 GSC_flag;
    U8 BF_POST_flag;
    U8 AUX_flag;
    U8 NR_flag;
    U8 NN_flag;
    U8 CNG_flag;
    U8 AES_flag;
    U8 EQ_flag;
    U8 AGC_flag;
    U8 NN_AES_flag;
    /*parameter*/
    U8 hdr;
    U16 fs;
    U16 wavChan;
    U16 micChan;
    U16 refChan ;
    U16 auxChanOffset;

    /*config*/
    U16 frameShift;
    aisp_s32_t micGain;
    aisp_s32_t refGain;
    aisp_s32_t aecGain;
    aisp_s32_t outGain;
    // for bfpost
    aisp_s32_t postGain;

    //wn
    U32 uiWnSil2Spe;
    U32 uiWnSpe2Sil;
    aisp_s32_t fWnPowerThresh1;
    aisp_s32_t fWnPowerThresh2;
    aisp_s32_t fWnPowerThresh3;
    aisp_s32_t fWnRateThresh1;
    aisp_s32_t fWnRateThresh2;
    aisp_s32_t fWnRateThresh3;
    aisp_s32_t fRefThresh;
    aisp_s32_t fWnMicSmoothAlpha;
    aisp_s32_t fWnRateSmoothAlpha;

    // aux
    aisp_s32_t fAlphaGain;

    //aec
    U8 ucAecPreemphSupport;
    U8 ucAecEmd;
    U16 usAecTaps;
    aisp_s16_t fAecPreemph;
    aisp_s32_t fAecMu;
    aisp_s32_t fAecLamda;
    aisp_s32_t fAecLamdaUp;
    aisp_s32_t fAecLamdaDown;
    aisp_s32_t fAecDelta;
    aisp_s32_t fAecDeltaFloorMin;

    //bf
    U8 ucBfBeamCount;
    U8 ucBfSprFlag;
    U32 uiDistanceMic;
    U32 uiBfTaps;
    aisp_s32_t fBfGainFloor;
    aisp_s32_t fBfDelta;
    aisp_s32_t fBfRatioTrig1;
    aisp_s32_t fBfRatioTrig2;
    aisp_s32_t fBfRatioTrig3;
    aisp_s32_t fBfMu;
    //gsc
    U16 usGscTaps;
    aisp_s32_t usGscMu;
    aisp_s32_t fMuOffset;
    aisp_s32_t fGscDeltaGain;
    aisp_s32_t fGscDeltaMin;
    aisp_s32_t fGscDeltaMax;
    // bfpost
    aisp_s32_t fMcraAs;
    aisp_s32_t fMcraAp;
    aisp_s32_t usMcraDelta;
    U8 ucU;
    U8 ucV;
    aisp_s32_t fBfpostMcraAd;
    aisp_s32_t fBfpostAlpha;
    aisp_s32_t fPriorMin;
    aisp_s32_t fGainFloor;
    aisp_s32_t fRatioYuZhi;
    U16 usK0;
    U16 usK1;
    aisp_s32_t fPesai0;
    aisp_s32_t fBeta;
    aisp_s32_t fOmegaLow;
    aisp_s32_t fOmegaHigh;
    //aux
    U32 uiAuxBin1;
    U32 uiAuxBin2;
    //nr
    aisp_s32_t fNrMcraAd;
    aisp_s32_t fNrGainTrig;
    aisp_s32_t fNrGainFloor;

    //nn
    U8 ucNnBarkSupport;
    U8 ucNnBandEnergySmooth;
    U16 ucNnFeatureDim;
    U16 usFeatureContextR;
    S8 scNnRnnModelFile[64];
    aisp_s64_t fNnGainFloor;
    aisp_s64_t fNnWindGainFloor;
    aisp_s32_t fAesGainThresh;

    // aes
    aisp_s32_t fGamma;
    aisp_s32_t alpha;
    aisp_s32_t alphaMax;
    aisp_s32_t alphaMin;
    aisp_s32_t fAesGainFloor;

    // vad
    aisp_s32_t fAmth;
    aisp_s32_t sil2spe;
    aisp_s32_t spe2sil;
    // sd
    aisp_s32_t fSdAlpha;
    aisp_s32_t fAlphaWearLevelUp;
    aisp_s32_t fAlphaWearLevelDown;
    aisp_s32_t fWindTrig;
    aisp_s32_t fWearTrig;
    aisp_s32_t fWindSmooth;
    U16 usBinStart;
    U16 usBinEnd;
    aisp_s32_t wind2quit;
    aisp_s32_t quit2wind;
    aisp_s32_t tight2loose;
    aisp_s32_t loose2tight;
    U16 usWindSwitch;
    U16 usWearSwitch;
    //aisp_s32_t vad_trig;
    //aisp_s32_t mcra_L;
    aisp_s32_t mcra_as;
    aisp_s32_t mcra_as1;
    aisp_s32_t mcra_as2;

    //agc
    //DRC_Gain_calc_parameters
    aisp_s32_t G_low;
    aisp_s32_t ratio1;
    aisp_s32_t ratio2;
    aisp_s32_t T1;
    aisp_s32_t T2;
    aisp_s32_t T2_gain;
    aisp_s32_t vad_smooth_short_len;
    aisp_s32_t vad_smooth_long_len;
    aisp_s32_t vad_smooth_short_thr;
    aisp_s32_t vad_smooth_long_thr;
    aisp_s32_t vad_thresh;
    aisp_s32_t gainThreshold;

    //Gain_smooth_parameters
#ifdef AISPEECH_FIXED
    aisp_s32_t agcAlphaABeginVAD;
    aisp_s32_t agcAlphaAInVAD;
    aisp_s32_t agcAlphaRSil;
    aisp_s32_t agcAlphaRInVAD;
#else
    aisp_s32_t AttackTime_beginVAD;
    aisp_s32_t AttackTime_inVAD;
    aisp_s32_t ReleaseTime_inVAD;
    aisp_s32_t ReleaseTime_sil;
#endif
    //shared params
    U16 usFftFrameSize;
    U16 usFrameSize;
    U16 usFftBin;
    U16 usFftScalar;
    S8 acResourseFile[128];
    aisp_s16_t fLembda;
    U16 *pusPosBuff;
    aisp_s16_t *pfHanningWindowSqrt;
    aisp_s16_t *pfHanningWindow;
#ifdef USE_MULT_THREAD
    aisp_fft_eng_t **ppstFftEng;
#else
    //aisp_fft_eng_t *pstFftEng;
    AISP_TSL_FFTENG_S  stFFTEng;
#endif

} SEVC_CONFIG_S;

typedef struct tag_sevc_wn
{
    U32 uiFrameCount;
    U32 uiWindIdx;
    U32 uiSpeCnt;
    U32 uiSilCnt;
    U32 uiWindNoiseState;
    aisp_s32_t fRateSmooth;
    aisp_s32_t *pfMicAmpSmooth;

    //shared info
    SEVC_CONFIG_S *pstSevcCfg;
} SEVC_WN_S;

typedef struct tag_sevc_aec_instance
{
    aisp_s32_t fDataX;
    aisp_s32_t *pfMicFramePreemph;
    aisp_cpx_s32_t *pErr;
    aisp_s32_t *pfErrFrame;
    aisp_cpx_s32_t *pOutEst;
    //adapter weight
    aisp_cpx_s32_t ** *pppWFDomain;

    SEVC_CONFIG_S *pstSevcCfg;
} SEVC_AEC_INSTANCE_S;

typedef struct tag_sevc_aec
{
    U32 uiFrameCount;
    U32 uiMicIdx;
    aisp_s32_t refVad;

    aisp_s32_t ** *pppfRefPower;
    aisp_s32_t *pfDiag;
    aisp_cpx_s32_t ** *pppRefX;
    SEVC_AEC_INSTANCE_S **ppstAecInstance;
    aisp_cpx_s32_t *pErrOut;
    aisp_s64_t *pfErrPower;

    CallBackFunc pCallBackFunc;
    VOID *pUsrData;

    //shared mem
    aisp_cpx_s32_t *pErrNorm;
    aisp_s64_t *pfDataTmp;
    aisp_s32_t *pfRefPowerSum;
    //shared info

    //aisp_fft_eng_t *pstFftEng;
    AISP_TSL_FFTENG_S  *pstFftEng;
    SEVC_CONFIG_S *pstSevcCfg;
} SEVC_AEC_S;

typedef struct tag_sevc_aes
{
    U32 uiFrameCount;
    CallBackFunc pCallBackFunc;
    VOID *pUsrData;
    // cross spectrum of error and mic
    aisp_cpx_s32_t *pEd;
    // power spectrum of estimated echo
    aisp_s64_t *pYy;
    aisp_s32_t *pAlpha;
    SEVC_CONFIG_S *pstSevcCfg;
} SEVC_AES_S;

typedef struct tag_sevc_bf
{
    U32 uiFrameCount;
    CallBackFunc pCallBackFunc;
    VOID *pUsrData;
    aisp_cpx_s32_t **ppW;
    aisp_cpx_s32_t *pOut;
#if SEVC_SWITCH_BF_SPR == 0
    aisp_s64_t *pfPowSum;
    aisp_s32_t **ppfXPow;
    aisp_cpx_s32_t **ppX;
    aisp_cpx_s32_t **ppWGsc;
#endif

    //shared mem
#if SEVC_SWITCH_BF_SPR
    aisp_s32_t *pfPsd;
    aisp_s32_t *pfPsdSum;
#else
    aisp_s32_t fMuRatio2;
    aisp_s32_t *pfSigPowMean;
    aisp_s32_t *pfLamda;
    aisp_s32_t *pfRatio;
    aisp_cpx_s32_t *pEst;
#endif
    //shared info
    SEVC_CONFIG_S *pstSevcCfg;
} SEVC_BF_S;

typedef struct tag_sevc_gsc
{
    U32 uiFrameCount;
    CallBackFunc pCallBackFunc;
    VOID *pUsrData;
    aisp_cpx_s32_t *pstErr;
    aisp_cpx_s32_t *pstErrNorm;
    aisp_cpx_s32_t *pstEst;
    aisp_cpx_s32_t **ppstNoise;
    aisp_cpx_s32_t **ppstWeight;
    aisp_s32_t **ppfNoisePow;
    aisp_s32_t *pfNoisePowSum;
    aisp_s64_t *pfErrPow;
    aisp_s32_t *pfDelta;
    SEVC_CONFIG_S *pstSevcCfg;
} SEVC_GSC_S;

typedef struct tag_sevc_bfpost
{
    // variables
    U8 vIdx;
    U8 uIdx;

    // unshared memory
    aisp_s64_t **pStoredMinGsc;
    aisp_s64_t *pSmGsc;
    aisp_s64_t *pMinGsc;
    aisp_s64_t *pTmpGsc;
    aisp_s32_t *pPkGsc;
    aisp_s64_t *pNoisePsGsc;
    aisp_s32_t *pGamma;
    aisp_s32_t *pGammaLast;
    aisp_s32_t *pGH1;
    aisp_s64_t *pNoisePs;

    // struct def
    QuietDetect_S stQuietDetectEng;
    // shared memory
    aisp_s64_t *pNoisyPowerGsc;
    aisp_s32_t *pSrkGsc;

    // shared info
    SEVC_CONFIG_S *pstSevcCfg;
    CallBackFunc pCallBackFunc;
    VOID *pUsrData;
    U32 frameCnt;

} SEVC_BFPOST_S;


typedef struct tag_sevc_aux
{
    U32 uiFrameCount;
    aisp_s16_t *pfFrameOld;
    aisp_s32_t *pfFrameWin;
    aisp_cpx_s32_t *pAuxBins;
    //shared info
    AISP_TSL_FFTENG_S  *pstFftEng;
    SEVC_CONFIG_S *pstSevcCfg;
} SEVC_AUX_S;

typedef struct tag_sevc_nr
{
    U32 uiFrameCount;
    CallBackFunc pCallBackFunc;
    VOID *pUsrData;

    aisp_s32_t *pfAdk;
    aisp_s32_t *pfNoisyPower;
    aisp_s32_t *pfNoisePs;
    aisp_cpx_s32_t *pOutFreq;
    //shared info
    SEVC_CONFIG_S *pstSevcCfg;
} SEVC_NR_S;

typedef struct tag_sevc_nn
{
    U32             uiFrameCount;
    U32             uiFrameIdx;
    U32             uiSeed;
    aisp_s32_t      fVad;
    aisp_s16_t      fMiu;

#if SEVC_SWITCH_CNG
    //CNG
    aisp_s32_t *pfPNoisy;
    aisp_s32_t *pfNoisePs;
#endif

    aisp_cpx_s32_t  **ppLastX;
    aisp_s32_t      *pfGains;

    aisp_cpx_s32_t  *pNnOut;
    NN_STATE_S     *pstNn;
    CallBackFunc    pCallBackFunc;
    VOID            *pUsrData;


    //shared mem
    //module shared
    aisp_s32_t      *pfGainNew;
#if SEVC_SWITCH_CNG
    //CNG
    aisp_cpx_s32_t *pCng;
#endif
    //function shared
#if SEVC_SWITCH_CNG
    //CNG
    aisp_s32_t *pfDataTmp;
    aisp_s32_t *pfAng;
#endif
    //shared info
    SEVC_CONFIG_S   *pstSevcCfg;
} SEVC_NN_S;

typedef struct tag_sevc_eq
{
    aisp_s32_t *pfEqGain;
    SEVC_CONFIG_S   *pstSevcCfg;
} SEVC_EQ_S;

typedef struct tag_sevc_agc_drc
{
    aisp_s32_t fXDb;
    aisp_s32_t fVadSmoothShort;
    aisp_s32_t fVadSmoothLong;
    aisp_s32_t fGainSmooth;
    aisp_s32_t fGainLinear;
} SEVC_AGC_DRC_S;

typedef struct tag_sevc_agc
{
    U32 uiFrameCount;
    U16 usVadBuffLen;
    U16 usVadIdx;
    U16 usVadShortIdx;
    aisp_s32_t *pfVadBuff;
    aisp_s32_t *pfVadShortBuff;
    aisp_s32_t fAlphaABeginVAD;
    aisp_s32_t fAlphaAInVAD;
    aisp_s32_t fAlphaRSil;
    aisp_s32_t fAlphaRInVAD;
    aisp_s32_t fGainSmooth;
    aisp_s16_t *pfFrameOut;

    SEVC_AGC_DRC_S drc;
    CallBackFunc pCallBackFunc;
    VOID *pUsrData;
    //shared info
    SEVC_CONFIG_S *pstSevcCfg;
    SEVC_NN_S *pstNnEng;
} SEVC_AGC_S;

typedef struct state_probe
{
    aisp_s32_t usStatus;
    aisp_s32_t fDStatus;

    aisp_s32_t fDetectTrig;
    aisp_s32_t fAlpha;

    U32 oneCnt;
    U32 zeroCnt;
    U32 zero2One;
    U32 one2Zero;
} StateProbe_S;

typedef struct tag_state_vad
{
    aisp_s32_t highNoise;
    aisp_s32_t noisePowMean;
    aisp_s32_t noisePow[33];
    aisp_s32_t noisyPow[33];
    aisp_s32_t noisePowTmp[33];
    aisp_s32_t noiseLevel;
    aisp_s32_t snrArray[33];
    aisp_s32_t snr;
    aisp_s32_t vad;
    aisp_s32_t pMin[33];
    aisp_s32_t pTmp[33];
    aisp_s32_t pkMcra[33];
    aisp_s32_t jj;
    aisp_s32_t u1;
    aisp_s32_t U;
    aisp_s32_t V;
    aisp_s32_t minGsc[4][33];
} StateVad_S;

typedef struct tag_sevc_state_probe
{
    U32 uiFrameCount;
    U16 usWindSwitch;
    U16 usWearSwitch;

    StateProbe_S stWindDetector;
    StateProbe_S stWearDetector;
    StateVad_S stVadDetector;

    aisp_s64_t *coh11;
    aisp_s64_t *coh22;
    aisp_s64_t *coh33;
    aisp_s64_t *ratioVector;

    aisp_cpx_s32_t *coh12;
    aisp_cpx_s32_t *coh13;

    U16 usWindStatus;
    U16 usWearStatus;
    aisp_s32_t windSpeed; // Q16
    aisp_s32_t wearLevel; // Q16

    SEVC_CONFIG_S *pstSevcCfg;

    CallBackFunc pCallBackFunc;
    VOID *pUsrData;

} SEVC_STATE_S;

typedef struct tag_sevc_vad
{
    U32 uiFrameCount;
    aisp_s32_t vadAmth;
    aisp_s32_t sil2Spe;
    aisp_s32_t spe2Sil;

    aisp_s32_t speCnt;
    aisp_s32_t silCnt;
    aisp_s32_t vadFlag;
    aisp_s32_t *pRefSum;

    SEVC_CONFIG_S *pstSevcCfg;
} SEVC_VAD_S;

typedef struct tag_sevc
{
    U32 uiFrameCount;
    aisp_s32_t fMemE;

    // global flags
    U32 uiQuietState;
    U32 uiWindStatus;
    U32 uiWearStatus;
    aisp_s32_t refVad;
    aisp_s32_t refVadLast;
    aisp_s32_t noiseUpdate;

    // for aux
    aisp_s32_t *innerMicGain2;
    aisp_s32_t *innerMicGain;
    // aux buffer end

    aisp_s16_t *pfHanningSqrt;
    aisp_s32_t *usOverlapGain;

    S16 *psFrameOut;
    // save for overlap add
    aisp_s16_t *pfErrFrameOld;
    aisp_s16_t *pfErrFrame;
    aisp_s32_t *pfBfSpp;
    //aux
    aisp_s16_t *pfAuxFrame;
    aisp_s32_t fVadStatus;
    //CNG
    aisp_s16_t *pfCngFrameOld;
    aisp_s16_t *pfCngFrame;
    // save for windowing mic
    aisp_s16_t **ppfMicFrameOld;
    // save for windowing ref
    aisp_s16_t *pfRefFrameOld;
    // mic data in frequency domain
    aisp_cpx_s32_t *pMicBins;
    // ref data in frequency domain
    aisp_cpx_s32_t *pRefBins;
    // single channel linear echo
    aisp_cpx_s32_t *pAecEstBuff;
    aisp_cpx_s32_t *pstBlockFrame;
    SEVC_WN_S *pstSevcWnEng;
    SEVC_AEC_S *pstSevcAecEng;
    SEVC_BF_S *pstSevcBfEng;
    SEVC_GSC_S *pstSevcGscEng;
    SEVC_BFPOST_S *pstSevcBfPostEng;
    SEVC_AUX_S *pstSevcAuxEng;
    SEVC_NR_S *pstSevcNrEng;
    SEVC_NN_S *pstSevcNnEng;
    SEVC_AES_S *pstSevcAesEng;
    SEVC_EQ_S *pstSevcEqEng;
    SEVC_AGC_S *pstSevcAgcEng;
    SEVC_STATE_S *pstSevcStateEng;
    SEVC_VAD_S *pstSevcVadEng;

    VOID *pUsrData;
    CallBackFunc pCallBackFunc;
    VOID *pUsrDataBackup;
    VOID *pUsrDataMiddle;
    CallBackFunc pCallBackFuncBackUp;
    CallBackFunc pCallBackFuncMiddle;
    //shared mem
    // store windowed data temporarily
    aisp_s32_t *pfFrameWide;

    //share info
    //aisp_fft_eng_t *pstFftEng;
    AISP_TSL_FFTENG_S  *pstFftEng;
    SEVC_CONFIG_S *pstSevcCfg;
} SEVC_S;

typedef struct tag_mem_locator
{
    S8 *pcShMemBase;
    U32 uiShMemOffset;
    U32 uiShMemSize;
    S8 *pcBase;
    U32 uiOffset;
    U32 uiSize;
} MEM_LOCATOR_S;

typedef struct tag_sevc_api
{
    U8 ucMemBlockSize;
    U8 ucFrameIdx;
    VOID *pUsrData;
    CallBackFunc pOutFrameProc;
    S8 *pcMemBase;
    U8 *pucOutFrame;
    U8 *pucChPopBuff;
    aisp_s16_t *pfFrameBuffer;
    SEVC_S *pstSevcEng;

    MEM_LOCATOR_S *pstMemLocator;
    SEVC_CONFIG_S *pstSevcCfg;
} SEVC_API_S;

S32 fftInit(AISP_TSL_FFTENG_S *pstFftEng, S32 fftSize);
//S32 fftProcess(BF_Api_S *pstBfApi, S8 *pcData);
//S32 fftProcessParallel(BF_Api_S *pstBfApi, S8 *pcChan1, S8 *pcChan2);
VOID fftDelete(AISP_TSL_FFTENG_S *pstFftEng);


#endif
