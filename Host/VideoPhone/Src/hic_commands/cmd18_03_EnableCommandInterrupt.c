#include "../../Inc/inc.h"

#ifdef __HIC_SUPPORT__

void __cmd18_03_EnableCommandInterrupt_CheckIntr (void)
{
#define PRIORITY_TOTAL			30
#define PRIORITY_AUDIO			1
#define PRIORITY_VIDEO			3
#define PRIORITY_MOTION_DETECT	3
#define PRIORITY_JPEG			3

	static int anPriority[4] = {0, 0, 0, 0};
	static int nInterrupt_Total = PRIORITY_TOTAL;
	int *pnPriority[sizeof (anPriority) / sizeof (anPriority[0])];
	BOOL bBreak;
	int i;
	
	/* Clear interrupt prioritys. */
	if (nInterrupt_Total >= PRIORITY_TOTAL)
	{
		nInterrupt_Total = 0;
		memset (&anPriority, 0, sizeof (anPriority));
	}
	
	/* Sort */
	for (i = 0; i < sizeof (anPriority) / sizeof (anPriority[0]); i++)
		pnPriority[i] = &anPriority[i];

	for (i = sizeof (pnPriority) / sizeof (pnPriority[0]); i > 0; i--)
	{
		int j;
		for (j = 1; j < i; j++)
		{
			if (*(pnPriority[j - 1]) > *(pnPriority[j]))
			{
				int *pnTmp = pnPriority[j];
				pnPriority[j] = pnPriority[j - 1];
				pnPriority[j - 1] = pnTmp;
			}
		}
	}


	/* Check the highest priority. */
	bBreak = FALSE;
	for (i = 0; i < sizeof (pnPriority) / sizeof (pnPriority[0]) && !bBreak; i++)
	{
		switch (pnPriority[i] - &anPriority[0])
		{
			case 0:	//Audio
			{
				VP_BUFFER_ENCODED_AUDIO_BITSTREAM_T *pBitstream;	
				vauLock ();
				pBitstream = vauGetEncBuffer_Local ();
				if (pBitstream != NULL)
				{
					if (hicSetInterruptEvent (((UINT32) CMD_INTR_AUDIO << 24)
						| ((UINT32) pBitstream->nSize & 0x00FFFFFF)))
					{
						PTL;
						anPriority[0] += PRIORITY_AUDIO;
						nInterrupt_Total++;
						bBreak = TRUE;
					}
				}
				vauUnlock ();
			
				break;
			}
			case 1:	//Video
			{
				VP_BUFFER_MP4_BITSTREAM_T *pBitstream;	
				sysDisableIRQ ();
				pBitstream = vmp4encGetBuffer ();
				if (pBitstream != NULL)
				{
					if (hicSetInterruptEvent (((UINT32) CMD_INTR_VIDEO << 24)
						| (pBitstream->uLength & 0x00FFFFFF)))
					{
					PTL;
						anPriority[1] += PRIORITY_VIDEO;
						nInterrupt_Total++;
						bBreak = TRUE;
					}
				}
				sysEnableIRQ ();
			
				break;
			}
			case 2:	//Motion detected
			{
				VP_VIDEO_T *pVideo = vdGetSettings ();
			
				if (pVideo->bIsEnableMotionDetect)
				{
					vdLock ();
					if (pVideo->uMotionsSinceLast != 0)
					{
						if (hicSetInterruptEvent (((UINT32) CMD_INTR_MOTION_DETECT << 24)
							| ((UINT32) pVideo->uMotionsSinceLast & 0x00FFFFFF)))
						{
							PTL;
							pVideo->uMotionsSinceLast = 0;
							anPriority[2] += PRIORITY_MOTION_DETECT;
							nInterrupt_Total++;
							bBreak = TRUE;
						}
					}
					vdUnlock ();
				}
				
				break;
			}
			case 3:	//JPEG
			{
				VP_BUFFER_JPEG_ENCODER_T *pEncBuf;	
				sysDisableIRQ ();
				pEncBuf = vjpegGetEncBuffer ();
				if (pEncBuf != NULL)
				{
					if (hicSetInterruptEvent (((UINT32) CMD_INTR_JPEG << 24)
						| (pEncBuf->uJPEGDatasize & 0x00FFFFFF)))
					{
						PTL;
						anPriority[3] += PRIORITY_JPEG;
						nInterrupt_Total++;
						bBreak = TRUE;
					}
				}
				sysEnableIRQ ();
			
				break;
			}
			default:;
		}
	}
	

}

void cmd18_03_EnableCommandInterrupt (HIC_THREAD_T *pHicThread, UCHAR ucCmd, UCHAR ucSubCmd,
	UCHAR ucD, UCHAR ucC, UCHAR ucB, UCHAR ucA)
{
	DUMP_HIC_COMMAND;
	

	pHicThread->bEnableCommandInterrupt = (ucD == 0 ? FALSE : TRUE);
	pHicThread->bEnableEventInterrupt = (ucC == 0 ? FALSE : TRUE);


	if (pHicThread->bEnableEventInterrupt)
	{
		__cmd18_03_EnableCommandInterrupt_CheckIntr ();
	}
	

	hicSetCmdReady (pHicThread);
}


#endif
