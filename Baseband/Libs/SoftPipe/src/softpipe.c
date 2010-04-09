#include "../../Platform/Inc/Platform.h"
#include "../include/softpipe.h"

void
spInit (SOFT_PIPE_T *pspThis)
{
	pspThis->fnOnData	= (SOFT_PIPE_ON_DATA) NULL;
	pspThis->uDataSent	= 0;
	pspThis->pspFrom	= NULL;
	pspThis->pspTo		= NULL;
}


void
spConnect (SOFT_PIPE_T *pspFrom, SOFT_PIPE_T *pspTo)
{
	pspFrom->pspTo	= pspTo;
	pspTo->pspFrom	= pspFrom;
}

unsigned int
spSend (SOFT_PIPE_T *pspThis, const void *pData, unsigned int uLen)
{
	SOFT_PIPE_T *pspTo = pspThis->pspTo;
	unsigned int uLenSent;
	unsigned int uLenSentThis;

	/* Do not send data with length 0,
	length 0 indicates a END flag. */
	if (uLen == 0)
		return 0;
	
	if (pspTo == NULL || pspTo->fnOnData == NULL)
		return 0;

	/* Send data to next PIPE. */
	for (uLenSent = 0; uLen > 0; )
	{
		uLenSentThis = (* pspTo->fnOnData) (pspTo, pData, uLen);
		if (uLenSentThis == 0)
			break;

		pspThis->uDataSent += uLenSentThis;
		uLenSent += uLenSentThis;
		uLen -= uLenSentThis;
	}

	return uLenSent;
}

void
spSendOver (SOFT_PIPE_T *pspThis)
{
	SOFT_PIPE_T *pspTo = pspThis->pspTo;

	/* Send data to next PIPE. */
	if (pspTo == NULL || pspTo->fnOnData == NULL)
		return;

	(* pspTo->fnOnData) (pspTo, NULL, 0);
}

unsigned int
spTransfer (SOFT_PIPE_T *pspThis)
{
	SOFT_PIPE_T *pspFrom = pspThis;
	SOFT_PIPE_T *pspTo;
	
	/* Search the first one in PIPE group. */
	while (pspFrom->pspFrom != NULL)
		pspFrom = pspFrom->pspFrom;
	
	/* Clear the length received and sent in each PIPE. */
	for (pspTo = pspFrom; pspTo != NULL; pspTo = pspTo->pspTo)
		pspTo->uDataSent = 0;

	/* Begin to send. */
	(*pspFrom->fnOnData) (pspFrom, NULL, 0);
	return pspFrom->uDataSent;
}


unsigned int
spGetLenSent (const SOFT_PIPE_T *pspThis)
{
	return pspThis->uDataSent;
}


unsigned int
spGetLenRecv (const SOFT_PIPE_T *pspThis)
{
	if (pspThis->pspFrom == NULL)
		return 0;
	else
		return pspThis->pspFrom->uDataSent;
}


