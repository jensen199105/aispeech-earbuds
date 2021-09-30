#ifndef __AISP_TSL_FFT_H__
#define __AISP_TSL_FFT_H__

#include "AISP_TSL_complex.h"
#include "AISP_TSL_types.h"

typedef struct aisp_fft_eng
{
    short  len;
    short  ncfft;
    short  width;
    short  stage_num;
    short *pIdx_tab;
    aisp_cpx_s16_t *twiddles;
    aisp_cpx_s16_t *super_twiddles;

    aisp_cpx_s32_t *cpx_tmp ;
    aisp_cpx_s32_t *tmp_buf ;
} aisp_fft_eng_t;

aisp_fft_eng_t *AISP_TSL_fft32_init(short nfft);
void AISP_TSL_fft_destroy(aisp_fft_eng_t *pstFftEng);
void AISP_TSL_fftr32(aisp_fft_eng_t *pstFftEng, const aisp_s16_t *time_data, aisp_cpx_s32_t *freq_data);
int  AISP_TSL_ifftr32(aisp_fft_eng_t *pstFftEng, aisp_cpx_s32_t *freq_data, aisp_s16_t *time_data);

#endif
