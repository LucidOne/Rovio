#ifndef __VP_AUDIO_H__
#define __VP_AUDIO_H__


INT vauInit (void);
void vauUninit (void);
INT vauStartPlay (void);
INT vauStopPlay (void);
INT vauStartRecord (void);
INT vauStopRecord (void);

INT vauSetQualityLevel (INT nLevel);
INT vauGetQualityLevel (void);
INT vauSetFormat (CMD_AUDIO_FORMAT_E eEncodeFormat, CMD_AUDIO_FORMAT_E eDecodeFormat);
INT vauSetPlayVolume (INT lvolume, INT rvolume);
INT vauSetRecordVolume (INT lvolume, INT rvolume);
INT vauSetNotificationVolume(INT volume);

VP_BUFFER_ENCODED_AUDIO_BITSTREAM_T *vauGetEncBuffer_Local (void);
VP_BUFFER_ENCODED_AUDIO_BITSTREAM_T *vauGetEncBuffer_Remote (void);
void vauDecode (CONST UCHAR *cpucData, UINT32 uLen);
void vauNotificationPlay(UCHAR *pucBuffer, INT32 iBufferLen, void(*callback)(void));
void vauNotificationGet(UCHAR *pucBuffer, UINT32 uiBufferLen);


void vauLock (void);
int vauTryLockInThd (void);
int vauTryLockInDsr (void);
int vauCanLock__remove_it (void);
void vauUnlock (void);

#endif
