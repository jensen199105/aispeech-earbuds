/************************************************************
  Copyright (C), AISpeechTech. Co., Ltd.

  FileName   : aec_api_config.h
  Project    :
  Module     :
  Version    :
  Date       : 2019/12/02
  Author     : shizhang.tang
  Document   :
  Description:

  Function List :

  History :
  <author>      <time>            <version >        <desc>
  shizhang.tang  2019/12/02         1.00              Create
************************************************************/
#ifndef __AEC_API_CONFIG_NEAR_FIELD_H__
#define __AEC_API_CONFIG_NEAR_FIELD_H__

#define SEVC_SWITCH_WN                      1
#define SEVC_SWITCH_AEC                     1
#define SEVC_SWITCH_BF                      1
#define SEVC_SWITCH_AUX                     0
#define SEVC_SWITCH_NR                      1
#define SEVC_SWITCH_NN                      1
#define SEVC_SWITCH_AGC                     0
#define SEVC_SWITCH_NN_AES                  0
#define SEVC_SWITCH_AES                     0
#define SEVC_SWITCH_CNG                     0
#define SEVC_SWITCH_AEC_PREEMPH             0
#define SEVC_SWITCH_AEC_EMD                 0
#define SEVC_SWITCH_BF_SPR                  0
#define SEVC_SWITCH_NN_BARK                 1

#define SEVC_WAV_CHAN                       3
#define SEVC_MIC_CHAN                       2
#define SEVC_REF_CHAN                       1
#define SEVC_AUX_CHAN_OFFSET                2

/*config*/
#define SEVC_HDR                            0
#ifdef FFT_SIZE_256
#define SEVC_FS                             8000
#define SEVC_FRAME_SHIFT                    128
#define SEVC_AEC_DELTA_FLOOR_MIN            481 //7e-9 * usFrameSize * usFrameSize); W32Q20
#else
#define SEVC_FS                             16000
#define SEVC_FRAME_SHIFT                    256
#define SEVC_AEC_DELTA_FLOOR_MIN            1924 //7e-9 * usFrameSize * usFrameSize); W32Q20
#endif
#define SEVC_MIC_GAIN                       (32768) // 1; Q15
#define SEVC_AEC_GAIN                       32768 // 1; Q15
#define SEVC_OUT_GAIN                       (2*32768) // 1; Q15

//wn
#define SEVC_WN_SIL2SPE                     10
#define SEVC_WN_SPE2SIL                     300
#define SEVC_WN_POWER_THRESH1               1000
#define SEVC_WN_POWER_THRESH2               4000
#define SEVC_WN_POWER_THRESH3               5000
#define SEVC_WN_RATE_THRESH1                1677722 //1.6 Q20 1572864 //1.5 Q20
#define SEVC_WN_RATE_THRESH2                1572864 //1.5 Q20 1363149 //1.3 Q20
#define SEVC_WN_RATE_THRESH3                1258291 //1.2 Q20
#define SEVC_WN_MIC_SMOOTH_ALPHA            6554 //0.2 Q15
#define SEVC_WN_RATE_SMOOTH_ALPHA           6554 //0.2 Q15

//aec
#define SEVC_AEC_TAPS                       1
#define SEVC_AEC_PREEMPH                    29491 //0.9;Q15
//#define SEVC_AEC_MU                         20972 //0.02; Q20
//#define SEVC_AEC_MU                         5243 //0.02; Q18
#define SEVC_AEC_MU                         104858 // 0.4 Q18
#define SEVC_AEC_LAMDA                      256901 //0.98; Q18
#define SEVC_AEC_LAMDA_UP                   16777 //0.016; Q20
#define SEVC_AEC_LAMDA_DOWN                 104 //9.92e-5; Q20
//#define SEVC_AEC_DALTA                      1992 //0.0076 Q18
#define SEVC_AEC_DALTA                      498 //0.0076 Q16

