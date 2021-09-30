#ifndef __AEC_API_H__
#define __AEC_API_H__
//#include "naes.h"



void *aec_api_new(const char *fileName);

void aec_api_delete(void *pstAecApi);

short aec_api_memSize(void *pstAecApi);

short aec_api_micNum(void *pstAecApi);

short aec_api_wavChan(void *pstAecApi);

short aec_api_freqOut(void *pstAecApi);

char aec_api_feed(void *pstAecApi, unsigned char *pPcm, short memBlockLen);

int aec_api_frameSize(void *pstAecApi);

void aec_api_reset(void *pstAecApi);

void aec_api_pop(void *pstAecApi, short *pErr, short index);

char aec_api_pesudo_pop(void *pstAecApi, void *data, short *pErr, short index);

char *aec_api_version(void);


#endif
