#include "lib_videophone.h"
#include "aulaw_interface.h"

#define READ_THREAD_BUFFER_SIZE (64*1024)
typedef struct READ_THREAD_INFO
{
	cyg_handle_t	cygAudioThreadHandle;
	cyg_thread		cygAudioThread;
	char			cAudioThreadBuffer[READ_THREAD_BUFFER_SIZE];
	
	cyg_handle_t	cygVideoThreadHandle;
	cyg_thread		cygVideoThread;
	char			cVideoThreadBuffer[READ_THREAD_BUFFER_SIZE];
	
}READ_THREAD_INFO_T;
READ_THREAD_INFO_T g_ReadThreadInfo;

#define BUFFER_SIZE 100*1024
__align(32) CHAR g_mpDataBuffer[BUFFER_SIZE];
__align(32) UCHAR g_mpAudioDataBuffer[BUFFER_SIZE/100];
__align(32) UCHAR g_mpLawDataBuffer[BUFFER_SIZE/200];

void thread_read_audio(cyg_addrword_t data);
void thread_read_video(cyg_addrword_t data);


static BOOL g_bDisableDebugMsg = FALSE;
void uart_write( const char *buf, size_t size )
{
#if 1
	if (!g_bDisableDebugMsg)
	{
		size_t i;
	
		for ( i = 0; i < size; i++ )
			hal_if_diag_write_char( buf[i] );
	}
#endif
}

int icount = 0;
int main()
{
	VP_SIZE_T tSize;
	VP_POINT_T tDisplayPos;
	VP_VIDEO_T *pVideo;
	UINT32 uFrameRate;
	
	cyg_tick_count_t timeBegin, timeEnd;
	
	int iRt;
	IO_THREAD_READ_T readarg;
	int i = 0;
	
	cyg_interrupt_disable();
	cyg_interrupt_disable();
	cyg_interrupt_disable();
	cyg_interrupt_enable();
	
	VideoPhoneInit();
	SuspendTvp5150();
	
	diag_printf("vlcmConfigure\n");
	ControlBackLight(6, 0);
	wb702ConfigureLCM(320, 240, 2, 1, 3, 0, TRUE, 1);
	ControlLcmTvPower(2);
	ControlBackLight(6, 1);	
	
	diag_printf("vcptSetWindow\n");
	wb702SetLocalVideoSource(640, 480, 0);
	
	diag_printf("vmp4encGetQuality\n");
	wb702SetVideoBitRate(128 * 1024);
	
	diag_printf("vmp4encSetFormat\n");
	wb702SetVideoFormat(CMD_VIDEO_MP4);
	wb702SetAudioType(CMD_AUDIO_PCM, CMD_AUDIO_PCM);
	
	diag_printf("vdEnableLocalMP4\n");
	wb702EnableMP4Encoder(TRUE);	//mask
	
	wb702EnableAudioEncoder(TRUE);	//mask
	
	diag_printf("vdEnableLocalJPEG\n");
    wb702SetJPEG(TRUE);
	
	diag_printf("all ok\n");
	
	//task_view_start();
	cyg_thread_create(10, thread_read_audio, NULL, "read_audio", 
						g_ReadThreadInfo.cAudioThreadBuffer, READ_THREAD_BUFFER_SIZE, 
						&g_ReadThreadInfo.cygAudioThreadHandle, &g_ReadThreadInfo.cygAudioThread);
	cyg_thread_create(10, thread_read_video, NULL, "read_video", 
						g_ReadThreadInfo.cVideoThreadBuffer, READ_THREAD_BUFFER_SIZE, 
						&g_ReadThreadInfo.cygVideoThreadHandle, &g_ReadThreadInfo.cygVideoThread);
	cyg_thread_resume(g_ReadThreadInfo.cygAudioThreadHandle);
	cyg_thread_resume(g_ReadThreadInfo.cygVideoThreadHandle);
	
	while(1) tt_msleep(1000000);

}


