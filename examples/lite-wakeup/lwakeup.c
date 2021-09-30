/************************************************************
  Copyright (C), AISpeechTech. Co., Ltd.

  FileName   : lwakeup.c
  Project    :
  Module     :
  Version    :
  Date       : 2019/07/22
  Author     : Youhai.Jiang
  Document   :
  Description:

  Function List :

  History :
  <author>      <time>            <version >        <desc>
  Youhai.Jiang  2019/07/22         1.00              Create

************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <Windows.h>
#endif
#include "lwakeup.h"

#define FEED_SIZE						(640)
#define MEM_MAX_SIZE               		(120*1024)

typedef struct content {
    wakeup_t* pstWkp;
    FILE* pstWavFile;
} content_t;

static int _handler(void *obj, int status, char *json, int bytes)
{
    printf("%s\n",json);
    LWKP_reset((wakeup_t*)obj);
    return 0;
}

/*
 * if _feature_handler return 1, it means features will not feed to wakeup module
 */
static int _feature_handler(void* obj, int* f, int count)
{
	return 0;
}

static void feed_offline(content_t* pstCtx)
{
    int iTotalLen = 0;

    while (!feof(pstCtx->pstWavFile))
    {
        char buf[FEED_SIZE];
        int iLen;
        int ret;

        iLen = fread(buf, 1, FEED_SIZE, pstCtx->pstWavFile);
        if (!iLen)
        {
            break;
        }
        iTotalLen += iLen;

        ret = LWKP_feed(pstCtx->pstWkp, buf, iLen);
		if (ret < 0)
		{
			printf("failed to feed data\n");
			break;
		}
	}
}

#ifdef _WIN32
#pragma comment(lib, "WINMM.LIB")

static DWORD wave_callback(HWAVEIN hWaveIn, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
	content_t* pstCtx = (content_t*)dwInstance;
	PWAVEHDR pWhdr = (PWAVEHDR)dwParam1;
	MMRESULT ret;

	switch (uMsg)
	{
	case WIM_OPEN:
		break;
	case WIM_DATA:
		ret = LWKP_feed(pstCtx->pstWkp, pWhdr->lpData, pWhdr->dwBytesRecorded);
		if (ret < 0)
		{
			printf("failed to feed data\n");
			break;
		}

		ret = fwrite(pWhdr->lpData, pWhdr->dwBytesRecorded, 1, pstCtx->pstWavFile);
		if (ret != 1)
		{
			printf("failed to write data\n");
			break;
		}
		fflush(pstCtx->pstWavFile);

		ret = waveInAddBuffer(hWaveIn, pWhdr, sizeof(WAVEHDR));
		if (MMSYSERR_NOERROR != ret)
		{
			printf("failed to set wave in buffer\n");
			break;
		}

		break;
	case WIM_CLOSE:
		waveInStop(hWaveIn);
		waveInReset(hWaveIn);
		waveInClose(hWaveIn);

		if (NULL != pstCtx->pstWavFile)
			fclose(pstCtx->pstWavFile);
		exit(0);
		break;
	default:
		break;
	}

	return 0;
}

