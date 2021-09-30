#ifndef __AEC_BASE_FIXED_H__
#define __AEC_BASE_FIXED_H__

//#include <math.h>
#include "AISP_TSL_types.h"
#include "AISP_TSL_complex.h"

#ifndef ATTRIBUTE_INLINE
#define ATTRIBUTE_INLINE
#endif
aisp_s32_t aispeech_abs32(aisp_s32_t x);
aisp_s32_t aispeech_sqrt32(aisp_s32_t x);

aisp_s32_t aispeech_abs(aisp_s32_t x);

/* Jiangyh added for aicar */
#define MIN(a,b)            ((a)>(b)?(b):(a))
#define MAX(a,b)            ((a)>(b)?(a):(b))

#define PDIV14(a,b)         ((a)>=0?((((a)<<14)+((b)>>1))/(b)):\
                                 ((((a)<<14)-((b)>>1))/(b)))
#define PDIV15(a,b)         ((a)>=0?((((a)<<15)+((b)>>1))/(b)):\
                                 ((((a)<<15)-((b)>>1))/(b)))

typedef struct get_symbol64
{
    unsigned long long value: 63;
    unsigned long long sign: 1;
} get_symbol64_t;

typedef struct get_symbol32
{
    unsigned int value: 31;
    unsigned int sign: 1;
} get_symbol32_t;

#define aispeech_abs16(x)   (aisp_s16_t)aispeech_abs32(x)
/* according to speex */
#define AISP_Q31_ONE        (2147483647)
#define AISP_Q31_SHIFT      (31)
#define AISP_Q30_ONE        (1073741824)
#define AISP_Q29_ONE        (536870912)
#define AISP_Q17_SHIFT      (17)
#define AISP_Q15_ONE        (32767)
#define AISP_Q15_SHIFT      (15)
#define AISP_Q14_ONE        (16383)
#define AISP_Q14_SHIFT      (14)
#define AISP_Q13_SHIFT      (13)
#define AISP_Q12_SHIFT      (12)
#define AISP_Q11_SHIFT      (11)

#define AISP_FIXED16(a)         (aisp_s16_t)((FLOAT_T)(a) * AISP_Q15_ONE + 0.5)
#define AISP_FIXED32(a)         (aisp_s32_t)((FLOAT_T)(a) * AISP_Q15_ONE + 0.5)
/* here a should be very very small */
#define AISP_FIXED32_2_16(a)    (aisp_s16_t)((FLOAT_T)(a) * AISP_Q31_ONE + 0.5)

#define FIXED_ABS(x)        ((x) < 0 ? (-(x)) : (x))
#define FIXED_MIN(a, b)     ((a) < (b) ? (a) : (b))
#define FIXED_MAX(a, b)     ((a) < (b) ? (b) : (a))

#define MIN16(a,b)          ((a) < (b) ? (a) : (b))   /**< Maximum 16-bit value.   */

#define NEG32(x)            (-(x))
#define ROUND16Q15F32(d, a)     do  \
                                {   \
                                    aisp_s16_t tmp1[4]= {0, 0, 0, 0}; \
                                    ae_int64   aeResult; \
                                    aeResult = AE_SLAA64((ae_int64)(a), 33); \
                                    *(ae_f32x2 *)tmp1 = AE_ROUND32F64SASYM(aeResult); \
                                    d = tmp1[1]; \
                                }while(0)

#define ROUND32Q15F64(d, a)     do  \
                                {   \
                                    aisp_s32_t tmp[2]= {0, 0}; \
                                    ae_int64   aeResult; \
                                    aeResult = AE_SLAA64((ae_int64)(a), 17); \
                                    *(ae_f32x2 *)tmp = AE_ROUND32F64SASYM(aeResult); \
                                    d = tmp[0]; \
                                }while(0)

