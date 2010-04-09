#include "lib_videophone.h"
#include "videophone_test.h"

READ_THREAD_INFO_T g_ReadThreadInfo;

__align(32) UCHAR g_mpDataBuffer[BUFFER_SIZE];
__align(32) UCHAR g_mpAudioDataBuffer[BUFFER_SIZE];


#ifdef TEST_DISPLAY_JPEG
/* JPEG data with 320 width and 240 height */
UCHAR g_jpeg[] =
{
#include "bg.h"
};
#endif

int testaudio_init(void);
void thread_read_audio(cyg_addrword_t data);
void thread_write_audio(cyg_addrword_t data);
void thread_read_video(cyg_addrword_t data);
void videoTestDynamicBitrate(void);

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

int main()
{
	cyg_interrupt_disable();
	cyg_interrupt_disable();
	cyg_interrupt_disable();
	cyg_interrupt_enable();
	
	diag_printf("main\n");
	VideoPhoneInit();
	SuspendTvp5150();
	
	diag_printf("vlcmConfigure\n");
	ControlBackLight(6, 0);
	wb702ConfigureLCM(320, 240, 2, 1, 3, 0, TRUE, 1);
	
	ControlLcmTvPower(2);
	ControlBackLight(6, 1);	
	//wb702SetOSDColor(5, 0x00, 0x00, 0xF8, 0xF8, 0xFC, 0xF8);
	
	diag_printf("vcptSetWindow\n");
	//wb702SetLocalVideoSource(176, 144, 0);
	//wb702SetLocalVideoSource(352, 288, 0);
	wb702SetLocalVideoSource(640, 480, 0);
	//wb702SetLocalVideoSource(320, 240, 0);
#ifdef TEST_DISPLAY_JPEG
    wb702SetJPEG(TRUE);
	wb702JPEG2RGB565(g_jpeg, vlcmGetLCMBuffer ()->aucData, sizeof(g_jpeg), 320, 240);
    wb702SetJPEG(FALSE);
#endif
	
	/*
	diag_printf("vdSetLocalWindow\n");
	wb702SetLocalVideoWindow(12, 12, 80, 60, 0);
	wb702EnableDisplay(CMD_DISPLAY_LOCAL_VIDEO, TRUE);
	
	diag_printf("vdSetRemoteWindow\n");
	wb702SetRemoteVideoWindow(32, 48, 72, 80, 0);
	wb702EnableDisplay(CMD_DISPLAY_REMOTE_VIDEO, TRUE);
	
	
	diag_printf("vlcmFillLCMBuffer\n");
	wb702FillOSD(TRUE, 0x00, 0x00, 0x00);
	diag_printf("vlcmFillOSDBuffer\n");
	wb702FillOSD(FALSE, 0x00, 0x00, 0xF8);
	*/
	
	diag_printf("vmp4encGetQuality\n");
	wb702SetVideoBitRate(128 * 1024);
	
	diag_printf("vmp4encSetFormat\n");
	wb702SetVideoFormat(CMD_VIDEO_MP4);
	wb702SetAudioType(CMD_AUDIO_PCM, CMD_AUDIO_PCM);
	//wb702SetAudioType(CMD_AUDIO_ALAW, CMD_AUDIO_ALAW);
	//wb702SetAudioType(CMD_AUDIO_ALAW, CMD_AUDIO_PCM);
	
	diag_printf("vdEnableLocalMP4\n");
	wb702EnableMP4Encoder(TRUE);	//mask
	
	wb702EnableAudioEncoder(TRUE);	//mask
	
	diag_printf("vdEnableLocalJPEG\n");
    wb702SetJPEG(TRUE);
    
	//wb702EnableMotionDetect(true, CMD_MOTION_DETECT_LOW);
	
	diag_printf("all ok\n");
	
	//task_view_start();
	testaudio_init();
	
	cyg_thread_create(10, thread_read_audio, NULL, "read_audio", 
						g_ReadThreadInfo.cAudioThreadBuffer, READ_THREAD_BUFFER_SIZE, 
						&g_ReadThreadInfo.cygAudioThreadHandle, &g_ReadThreadInfo.cygAudioThread);
	cyg_thread_create(10, thread_write_audio, NULL, "write_audio", 
						g_ReadThreadInfo.cAudioWriteThreadBuffer, READ_THREAD_BUFFER_SIZE, 
						&g_ReadThreadInfo.cygAudioWriteThreadHandle, &g_ReadThreadInfo.cygAudioWriteThread);
	cyg_thread_create(10, thread_read_video, NULL, "read_video", 
						g_ReadThreadInfo.cVideoThreadBuffer, READ_THREAD_BUFFER_SIZE, 
						&g_ReadThreadInfo.cygVideoThreadHandle, &g_ReadThreadInfo.cygVideoThread);
	cyg_thread_resume(g_ReadThreadInfo.cygAudioThreadHandle);
	cyg_thread_resume(g_ReadThreadInfo.cygAudioWriteThreadHandle);
	cyg_thread_resume(g_ReadThreadInfo.cygVideoThreadHandle);
	
	while(1)
	{
#ifdef TEST_DISPLAY_JPEG
		wb702JPEG2RGB565(g_jpeg, vlcmGetLCMBuffer ()->aucData, sizeof(g_jpeg), 320, 240);
		tt_msleep(10000);
#else
		tt_msleep(1000000);
#endif
	}

}


