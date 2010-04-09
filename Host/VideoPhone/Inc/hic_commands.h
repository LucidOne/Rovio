#ifndef __HIC_COMMMANDS_H__
#define __HIC_COMMMANDS_H__

#ifdef __HIC_SUPPORT__

#define DECLARE_HIC_CMD(cmd) \
	void (cmd) (HIC_THREAD_T *pHicThread, UCHAR ucCmd, UCHAR ucSubCmd, UCHAR ucD, UCHAR ucC, UCHAR ucB, UCHAR ucA)
	
DECLARE_HIC_CMD (cmdNull);
DECLARE_HIC_CMD (cmd18_00_GetFirmwareVersion);
DECLARE_HIC_CMD (cmd18_01_GetSystemCapability);
DECLARE_HIC_CMD (cmd18_02_GetInterruptEvent);
DECLARE_HIC_CMD (cmd18_03_EnableCommandInterrupt);
DECLARE_HIC_CMD (cmd18_04_SendMediaBitstream);
DECLARE_HIC_CMD (cmd18_05_RecvMediaBitstream);
DECLARE_HIC_CMD (cmd18_06_ConfigureLCM);
DECLARE_HIC_CMD (cmd18_07_SelectLCM);
DECLARE_HIC_CMD (cmd18_08_SetOSDColor);
DECLARE_HIC_CMD (cmd18_09_WriteOSD);
DECLARE_HIC_CMD (cmd18_0A_ClearOSD);
DECLARE_HIC_CMD (cmd18_0B_EnableDisplay);
DECLARE_HIC_CMD (cmd18_0C_SetLocalVideoWindow);
DECLARE_HIC_CMD (cmd18_0D_SetRemoteVideoWindow);
DECLARE_HIC_CMD (cmd18_0E_SetLocalVideoSource);
DECLARE_HIC_CMD (cmd18_0F_EnableVideoEncoder);
DECLARE_HIC_CMD (cmd18_10_SetVideoZIndex);
DECLARE_HIC_CMD (cmd18_11_EnableAudioEncoder);
DECLARE_HIC_CMD (cmd18_12_SetAudioQuality);
DECLARE_HIC_CMD (cmd18_13_GetAudioQuality);
DECLARE_HIC_CMD (cmd18_14_SetVideoBitrate);
DECLARE_HIC_CMD (cmd18_15_GetVideoBitrate);
DECLARE_HIC_CMD (cmd18_16_SetVideoFormat);
DECLARE_HIC_CMD (cmd18_17_GetDecodedMedia);
DECLARE_HIC_CMD (cmd18_18_EnableVideoBlur);
DECLARE_HIC_CMD (cmd18_19_EnableMotionDetect);
DECLARE_HIC_CMD (cmd18_1A_GetMotionsNum);
DECLARE_HIC_CMD (cmd18_1B_SetAudioFormat);
DECLARE_HIC_CMD (cmd18_1C_ForceMP4IFrame);
DECLARE_HIC_CMD (cmd18_1D_StretchVideo);
DECLARE_HIC_CMD (cmd18_1E_EnableJPEGEncoder);
DECLARE_HIC_CMD (cmd18_1F_SetDateTime);
DECLARE_HIC_CMD (cmd18_20_SetVideoFramerate);
DECLARE_HIC_CMD (cmd18_21_GetVideoFramerate);
DECLARE_HIC_CMD (cmd18_22_GetImageDecodeSize);
DECLARE_HIC_CMD (cmd18_FF_GetDebugBuffer);

DECLARE_HIC_CMD (cmd57_00_GetFirmwareVersion);
DECLARE_HIC_CMD (cmd57_50_ConfigureLCM);
DECLARE_HIC_CMD (cmd57_56_SuspendModule);
DECLARE_HIC_CMD (cmd57_68_EnableOSD);
DECLARE_HIC_CMD (cmd57_69_SetOSDKey);
DECLARE_HIC_CMD (cmd57_6A_SendOSDPattern);



typedef DECLARE_HIC_CMD (*FUN_CMD_PROC);


typedef struct
{
	UCHAR			ucSubCmd;	//Register E;
	FUN_CMD_PROC	fnCmdProcessor;
} CMD_ENTRY_T;


extern CMD_ENTRY_T	g_pCmdEntry_18[];
extern UINT32		g_uCmdEntryNum_18;

extern CMD_ENTRY_T	g_pCmdEntry_57[];
extern UINT32		g_uCmdEntryNum_57;



void __cmd18_03_EnableCommandInterrupt_CheckIntr (void);

#endif

#endif

