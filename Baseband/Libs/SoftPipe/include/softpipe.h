#ifndef __SOFTPIPE_H__
#define __SOFTPIPE_H__

typedef struct tagSOFT_PIPE_T SOFT_PIPE_T;

/*
 * Name: SOFT_PIPE_ON_DATA
 * Description: Pipe data receiver. For first pipe, the function is
 *              used to produce and transfer data to next pipe.
 *
 * Parameter:
 *  pspThis[in]: Pointer to this pipe.
 *  pData[in]: The data transferred to this pipe. Ignored for first pipe.
 *  uLen[in]: Length of the data transferred. Ignored for first pipe.
 * Return value:
 *  Length of data successfully transferred.
 *  The return value should be little than "uLen".
 */
typedef unsigned int (*SOFT_PIPE_ON_DATA) (
	SOFT_PIPE_T *pspThis,
	const void *pData,
	unsigned int uLen);


struct tagSOFT_PIPE_T
{
	SOFT_PIPE_ON_DATA	fnOnData;
	//unsigned int		uDataRecv;	//Data length recieved (not accumulative)
	unsigned int		uDataSent;	//Data length sent (not accumulative)
	SOFT_PIPE_T			*pspFrom;
	SOFT_PIPE_T			*pspTo;
};

void
spInit (SOFT_PIPE_T *pspThis);

void
spConnect (SOFT_PIPE_T *pspFrom, SOFT_PIPE_T *pspTo);

unsigned int
spSend (SOFT_PIPE_T *pspThis, const void *pData, unsigned int uLen);

void
spSendOver (SOFT_PIPE_T *pspThis);

unsigned int
spTransfer (SOFT_PIPE_T *pspThis);

unsigned int
spGetLenSent (const SOFT_PIPE_T *pspThis);

unsigned int
spGetLenRecv (const SOFT_PIPE_T *pspThis);


#include "./ininc/bufferpipe.h"

#endif
