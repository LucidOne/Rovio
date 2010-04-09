#include <stdio.h>
#include <assert.h>
#include "../include/softpipe.h"

static unsigned int
__spStdin (SOFT_PIPE_T *pspThis, const void *pData, unsigned int uLen)
{
	/* No data transferred from previous PIPE for the first one in PIPE group. */
	assert (pData == NULL && uLen == 0);

	while (1)
	{
		unsigned int uOK;
		char ac[128];
		uOK = read (0, ac, sizeof (ac));
		if (uOK > 0)
			spSend (pspThis, (const void *) ac, uOK);
		else
			break;
	}
	return spGetLenSent (pspThis);
}


static unsigned int
__spStdout (SOFT_PIPE_T *pspThis, const void *pData, unsigned int uLen)
{
	return write (1, pData, uLen);
}


void spInitStdin (SOFT_PIPE_T *psp)
{
	spInit (psp);
	psp->fnOnData	= __spStdin;
}


void spInitStdout (SOFT_PIPE_T *psp)
{
	spInit (psp);
	psp->fnOnData	= __spStdout;
}


int main()
{
	SOFT_PIPE_T myin;
	SOFT_PIPE_T myout;

	spInitStdin (&myin);
	spInitStdout (&myout);

	spConnect (&myin, &myout);
	spTransfer (&myin);

	return 0;
}
