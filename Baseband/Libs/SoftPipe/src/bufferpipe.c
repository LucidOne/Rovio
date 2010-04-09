#include "../../Platform/Inc/Platform.h"
#include "../include/softpipe.h"


static unsigned int
__spSourceBuffer (SOFT_PIPE_T *pspThis,
	const void *pData,
	unsigned int uLen)
{
	SOURCE_BUFFER_PIPE_T *psbpThis =
		SP_GET_PARENT_ADDR (pspThis, SOURCE_BUFFER_PIPE_T, pipe);

	spSend (pspThis, psbpThis->pBuffer, psbpThis->uLen);
	return spGetLenSent (pspThis);
}




void
spSourceBufferInit (SOURCE_BUFFER_PIPE_T *psbpThis,
					const void *pBuffer,
					unsigned int uLen)
{
	spInit (&psbpThis->pipe);
	psbpThis->pipe.fnOnData	= __spSourceBuffer;
	psbpThis->pBuffer		= pBuffer;
	psbpThis->uLen			= uLen;
}



static unsigned int
__spTargetBuffer (SOFT_PIPE_T *pspThis,
	const void *pData,
	unsigned int uLen)
{
	SOURCE_BUFFER_PIPE_T *ptbpThis =
		SP_GET_PARENT_ADDR (pspThis, SOURCE_BUFFER_PIPE_T, pipe);

	unsigned int uLenRecv = spGetLenRecv (pspThis);

	/* The max length can be received for this time. */
	unsigned int uLenThis =
		(ptbpThis->uLen > uLenRecv ? ptbpThis->uLen - uLenRecv : 0);
	/* The real length that's received this time. */
	if (uLenThis > uLen)
		uLenThis = uLen;

	memcpy ((char *) ptbpThis->pBuffer + uLenRecv, pData, uLenThis);

	return uLenThis;
}


void
spTargetBufferInit (TARGET_BUFFER_PIPE_T *ptbpThis,
					void *pBuffer,
					unsigned int uLen)
{
	spInit (&ptbpThis->pipe);
	ptbpThis->pipe.fnOnData	= __spTargetBuffer;
	ptbpThis->pBuffer		= pBuffer;
	ptbpThis->uLen			= uLen;
}
