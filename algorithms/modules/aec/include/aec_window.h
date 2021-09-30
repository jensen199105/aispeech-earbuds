#ifndef __AISP_TSL_WINDOW_H__
#define __AISP_TSL_WINDOW_H__

#include "AISP_TSL_common.h"
#include "AISP_TSL_types.h"
S32    AISP_TSL_hanningWindow(U32 N, S32 * pdBuf);
aisp_s16_t *AISP_TSL_hanningWindowWithSqrt(U32 N);
aisp_s16_t AISP_TSL_hanningWindowInit(U32 N, aisp_s16_t * pdBuf);

//aisp_s16_t hanning_sqrt_256[256];
//aisp_s16_t hanning_sqrt_512[512];
//aisp_s16_t hanning_sqrt_1024[1024];



#endif
