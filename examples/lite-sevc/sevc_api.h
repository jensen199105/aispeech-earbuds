#ifndef _SEVC_API_H_
#define _SEVC_API_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#define SEVC_VISIBLE_ATTR
#else
#define SEVC_VISIBLE_ATTR    __attribute__ ((visibility ("default")))
#endif

void *SEVC_API_New(const char *pcResouce, char *pcBuff, unsigned int uiBuffSize, unsigned int uiSampleRate);

char SEVC_API_Feed(void *pstAecApi, unsigned char *pucBuff, unsigned int uiBuffSize);

void SEVC_API_CallbackRegister(void *pstSevcApi, void *pCBFunc, void *pUsrData);

void SEVC_API_CallbackRegisterMiddle(void *pstSevcApi, void *pCBFunc, void *pUsrData);

void SEVC_API_Reset(void *pstAecApi);

void SEVC_API_Delete(void *pstAecApi);

short SEVC_API_MemSize(void *pstAecApi);

unsigned int SEVC_API_FrameSize(void *pstAecApi);

short SEVC_API_MicNum(void *pstAecApi);

short SEVC_API_WavChan(void *pstAecApi);

short SEVC_API_Hdr(void *pstAecApi);

unsigned int SEVC_API_MemSizeGet(const char *pcResouce, unsigned int uiSampleRate);

char *SEVC_API_Version(void);

void SEVC_API_ParamsShow(void *pstSevcApi);

int SEVC_API_IoCtrl(void *pvArg, int iType, void *pvBuf, int *len);

void SEVC_API_GetWindStatus(void *pstSevcApi, unsigned int *windStatus, int *windSpeed);

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
