#include "lib_videophone.h"
#include "videophone_test.h"

typedef struct TESTAUDIO_INFO
{
	TT_PC_T	pcAudio;
	LIST_T	listAudio;
}TESTAUDIO_INFO_T;

TESTAUDIO_INFO_T g_testaudio;

#define BUF_TEST_AUDIO_NUM	40
IMPLEMENT_MEM_POOL (bufTestAudio, VP_BUFFER_TEST_AUDIO_BITSTREAM_T, BUF_TEST_AUDIO_NUM)

int testaudio_init(void)
{
	bufTestAudioInitMemoryPool ();
	
	listInit (&g_testaudio.listAudio);
	tt_pc_init (&g_testaudio.pcAudio, bufTestAudioGetTotalBlocksNum ());
	
	return 0;
}


void thread_read_audio(cyg_addrword_t data)
{
	VP_BUFFER_TEST_AUDIO_BITSTREAM_T *ptestaudio;
	int audiocount = 0;
	cyg_tick_count_t timeBegin, timeEnd;
	
	int iRt;
	IO_THREAD_READ_T readarg;
	int i = 0;
	
	timeBegin = cyg_current_time();
	while(1)
	{
		readarg.txlen = BUFFER_SIZE;
		iRt = iothread_ReadAudio(&readarg);
		if(iRt == -1)
		{
			diag_printf("Encode Core quit\n");
			cyg_thread_exit();
	    }
		else if(iRt == 0)
		{
			diag_printf("%d buffer too small\n", readarg.mediatype);
			continue;
        }
		else
        {
			switch(readarg.mediatype)
	        {
               case MEDIA_TYPE_AUDIO:
					audiocount++;
					
					cyg_semaphore_wait(&g_testaudio.pcAudio.producer);
					ptestaudio = bufTestAudioNew();
					if(ptestaudio == NULL)
					{
						diag_printf("No audio read buffer!!!\n");
						while(1);
					}
					
					if( readarg.txlen <= sizeof(ptestaudio->aucAudio))
					{
						memcpy(ptestaudio->aucAudio, readarg.txbuf, readarg.txlen);
						ptestaudio->iAudioLen = readarg.txlen;
						
						listAttach(&g_testaudio.listAudio, &ptestaudio->list);
						cyg_semaphore_post(&g_testaudio.pcAudio.consumer);
					}
					else
					{
						bufTestAudioDecRef(ptestaudio);
						diag_printf("audio too large %d>%d\n", readarg.txlen, sizeof(ptestaudio->aucAudio));
						cyg_semaphore_post(&g_testaudio.pcAudio.producer);
					}
					iothread_ReadAudio_Complete(&readarg);
					//readarg.txbuf = (unsigned char*)g_mpDataBuffer;
					
#if 0
					/* Decode law to pcm */
					{
						DecodeLAW_T decode_info;
						
						decode_info.dfSrcFormat = DecodeLAW_FORMAT_ALAW;
						decode_info.dfDstFormat = DecodeLAW_FORMAT_LINEAR;
						decode_info.ucSrcData = g_mpDataBuffer;
						decode_info.iSrcDataSize = readarg.txlen;
						decode_info.ucDstData = g_mpAudioDataBuffer;
						decode_info.iDstDataSize = sizeof(g_mpAudioDataBuffer);
						
						readarg.txlen = DecodeLAW(&decode_info);
						if(readarg.txlen == -1)
						{
							diag_printf("decode law failed\n");
							continue;
						}
						
						readarg.mediatype = MEDIA_TYPE_AUDIO;
						readarg.txbuf = g_mpAudioDataBuffer;
					}
#endif
					iothread_Write(&readarg);
					//diag_printf("get audio, len=%d\n", readarg.txlen);
                    break;
              default:
					diag_printf("unknown type %d\n", readarg.mediatype);
                    break;
            }
		}
		
		if(audiocount >= 500 && audiocount % 500 == 0)
		{
			int timeoffset;
			timeEnd = cyg_current_time();
			timeoffset = timeEnd-timeBegin;
			diag_printf("audiocount=%d\n", audiocount);
			if(timeoffset != 0 && timeoffset > 100) diag_printf("audiorate=%d\n", audiocount/(timeoffset/100));
        }
	}
	cyg_thread_exit();
}

void thread_write_audio(cyg_addrword_t data)
{
	VP_BUFFER_TEST_AUDIO_BITSTREAM_T *ptestaudio;
	IO_THREAD_READ_T readarg;
	
	while(1)
	{
		cyg_semaphore_wait(&g_testaudio.pcAudio.consumer);
		if (!listIsEmpty (&g_testaudio.listAudio))
		{
			ptestaudio = GetParentAddr (g_testaudio.listAudio.pNext, VP_BUFFER_TEST_AUDIO_BITSTREAM_T, list);
			listDetach(&ptestaudio->list);
		}
		else
		{
			diag_printf("No auido data???\n");
			while(1);
		}
		
		readarg.txbuf = ptestaudio->aucAudio;
		readarg.txlen = ptestaudio->iAudioLen;
		readarg.mediatype = MEDIA_TYPE_AUDIO;
		iothread_Write(&readarg);
		
		bufTestAudioDecRef(ptestaudio);
		cyg_semaphore_post(&g_testaudio.pcAudio.producer);
	}
	cyg_thread_exit();
}