#define SHL(a,shift)        ((a) << (shift))
#define SHR(a,shift)        ((a) >> (shift))
#define SHL32(a,shift)      ((aisp_s32_t)(a) << (shift))
#define SHL64(a,shift)      ((aisp_s64_t)(a) << (shift))
#define SHR32(a,shift)      ((aisp_s32_t)(a) >> (shift))
/*
#define PSHR(a,shift)       (a>=0?(SHR((a)+((1<<shift)>>1),shift)):(SHR((a)+((1<<shift)>>1)-1,shift)))
                                  */
//#define PSHR(a,shift)       (SHR((a)+((1<<(shift))>>1),(shift)))

#define PSHR16(a,shift)     (SHR16((a)+((1<<((shift))>>1)),(shift)))
#define PSHR32(a,shift)       (a>=0?(SHR((a)+((1<<shift)>>1),shift)):(SHR((a)+((1<<shift)>>1)-1,shift)))
//static inline int PSHR32(int a, int shift)
//              {
//                  get_symbol32_t* aa = (get_symbol32_t*)&a;
//                  return (int)SHR((a)+((1<<((shift))>>1))-aa->sign, (shift));
//              }
#if 1
//#define PSHR64(a,shift)       (SHR((aisp_s64_t)(a)+(((aisp_s64_t)1<<(shift))>>1),(shift)))
#define PSHR64(a,shift)         ((a)>=0?(SHR((a)+(((aisp_s64_t)1<<shift)>>1),shift)):\
                                 (SHR((a)+(((aisp_s64_t)1<<shift)>>1)-1,shift)))
#else
static inline unsigned long long PSHR64(aisp_s64_t a, int shift)
{
    get_symbol64_t *aa = (get_symbol64_t *)&a;
    return SHR((a) + (((long long)1 << shift) >> 1) - aa->sign, (shift));
}
#endif
#define VSHR32(a, shift)    (((shift)>0) ? SHR32(a, shift) : SHL32(a, -(shift)))

#define SHL16(a,shift)      ((a) << (shift))
#define SHR16(a,shift)      ((a) >> (shift))

#define ADD16(a,b)          ((aisp_s16_t)((aisp_s16_t)(a)+(aisp_s16_t)(b)))
#define SUB16(a,b)          ((aisp_s16_t)(a)-(aisp_s16_t)(b))
#define ADD32(a,b)          ((aisp_s32_t)(a)+(aisp_s32_t)(b))
#define SUB32(a,b)          ((aisp_s32_t)(a)-(aisp_s32_t)(b))

#define MULT16_16(a,b)      (((aisp_s32_t)(aisp_s16_t)(a))*((aisp_s32_t)(aisp_s16_t)(b)))
#define MULT16_32(a,b)      ((aisp_s32_t)((aisp_s16_t)(a))*((aisp_s32_t)(b)))
#define MULT16_16_P13(a,b)  (SHR(ADD32(4096,MULT16_16((a),(b))),13))
#define MULT16_16_P15(a,b)  (PSHR(MULT16_16((a),(b)),15))
#define MULT16_16_P15_OPT(c, a, b)  do  \
                                    {   \
                                        aisp_s64_t tmp64; \
                                        aisp_s16_t tmp1[4]= {0, 0, 0, 0}; \
                                        ae_int64   aeResult; \
                                        tmp64 = (aisp_s32_t)((aisp_s16_t)(a)) * (aisp_s32_t)((aisp_s16_t)(b));\
                                        aeResult = AE_SLAA64((ae_int64)tmp64, 33); \
                                        *(ae_f32x2 *)tmp1 = AE_ROUND32F64SASYM(aeResult); \
                                        c = tmp1[1]; \
                                    }while(0)
#define MULT16_32_P15(a,b)  ((aisp_s32_t)(PSHR(MULT16_32((a),(b)),15)))

#define MULT16_32_Q15(a,b)  ADD32(MULT16_16((a),SHR((b),15)), SHR(MULT16_16((a),((b)&0x00007fff)),15))

#define MULT16_16_Q14(a,b)  (SHR(MULT16_16((a),(b)),14))
#define MULT16_16_P14(a,b)  (SHR(ADD32(8192,MULT16_16((a),(b))),14))

