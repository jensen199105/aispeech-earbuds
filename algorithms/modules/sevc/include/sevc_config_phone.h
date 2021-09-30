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
#ifndef __AEC_API_CONFIG_PHONE_H__
#define __AEC_API_CONFIG_PHONE_H__

#define SEVC_SWITCH_WN                      1
#define SEVC_SWITCH_AEC                     1
#define SEVC_SWITCH_BF                      1
#define SEVC_SWITCH_GSC                     1
#define SEVC_SWITCH_BF_POST                 1
#define SEVC_SWITCH_AUX                     0
#define SEVC_SWITCH_NR                      0
#define SEVC_SWITCH_NN                      1
#define SEVC_SWITCH_CNG                     0
#define SEVC_SWITCH_AES                     1
#define SEVC_SWITCH_EQ                      0
#define SEVC_SWITCH_AGC                     1
#define SEVC_SWITCH_NN_AES                  0
#define SEVC_SWITCH_VAD                     1


#define SEVC_SWITCH_AEC_PREEMPH             0
#define SEVC_SWITCH_AEC_EMD                 0
#define SEVC_SWITCH_BF_SPR                  0
#define SEVC_SWITCH_NN_BARK                 1

#define SEVC_WAV_CHAN                       4
#define SEVC_MIC_CHAN                       3
#define SEVC_REF_CHAN                       1
#define SEVC_AUX_CHAN_OFFSET                2

/*config*/
#define SEVC_HDR                            0
#ifdef FFT_SIZE_256
#define SEVC_FS                             8000
#define SEVC_FFT_SIZE                       256
#define SEVC_AEC_DELTA_FLOOR_MIN            481 //7e-9 * usFrameSize * usFrameSize); W32Q20
#else
#define SEVC_FS                             16000
#define SEVC_FFT_SIZE                       512
#define SEVC_AEC_DELTA_FLOOR_MIN            1924 //7e-9 * usFrameSize * usFrameSize); W32Q20
#endif
// variable frameshift
#define SEVC_FRAME_SHIFT                    384

#define SEVC_MIC_GAIN                       (32768) // 1; Q15
#define SEVC_REF_GAIN                       (32768) // 1; Q15
#define SEVC_AEC_GAIN                       32768 // 1; Q15
#define SEVC_OUT_GAIN                       (32768) // 10; Q15
#define SEVC_POST_GAIN                      (2*32768) // 2; Q15

//wn
#define SEVC_WN_SIL2SPE                     10
#define SEVC_WN_SPE2SIL                     300
#define SEVC_WN_POWER_THRESH1               1000
#define SEVC_WN_POWER_THRESH2               4000
#define SEVC_WN_POWER_THRESH3               5000
#define SEVC_WN_REF_POWER_THRESH            500
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
#define SEVC_AEC_DALTA                      498 //0.0076 Q16
// #define SEVC_AEC_LAMDA_UP                   8389 // 0.008 Q20
// #define SEVC_AEC_LAMDA_DOWN                 52   // 4.96e-5 Q20
// #define SEVC_AEC_DALTA                      963  // 0.0147 Q16

//bf
#define SEVC_BF_MIC_DISTANCE               29 //mm
#define SEVC_BF_BEAM_COUNT                 3
#define SEVC_BF_TAPS                       1
#define SEVC_BF_GAIN_FLOOR                 1677722   //0.1 W32Q24
//#define SEVC_BF_DELTA                      6554      //0.1 W32Q16
#define SEVC_BF_DELTA                      104308128   //1.591615728102624e+03 W32Q16
#define SEVC_BF_RATIO_TRIG1                754975    //0.36 W32Q21
#define SEVC_BF_RATIO_TRIG2                2097152   //1 W32Q21
#define SEVC_BF_RATIO_TRIG3                524288    //0.25 W32Q21
//#define SEVC_BF_MU                         13421773   //0.8 W32Q24
#define SEVC_BF_MU                         16777216   //1 W32Q24

//gsc
#define SEVC_GSC_TAPS                      1
#define SEVC_GSC_MU                        26214 // 0.4 Q16
#define SEVC_GSC_DELTA_GAIN                2097152 // 32 Q16
//#define SEVC_GSC_DELTA_GAIN                1048576 // 16 Q16
#define SEVC_GSC_DELTA_MIN                 1311    // 0.02 Q16
#define SEVC_GSC_DELTA_MAX                 137438953 //Q16

