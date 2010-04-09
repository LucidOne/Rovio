#ifndef __VIDEOPHONE_TEST_H__
#define __VIDEOPHONE_TEST_H__

//#define TEST_DISPLAY_JPEG
//#define TEST_MP4_DYNAMIC_BITRATE

#define BUFFER_SIZE 100*1024

#define READ_THREAD_BUFFER_SIZE (64*1024)
typedef struct READ_THREAD_INFO
{
	cyg_handle_t	cygAudioThreadHandle;
	cyg_thread		cygAudioThread;
	char			cAudioThreadBuffer[READ_THREAD_BUFFER_SIZE];
	
	cyg_handle_t	cygAudioWriteThreadHandle;
	cyg_thread		cygAudioWriteThread;
	char			cAudioWriteThreadBuffer[READ_THREAD_BUFFER_SIZE];
	
	cyg_handle_t	cygVideoThreadHandle;
	cyg_thread		cygVideoThread;
	char			cVideoThreadBuffer[READ_THREAD_BUFFER_SIZE];
	
}READ_THREAD_INFO_T;

typedef struct
{
	UCHAR aucAudio[MEM_ALIGN_SIZE (OPT_AUDIO_BUFFER_SIZE)];
	int iAudioLen;

	LIST_T list;
	int ref_count;
} VP_BUFFER_TEST_AUDIO_BITSTREAM_T;
DECLARE_MEM_POOL (bufTestAudio, VP_BUFFER_TEST_AUDIO_BITSTREAM_T)



#endif
