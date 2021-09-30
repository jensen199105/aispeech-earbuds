/************************************************************
  Copyright (C), AISpeechTech. Co., Ltd.

  FileName   : fend_pcm2wav.c
  Project    :
  Module     :
  Version    :
  Date       : 2016/11/30
  Author     : Youhai.Jiang
  Document   :
  Description:

  Function List :

  History :
  <author>      <time>            <version >        <desc>
  Youhai.Jiang  2016/11/30         1.00              Create

************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "AISP_TSL_str.h"

#define AISP_TSL_FUNC_ATTR __attribute__((visibility("default")))
typedef   struct
{
    char         fccID[4];
    unsigned int dwSize;
    char         fccType[4];
} AISP_TSL_head_t;

typedef   struct
{
    char       fccID[4];
    unsigned   int       dwSize;
    unsigned   short     wFormatTag;
    unsigned   short     wChannels;
    unsigned   int       dwSamplesPerSec;
    unsigned   int       dwAvgBytesPerSec;
    unsigned   short     wBlockAlign;
    unsigned   short     uiBitsPerSample;
} AISP_TSL_fmt_t;

typedef   struct
{
    char           fccID[4];
    unsigned int   dwSize;
} AISP_TSL_data_t;

/************************************************************
  Function   : fend_pcmAddWavHeader()

  Description: pcm to wav
  Calls      :
  Called By  :
  Input      : char *dst_file
               int channels
               int bits
               int sample_rate
               int len
  Output     :
  Return     :
  Others     :

  History    :
    2016/02/04, youhai create

************************************************************/
AISP_TSL_FUNC_ATTR int AISP_TSL_pcmAddWavHeader(FILE *fp, int channels, int bits, int sample_rate, int len)
{
    AISP_TSL_head_t pcmHEADER;
    AISP_TSL_fmt_t  pcmFMT;
    AISP_TSL_data_t pcmDATA;

    if (NULL == fp)
    {
#ifdef AISP_TSL_INFO
        printf("Input file ptr is null.\n");
#endif
        return -1;
    }

    AISP_TSL_memcpy(pcmHEADER.fccID, "RIFF", AISP_TSL_strlen("RIFF"));
    AISP_TSL_memcpy(pcmHEADER.fccType, "WAVE", AISP_TSL_strlen("WAVE"));
    pcmHEADER.dwSize = 36 + len;
    fseek(fp, 0, SEEK_SET);
    fwrite(&pcmHEADER, sizeof(AISP_TSL_head_t), 1, fp);
    pcmFMT.dwSamplesPerSec = sample_rate;
    pcmFMT.uiBitsPerSample = bits;
    AISP_TSL_memcpy(pcmFMT.fccID, "fmt ", AISP_TSL_strlen("fmt "));
    pcmFMT.dwSize = 16;
    pcmFMT.wChannels = channels;
    pcmFMT.wBlockAlign = bits / 8;
    pcmFMT.wFormatTag = 1;
    pcmFMT.dwAvgBytesPerSec = pcmFMT.dwSamplesPerSec * pcmFMT.wBlockAlign;
    fseek(fp, sizeof(AISP_TSL_head_t), SEEK_SET);
    fwrite(&pcmFMT, sizeof(AISP_TSL_fmt_t), 1, fp);
    /* update header */
    AISP_TSL_memcpy(pcmDATA.fccID, "data", AISP_TSL_strlen("data"));
    pcmDATA.dwSize = len;
    fseek(fp, sizeof(AISP_TSL_fmt_t) + sizeof(AISP_TSL_head_t), SEEK_SET);
    fwrite(&pcmDATA, sizeof(AISP_TSL_data_t), 1, fp);
    return 0;
}