#define ADD64(a,b)          ((aisp_s64_t)(a)+(aisp_s64_t)(b))
#define MULT32_32(a,b)      (((aisp_s64_t)(aisp_s32_t)(a))*((aisp_s64_t)(aisp_s32_t)(b)))
#define MULT32_32_P15(a,b)  ((aisp_s32_t)(PSHR(MULT32_32((a),(b)) , 15)))
#define MULT32_32_P15_OPT(c,a,b)    do \
                                    { \
                                        aisp_s32_t tmp[2]; \
                                        aisp_s64_t tmp64; \
                                        ae_int64   aeResult; \
                                        tmp64 = (aisp_s64_t)((aisp_s32_t)(a)) * (aisp_s64_t)((aisp_s32_t)(b));\
                                        aeResult = AE_SLAA64((ae_int64)tmp64, 17); \
                                        *(ae_f32x2 *)tmp = AE_ROUND32F64SASYM(aeResult); \
                                        c = tmp[0]; \
                                    }while(0)

/* Jiangyh added */

#define PDIV32_16(a,b)      ((a>=0)?((aisp_s16_t)(((aisp_s32_t)(a)+((aisp_s16_t)(b)>>1))/((aisp_s16_t)(b))))\
                                                    :((aisp_s16_t)(((aisp_s32_t)(a)-((aisp_s16_t)(b)>>1))/((aisp_s16_t)(b)))))
#define PDIV32_16_POSITIVE(a,b)      ((aisp_s16_t)(((aisp_s32_t)(a)+((aisp_s16_t)(b)>>1))/((aisp_s16_t)(b))))

#define PDIV32_32(a,b)          (((a)>=0)?((aisp_s32_t)(((aisp_s64_t)(a)+((aisp_s32_t)(b)>>1))/((aisp_s32_t)(b))))\
                                   :((aisp_s32_t)(((aisp_s64_t)(a)-((aisp_s32_t)(b)>>1))/((aisp_s32_t)(b)))))
#define PDIV32(a,b)         ((a>=0)?((aisp_s32_t)(a)+((aisp_s16_t)(b)>>1))/((aisp_s16_t)(b))\
                                                    :((aisp_s32_t)(a)-((aisp_s16_t)(b)>>1))/((aisp_s16_t)(b)))

#define PDIV64_POSITIVE(a,b)         (((aisp_s64_t)(a)+((aisp_s32_t)(b)>>1))/((aisp_s32_t)(b)))

#define PDIV16_16_P15(a,b)  ((a>=0)?((SHL((aisp_s32_t)(a),AISP_Q15_SHIFT)+SHR16((aisp_s16_t)(b),1))/((aisp_s16_t)(b)))\
                                                    :((SHL((aisp_s32_t)(a),AISP_Q15_SHIFT)-SHR16((aisp_s16_t)(b),1))/((aisp_s16_t)(b))))
#define PDIV32_16_P15(a,b)  ((a>=0)?((SHL((aisp_s64_t)(a),AISP_Q15_SHIFT)+SHR16((aisp_s16_t)(b),1))/((aisp_s16_t)(b)))\
                                                    :((SHL((aisp_s64_t)(a),AISP_Q15_SHIFT)-SHR16((aisp_s16_t)(b),1))/((aisp_s16_t)(b))))
#define PDIV32_32_P15(a,b)  ((a>=0)?((SHL((aisp_s64_t)(a),AISP_Q15_SHIFT)+SHR32((aisp_s32_t)(b),1))/((aisp_s32_t)(b)))\
                                                    :((SHL((aisp_s64_t)(a),AISP_Q15_SHIFT)-SHR32((aisp_s32_t)(b),1))/((aisp_s32_t)(b))))

#define EXTRACT16(x)        ((aisp_s16_t)(x))
#define EXTEND32(x)         ((aisp_s32_t)(x))


#endif
