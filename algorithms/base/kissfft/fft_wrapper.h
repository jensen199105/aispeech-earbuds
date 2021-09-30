#ifndef _FFT_WRAPPER_H_
#define _FFT_WRAPPER_H_

#ifndef USE_NE10
#include "kiss_fftr.h"
#include "_kiss_fft_guts.h"
typedef struct tagFFT_Wrapper
{
    NR_kiss_fftr_cfg fftCfg;
    NR_kiss_fftr_cfg ifftCfg;
} AISP_TSL_FFTENG_S;

#ifdef JIELI_BR28
extern void  REAL_FFT_FUN(int *data, int blockbit, int *oudata);
extern void  REAL_IFFT_FUN(int *data, int blockbit, int *oudata);

#define AISP_TSL_FFT_SCALAR(FFTSIZE) (1)
#define AISP_TSL_FFT(WRAPPER, IN, OUT)\
    do{\
            REAL_FFT_FUN(IN, 9, (S32*)OUT);\
    }while(0);

#define AISP_TSL_IFFT(WRAPPER, IN, OUT)\
    do{\
            REAL_IFFT_FUN((int*)IN, 9, (S32*)OUT);\
    }while(0);
#elif (defined JIELI_AC790N)
extern void jl_fft_512(int *in , int *out);
extern void jl_ifft_512(int *in , int *out);

#define AISP_TSL_FFT_SCALAR(FFTSIZE) (1)
#define AISP_TSL_FFT(WRAPPER, IN, OUT)\
    do{\
            jl_fft_512(IN,OUT);\
    }while(0);

#define AISP_TSL_IFFT(WRAPPER, IN, OUT)\
    do{\
            jl_ifft_512(IN,OUT);\
    }while(0);
#else

#define AISP_TSL_FFT_SCALAR(FFTSIZE) (FFTSIZE)

#define AISP_TSL_FFT(WRAPPER, IN, OUT)\
    do{\
            NR_kiss_fftr(WRAPPER.fftCfg, (const NR_kiss_fft_scalar *)(IN), (NR_kiss_fft_cpx*) (OUT));\
    }while(0);

#define AISP_TSL_IFFT(WRAPPER, IN, OUT)\
    do{\
            NR_kiss_fftri(WRAPPER.ifftCfg, (NR_kiss_fft_cpx *)(IN), (NR_kiss_fft_scalar *)(OUT));\
    }while(0);
#endif

#else
#include "NE10_dsp.h"

typedef struct tagFFT_Wrapper
{
    ne10_fft_r2c_cfg_float32_t fftCfg;
} AISP_TSL_FFTENG_S;

#define AISP_TSL_FFT_SCALAR(FFTSIZE) (1)
#define AISP_TSL_FFT(WRAPPER, IN, OUT)  \
    do{\
        ne10_fft_r2c_1d_float32_neon((ne10_fft_cpx_float32_t *)(OUT), (ne10_float32_t *)(IN), (WRAPPER).fftCfg);\
    }while(0)

#define AISP_TSL_IFFT(WRAPPER, IN, OUT)\
    do{\
        ne10_fft_c2r_1d_float32_neon((ne10_float32_t *)(OUT), (ne10_fft_cpx_float32_t *)(IN), (WRAPPER).fftCfg);\
    }while(0)

#endif
char AISP_TSL_fft_alloc(AISP_TSL_FFTENG_S *wrapper, int fftSize);
void AISP_TSL_fft_free(AISP_TSL_FFTENG_S *wrapper);

#endif
