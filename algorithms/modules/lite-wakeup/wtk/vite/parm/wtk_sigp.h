#ifndef WTK_MATH_WTK_SIGP_H_
#define WTK_MATH_WTK_SIGP_H_

#include "kiss_fftr.h"
#include "wtk/core/wtk_type.h"
#include "wtk/vite/math/wtk_vector.h"
#include "wtk/vite/math/wtk_math.h"
#include "wtk_parm_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct wtk_sigp wtk_sigp_t;

typedef struct
{
    int klo;
	int khi;           /* lopass to hipass cut-off fft indices */
    int fftN;
    int *x;            /* array[1..fftN] of fftchans */
}FBankInfo;

struct wtk_sigp
{
    NR_kiss_fftr_cfg fftCfg;
    wtk_parm_cfg_t *cfg;
    FBankInfo fbInfo;       /* FBank info used for filterbank analysis */
    aisp_s32_t *fft_out;
};

int wtk_sigp_init(wtk_sigp_t *s,wtk_parm_cfg_t *cfg);
int wtk_sigp_clean(wtk_sigp_t *s);
int wtk_sigp_procss(wtk_sigp_t *s,wtk_short_vector_t *v,aisp_s32_t *feature);
#ifdef __cplusplus
};
#endif
#endif