void thread_read_video(cyg_addrword_t data)
{
	int videocount = 0, jpegcount = 0;
	int videosize = 0, jpegsize = 0;
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
					videosize += readarg.txlen;
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
					jpegsize += readarg.txlen;
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
		
		if((videocount + jpegcount) >= 300 && (videocount + jpegcount) % 300 == 0)
		{
			int timeoffset;
			timeEnd = cyg_current_time();
			timeoffset = timeEnd-timeBegin;
			diag_printf("videocount=%d jpegcount=%d\n", videocount, jpegcount);
			diag_printf("videorate=%d jpegrate=%d\n", videocount/(timeoffset/100), jpegcount/(timeoffset/100));
#ifdef TEST_MP4_DYNAMIC_BITRATE
			diag_printf("videosize=%d, jpegsize=%d\n", videosize/videocount, jpegsize/jpegcount);
			videosize = jpegsize = 0;
			videocount = jpegcount = 0;
			videoTestDynamicBitrate();
#endif
        }
	}
	cyg_thread_exit();
}

void videoTestDynamicBitrate(void)
{
	static int bitrateCount = 0;
	
#if 0
	switch (bitrateCount % 4)
	{
		case 0:
			diag_printf("Set bitrate to 128*1024\n");
			wb702SetVideoBitRate(128 * 1024);
			break;
		case 1:
			diag_printf("Set bitrate to 512*1024\n");
			wb702SetVideoBitRate(512 * 1024);
			break;
		case 2:
			diag_printf("Set bitrate to 1024*1024\n");
			wb702SetVideoBitRate(1024 * 1024);
			break;
		case 3:
			diag_printf("Set bitrate to 2304*1024\n");
			wb702SetVideoBitRate(2304 * 1024);
			break;
		default:
			break;
	}
#else
	switch (bitrateCount % 4)
	{
		case 0:
			diag_printf("Set bitrate to 128*1024\n");
			wb702SetVideoDynamicBitrate(128 * 1024);
			break;
		case 1:
			diag_printf("Set bitrate to 512*1024\n");
			wb702SetVideoDynamicBitrate(512 * 1024);
			break;
		case 2:
			diag_printf("Set bitrate to 1024*1024\n");
			wb702SetVideoDynamicBitrate(1024 * 1024);
			break;
		case 3:
			diag_printf("Set bitrate to 2304*1024\n");
			wb702SetVideoDynamicBitrate(2304 * 1024);
			break;
		default:
			break;
	}
#endif
	bitrateCount++;
	tt_msleep(2000);
}