//bf
#define SEVC_BF_MIC_DISTANCE               28 //mm
#define SEVC_BF_BEAM_COUNT                 2
#define SEVC_BF_TAPS                       1
#define SEVC_BF_GAIN_FLOOR                 1677722   //0.1 W32Q24
//#define SEVC_BF_DELTA                      6554      //0.1 W32Q16
#define SEVC_BF_DELTA                      104308128   //1.591615728102624e+03 W32Q16
#define SEVC_BF_RATIO_TRIG1                754975    //0.36 W32Q21
#define SEVC_BF_RATIO_TRIG2                2097152   //1 W32Q21
#define SEVC_BF_RATIO_TRIG3                524288    //0.25 W32Q21
//#define SEVC_BF_MU                         13421773   //0.8 W32Q24
#define SEVC_BF_MU                         16777216   //1 W32Q24

//aux
#define SEVC_AUX_BIN1                      19
#define SEVC_AUX_BIN2                      29

//nr
#define SEVC_NR_MCRA_AD                    629146
#define SEVC_NR_GAIN_TRIG                  104858
#define SEVC_NR_GAIN_FLOOR                 524288

//nn
#define SEVC_NN_GAIN_FLOOR                 21990232556 //Q40

//aes
#define SEVC_AES_GAIN_THRESH                838861
#define SEVC_AES_LAMDA_X                    16441672 ////15602811=0.9*(1<<24)
//#define SEVC_AES_SCAFT                      419430400 // //419430400= 25*(1<<24);
#define SEVC_AES_SCAFT                      16777216 // //419430400= 25*(1<<24);
#define SEVC_AES_GAMMA                      167772  // // 167772=0.01*(1<<24);
#define SEVC_AES_GAIN_FLOOR                 16777  // // 167772=0.01*(1<<24);
#define SEVC_AES_NOISE_FLOOR                17 //  2 = 0.000001*(1<<24);
#define SEVC_AES_EST_BIN_COUNT              20
#define SEVC_AES_EST_BIN_START              15

//cng
//#define SEVC_CNG_VAD_TRIG                   1677722 //0.1*(1<<24)
//#define SEVC_CNG_MCRA_L                     100
#define SEVC_CNG_MCRA_AS                    11744051 //0.7*(1<<24)
#define SEVC_CNG_MCRA_AS1                   16760439//15938355; //0.999*(1<<24)
#define SEVC_CNG_MCRA_AS2                   335544  // 0.02*(1<<24)

//agc
#define SEVC_AGC_VAD_SMOOTH_SHORT_LEN       3
#define SEVC_AGC_VAD_SMOOTH_LONG_LEN        20
//838860800=50*(1<<24);2130706432=127*(1<<24);//1342177280=80*(1<<24);//1509949440=90*(1<<24);
#define SEVC_AGC_GAIN_LOW                   -2130706432
//1677722=0.1*(1<<24); 11744051=0.7*(1<<24); 8388608=0.5*(1<<24);
#define SEVC_AGC_RATIO1                     8388608
#define SEVC_AGC_RATIO2                     1677722 //0.1;W32Q24
#define SEVC_AGC_T1                         -922746880 // -55; W32Q24
#define SEVC_AGC_T2                         -150994944 //-9;W32Q24
#define SEVC_AGC_T2_GAIN                    0
#define SEVC_AGC_VAD_SMOOTH_SHORT_THR       838861 //0.05; W32Q24
#define SEVC_AGC_VAD_SMOOTH_LONG_THR        3355443 //0.2; W32Q24
#define SEVC_AGC_VAD_THRESH                 3355443 //0.2; W32Q24
//Gain_smooth_parameters
#define SEVC_AGC_ALPHA_ATTACK_BEGIN_VAD     8305433  //W32Q24  fs=16000 frameShift=256 agcAttackTimeBeginVAD=20
#define SEVC_AGC_ALPHA_ATTACK_IN_VAD        11804323
#define SEVC_AGC_ALPHA_RELEASE_IN_VAD       16484886 //16747751; //W32Q24 fs=16000 frameShift=160 agcAlphaRInVAD=5
#define SEVC_AGC_ALPHA_RELEASE_SIL          14072799 //W32Q24 fs=16000 frameShift=160 agcAlphaRSil=3


#endif

