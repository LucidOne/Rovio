#include "../../Inc/inc.h"

#ifdef __HIC_SUPPORT__


void cmd18_11_EnableAudioEncoder (HIC_THREAD_T *pHicThread, UCHAR ucCmd, UCHAR ucSubCmd,
	UCHAR ucD, UCHAR ucC, UCHAR ucB, UCHAR ucA)
{
	BOOL bEnable = (ucD == 0 ? FALSE : TRUE);

	DUMP_HIC_COMMAND;
	
	if (bEnable)
	{
		vauStartRecord ();
	}
	else
	{
		vauStopRecord ();
		
		/* Clear encoded buffer. */
		while (1)
		{
			VP_BUFFER_ENCODED_AUDIO_BITSTREAM_T *pBitstream;
			
			vauLock ();
			pBitstream = vauGetEncBuffer_Local ();
			if (pBitstream != NULL)
			{
				listDetach (&pBitstream->list);
				bufEncAudioDecRef (pBitstream);
			}
			vauUnlock ();
			
			if (pBitstream == NULL)
				break;
		}
	}

	hicSetCmdReady (pHicThread);
}

#endif
