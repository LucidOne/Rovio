#include "lib_videophone.h"
#include "aulaw_interface.h"

//#define TONE_COMPARE_PCM_PCMFROMALAW
//#define TONE_COMPARE_PCM_PCMFROMMYCONVERT
#define TONE_COMPARE_PCM_PCMCONVERT

/* This is original PCM data */
UCHAR g_mpAudioDataBuffer[] =
{
#include "boot_org.h"
};

/* This is ALAW data converted from boot_org.h by external software */
UCHAR g_mpLawDataBuffer[] =
{
#include "alawout.h"
};

/* This is PCM data converted from alawout.h by external software */
UCHAR g_mpAudioDataBuffer1[] =
{
#include "boot_fromalaw.h"
};


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
	
	wb702SetAudioType(CMD_AUDIO_PCM, CMD_AUDIO_PCM);
	
	wb702EnableAudioEncoder(TRUE);	//mask
	diag_printf("all ok\n");
	
#if !defined(TONE_COMPARE_PCM_PCMFROMALAW) && !defined(TONE_COMPARE_PCM_PCMFROMMYCONVERT) && !defined(TONE_COMPARE_PCM_PCMCONVERT)
	diag_printf("Error: one compare must be defined!!!\n");
	return 0;
#endif
	
	/* Compare original PCM and the converted PCM */
#ifdef TONE_COMPARE_PCM_PCMFROMALAW
	while(1)
	{
		diag_printf("play original!!!\n");
		readarg.txlen = sizeof(g_mpAudioDataBuffer);
		readarg.mediatype = MEDIA_TYPE_AUDIO;
		readarg.txbuf = g_mpAudioDataBuffer;
		iothread_Write(&readarg);
		tt_msleep(1);
		diag_printf("play new!!!\n");
		readarg.txlen = sizeof(g_mpAudioDataBuffer1);
		readarg.mediatype = MEDIA_TYPE_AUDIO;
		readarg.txbuf = g_mpAudioDataBuffer1;
		iothread_Write(&readarg);
	}
#endif
	
	/* Compare the converted PCM and the PCM converted by my ALAW decode program */
#ifdef TONE_COMPARE_PCM_PCMFROMMYCONVERT
	while(1)
	{
		DecodeLAW_T decode_info;
		
		diag_printf("play original!!!\n");
		readarg.txlen = sizeof(g_mpAudioDataBuffer1);
		readarg.mediatype = MEDIA_TYPE_AUDIO;
		readarg.txbuf = g_mpAudioDataBuffer1;
		iothread_Write(&readarg);
		tt_msleep(1);
		diag_printf("play new!!!\n");
		
		decode_info.dfSrcFormat = DecodeLAW_FORMAT_ALAW;
		decode_info.dfDstFormat = DecodeLAW_FORMAT_LINEAR;
		decode_info.ucSrcData = g_mpLawDataBuffer;
		decode_info.iSrcDataSize = sizeof(g_mpLawDataBuffer);
		decode_info.ucDstData = g_mpAudioDataBuffer;
		decode_info.iDstDataSize = sizeof(g_mpAudioDataBuffer);
		diag_printf("%d %d\n", decode_info.iSrcDataSize, decode_info.iDstDataSize);
		
		readarg.txlen = DecodeLAW(&decode_info);
		if(readarg.txlen == -1)
		{
			diag_printf("decode law failed\n");
			return 0;
		}
		
		readarg.mediatype = MEDIA_TYPE_AUDIO;
		readarg.txbuf = g_mpAudioDataBuffer;
		diag_printf("decode data %d\n", readarg.txlen);
		
		iothread_Write(&readarg);
	}
#endif
	
	/* Compare the original PCM and the PCM converted by my ALAW encode and decode program */
#ifdef TONE_COMPARE_PCM_PCMCONVERT
	while(1)
	{
		EncodeLAW_T encode_info;
		DecodeLAW_T decode_info;
		int			lawsize;
		
		diag_printf("play original!!!\n");
		readarg.txlen = sizeof(g_mpAudioDataBuffer);
		readarg.mediatype = MEDIA_TYPE_AUDIO;
		readarg.txbuf = g_mpAudioDataBuffer;
		iothread_Write(&readarg);
		tt_msleep(1);
		diag_printf("play new!!!\n");
		
						
		encode_info.dfSrcFormat = DecodeLAW_FORMAT_ALAW;
		encode_info.dfDstFormat = DecodeLAW_FORMAT_LINEAR;
		encode_info.ucSrcData = g_mpAudioDataBuffer;
		encode_info.iSrcDataSize = sizeof(g_mpAudioDataBuffer);
		encode_info.ucDstData = g_mpLawDataBuffer;
		encode_info.iDstDataSize = sizeof(g_mpLawDataBuffer);
		
		lawsize = EncodeLAW(&encode_info);
		if(lawsize == -1)
		{
			diag_printf("encode law failed\n");
			continue;
		}
		diag_printf("alaw %d\n", lawsize);
		
		decode_info.dfSrcFormat = DecodeLAW_FORMAT_ALAW;
		decode_info.dfDstFormat = DecodeLAW_FORMAT_LINEAR;
		decode_info.ucSrcData = g_mpLawDataBuffer;
		decode_info.iSrcDataSize = lawsize;
		decode_info.ucDstData = g_mpAudioDataBuffer1;
		decode_info.iDstDataSize = sizeof(g_mpAudioDataBuffer1);
		
		readarg.txlen = DecodeLAW(&decode_info);
		if(readarg.txlen == -1)
		{
			diag_printf("decode law failed\n");
			return 0;
		}
		
		readarg.mediatype = MEDIA_TYPE_AUDIO;
		readarg.txbuf = g_mpAudioDataBuffer1;
		diag_printf("decode data %d\n", readarg.txlen);
		
		iothread_Write(&readarg);
	}
#endif
	
	return 0;
}


