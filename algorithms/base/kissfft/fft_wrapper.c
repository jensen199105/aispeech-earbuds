#include "fft_wrapper.h"

#ifdef STATIC_MEM_FIXED_FFT
#include "kiss_fft_table.h"

#ifdef _WIN32
#define __attribute__(x)
#endif

__attribute__((aligned(8))) char aisp_fftHeap[4688] = {0};

/*  facbuf is populated by p1,m1,p2,m2, ...
 *  where
 *  p[i] * m[i] = m[i-1]
 *  m0 = n                  */
void kf_factor_fixed(int n, int *facbuf)
{
    int p = 4;
#if 0
    double floor_sqrt = 0.0f;
    floor_sqrt = floor(sqrt((double)n));
#else
    int floor_sqrt = 22; /* n = 512 by default */
#endif

    /*factor out powers of 4, powers of 2, then any remaining primes */
    do
    {
        while (n % p)
        {
            switch (p)
            {
                case 4:
                    p = 2;
                    break;

                case 2:
                    p = 3;
                    break;

                default:
                    p += 2;
                    break;
            }

            if (p > floor_sqrt)
            {
                p = n;    /* no more factors, skip to end */
            }
        }

        n /= p;
        *facbuf++ = p;
        *facbuf++ = n;
    }
    while (n > 1);
}
#endif


#ifndef USE_NE10
char AISP_TSL_fft_alloc(AISP_TSL_FFTENG_S *wrapper, int fftSize)
{
#ifdef STATIC_MEM_FIXED_FFT
    /* alloc a whole memory to avoid using malloc and use fft table instead of float calculation */
    char *pcRdmaFftHeap = aisp_fftHeap;
    int iMemSizeAlloc = 0;
    /* fftCfg size */
    wrapper->fftCfg = (NR_kiss_fftr_cfg)pcRdmaFftHeap;
    iMemSizeAlloc += sizeof(struct NR_kiss_fftr_state);
    wrapper->fftCfg->substate = (NR_kiss_fft_cfg)(pcRdmaFftHeap + iMemSizeAlloc);
    iMemSizeAlloc += sizeof(struct NR_kiss_fft_state);
    wrapper->fftCfg->tmpbuf = (NR_kiss_fft_cpx *)(pcRdmaFftHeap + iMemSizeAlloc);
    iMemSizeAlloc += sizeof(NR_kiss_fft_cpx) * (fftSize >> 1);     /* tmpbuf */
    /* ifftCfg size */
    wrapper->ifftCfg = (NR_kiss_fftr_cfg)(pcRdmaFftHeap + iMemSizeAlloc);
    iMemSizeAlloc += sizeof(struct NR_kiss_fftr_state);
    wrapper->ifftCfg->substate = (NR_kiss_fft_cfg)(pcRdmaFftHeap + iMemSizeAlloc);
    iMemSizeAlloc += sizeof(struct NR_kiss_fft_state);
    wrapper->ifftCfg->tmpbuf = (NR_kiss_fft_cpx *)(pcRdmaFftHeap + iMemSizeAlloc);
    iMemSizeAlloc += sizeof(NR_kiss_fft_cpx) * (fftSize >> 1);
    /* Init the FFTENG struction for fftCfg */
    wrapper->fftCfg->substate->nfft    = fftSize >> 1;
    wrapper->fftCfg->substate->inverse = 0;
    kf_factor_fixed(fftSize >> 1, &(wrapper->fftCfg->substate->factors[0]));
    /* Init the FFTENG struction for ifftCfg */
    wrapper->ifftCfg->substate->nfft       = fftSize >> 1;
    wrapper->ifftCfg->substate->inverse    = 1;
    kf_factor_fixed(fftSize >> 1, &(wrapper->ifftCfg->substate->factors[0]));

    if (fftSize == 512)
    {
        wrapper->fftCfg->substate->twiddles = (NR_kiss_fft_cpx *)fft_table_512;
        wrapper->fftCfg->super_twiddles     = (NR_kiss_fft_cpx *)fft_supertable_512;
        wrapper->ifftCfg->substate->twiddles = (NR_kiss_fft_cpx *)ifft_table_512;
        wrapper->ifftCfg->super_twiddles     = (NR_kiss_fft_cpx *)ifft_supertable_512;
    }
    else
    {
        wrapper->fftCfg->substate->twiddles = (NR_kiss_fft_cpx *)fft_table_256;
        wrapper->fftCfg->super_twiddles     = (NR_kiss_fft_cpx *)fft_supertable_256;
        wrapper->ifftCfg->substate->twiddles = (NR_kiss_fft_cpx *)ifft_table_256;
        wrapper->ifftCfg->super_twiddles     = (NR_kiss_fft_cpx *)ifft_supertable_256;
    }

#else
    wrapper->fftCfg = NR_kiss_fftr_alloc(fftSize, 0, 0, 0);

    if (NULL == wrapper->fftCfg)
    {
#ifdef AISP_TSL_INFO
        printf("KISS fft config alloc fail.\n");
#endif
        return -1;
    }

    wrapper->ifftCfg = NR_kiss_fftr_alloc(fftSize, 1, 0, 0);

    if (NULL == wrapper->ifftCfg)
    {
#ifdef AISP_TSL_INFO
        printf("KISS ifft config alloc fail.\n");
#endif
#ifndef STATIC_MEM_FIXED_FFT
        free(wrapper->fftCfg);
#endif
        return -1;
    }

#endif
    return 0;
}

void AISP_TSL_fft_free(AISP_TSL_FFTENG_S *wrapper)
{
#ifndef STATIC_MEM_FIXED_FFT

    if (wrapper)
    {
        if (wrapper->fftCfg)
        {
            free(wrapper->fftCfg);
            wrapper->fftCfg = NULL;
        }

        if (wrapper->ifftCfg)
        {
            free(wrapper->ifftCfg);
            wrapper->ifftCfg = NULL;
        }
    }

#endif
}

#else
char AISP_TSL_fft_alloc(AISP_TSL_FFTENG_S *wrapper, int fftSize)
{
    wrapper->fftCfg = ne10_fft_alloc_r2c_float32(fftSize);

    if (wrapper->fftCfg)
    {
        return 0;
    }
    else
    {
#ifdef AISP_TSL_INFO
        printf("NE10 alloc fail");
#endif
        return -1;
    }
}

void AISP_TSL_fft_free(AISP_TSL_FFTENG_S *wrapper)
{
    if (wrapper && wrapper->fftCfg)
    {
        free(wrapper->fftCfg);
        wrapper->fftCfg = NULL;
    }
}

#endif


