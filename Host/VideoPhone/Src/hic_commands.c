#include "../Inc/inc.h"


#ifdef __HIC_SUPPORT__


void cmdNull (HIC_THREAD_T *pHicThread, UCHAR ucCmd, UCHAR ucSubCmd,
	UCHAR ucD, UCHAR ucC, UCHAR ucB, UCHAR ucA)
{
	hicSaveToReg1 (HIC_ERR_UNKNOWN_CMD);
	sysDisableIRQ();
	while (1);
	hicSetCmdReady (pHicThread);
	
}



/* All HIC commands list here. */
CMD_ENTRY_T g_pCmdEntry_18[] =
{
	{0x00, &cmd18_00_GetFirmwareVersion},
	{0x01, &cmd18_01_GetSystemCapability},
	{0x02, &cmd18_02_GetInterruptEvent},
	{0x03, &cmd18_03_EnableCommandInterrupt},
	{0x04, &cmd18_04_SendMediaBitstream},
	{0x05, &cmd18_05_RecvMediaBitstream},
	{0x06, &cmd18_06_ConfigureLCM},
	{0x07, &cmd18_07_SelectLCM},
	{0x08, &cmd18_08_SetOSDColor},
	{0x09, &cmd18_09_WriteOSD},
	{0x0A, &cmd18_0A_ClearOSD},
	{0x0B, &cmd18_0B_EnableDisplay},
	{0x0C, &cmd18_0C_SetLocalVideoWindow},
	{0x0D, &cmd18_0D_SetRemoteVideoWindow},
	{0x0E, &cmd18_0E_SetLocalVideoSource},
	{0x0F, &cmd18_0F_EnableVideoEncoder},
	{0x10, &cmd18_10_SetVideoZIndex},
	{0x11, &cmd18_11_EnableAudioEncoder},
	{0x12, &cmd18_12_SetAudioQuality},
	{0x13, &cmd18_13_GetAudioQuality},
	{0x14, &cmd18_14_SetVideoBitrate},
	{0x15, &cmd18_15_GetVideoBitrate},
	{0x16, &cmd18_16_SetVideoFormat},
	{0x17, &cmd18_17_GetDecodedMedia},
	{0x18, &cmdNull},	//Do not support it! cmd18_18_EnableVideoBlur},
	{0x19, &cmd18_19_EnableMotionDetect},
	{0x1A, &cmd18_1A_GetMotionsNum},
	{0x1B, &cmd18_1B_SetAudioFormat},
	{0x1C, &cmd18_1C_ForceMP4IFrame},
	{0x1D, &cmd18_1D_StretchVideo},
	{0x1E, &cmd18_1E_EnableJPEGEncoder},
	{0x1F, &cmd18_1F_SetDateTime},
	{0x20, &cmd18_20_SetVideoFramerate},
	{0x21, &cmd18_21_GetVideoFramerate},
	{0x22, &cmd18_22_GetImageDecodeSize},
	{0x23, &cmdNull},
	{0x24, &cmdNull},
	{0x25, &cmdNull},
	{0x26, &cmdNull},
	{0x27, &cmdNull},
	{0x28, &cmdNull},
	{0x29, &cmdNull},
	{0x2A, &cmdNull},
	{0x2B, &cmdNull},
	{0x2C, &cmdNull},
	{0x2D, &cmdNull},
	{0x2E, &cmdNull},
	{0x2F, &cmdNull},
	{0x30, &cmdNull},
	{0x31, &cmdNull},
	{0x32, &cmdNull},
	{0x33, &cmdNull},
	{0x34, &cmdNull},
	{0x35, &cmdNull},
	{0x36, &cmdNull},
	{0x37, &cmdNull},
	{0x38, &cmdNull},
	{0x39, &cmdNull},
	{0x3A, &cmdNull},
	{0x3B, &cmdNull},
	{0x3C, &cmdNull},
	{0x3D, &cmdNull},
	{0x3E, &cmdNull},
	{0x3F, &cmdNull},
	{0x40, &cmdNull},
	{0x41, &cmdNull},
	{0x42, &cmdNull},
	{0x43, &cmdNull},
	{0x44, &cmdNull},
	{0x45, &cmdNull},
	{0x46, &cmdNull},
	{0x47, &cmdNull},
	{0x48, &cmdNull},
	{0x49, &cmdNull},
	{0x4A, &cmdNull},
	{0x4B, &cmdNull},
	{0x4C, &cmdNull},
	{0x4D, &cmdNull},
	{0x4E, &cmdNull},
	{0x4F, &cmdNull},
	{0x50, &cmdNull},
	{0x51, &cmdNull},
	{0x52, &cmdNull},
	{0x53, &cmdNull},
	{0x54, &cmdNull},
	{0x55, &cmdNull},
	{0x56, &cmdNull},
	{0x57, &cmdNull},
	{0x58, &cmdNull},
	{0x59, &cmdNull},
	{0x5A, &cmdNull},
	{0x5B, &cmdNull},
	{0x5C, &cmdNull},
	{0x5D, &cmdNull},
	{0x5E, &cmdNull},
	{0x5F, &cmdNull},
	{0x60, &cmdNull},
	{0x61, &cmdNull},
	{0x62, &cmdNull},
	{0x63, &cmdNull},
	{0x64, &cmdNull},
	{0x65, &cmdNull},
	{0x66, &cmdNull},
	{0x67, &cmdNull},
	{0x68, &cmdNull},
	{0x69, &cmdNull},
	{0x6A, &cmdNull},
	{0x6B, &cmdNull},
	{0x6C, &cmdNull},
	{0x6D, &cmdNull},
	{0x6E, &cmdNull},
	{0x6F, &cmdNull},
	{0x70, &cmdNull},
	{0x71, &cmdNull},
	{0x72, &cmdNull},
	{0x73, &cmdNull},
	{0x74, &cmdNull},
	{0x75, &cmdNull},
	{0x76, &cmdNull},
	{0x77, &cmdNull},
	{0x78, &cmdNull},
	{0x79, &cmdNull},
	{0x7A, &cmdNull},
	{0x7B, &cmdNull},
	{0x7C, &cmdNull},
	{0x7D, &cmdNull},
	{0x7E, &cmdNull},
	{0x7F, &cmdNull},
	{0x80, &cmdNull},
	{0x81, &cmdNull},
	{0x82, &cmdNull},
	{0x83, &cmdNull},
	{0x84, &cmdNull},
	{0x85, &cmdNull},
	{0x86, &cmdNull},
	{0x87, &cmdNull},
	{0x88, &cmdNull},
	{0x89, &cmdNull},
	{0x8A, &cmdNull},
	{0x8B, &cmdNull},
	{0x8C, &cmdNull},
	{0x8D, &cmdNull},
	{0x8E, &cmdNull},
	{0x8F, &cmdNull},
	{0x90, &cmdNull},
	{0x91, &cmdNull},
	{0x92, &cmdNull},
	{0x93, &cmdNull},
	{0x94, &cmdNull},
	{0x95, &cmdNull},
	{0x96, &cmdNull},
	{0x97, &cmdNull},
	{0x98, &cmdNull},
	{0x99, &cmdNull},
	{0x9A, &cmdNull},
	{0x9B, &cmdNull},
	{0x9C, &cmdNull},
	{0x9D, &cmdNull},
	{0x9E, &cmdNull},
	{0x9F, &cmdNull},
	{0xA0, &cmdNull},
	{0xA1, &cmdNull},
	{0xA2, &cmdNull},
	{0xA3, &cmdNull},
	{0xA4, &cmdNull},
	{0xA5, &cmdNull},
	{0xA6, &cmdNull},
	{0xA7, &cmdNull},
	{0xA8, &cmdNull},
	{0xA9, &cmdNull},
	{0xAA, &cmdNull},
	{0xAB, &cmdNull},
	{0xAC, &cmdNull},
	{0xAD, &cmdNull},
	{0xAE, &cmdNull},
	{0xAF, &cmdNull},
	{0xB0, &cmdNull},
	{0xB1, &cmdNull},
	{0xB2, &cmdNull},
	{0xB3, &cmdNull},
	{0xB4, &cmdNull},
	{0xB5, &cmdNull},
	{0xB6, &cmdNull},
	{0xB7, &cmdNull},
	{0xB8, &cmdNull},
	{0xB9, &cmdNull},
	{0xBA, &cmdNull},
	{0xBB, &cmdNull},
	{0xBC, &cmdNull},
	{0xBD, &cmdNull},
	{0xBE, &cmdNull},
	{0xBF, &cmdNull},
	{0xC0, &cmdNull},
	{0xC1, &cmdNull},
	{0xC2, &cmdNull},
	{0xC3, &cmdNull},
	{0xC4, &cmdNull},
	{0xC5, &cmdNull},
	{0xC6, &cmdNull},
	{0xC7, &cmdNull},
	{0xC8, &cmdNull},
	{0xC9, &cmdNull},
	{0xCA, &cmdNull},
	{0xCB, &cmdNull},
	{0xCC, &cmdNull},
	{0xCD, &cmdNull},
	{0xCE, &cmdNull},
	{0xCF, &cmdNull},
	{0xD0, &cmdNull},
	{0xD1, &cmdNull},
	{0xD2, &cmdNull},
	{0xD3, &cmdNull},
	{0xD4, &cmdNull},
	{0xD5, &cmdNull},
	{0xD6, &cmdNull},
	{0xD7, &cmdNull},
	{0xD8, &cmdNull},
	{0xD9, &cmdNull},
	{0xDA, &cmdNull},
	{0xDB, &cmdNull},
	{0xDC, &cmdNull},
	{0xDD, &cmdNull},
	{0xDE, &cmdNull},
	{0xDF, &cmdNull},
	{0xE0, &cmdNull},
	{0xE1, &cmdNull},
	{0xE2, &cmdNull},
	{0xE3, &cmdNull},
	{0xE4, &cmdNull},
	{0xE5, &cmdNull},
	{0xE6, &cmdNull},
	{0xE7, &cmdNull},
	{0xE8, &cmdNull},
	{0xE9, &cmdNull},
	{0xEA, &cmdNull},
	{0xEB, &cmdNull},
	{0xEC, &cmdNull},
	{0xED, &cmdNull},
	{0xEE, &cmdNull},
	{0xEF, &cmdNull},
	{0xF0, &cmdNull},
	{0xF1, &cmdNull},
	{0xF2, &cmdNull},
	{0xF3, &cmdNull},
	{0xF4, &cmdNull},
	{0xF5, &cmdNull},
	{0xF6, &cmdNull},
	{0xF7, &cmdNull},
	{0xF8, &cmdNull},
	{0xF9, &cmdNull},
	{0xFA, &cmdNull},
	{0xFB, &cmdNull},
	{0xFC, &cmdNull},
	{0xFD, &cmdNull},
	{0xFE, &cmdNull},
	{0xFF, &cmd18_FF_GetDebugBuffer},
};


