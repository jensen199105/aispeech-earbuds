#ifndef WTK_MATH_WTK_MATH_H_
#define WTK_MATH_WTK_MATH_H_

#ifdef AISP_TSL_INFO
#include <stdio.h>
#endif

#include "AISP_TSL_types.h"
#include "AISP_TSL_common.h"
#include "AISP_TSL_math.h"
#include "wtk_vector.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PZERO   (0x80000000)        /* -2^31 */

/* ZeroMeanFrame: remove dc offset from given vector */
void wtk_vector_zero_mean_frame(wtk_short_vector_t *v);

void wtk_math_do_diff(aisp_s32_t **pv, int window_size, aisp_s16_t sigma, int start_pos, int step);
int   wtk_fb_fftr(short *out, short *in, int len);
void  wtk_fb_fftr32(int *out, int *in, int len);
short wtk_fxlog(int x);   // fixed log , input: Q0 , output: Q10;
int fxlog_q31(int x);

#ifdef __cplusplus
};
#endif
#endif
