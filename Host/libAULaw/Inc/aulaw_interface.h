#ifndef __AULAW_INTERFACE_H__
#define __AULAW_INTERFACE_H__

#include "wb_syslib_addon.h"

typedef enum
{
	DecodeLAW_FORMAT_ALAW = 0,
	DecodeLAW_FORMAT_ULAW,
	DecodeLAW_FORMAT_723ALAW,
	DecodeLAW_FORMAT_723ULAW,
	DecodeLAW_FORMAT_LINEAR
}DecodeLAW_FORMAT;

typedef enum
{
	EncodeLAW_FORMAT_ALAW = 0,
	EncodeLAW_FORMAT_ULAW,
	EncodeLAW_FORMAT_723ALAW,
	EncodeLAW_FORMAT_723ULAW,
	EncodeLAW_FORMAT_LINEAR
}EncodeLAW_FORMAT;

typedef struct
{
	DecodeLAW_FORMAT	dfSrcFormat;	// source format
	DecodeLAW_FORMAT	dfDstFormat;	// destination format
	UCHAR				*ucSrcData;		// source data
	INT					iSrcDataSize;	// source data size
	UCHAR				*ucDstData;		// destination buffer
	INT					iDstDataSize;	// destination buffer size
}DecodeLAW_T, EncodeLAW_T;

int Encode723LAW(EncodeLAW_T *encode_info);
int Decode723LAW(DecodeLAW_T *decode_info);
int EncodeLAW(EncodeLAW_T *encode_info);
int DecodeLAW(DecodeLAW_T *decode_info);

#endif