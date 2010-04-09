#ifndef __VP_LCM_H__
#define __VP_LCM_H__






void vlcmInit (void);
void vlcmLock (void);
int vlcmTryLock (void);
void vlcmUnlock (void);
#ifdef ECOS
void vlcmLock1 (void);
int vlcmTryLock1 (void);
void vlcmUnlock1 (void);
#endif

int vlcmConfigure (USHORT usWidth,		/* LCM width */
				  USHORT usHeight,		/* LCM height */

				  CMD_LCM_COLOR_WIDTH_E eLcmColor,
				  CMD_LCM_CMD_BUS_WIDTH_E eCmdBus,
				  CMD_LCM_DATA_BUS_WIDTH_E eDataBus,
				  int iMPU_Cmd_RS_pin,	/* 0 or 1 (when send MPU command) */
				  BOOL b16t18,			/* Convert 16-bit video to 18-bit? */
				  CMD_LCM_MPU_MODE_E eMpuMode
				  );
int vlcmSetOSDColorMode (CMD_LCM_OSD_COLOR_E eOsdColor,
						UCHAR ucKeyColor_R,
						UCHAR ucKeyColor_G,
						UCHAR ucKeyColor_B,
						UCHAR ucKeyMask_R,
						UCHAR ucKeyMask_G,
						UCHAR ucKeyMask_B);
int vlcmGetOSDColorMode (CMD_LCM_OSD_COLOR_E *peOsdColor,
						UCHAR *pucKeyColor_R,
						UCHAR *pucKeyColor_G,
						UCHAR *pucKeyColor_B,
						UCHAR *pucKeyMask_R,
						UCHAR *pucKeyMask_G,
						UCHAR *pucKeyMask_B);

VP_BUFFER_LCM_T *vlcmGetLCMBuffer (void);
VP_BUFFER_OSD_T *vlcmGetOSDBuffer (void);
VP_BUFFER_LCM_T *vlcmSetLCMBuffer (VP_BUFFER_LCM_T *pLCMBuffer);
VP_BUFFER_OSD_T *vlcmSetOSDBuffer (VP_BUFFER_OSD_T *pOSDBuffer);

void vlcmFillLCMBuffer (UCHAR ucR, UCHAR ucG, UCHAR ucB);
void vlcmFillOSDBuffer (UCHAR ucR, UCHAR ucG, UCHAR ucB);

void vlcmGetSize (USHORT *usWidth, USHORT *usHeight);
UINT32 vlcmGetOSDColorWidth (void);
BOOL vlcmIsConfigured (void);
void vlcmStartRefresh (BOOL bTryMode);
void vlcmWaitRefreshOK (void);

void ControlBackLight(int gpio, int value);
void ControlLcmTvPower(int flag);


__inline UINT32 __vlcmGetOSDColorWidth (CMD_LCM_OSD_COLOR_E eOsdColor)
{
	switch (eOsdColor)
	{
		case CMD_LCM_OSD_RGB332:
			return 1;
		case CMD_LCM_OSD_YUV422:
		case CMD_LCM_OSD_YCbCr422:
		case CMD_LCM_OSD_RGB565:
		case CMD_LCM_OSD_RGB444low:
		case CMD_LCM_OSD_RGB444high:
			return 2;
		case CMD_LCM_OSD_RGB888:
		case CMD_LCM_OSD_RGB666:
		default:
			return 4;
	}
}


#endif

