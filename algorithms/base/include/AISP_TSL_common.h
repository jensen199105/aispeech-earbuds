/************************************************************
  Copyright (C), AISpeechTech. Co., Ltd.

  FileName   : AISP_TSL_types.h
  Project    :
  Module     :
  Version    :
  Date       : 2019/04/28
  Author     : Chao.Xu
  Document   :
  Description:

  Function List :

  History :
  <author>      <time>            <version >        <desc>
  Chao.Xu       2019/04/28        1.00              Create

************************************************************/
#ifndef __AISP_TSL_COMMON_H__
#define __AISP_TSL_COMMON_H__

#define AISP_TSL_MAX(a, b)              ((a) > (b) ? (a) : (b))
#define AISP_TSL_MIN(a, b)              ((a) < (b) ? (a) : (b))
#define AISP_TSL_ABS(a)                 ((a) < 0 ? (-(a)): (a))

#ifdef AISPEECH_FIXED
#define SHL(a,shift)        ((a) << (shift))
#define SHR(a,shift)        ((a) >> (shift))

#if 0
#define PSHR(a,shift)                   ((a)>=0?(SHR((a)+(1<<(shift-1)),shift)):\
                                                (SHR((a)+(1<<(shift-1))-1,shift)))
#define PSHR64(a,shift)         ((a)>=0?(SHR((a)+(((aisp_s64_t)1<<shift)>>1),shift)):\
                                 (SHR((a)+(((aisp_s64_t)1<<shift)>>1)-1,shift)))
#else
#define PSHR(a,shift)       ((a) >> (shift))
#define PSHR64(a,shift)     ((a) >> (shift))
#endif

#define PSHR_POSITIVE(a,shift)          (SHR((a)+(1<<(shift-1)),shift))
#define PSHR_NEGTIVE(a, shift)          (SHR((a)+(1<<(shift-1))-1,shift))

#define PDIV(a,b)                       (((a)>=0) ? (((a)+((b)>>1))/(b)) : (((a)-((b)>>1))/(b)))
#define PDIV64(a,b)                     (((a)>=0)?((long long)(a)+((long long)(b)>>1))/((long long)(b)):\
                                               ((long long)(a)-((long long)(b)>>1))/((long long)(b)))
#endif
#define AISP_ALIGN_8_MASK               (7)
#define AISP_ALIGN_SIZE(size, mask)     ((size + mask) & ~mask)
#define AISP_MEM_ALIGN(addr, align)     (void *)((align - ((size_t)addr%align))%align + (size_t)addr)
#define AISP_TSL_FLOOR(NUM, ALIGN)      ((NUM) & ~(ALIGN - 1))

#define AISP_PTR_CHECK(PTR, FLAG)       do \
                                        {\
                                            if (NULL == PTR) \
                                            {\
                                                goto FLAG;  \
                                            }\
                                        } while(0)
#define AISP_PTR_FREE(PTR)              do \
                                        {\
                                            if (PTR)\
                                            {\
                                                free(PTR);\
                                                PTR = NULL;\
                                            }\
                                        } while(0)

#endif