static void feed_online(content_t* pstCtx)
{
	HWAVEIN hWaveIn;
	WAVEFORMATEX stWfx;
	char* pcBuf = NULL;
	WAVEHDR stWhdr;
	MMRESULT ret;

	memset(&stWfx, 0, sizeof(stWfx));
	memset(&stWhdr, 0, sizeof(stWhdr));

	stWfx.wFormatTag = WAVE_FORMAT_PCM;
	stWfx.nChannels = 1;
	stWfx.nSamplesPerSec = 16000;
	stWfx.nAvgBytesPerSec = 32000;
	stWfx.nBlockAlign = 2;
	stWfx.wBitsPerSample = 16;
	stWfx.cbSize = 0;

	ret = waveInOpen(&hWaveIn, WAVE_MAPPER, &stWfx, (DWORD_PTR)wave_callback, (DWORD_PTR)pstCtx, CALLBACK_FUNCTION);
	if (MMSYSERR_NOERROR != ret)
	{
		printf("failed to open mic\n");
		goto error;
	}

	pcBuf = malloc(FEED_SIZE);
	if (NULL == pcBuf)
	{
		printf("no more memory\n");
		goto error;
	}

	stWhdr.lpData = pcBuf;
	stWhdr.dwBufferLength = FEED_SIZE;

	ret = waveInPrepareHeader(hWaveIn, &stWhdr, sizeof(stWhdr));
	if (MMSYSERR_NOERROR != ret)
	{
		printf("failed to prepare wave in header\n");
		goto error;
	}

	ret = waveInAddBuffer(hWaveIn, &stWhdr, sizeof(stWhdr));
	if (MMSYSERR_NOERROR != ret)
	{
		printf("failed to set wave in buffer\n");
		goto error;
	}

	ret = waveInStart(hWaveIn);
	if (MMSYSERR_NOERROR != ret)
	{
		printf("failed to start wave in\n");
		goto error;
	}

	while (TRUE)
	{
		MSG stMsg;

		if (GetMessage(&stMsg, NULL, 0, 0, PM_REMOVE))
		{
			if (stMsg.message == WM_QUIT)
				break;
			TranslateMessage(&stMsg);
			DispatchMessage(&stMsg);
		}
	}

	return;
error:
	if (pcBuf)
		free(pcBuf);
	if (INVALID_HANDLE_VALUE != hWaveIn)
		waveInClose(hWaveIn);
}
#else
static void feed_online(content_t* pstCtx)
{
	//TODO: implement
}
#endif

int main(int argc, char *argv[])
{
    const int memsize = MEM_MAX_SIZE;
    content_t stCtx;
    char* wavFile;
    char* pcWakeupWords;
    char *pmembase = NULL;
    int iOnline = 0;
    int ret = -1;

    memset(&stCtx, 0, sizeof(stCtx));

    if (argc == 3 || argc == 4)
    {
        wavFile = argv[1];
        pcWakeupWords = argv[2];

        if (argc == 4)
        {
            iOnline = !!atoi(argv[3]);
        }
    }
    else
    {
        printf("***************************************************\n"
               "version: %s\n"
               "usage:\n"
               "    %s <wav|pcm> <env> {online}\n"
               "***************************************************\n",
               LWKP_version(), argv[0]);
        return ret;
    }

    if (iOnline)
    {
        stCtx.pstWavFile = fopen(wavFile, "wb");
    }
    else
    {
        stCtx.pstWavFile = fopen(wavFile, "rb");
    }

    if (NULL == stCtx.pstWavFile)
    {
        printf("failed to open file\n");
        goto out;
    }

    if (!iOnline)
    {
        if (NULL != strstr(wavFile, ".wav"))
        {
            fseek(stCtx.pstWavFile, 44, SEEK_SET);
        }
    }

    pmembase = calloc(1, memsize);
    if (NULL == pmembase)
    {
        goto out;
    }

    stCtx.pstWkp = LWKP_new(pmembase, memsize);
    if (NULL == stCtx.pstWkp)
    {
        printf("failed to new wakeup\n");
        goto out;
    }

    LWKP_reset(stCtx.pstWkp);
    LWKP_registerHandler(stCtx.pstWkp, stCtx.pstWkp, _handler);
	LWKP_registerFeatureHandler(stCtx.pstWkp, stCtx.pstWkp, _feature_handler);
    if (0 != LWKP_start(stCtx.pstWkp, pcWakeupWords, strlen(pcWakeupWords)))
    {
        printf("failed to start wakeup\n");
        goto out;
    }

    if (iOnline)
    {
        feed_online(&stCtx);
    }
    else
    {
        feed_offline(&stCtx);
    }
    LWKP_end(stCtx.pstWkp);

    ret = 0;

out:
    if (stCtx.pstWkp)
    {
        LWKP_delete(stCtx.pstWkp);
    }

    if (stCtx.pstWavFile)
    {
        fclose(stCtx.pstWavFile);
    }

    if (pmembase)
    {
        free(pmembase);
    }

    return ret;
}

