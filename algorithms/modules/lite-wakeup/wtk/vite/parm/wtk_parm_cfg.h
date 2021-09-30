#ifndef WTK_MATH_WTK_PARM_CFG_H_
#define WTK_MATH_WTK_PARM_CFG_H_

#include "wtk/vite/math/wtk_vector.h"

#if defined(WAKEUP_NN_DNN)
#include "wtk/vite/parm/post/dnn/wtk_dnn_cfg.h"
#elif defined(WAKEUP_NN_FSMN)
#include "wtk/vite/parm/post/fsmn/wtk_fsmn_cfg.h"
#endif
#include "wtk/vite/parm/post/cmn/wtk_zmean_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct wtk_parm_cfg wtk_parm_cfg_t;
#define MAX_PARM_DIFFER_WIN 11

struct wtk_parm_cfg
{
    // dependent cfg
    int window_size;        // WINDOWSIZE in 100ns
    int window_step;        // TARGETRATE in 100ns
    char use_hamming;
    char use_hanning;
    int src_sample_rate;    // SOURCERATE   100ns unit ns=10*-9
    // calc cfg
    int frame_size;         // samples for window
    int frame_step;         // samples  for frame step;
    char fbank_num_chans;   // NUMCHANS
    int numCepCoef;         /* NUMCEPS: Number of cepstral coef */
    int cepLifter;          // CEPLIFTER
    int lpc_order;          // LPCORDER
    int feature_basic_cols; // basic feature cols;
    int del_win;            // DELTAWINDOW
    int acc_win;            // ACCWINDOW
    int third_win;          // THIRDWINDOW
    // fbankinfo configure.
    // float preEmph;         // PREEMCOEF
    // float cepScale;        // CEPSCALE: Scaling factor to avoid arithmetic problems */
    // float loFBankFreq;     /* LOFREQ: Fbank lo frequency cut-off */
    // float hiFBankFreq;     /* HIREQ: Fbank hi frequency cut-off */
    // float warpFreq;        /* WARPFREQ: Warp freq axis for vocal tract normalisation */
    // float warpLowerCutOff; /* WARPLCUTOFF: lower and upper threshold frequencies */
    // float warpUpperCutOff; /* WARPUCUTOFF: for linear frequency warping */
    // float compressFact;    // COMPRESSFACT  Compression factor for PLP */
    // float ESCALE;
    // float SILFLOOR;
    // float addDither; // ADDDITHER
    short sigma[3];
    //================= information section ===========
    int feature_cols;
    int static_feature_cols;
    //================== post section ===========
    wtk_zmean_cfg_t zmean;
#if defined(WAKEUP_NN_DNN)
    wtk_dnn_cfg_t dnn;
#elif defined(WAKEUP_NN_FSMN)
    wtk_fsmn_cfg_t fsmn;
#endif

    const aisp_s16_t *hwin;
    const aisp_s8_t *loChan;
    const aisp_s16_t *loWt;

    int cache_size;
    int align;
    unsigned use_z : 1;
    unsigned use_cmn : 1;
    //----------------------- sigp section---------------
    unsigned zMeanSrc : 1;   // ZMEANSOURCE:  Zero mean source waveform before analysis */
    unsigned rawEnergy : 1;  // RAWENERGY: Use raw energy before preEmp and ham */
    unsigned use_power : 1;  // USEPOWER
    unsigned fbankFixed : 2; // FBANKFIXED
    unsigned double_fft : 1; // DOUBLEFFT
    // unsigned use_mfcc:1;         //MFCC
    // unsigned use_plp:1;              //PLP
    // unsigned use_fbank:1;            //FABANK
    unsigned ENERGY : 1; // TARGETKIND: E
    unsigned DELTA : 1;  // D
    unsigned NULLE : 1;  // N
    unsigned ACCS : 1;   // A
    unsigned THIRD : 1;  // T
    unsigned ZMEAN : 1;  // Z
    unsigned ZERO : 1;   // 0
    unsigned SIMPLEDIFFS : 1;
};

int wtk_parm_cfg_init(wtk_parm_cfg_t *cfg);
int wtk_parm_cfg_clean(wtk_parm_cfg_t *cfg);

/**
 * @brief used for other loader;
 */
int wtk_parm_cfg_update2(wtk_parm_cfg_t *cfg);
int wtk_parm_cfg_update_local(wtk_parm_cfg_t *cfg);

#ifdef __cplusplus
};
#endif
#endif
