#ifndef __ASF_ENC__
#define __ASF_ENC__

#include "asf_object.h"
#include "asf_buffer_stream.h"
//#include "MFL_Config.h"


//typedef struct {
//	UINT8	pucMpeg4Header[M4V_MAX_HEADER_LEN];	/* video decoder specific data */
//	UINT32	uMpeg4HeaderLength;	/* length of video decoder specific data */
//}MPEG4_HEADER_STRUCT;

#ifndef ASF_IMA_SIZE
#define ASF_IMA_SIZE
#define ASF_IMA_ADPCM_SIZE  512 
#endif

int data_packet_write(const Rec_Buffer_Stream *ppRecBuffStream, Asf_Stream *pStream);
int last_packet_write(Asf_Stream *pStream);
int head_object_write(const Head_Object_For_Record *pObject, Asf_Stream *pStream);
//void file_head_fill(MV_CFG_T *ptMvCfg);
//void  data_object_info_init(UINT8* info_buff);

//ASF_BYTE video_header_info[18] = {0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x20,
//										 0x00, 0x84, 0x5d, 0x4c, 0x28, 0x50, 0x20, 0xf0,
//										 0xa3, 0x1f};

static ASF_WCHAR asf_video_codec_name[20] = {'I','S','O',' ','M','P','E','G',
											'-','4',' ','V','I','D','E','O',
											' ','V','1',0};

static ASF_BYTE asf_video_codec_inf[4] = {'M','4','S','2'};

static ASF_WCHAR asf_audio_codec_name[10] = {'I','M','A',' ','A','D','P','C','M',0};

static unsigned char audio_proper[] =
{
#if 0	//ADPCM
	0x01, 0x00, 0x01, 0x00, 0xC0, 0x5D, 0x00, 0x00,
	0x80, 0x3E, 0x00, 0x00, 0x40, 0x01, 0x10, 0x00,
	0x02, 0x00, 0xF9, 0x03,
#else	//IMA ADPCM
	0x11, 0x00, 0x01, 0x00, 0xC0, 0x5D, 0x00, 0x00,
	0x32, 0x2F, 0x00, 0x00, 0x00, 0x02, 0x04, 0x00,
	0x02, 0x00, 0xF9, 0x03,
#endif
};

static unsigned char error_concealment[] =
{
	0x01, 0x00, 0x02, 0x00, 0x02, 0x01, 0x00, 0x00,
};

#endif