CMD_ENTRY_T g_pCmdEntry_57[] =
{
	{0x00, &cmd57_00_GetFirmwareVersion},
	{0x01, &cmdNull},
	{0x02, &cmdNull},
	{0x03, &cmdNull},
	{0x04, &cmdNull},
	{0x05, &cmdNull},
	{0x06, &cmdNull},
	{0x07, &cmdNull},
	{0x08, &cmdNull},
	{0x09, &cmdNull},
	{0x0A, &cmdNull},
	{0x0B, &cmdNull},
	{0x0C, &cmdNull},
	{0x0D, &cmdNull},
	{0x0E, &cmdNull},
	{0x0F, &cmdNull},
	{0x10, &cmdNull},
	{0x11, &cmdNull},
	{0x12, &cmdNull},
	{0x13, &cmdNull},
	{0x14, &cmdNull},
	{0x15, &cmdNull},
	{0x16, &cmdNull},
	{0x17, &cmdNull},
	{0x18, &cmdNull},
	{0x19, &cmdNull},
	{0x1A, &cmdNull},
	{0x1B, &cmdNull},
	{0x1C, &cmdNull},
	{0x1D, &cmdNull},
	{0x1E, &cmdNull},
	{0x1F, &cmdNull},
	{0x20, &cmdNull},
	{0x21, &cmdNull},
	{0x22, &cmdNull},
	{0x23, &cmdNull},
	{0x24, &cmdNull},
	{0x25, &cmdNull},
	{0x26, &cmdNull},
	{0x27, &cmdNull},
	{0x28, &cmdNull},
	{0x29, &cmdNull},
	{0x2A, &cmdNull},
	{0x2B, &cmdNull},
	{0x2C, &cmdNull},
	{0x2D, &cmdNull},
	{0x2E, &cmdNull},
	{0x2F, &cmdNull},
	{0x30, &cmdNull},
	{0x31, &cmdNull},
	{0x32, &cmdNull},
	{0x33, &cmdNull},
	{0x34, &cmdNull},
	{0x35, &cmdNull},
	{0x36, &cmdNull},
	{0x37, &cmdNull},
	{0x38, &cmdNull},
	{0x39, &cmdNull},
	{0x3A, &cmdNull},
	{0x3B, &cmdNull},
	{0x3C, &cmdNull},
	{0x3D, &cmdNull},
	{0x3E, &cmdNull},
	{0x3F, &cmdNull},
	{0x40, &cmdNull},
	{0x41, &cmdNull},
	{0x42, &cmdNull},
	{0x43, &cmdNull},
	{0x44, &cmdNull},
	{0x45, &cmdNull},
	{0x46, &cmdNull},
	{0x47, &cmdNull},
	{0x48, &cmdNull},
	{0x49, &cmdNull},
	{0x4A, &cmdNull},
	{0x4B, &cmdNull},
	{0x4C, &cmdNull},
	{0x4D, &cmdNull},
	{0x4E, &cmdNull},
	{0x4F, &cmdNull},
	{0x50, &cmd57_50_ConfigureLCM},
	{0x51, &cmdNull},
	{0x52, &cmdNull},
	{0x53, &cmdNull},
	{0x54, &cmdNull},
	{0x55, &cmdNull},
	{0x56, &cmd57_56_SuspendModule},
	{0x57, &cmdNull},
	{0x58, &cmdNull},
	{0x59, &cmdNull},
	{0x5A, &cmdNull},
	{0x5B, &cmdNull},
	{0x5C, &cmdNull},
	{0x5D, &cmdNull},
	{0x5E, &cmdNull},
	{0x5F, &cmdNull},
	{0x60, &cmdNull},
	{0x61, &cmdNull},
	{0x62, &cmdNull},
	{0x63, &cmdNull},
	{0x64, &cmdNull},
	{0x65, &cmdNull},
	{0x66, &cmdNull},
	{0x67, &cmdNull},
	{0x68, &cmd57_68_EnableOSD},
	{0x69, &cmd57_69_SetOSDKey},
	{0x6A, &cmd57_6A_SendOSDPattern},
	{0x6B, &cmdNull},
	{0x6C, &cmdNull},
	{0x6D, &cmdNull},
	{0x6E, &cmdNull},
	{0x6F, &cmdNull},
	{0x70, &cmdNull},
	{0x71, &cmdNull},
	{0x72, &cmdNull},
	{0x73, &cmdNull},
	{0x74, &cmdNull},
	{0x75, &cmdNull},
	{0x76, &cmdNull},
	{0x77, &cmdNull},
	{0x78, &cmdNull},
	{0x79, &cmdNull},
	{0x7A, &cmdNull},
	{0x7B, &cmdNull},
	{0x7C, &cmdNull},
	{0x7D, &cmdNull},
	{0x7E, &cmdNull},
	{0x7F, &cmdNull},
	{0x80, &cmdNull},
	{0x81, &cmdNull},
	{0x82, &cmdNull},
	{0x83, &cmdNull},
	{0x84, &cmdNull},
	{0x85, &cmdNull},
	{0x86, &cmdNull},
	{0x87, &cmdNull},
	{0x88, &cmdNull},
	{0x89, &cmdNull},
	{0x8A, &cmdNull},
	{0x8B, &cmdNull},
	{0x8C, &cmdNull},
	{0x8D, &cmdNull},
	{0x8E, &cmdNull},
	{0x8F, &cmdNull},
	{0x90, &cmdNull},
	{0x91, &cmdNull},
	{0x92, &cmdNull},
	{0x93, &cmdNull},
	{0x94, &cmdNull},
	{0x95, &cmdNull},
	{0x96, &cmdNull},
	{0x97, &cmdNull},
	{0x98, &cmdNull},
	{0x99, &cmdNull},
	{0x9A, &cmdNull},
	{0x9B, &cmdNull},
	{0x9C, &cmdNull},
	{0x9D, &cmdNull},
	{0x9E, &cmdNull},
	{0x9F, &cmdNull},
	{0xA0, &cmdNull},
	{0xA1, &cmdNull},
	{0xA2, &cmdNull},
	{0xA3, &cmdNull},
	{0xA4, &cmdNull},
	{0xA5, &cmdNull},
	{0xA6, &cmdNull},
	{0xA7, &cmdNull},
	{0xA8, &cmdNull},
	{0xA9, &cmdNull},
	{0xAA, &cmdNull},
	{0xAB, &cmdNull},
	{0xAC, &cmdNull},
	{0xAD, &cmdNull},
	{0xAE, &cmdNull},
	{0xAF, &cmdNull},
	{0xB0, &cmdNull},
	{0xB1, &cmdNull},
	{0xB2, &cmdNull},
	{0xB3, &cmdNull},
	{0xB4, &cmdNull},
	{0xB5, &cmdNull},
	{0xB6, &cmdNull},
	{0xB7, &cmdNull},
	{0xB8, &cmdNull},
	{0xB9, &cmdNull},
	{0xBA, &cmdNull},
	{0xBB, &cmdNull},
	{0xBC, &cmdNull},
	{0xBD, &cmdNull},
	{0xBE, &cmdNull},
	{0xBF, &cmdNull},
	{0xC0, &cmdNull},
	{0xC1, &cmdNull},
	{0xC2, &cmdNull},
	{0xC3, &cmdNull},
	{0xC4, &cmdNull},
	{0xC5, &cmdNull},
	{0xC6, &cmdNull},
	{0xC7, &cmdNull},
	{0xC8, &cmdNull},
	{0xC9, &cmdNull},
	{0xCA, &cmdNull},
	{0xCB, &cmdNull},
	{0xCC, &cmdNull},
	{0xCD, &cmdNull},
	{0xCE, &cmdNull},
	{0xCF, &cmdNull},
	{0xD0, &cmdNull},
	{0xD1, &cmdNull},
	{0xD2, &cmdNull},
	{0xD3, &cmdNull},
	{0xD4, &cmdNull},
	{0xD5, &cmdNull},
	{0xD6, &cmdNull},
	{0xD7, &cmdNull},
	{0xD8, &cmdNull},
	{0xD9, &cmdNull},
	{0xDA, &cmdNull},
	{0xDB, &cmdNull},
	{0xDC, &cmdNull},
	{0xDD, &cmdNull},
	{0xDE, &cmdNull},
	{0xDF, &cmdNull},
	{0xE0, &cmdNull},
	{0xE1, &cmdNull},
	{0xE2, &cmdNull},
	{0xE3, &cmdNull},
	{0xE4, &cmdNull},
	{0xE5, &cmdNull},
	{0xE6, &cmdNull},
	{0xE7, &cmdNull},
	{0xE8, &cmdNull},
	{0xE9, &cmdNull},
	{0xEA, &cmdNull},
	{0xEB, &cmdNull},
	{0xEC, &cmdNull},
	{0xED, &cmdNull},
	{0xEE, &cmdNull},
	{0xEF, &cmdNull},
	{0xF0, &cmdNull},
	{0xF1, &cmdNull},
	{0xF2, &cmdNull},
	{0xF3, &cmdNull},
	{0xF4, &cmdNull},
	{0xF5, &cmdNull},
	{0xF6, &cmdNull},
	{0xF7, &cmdNull},
	{0xF8, &cmdNull},
	{0xF9, &cmdNull},
	{0xFA, &cmdNull},
	{0xFB, &cmdNull},
	{0xFC, &cmdNull},
	{0xFD, &cmdNull},
	{0xFE, &cmdNull},
	{0xFF, &cmdNull},
};


UINT32 g_uCmdEntryNum_18 = sizeof (g_pCmdEntry_18) / sizeof (g_pCmdEntry_18[0]);
UINT32 g_uCmdEntryNum_57 = sizeof (g_pCmdEntry_57) / sizeof (g_pCmdEntry_57[0]);

#endif