void thread_read_audio(cyg_addrword_t data)
{
	int audiocount = 0;
	cyg_tick_count_t timeBegin, timeEnd;
	
	int iRt;
	IO_THREAD_READ_T readarg;
	int i = 0;
		
	diag_printf("thread_read_audio(): begin=======\n");
	timeBegin = cyg_current_time();
	while(1)
	{
		//cyg_thread_delay(1000);
		//continue;
		
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
					if( readarg.txlen <= sizeof(g_mpAudioDataBuffer))
					{
						memcpy(g_mpAudioDataBuffer, readarg.txbuf, readarg.txlen);
					}
					else
					{
						diag_printf("audio too large %d>%d\n", readarg.txlen, sizeof(g_mpAudioDataBuffer));
					}
					iothread_ReadAudio_Complete(&readarg);
					
					/* Encode pcm to law */
#if 1
					{
						EncodeLAW_T encode_info;
						
						encode_info.dfSrcFormat = EncodeLAW_FORMAT_LINEAR;
						encode_info.dfDstFormat = EncodeLAW_FORMAT_ALAW;
						encode_info.ucSrcData = g_mpAudioDataBuffer;
						encode_info.iSrcDataSize = readarg.txlen;
						encode_info.ucDstData = g_mpLawDataBuffer;
						encode_info.iDstDataSize = sizeof(g_mpLawDataBuffer);
						
						readarg.txlen = EncodeLAW(&encode_info);
						if(readarg.txlen == -1)
						{
							diag_printf("decode law failed\n");
							continue;
						}
						//diag_printf("%d\n", readarg.txlen);
					}
					
					/* Decode law to pcm */
					{
						DecodeLAW_T decode_info;
						
						decode_info.dfSrcFormat = DecodeLAW_FORMAT_ALAW;
						decode_info.dfDstFormat = DecodeLAW_FORMAT_LINEAR;
						decode_info.ucSrcData = g_mpLawDataBuffer;
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
						
						iothread_Write(&readarg);
					}
#else
					readarg.txbuf = g_mpAudioDataBuffer;
					//iothread_Write(&readarg);
#endif
                    break;
              default:
					diag_printf("unknown type %d\n", readarg.mediatype);
                    break;
            }
			//tt_msleep(2000);
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

void thread_read_video(cyg_addrword_t data)
{
	int videocount = 0, jpegcount = 0;
	cyg_tick_count_t timeBegin, timeEnd;
	
	int iRt;
	IO_THREAD_READ_T readarg;
	int i = 0;
	
	diag_printf("thread_read_video(): begin=======\n");
	timeBegin = cyg_current_time();
	while(1)
	{
		readarg.txlen = BUFFER_SIZE;
		iRt = iothread_ReadVideo(&readarg);
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
                case MEDIA_TYPE_VIDEO:
					videocount++;
					if( readarg.txlen <= sizeof(g_mpDataBuffer))
					{
						memcpy(g_mpDataBuffer, readarg.txbuf, readarg.txlen);
					}
					else
					{
						diag_printf("video too large %d>%d\n", readarg.txlen, sizeof(g_mpDataBuffer));
					}
					iothread_ReadVideo_Complete(&readarg);
					readarg.txbuf = (unsigned char*)g_mpDataBuffer;
					//iothread_Write(&readarg);
					//diag_printf("get video, len=%d\n", readarg.txlen);
                    break;
                case MEDIA_TYPE_JPEG:
					jpegcount++;
					if( readarg.txlen <= sizeof(g_mpDataBuffer))
					{
						memcpy(g_mpDataBuffer, readarg.txbuf, readarg.txlen);
					}
					else
					{
						diag_printf("jpeg too large %d>%d\n", readarg.txlen, sizeof(g_mpDataBuffer));
					}
					iothread_ReadVideo_Complete(&readarg);
					readarg.txbuf = (unsigned char*)g_mpDataBuffer;
					//iothread_Write(&readarg);
					//diag_printf("get jpeg, len=%d\n", readarg.txlen);
                    break;
                case MEDIA_TYPE_MOTION:
					iothread_ReadVideo_Complete(&readarg);
					diag_printf("get motion\n");
                    break;
                default:
					diag_printf("unknown type %d\n", readarg.mediatype);
                    break;
            }
			//tt_msleep(2000);
		}
		if((videocount + jpegcount) >= 100 && (videocount + jpegcount) % 100 == 0)
		{
			int timeoffset;
			timeEnd = cyg_current_time();
			timeoffset = timeEnd-timeBegin;
			diag_printf("videocount=%d jpegcount=%d\n", videocount, jpegcount);
			diag_printf("videorate=%d jpegrate=%d\n", videocount/(timeoffset/100), jpegcount/(timeoffset/100));
        }
	}
	cyg_thread_exit();
}


