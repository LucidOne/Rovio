#ifndef __ASF_BUFFER_STREAM_H__

#define __ASF_BUFFER_STREAM_H__


#include "asf_stream.h"
#include "wbtypes.h"

#ifndef ASF_IMA_SIZE
#define ASF_IMA_SIZE
#define ASF_IMA_ADPCM_SIZE  512 
#endif

#ifdef USE_SD
typedef struct
{
	Asf_Stream Stream;
	char Buffer[128];
	int Pos;
	int hFile;
} Asf_Buffer_Stream;
#else
typedef struct
{
	Asf_Stream Stream;
	char *Buffer;
	int Size;
	int Pos;
} Asf_Buffer_Stream;
#endif

typedef struct
{
	Video_Stream Stream;
	char *Buffer;
	int Size;
	int Pos;
	int TotalPayloads;
	short StreamNumber[25];
	int SubSize[25];
	int SubObject[25];
	int SubTime[25];
	unsigned char ObjectType;
} Video_Buffer_Stream ;


typedef struct 
{
	char buffer[255*1024];
	int pos;
	int last_play;
}Video_play_buffer;

typedef struct
{
	UINT8   uRecAudioBuff[ASF_IMA_ADPCM_SIZE*20];
	UINT8   uRecVideoBuff[255*1024];
	UINT32	AudioBuffCount;
	UINT32	AudioBuffCur;
	UINT32	AudioBuffSize[20];
	UINT32	AudioRecTime[20];
	UINT32	VideoBuffSize;
	UINT32	VideoRecTime;
	UINT32	VideoBuffPos;
}Rec_Buffer_Stream;

#ifdef USE_SD
void asfFileStream_Init (Asf_Buffer_Stream *pBS, char *pBuffer);
#else
void asfBufferStream_Init (Asf_Buffer_Stream *pBS, char *pBuffer, int nSize);
#endif
void videoBufferStream_Init (Video_Buffer_Stream *pBS, char *pBuffer, int nSize);

#endif


