#ifndef __VP_JPEG_H__
#define __VP_JPEG_H__

#define JPEG_DEC_WITH_MP4_BUFFER

typedef enum
{
	VP_JPEG_ERROR,
	VP_JPEG_OK
} VP_JPEG_STATE_E;

typedef enum
{
	VP_JPEG_ENC_ERROR,
	VP_JPEG_ENC_OK
} VP_JPEG_ENC_STATE_E;

typedef enum
{
	VP_JPEG_DEC_ERROR,
	VP_JPEG_DEC_OK,
	VP_JPEG_DEC_WAIT
} VP_JPEG_DEC_STATE_E;

typedef enum
{
	G_JPEG_ENCODE_THIMG = 1,
	G_JPEG_DECODE_PRIMG = 13
}VP_JPEG_ENTRY_SEC;

typedef enum
{
	G_JPEG_QUALITY_H = 1,
	G_JPEG_QUALITY_N = 2,
	G_JPEG_QUALITY_L = 3
}VP_JPEG_QUALITY_E;

typedef struct
{
	UINT32  uPixels;                                // Image width
	UINT32  uLines;                                 // Image height
}JPEGDECINFO_T, JPEGENCINFO_T;

typedef struct
{
#ifndef ECOS
 	TT_RMUTEX_T		mtLock;
	TT_RMUTEX_T		mtLockEncoder;
	TT_RMUTEX_T		mtLockDecoder;
#else
 	cyg_mutex_t		mtLock;
	cyg_mutex_t		mtLockEncoder;
	cyg_mutex_t		mtLockDecoder;
#endif
	int				nRefCount;
	int				nRefCount_Encoder;
	int				nRefCount_Decoder;

#ifndef ECOS
    TT_SEM_T        semEncoder;
    TT_SEM_T        semDecoder;
#else
    cyg_sem_t        semEncoder;
    cyg_sem_t        semDecoder;
#endif

    
	JPEG_ENCODE_SETTING_T       tJpegEncodeSetting;
	JPEG_DECODE_SETTING_T       tJpegDecodeSetting;
	//BOOL bInit;

    VP_BUFFER_JPEG_ENCODER_T    *pJPEGEncodeBuffer;

	LIST_T					listEncodedJPEG;
	JPEGDECINFO_T			jpegdecinf;
	JPEGENCINFO_T			jpegencinf;

	UINT32			nJPEGQua;

	BOOL			bOnTheFly;
	UINT32			nOnTheFlyCount;

#ifdef ECOS
	cyg_handle_t	cygIntrHandle;
	cyg_interrupt	cygIntrBuffer;
#endif
}VP_JPEG_T;

extern VP_JPEG_T g_vpJPEG;

UINT32 vjpegEntry(void *pData, UINT32 uItemNo);
void vjpegInit(void);
void vjpegEncInit(void);
void vjpegDecInit(void);
void vjpegDecSetHW(int iWidth, int iHeight);

UINT32 encodeJPEGwithThumbnailImage(UINT32 uYAddr, UINT32 uYSize, UINT32 uUVSize);
#ifdef JPEG_DEC_WITH_MP4_BUFFER
UINT32 decodeJPEGPrimaryImage(VP_BUFFER_MP4_DECODER_T *pJpeg);
#else
UINT32 decodeJPEGPrimaryImage(VP_BUFFER_JPEG_DECODER_T *pJpeg);
#endif

UINT32 vjpegEncoderCom_Callback(void);
UINT32 vjpegDecoderCom_Callback(void);
UINT32 vjpegDecoderErr_Callback(void);
void vjpegWaitEncoderCom(void);
void vjpegWaitDecoderCom(void);
UINT32 vjpegOnTheFlyCom_Callback(void);

void vjpegAddEncBuffer(void);
void vjpegSendEncBuffer(void);
VP_BUFFER_JPEG_ENCODER_T *vjpegGetEncBuffer(void);

void vjpegClearBuffer (void);

VP_BUFFER_JPEG_ENCODER_T *vjpegNewEncBuffer(void);
void vjpegFreeEncBuffer(VP_BUFFER_JPEG_ENCODER_T *pEncBuf);

#ifndef JPEG_DEC_WITH_MP4_BUFFER
VP_BUFFER_JPEG_DECODER_T *vjpegNewDecBuffer(void);
void vjpegFreeDecBuffer(VP_BUFFER_JPEG_DECODER_T *pDecBuf);
#endif

void vjpegLock (void);
BOOL vjpegTryLock (void);
void vjpegUnlock (void);

JPEGDECINFO_T *vjpegdecGetInfo (void);
UINT32 vjpegdecGetYSize (JPEGDECINFO_T *pJPEGInfo);
UINT32 vjpegdecGetUVSize (JPEGDECINFO_T *pJPEGInfo);

void vjpegSetQuality(UINT32 uQua);
UINT32 vjpegGetQuality(void);

#endif