//bf post
#define SEVC_BFPOST_MCRA_AS                13421773 // 0.8 W32Q24
#define SEVC_BFPOST_MCRA_AP                1677722  // 0.1 W32Q24
#define SEVC_BFPOST_MCRA_DELTA             5        // 5   W32Q0
#define SEVC_BFPOST_MCRA_AD                13421773 // 0.8 W32Q24
#define SEVC_BFPOST_ALPHA                  15435039 // 0.92W32Q24
#define SEVC_BFPOST_PRIOR_MIN              53054    // 10^(-2.5) W32Q24
#define SEVC_BFPOST_GAIN_FLOOR             2983458  // 10^(-0.75)W32Q24
#define SEVC_BFPOST_RATIO_YUZHI            20132659 // 1.2 W32Q24
#define SEVC_BFPOST_PESAI_ZERO             4194304  // 0.25W32Q24
#define SEVC_BFPOST_BETA                   33218887 // 1.98W32Q24
#define SEVC_BFPOST_OMEGA_LOW              8388608  // 0.5 W32Q24
#define SEVC_BFPOST_OMEGA_HIGH             33554432 // 2   W32Q24
//aux
#define SEVC_AUX_BIN1                      19
#define SEVC_AUX_BIN2                      29
#define SEVC_AUX_ALPHAGAIN                 996147 // 0.95 Q20

//nr
#define SEVC_NR_MCRA_AD                    629146
#define SEVC_NR_GAIN_TRIG                  104858
#define SEVC_NR_GAIN_FLOOR                 524288

//nn
// #define SEVC_NN_GAIN_FLOOR                 329853488333 // 0.3Q40
#define SEVC_NN_GAIN_FLOOR                 109951162777 // 0.1Q40
#define SEVC_NN_WIND_GAIN_FLOOR            329853488333 // 0.1Q40 //54975581389 //Q40

// vad
#define SEVC_VAD_AMTH                      164 // 5e-3 Q15
#define SEVC_VAD_SILSPE                    1
#define SEVC_VAD_SPESIL                    25
// sd
#define SEVC_SD_BINSTART                   3
#define SEVC_SD_BINEND                     19 // off the end
#define SEVC_SD_ALPHA                      26214 // 0.8 Q15
#define SEVC_SD_WINDTRIG                   26214 // 0.4 Q16 //32113 // 0.49 Q16
#define SEVC_SD_WEARTRIG                   131072 // 2 Q16
#define SEVC_SD_WINDSMOOTH                 29491 // 0.9 Q15
#define SEVC_SD_WINDQUIT                   100
#define SEVC_SD_QUITWIND                   40
#define SEVC_SD_TIGHTLOOSE                 300
#define SEVC_SD_LOOSETIGHT                 50
#define SEVC_SD_ALPHAWEARLEVELUP           64881 // 0.99 Q16
#define SEVC_SD_ALPHAWEARLEVELDOWN         65208 // 0.995 Q16
#define SEVC_SD_WINDSWITCH                 1
#define SEVC_SD_WEARSWITCH                 1
//aes
#define SEVC_AES_GAMMA                      209715 // 0.2 Q20 // 838861 // 0.8 Q20
#define SEVC_AES_ALPHA                      655360 // 10 Q16
#define SEVC_AES_ALPHAMAX                   1966080 // 30 Q16
#define SEVC_AES_ALPHAMIN                   524288 // 8 Q16
#define SEVC_AES_GAINFLOOR                  104858 // Q20

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
// #define SEVC_AGC_GAIN_LOW                   -2130706432
#define SEVC_AGC_GAIN_LOW                   -1006632960 // -60 Q24
//1677722=0.1*(1<<24); 11744051=0.7*(1<<24); 8388608=0.5*(1<<24);
#define SEVC_AGC_RATIO1                     16777216 // 1.0 Q24
#define SEVC_AGC_RATIO2                     1677722 //0.1;W32Q24
#define SEVC_AGC_GAINTHR                    -50331648 // -3 Q24
#define SEVC_AGC_T1                         -1006632960 // -60; W32Q24
#define SEVC_AGC_T2                         -201326592 //-12;W32Q24
#define SEVC_AGC_T2_GAIN                    100663296 // 6 Q24
#define SEVC_AGC_VAD_SMOOTH_SHORT_THR       838861 //0.05; W32Q24
#define SEVC_AGC_VAD_SMOOTH_LONG_THR        3355443 //0.2; W32Q24
#define SEVC_AGC_VAD_THRESH                 3355443 //0.2; W32Q24
//Gain_smooth_parameters
#define SEVC_AGC_ALPHA_ATTACK_BEGIN_VAD     8305433  //W32Q24  fs=16000 frameShift=256 agcAttackTimeBeginVAD=20
#define SEVC_AGC_ALPHA_ATTACK_IN_VAD        11804323
#define SEVC_AGC_ALPHA_RELEASE_IN_VAD       16186033 //16747751; //W32Q24 fs=16000 frameShift=160 agcAlphaRInVAD=5
#define SEVC_AGC_ALPHA_RELEASE_SIL          16186033 //W32Q24 fs=16000 frameShift=160 agcAlphaRSil=3


#endif

