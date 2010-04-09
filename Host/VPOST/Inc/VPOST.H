/****************************************************************************
 * 
 * FILENAME
 *     VPOST.h
 *
 * VERSION
 *     1.0 
 *
 * DESCRIPTION
 *
 *
 *
 *
 * DATA STRUCTURES
 *     None
 *
 * FUNCTIONS
 *
 *
 *     
 * HISTORY
 *     2006.03.28		Rework
 *
 *
 * REMARK
 *     None
 *
 *
 **************************************************************************/



#ifndef __VPOST_H
#define __VPOST_H

//#define MPU_LCD
#define SYNC_BASE

#define LCM_ERR_CRC     		0xFFFF0401	/* LCM library ID */
/* LCM */
#define SAMSUNG_80_16BIT 0
#define TOPPOLY_80_16BIT 1
#define TOPPOLY_80_8BIT 2
#define TOPPOLY_80_9BIT 3
#define TOPPOLY_80_18BIT 4
#define TOPPOLY_68_16BIT 5
#define TOPPOLY_68_8BIT 6
#define TOPPOLY_68_9BIT 7
#define TOPPOLY_68_18BIT 8
#define SAMSUNG_80_9BIT 9
#define SAMSUNG_80_18BIT 10
#define HITACHI_80_18BIT 11
#define NEC_80_18BIT 12

////sync-bass
#define OPT_NTSC 13
#define OPT_NTSC_NONINT_PATCH 14

#define OPT_EPSON 15
#define OPT_UNIPAC 16
#define OPT_PAL 17
#define OPT_CASIO 18
#define OPT_UNIPAC_NONINT	30//clyu
#define OPT_TPG038D	31
#define OPT_TOPPOLY_TDA025 32

/*VPOST Interrupt*/
#define VPOST_Disp_F 1
#define VPOST_UNDERRUN 2
#define VPOST_BUS_ERROR 3

/*usSRc Video*/
#define VA_YUV422 1
#define VA_YCbCr422 2
#define VA_RGB888 3
#define VA_RGB666 4
#define VA_RGB565 5
#define VA_RGB444low 6
#define VA_RGB444high 7

/*usSRc OSD*/
#define OSD_YUV422 1
#define OSD_YCbCr422 2
#define OSD_RGB888 3
#define OSD_RGB666 4
#define OSD_RGB565 5
#define OSD_RGB444low 6
#define OSD_RGB444high 7
#define OSD_RGB332 8

/*Single mode*/
#define DISPLAY_SINGLE 1
#define DISPLAY_CONTINUOUS 2

/*OSD Display mode*/
#define MATCH_V_UNMATCH_OSD 1
#define MATCH_V_UNMATCH_S 2
#define MATCH_OSD_UNMATCH_V 3
#define MATCH_OSD_UNMATCH_S 4
#define MATCH_S_UNMATCH_V 5
#define MATCH_S_UNMATCH_OSD 6
#define MATCH_OSD_UNMATCH_OSD 7

/*VA_CRTC*/
#define CRTC_Retrace_Offset 64
#define CRTC_Retrace_HOffset 6

#define OSD_HUP_2X  1
#define OSD_VUP_2X  1
#define OSD_VUP_4X  2

#define SCALE_MODE_INTERPOLATION 0
#define SCALE_MODE_DUPLICATION   1

#define SHOW_MODE_VIDEO     0
#define SHOW_MODE_OSD       1
#define SHOW_MODE_SYNTHESIS 2

#define MPU_CmdHigh    0
#define MPU_CmdLow     1

#define MPU_Cm16t18_L0     0
#define MPU_Cm16t18_H0     1

#define MPU_CmdBus_Width_8BIT     0
#define MPU_CmdBus_Width_16BIT    1

#define MPU_DataBus_Width_8BIT     0
#define MPU_DataBus_Width_9BIT     1
#define MPU_DataBus_Width_16BIT    2
#define MPU_DataBus_Width_18BIT    3

#define MPU_68_MODE     0
#define MPU_80_MODE     1

