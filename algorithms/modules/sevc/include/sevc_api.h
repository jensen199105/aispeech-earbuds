#ifndef _SEVC_API_H_
#define _SEVC_API_H_

#include "AISP_TSL_types.h"
#ifdef __cplusplus
extern "C" {
#endif
//#include "sevc_func.h"

#ifdef _WIN32
#define SEVC_VISIBLE_ATTR
#else
#define SEVC_VISIBLE_ATTR    __attribute__ ((visibility ("default")))
#endif


VOID *SEVC_API_New(const S8 *pcResouce, S8 *pcBuff, U32 uiBuffSize, U32 uiSampleRate);

S8 SEVC_API_Feed(VOID *pstSevcApi, U8 *pucBuff, U32 uiBuffSize);

VOID SEVC_API_CallbackRegister(VOID *pstSevcApi, VOID *pCBFunc, VOID *pUsrData);

VOID SEVC_API_CallbackRegisterMiddle(VOID *pstSevcApi, VOID *pCBFunc, VOID *pUsrData);

VOID SEVC_API_Reset(VOID *pstSevcApi);

VOID SEVC_API_Delete(VOID *pstSevcApi);

S16 SEVC_API_MemSize(VOID *pstSevcApi);

U32 SEVC_API_FrameSize(VOID *pstSevcApi);

S16 SEVC_API_MicNum(VOID *pstSevcApi);

S16 SEVC_API_WavChan(VOID *pstSevcApi);

S16 SEVC_API_Hdr(VOID *pstSevcApi);

U32 SEVC_API_MemSizeGet(const S8 *pcResouce, U32 uiSampleRate);

S8 *SEVC_API_Version(VOID);

int SEVC_API_IoCtrl(void *pvArg, int iType, void *pvBuf, int *len);

VOID SEVC_API_GetWindStatus(VOID *pstSevcApi, U32 *windStatus, S32 *windSpeed);

typedef enum io_type
{
    IOCTL_GET_INPUT_LEN = 1,
    IOCTL_GET_GAINS,
    IOCTL_GET_FRAME_R_NUM,
    IOCTL_GET_MODEL_INFO,
    IOCTL_GET_SEVC_INFO,
    IOCTL_SET_MIDDLE_FREQ_BINS,

    IOCTL_MAX
} IO_TYPE_E;


#ifdef __cplusplus
};

#endif
#endif