#define MPU_4096_Col_MODE     0
#define MPU_64k_Col_MODE      1
#define MPU_256k_Col_MODE     2
#define MPU_True_Col_MODE     3

#define TV_INTERLACE 0
#define TV_NONINTERLACE 1

typedef struct {
    UINT16 	usDevWidth;
    UINT16	usDevHeigth;
    UINT8       ucCmdLow;
    UINT8       ucCmd16t18;
    UINT8       ucCmdBusWidth;
    UINT8	ucDataBusWidth;
    UINT8       ucMPU_Mode;
    UINT8       ucMPU_Color_Mode;	
} MPU_Dev_Type_T;

void vpostEnable_Int(void);
void vpostDisable_Int(void);
void vpostSet_Irq(UINT8 ucIntType,PVOID pvVPOSTISR);

void vpostVA_LCMInitSetting(UINT16 usDevWidth,UINT16 usDevHeigth,UINT8 ucCmdLow,UINT8 ucCmd16t18,
                        UINT8 ucCmdBusWidth,UINT8 ucDataBusWidth,UINT8 ucMPU_Mode,UINT8 ucMPU_Color_Mode);
void vpostVA_MPUInit(UINT8 ucLCM_NUM,UINT8 ucVASrc_Type,UINT8 ucVADisMode,UINT32 udisp_addr0,UINT8 ucscall_mode,UINT16 usPic_Width);
void vpostSetupMpuLCD(UINT8 ucDev_Type,UINT8 ROT90);
//VA
void RGBHardWareRegistersInit(void);
//void vpostLCM_Select(UINT8 ucLCM_NUM);
void vpostVA_Init(UINT8 ucLCM_NUM,UINT8 ucDev_Type,UINT8 ucVASrc_Type,UINT8 ucVADisMode,UINT32 udisp_addr0,UINT8 ucscall_mode,UINT16 usPic_Width,UINT8 ucROT90);
void vpostVA_Enable(void);
void vpostVA_Disable(void);
void vpostVA_Stream_Scaling_Ctrl(UINT8 ucScall_V_Round,UINT16 usScall_V_Decimal,
                                  UINT8 ucScall_H_Round,UINT16 usScall_H_Decimal,UINT8 ucscall_mode);
void vpostVA_Trigger(void);
void vpostVA_DiSBuf0_Set(UINT32 uSrc_Buffer0);
void vpostVA_DiSBuf1_Set(UINT32 uSrc_Buffer1);

//LCM_Infomation
void vpostGet_LCM_Info(UINT8 ucDev_Type,UINT16 *usLCM_Width,UINT16 *usLCM_Height);

//OSD
void vpostOSD_Enable(void);
void vpostOSD_Disable(void);
void vpostOSD_Init(UINT8 ucDev_Type,UINT8 ucOSDSrc_Type,UINT16 usxst,UINT16 ususyst,UINT16 usxed,UINT16 usyed,UINT32 uOSD_Src_Addr,UINT16 usPic_OSD_Width);
void vpostOSD_DispBuf_Set(UINT32 uOSD_Buffer);
void vpostOSD_ColKeyMask_Trigger(UINT8 ucMatch_UnMatch_Type,UINT32 uCKey_Value,UINT32 uCMask_Value,UINT8 ucOSD_Synthesis_Weight);
//void vpostOSD_Blink_Display(UINT8 ucMatch_UnMatch_Type,UINT8 ucOSD_Blk_Vcnt,UINT8 ucOSD_Synthesis_Weight);
void vpostOSD_Scaling_Control(UINT8 ucScall_V_Round,UINT8 ucScall_H_Round);
void vpostOSD_Blinking_Setting(UINT8 ucOSD_Blink_Vcnt);

//CRC_Check
int vpostCheck_CRC(UINT16 usSR,UINT16 usSG,UINT16 usSB);

//TV
#ifdef SYNC_BASE
void vpostSetTVDispMode(UINT8 ucTVmode);
void TPG038D_WriteLCMRegister(UINT8 addr,UINT8 data);//clyu add
#endif
#endif /* __VPOST_H */
