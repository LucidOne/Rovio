 /****************************************************************************
 * 
 * FILENAME
 *     VPOST.c
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


#ifdef ECOS

#include "drv_api.h"

#endif

#include "wbtypes.h"
#include "wbio.h"
#include "wblib.h"
#include "vpost.h"
#include "w99702_reg.h"

#define _LCM_SDO	(1<<4)	//GPIOA-04
#define _802_SCL	(1<<1)	//GPIO-01
#define _802_SDA	(1<<5)	//GPIO-05

/* Interrupt Handler Table */
typedef void (*_gVPOSTFunPtr)();   /* function pointer */
_gVPOSTFunPtr WB_VPOSTIrqHandlerTable[3]= {0 ,0 ,0};

MPU_Dev_Type_T _vpostLCMInfo_t;

#ifdef ECOS
cyg_handle_t	int_handle_vpost;
cyg_interrupt	int_holder_vpost;
#endif

void vpostIntHandler(void);
void vpostDisp_F_ISR(void);
void vpostUNDERRUN_ISR(void);
void vpostBUS_ERROR_ISR(void);
//Set_MPU_LCD
void vpostLCDWriteCMDAddr(UINT8 Dev_T,UINT16 uscmd);
void vpostLCDWriteCMDReg(UINT8 Dev_T,UINT16 uscmd);
UINT32 vpostLCDReadCMDReg(UINT8 Dev_T);
void vpostInitialFunction1(UINT8 Dev_T,UINT8 ROT90Flg);
void vpostInitialFunction2(UINT8 Dev_T);
void vpostPowerSettingFunction(UINT8 Dev_T);
void vpostDisplayOffFunction(UINT8 Dev_T);
void vpostDisplayOnFunction(UINT8 Dev_T,UINT8 ROT90Flg);
void vpostdelay_long(UINT16 usdelaycnt);
//void vpostSetupMpuLCD(UINT8 ucDev_Type,UINT8 ROT90);
//VA
void vpostDisplay_Mode(UINT8 ucvdismode);
void vpostDual_Buffer_Switch(void);
void vpostVA_MoveXY(UINT16 usxp,UINT16 usyp,UINT32 upic_addr,UINT16 usPic_Width);

void vpostCRTC_Setting(UINT8 Dev_T,UINT8 ucVA_Type,UINT16 uswidth,UINT16 ushigth,UINT16 usPic_Width);
//OSD
void vpostOSD_Window_Set(UINT16 usxstart,UINT16 usxend,UINT16 usystart,UINT16 usyend,UINT32 uOSD_Src);
void vpostOSD_Setting(UINT8 ucMatch_OSD_Mode,UINT8 ucUnMatch_OSD_Mode,UINT8 ucOSD_Syn_Weight);
void vpostColKey_Setting(UINT32 uCKey_Val);
void vpostColMask_Setting(UINT32 uCMask_Val);
void vpostOSD_DispBuf_Set(UINT32 uOSD_Buffer);

void vpostHitachiInit(UINT8 Dev_T,UINT8 ROT90Flg);
void vpostHitachiDisplayOn(UINT8 Dev_T,UINT8 ROT90Flg);

void vpostNECInit(UINT8 Dev_T,UINT8 ROT90Flg);

/* sjlu 20050512*/
void TPG038D_WriteLCMRegister(UINT8 addr,UINT8 data);
UINT8 TPG038D_ReadLCMRegister(UINT8 addr);
void vpostInitAtFirst(void);
void InitialTda025LCM(void);

UINT32 gint_status;
#ifdef ECOS
cyg_uint32 VPOST_INT_Handler(cyg_vector_t vector, cyg_addrword_t data)
#else
void vpostIntHandler(void)
#endif
{
#ifndef ECOS
   /* clear VPOST interrupt state */
   UINT32 tmp;
   
   tmp = readw(REG_LCM_INT_CS);
   if (tmp&0x40000000)
      vpostDisp_F_ISR();
   else if (tmp&0x20000000)
      vpostUNDERRUN_ISR();
   else if (tmp&0x10000000)
      vpostBUS_ERROR_ISR();
#else
	cyg_interrupt_mask(vector);
	gint_status = readw(REG_LCM_INT_CS);
#endif

#ifdef ECOS
	cyg_interrupt_acknowledge(vector);
//	return CYG_ISR_HANDLED;
	return CYG_ISR_CALL_DSR;
#endif
}

#ifdef ECOS
void VPOST_INT_HandlerDSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
   if (gint_status&0x40000000)
      vpostDisp_F_ISR();
   else if (gint_status&0x20000000)
      vpostUNDERRUN_ISR();
   else if (gint_status&0x10000000)
      vpostBUS_ERROR_ISR();
   
   cyg_interrupt_unmask(vector);
}
#endif

void vpostSet_Irq(UINT8 ucIntType,PVOID pvVPOSTISR)
{
    if (ucIntType==VPOST_Disp_F)
        WB_VPOSTIrqHandlerTable[0]=(_gVPOSTFunPtr)(pvVPOSTISR);
    else if (ucIntType==VPOST_UNDERRUN)
        WB_VPOSTIrqHandlerTable[1]=(_gVPOSTFunPtr)(pvVPOSTISR);
    else if (ucIntType==VPOST_BUS_ERROR)
        WB_VPOSTIrqHandlerTable[2]=(_gVPOSTFunPtr)(pvVPOSTISR);
}

#if 0
void vpostDisp_F_ISR(void)
{
    if (WB_VPOSTIrqHandlerTable[0]!=0)
        WB_VPOSTIrqHandlerTable[0]();
    writew(REG_LCM_INT_CS,readw(REG_LCM_INT_CS)|0x40000000);
}
#else
void vpostDisp_F_ISR(void)
{
    writew(REG_LCM_INT_CS,readw(REG_LCM_INT_CS)|0x40000000);
    if (WB_VPOSTIrqHandlerTable[0]!=0)
        WB_VPOSTIrqHandlerTable[0]();
}
#endif

void vpostUNDERRUN_ISR(void)
{
    if (WB_VPOSTIrqHandlerTable[1]!=0)
        WB_VPOSTIrqHandlerTable[1]();
    writew(REG_LCM_INT_CS,readw(REG_LCM_INT_CS)|0x20000000);
}

void vpostBUS_ERROR_ISR(void)
{
    if (WB_VPOSTIrqHandlerTable[2]!=0)
        WB_VPOSTIrqHandlerTable[2]();
    writew(REG_LCM_INT_CS,readw(REG_LCM_INT_CS)|0x10000000);
}

void vpostEnable_Int(void)
{
#ifdef ECOS
	cyg_interrupt_disable();
    cyg_interrupt_create(IRQ_LCM, 1, 0, VPOST_INT_Handler, VPOST_INT_HandlerDSR, &int_handle_vpost, &int_holder_vpost);
    cyg_interrupt_attach(int_handle_vpost);
    cyg_interrupt_unmask(IRQ_LCM);
    cyg_interrupt_enable();
#else
    sysInstallISR(IRQ_LEVEL_1, IRQ_LCM, (PVOID)vpostIntHandler);
    /* enable VPOST interrupt */
    sysEnableInterrupt(IRQ_LCM);
#endif
    writew(REG_LCM_DCCS,readw(REG_LCM_DCCS)|0x00000010);
    writew(REG_LCM_INT_CS,readw(REG_LCM_INT_CS)|0x00000003);
}

void vpostDisable_Int(void)
{
    writew(REG_LCM_DCCS,readw(REG_LCM_DCCS)&0xfffffef);
    writew(REG_LCM_INT_CS,readw(REG_LCM_INT_CS)&0xfffffffc);
}

void vpostdelay_long(UINT16 usdelaycnt)
{
    UINT16 i=usdelaycnt;                  //40ms/10us=20000
    while(i--);                     //i=i-1 & cjne > 10us
}

void vpostDisplay_Mode(UINT8 ucvdismode)
{
    if (ucvdismode==DISPLAY_SINGLE)
        writew(REG_LCM_DCCS,readw(REG_LCM_DCCS)|0x00000080); //rgb565,single 
    else if (ucvdismode==DISPLAY_CONTINUOUS)
        writew(REG_LCM_DCCS,readw(REG_LCM_DCCS)&0xffffff7f); //rgb565,continous
}


void vpostVASrc(UINT8 ucVASrc_Type)
{
		
	unsigned int regDCCS;
	
	regDCCS = readw(REG_LCM_DCCS)&0xfffff0ff; // Clear VA_SRC setting
	
	switch (ucVASrc_Type)
	{
		
		case VA_YUV422:
			break;
		
		case VA_YCbCr422:
			regDCCS |= 0x00000100;
			break;
		case VA_RGB565:
			regDCCS |= 0x00004400;//clyu 0x00000400 to 0x00004400 for support RGB through mode
			break;
		case VA_RGB888:
			regDCCS |= 0x00000200;
			break;
		case VA_RGB666:
			regDCCS |= 0x00000300;
			break;
		case VA_RGB444low:
			regDCCS |= 0x00000500;
			break;
		case VA_RGB444high:
			regDCCS |= 0x00000700;
			break;
		default: // YUV422
			break;
	
	
	}
	 writew(REG_LCM_DCCS, regDCCS);

}

void vpostDeviceControl(UINT8 ucDev_Type, UINT8 ucVASrc_Type)
{
	unsigned int regDeviceCtrl;
	
	switch(ucDev_Type) {
		case OPT_NTSC:
			regDeviceCtrl = 0xA1C10004; //8-bit,65536,packed YUV, CNLi
			break;
	         
    	case OPT_PAL:
         	regDeviceCtrl = 0xA1C10000; //8-bit,65536,packed YUV, CNLi //clyu 0xA1C00004-->0xA1C00000
         	break;
         	
     	case OPT_NTSC_NONINT_PATCH:
        	 regDeviceCtrl = 0xA1000004; //8-bit,65536,packed YUV, CNLi
         	break;
         	
     	case OPT_EPSON:
         	regDeviceCtrl = 0xA1CC00a0; //8-bit,262144,epson, CNLi
         	break;
         	
   		case OPT_UNIPAC:
         	regDeviceCtrl = 0xA1EC6b90; //8-bit,65536,unipac, CNLi
         	break;
         	
     	case OPT_UNIPAC_NONINT://clyu
        	regDeviceCtrl = 0xA1020082; //8-bit,65536,unipac, Non-interlace
        	break;
        	
     	case OPT_TPG038D://clyu
         	regDeviceCtrl = 0xA1010002; //8-bit,65536,unipac, Non-interlace
         	break;
         	
     	case OPT_TOPPOLY_TDA025://clyu
       		//regDeviceCtrl = 0x13C90006; //8-bit,True color, interlace
       		regDeviceCtrl = 0x13C90002;
         	break;
         	
    	case OPT_CASIO:
        	regDeviceCtrl = 0xA1380080; //8-bit,262144,CASIO, CNLi
        	break;
		default: // Set to NTSC TV mode
			regDeviceCtrl = 0xA1C10004; //8-bit,65536,packed YUV, CNLi
			break;
			        	
	}
	 writew(REG_LCM_DEV_CTRL, regDeviceCtrl);
	 
}

void vpostVAScale(UINT8 ucDev_Type, UINT8 ucscale_mode, UINT8 ucVASrc_Type)
{

		switch(ucDev_Type)
		{
		
#ifdef MPU_LCD
			case TOPPOLY_80_8BIT:
			case TOPPOLY_80_9BIT:
			case TOPPOLY_68_8BIT:
			case TOPPOLY_68_9BIT:
				vpostVA_Stream_Scaling_Ctrl(1,0,2,0,ucscale_mode);
				break;
#endif		
				
#ifdef SYNC_BASE
			case OPT_TOPPOLY_TDA025://clyu
		   			vpostVA_Stream_Scaling_Ctrl(2,0,2,0,SCALE_MODE_DUPLICATION);
		   		
         		break;
         		
         	case OPT_UNIPAC_NONINT: //clyu
           		vpostVA_Stream_Scaling_Ctrl(1,0,3,0,SCALE_MODE_DUPLICATION);
         		break;
		 		
		 	case OPT_TPG038D://clyu
           		vpostVA_Stream_Scaling_Ctrl(1,0,2,0,SCALE_MODE_DUPLICATION);
         		break;
			
			case OPT_EPSON:
			case OPT_UNIPAC:
				vpostVA_Stream_Scaling_Ctrl(2,0,2,0, ucscale_mode);
			 	break;
#endif			 	
			default:			 	
			 	vpostVA_Stream_Scaling_Ctrl(1,0,1,0,SCALE_MODE_DUPLICATION);
			 	break;
		}
}		

void vpostVA_Init(UINT8 ucLCM_NUM,UINT8 ucDev_Type,UINT8 ucVASrc_Type,
		UINT8 ucVADisMode,UINT32 udisp_addr0,UINT8 ucscall_mode,UINT16 usPic_Width,UINT8 ucROT90)
{
	 if (ucDev_Type==OPT_TOPPOLY_TDA025)//clyu
	 {
	 	vpostInitAtFirst();
		InitialTda025LCM();
	 }
     
     if (ucLCM_NUM==0)
       writew(REG_MISCR,0x00);
     else if (ucLCM_NUM==1)
       writew(REG_MISCR,0x01);
     
     vpostVA_DiSBuf0_Set(udisp_addr0);
//    vpostVA_DiSBuf1_Set(disp_addr1);
    
     vpostDisplay_Mode(ucVADisMode);
     
     vpostVASrc(ucVASrc_Type);

         
#ifdef SYNC_BASE
	vpostDeviceControl(ucDev_Type, ucVASrc_Type);

#endif

#ifdef MPU_LCD // added by Jllin
     if (ucDev_Type==SAMSUNG_80_16BIT)
         writew(REG_LCM_DEV_CTRL,0xA50000E0); //16-bit,65536,80mode,mpu-based, CNLi
     if (ucDev_Type==SAMSUNG_80_18BIT)
         writew(REG_LCM_DEV_CTRL,0xA60000E0); //18-bit,262144,80mode,mpu-based,cmd18-16 L0 CNLi
     if (ucDev_Type==SAMSUNG_80_9BIT)
         writew(REG_LCM_DEV_CTRL,0xA20000E0); //18-bit,262144,80mode,mpu-based,cmd18-16 L0 CNLi
     if (ucDev_Type==TOPPOLY_80_8BIT)
         writew(REG_LCM_DEV_CTRL,0x810000E0); //8bit,65536,80mode,mpu-based, CNLi
     if (ucDev_Type==TOPPOLY_80_9BIT)
         writew(REG_LCM_DEV_CTRL,0xC20000E0); //9-bit,262144,80mode,mpu-based, CNLi
#if 0
     if (ucDev_Type==TOPPOLY_80_18BIT)
         writew(REG_LCM_DEV_CTRL,0xC60000E0); //18-bit,262144,80mode,mpu-based, CNLi
     if (ucDev_Type==TOPPOLY_80_16BIT)
         writew(REG_LCM_DEV_CTRL,0x850000E0); //16-bit,65536,80mode,mpu-based, CNLi
#else
     if (ucDev_Type==TOPPOLY_80_18BIT)
         writew(REG_LCM_DEV_CTRL,0x810000E0); //18-bit,262144,80mode,mpu-based, CNLi
     if (ucDev_Type==TOPPOLY_80_16BIT)
         writew(REG_LCM_DEV_CTRL,0x810000E0); //16-bit,65536,80mode,mpu-based, CNLi
#endif
     if (ucDev_Type==TOPPOLY_68_16BIT)
         writew(REG_LCM_DEV_CTRL,0x8D0000E0); //16-bit,65536,68mode,mpu-based, CNLi
     if (ucDev_Type==TOPPOLY_68_8BIT)
         writew(REG_LCM_DEV_CTRL,0x890000E0); //8bit,65536,68mode,mpu-based, CNLi
     if (ucDev_Type==TOPPOLY_68_18BIT)
         writew(REG_LCM_DEV_CTRL,0xCE0000E0); //18-bit,262144,68mode,mpu-based, CNLi
     if (ucDev_Type==TOPPOLY_68_9BIT)
         writew(REG_LCM_DEV_CTRL,0xCA0000E0); //9-bit,262144,68mode,mpu-based, CNLi
     if (ucDev_Type==HITACHI_80_18BIT)
         writew(REG_LCM_DEV_CTRL,0xA60000E0); //18-bit,262144,80mode,mpu-based, cmd18-16 L0 CNLi
     if (ucDev_Type==NEC_80_18BIT)
         writew(REG_LCM_DEV_CTRL,0xE60000E0); //18-bit,262144,80mode,mpu-based, cmd18-16 L0 CNLi
	
     if ((ucDev_Type==SAMSUNG_80_16BIT)&&(ucROT90==1))
         vpostCRTC_Setting(ucDev_Type,ucVASrc_Type,161,127,usPic_Width);
     else if ((ucDev_Type==SAMSUNG_80_18BIT)&&(ucROT90==1))
         vpostCRTC_Setting(ucDev_Type,ucVASrc_Type,160,128,usPic_Width);
     else if ((ucDev_Type==SAMSUNG_80_9BIT)&&(ucROT90==1))
         vpostCRTC_Setting(ucDev_Type,ucVASrc_Type,160,128,usPic_Width);
     else if ((ucDev_Type==HITACHI_80_18BIT)&&(ucROT90==1))
         vpostCRTC_Setting(ucDev_Type,ucVASrc_Type,220,176,usPic_Width);
     else if ((ucDev_Type==HITACHI_80_18BIT)&&(ucROT90==0))
         vpostCRTC_Setting(ucDev_Type,ucVASrc_Type,176,220,usPic_Width);
     else if ((ucDev_Type==NEC_80_18BIT)&&(ucROT90==1))
         vpostCRTC_Setting(ucDev_Type,ucVASrc_Type,320,240,usPic_Width);
     else if ((ucDev_Type==NEC_80_18BIT)&&(ucROT90==0))
         vpostCRTC_Setting(ucDev_Type,ucVASrc_Type,240,320,usPic_Width);
#endif
     
     //else if ((ucDev_Type==OPT_TOPPOLY_TDA025)&&(ucROT90==0))//clyu
     if ((ucDev_Type==OPT_TOPPOLY_TDA025)&&(ucROT90==0))//clyu
     	 vpostCRTC_Setting(ucDev_Type,ucVASrc_Type,320,240,usPic_Width);
     else
         vpostCRTC_Setting(ucDev_Type,ucVASrc_Type,128,160,usPic_Width);
         
           
     vpostVAScale(ucDev_Type, ucscall_mode, ucVASrc_Type);         

     

#ifdef MPU_LCD
    vpostSetupMpuLCD(ucDev_Type,ucROT90);
#if 1
    if (ucDev_Type==TOPPOLY_80_18BIT)
        writew(REG_LCM_DEV_CTRL,0xC60000E0); //18-bit,262144,80mode,mpu-based, CNLi
    if (ucDev_Type==TOPPOLY_80_16BIT)
        writew(REG_LCM_DEV_CTRL,0x850000E0); //16-bit,65536,80mode,mpu-based, CNLi
#endif
#endif
}

#ifdef MPU_LCD //Added by JLLin
void vpostVA_LCMInitSetting(UINT16 usDevWidth,UINT16 usDevHeigth,UINT8 ucCmdLow,UINT8 ucCmd16t18,
                        UINT8 ucCmdBusWidth,UINT8 ucDataBusWidth,UINT8 ucMPU_Mode,UINT8 ucMPU_Color_Mode)
{
     _vpostLCMInfo_t.usDevWidth=usDevWidth;
     _vpostLCMInfo_t.usDevHeigth=usDevHeigth;
     _vpostLCMInfo_t.ucCmdLow=ucCmdLow;
     _vpostLCMInfo_t.ucCmd16t18=ucCmd16t18;
     _vpostLCMInfo_t.ucCmdBusWidth=ucCmdBusWidth;
     _vpostLCMInfo_t.ucDataBusWidth=ucDataBusWidth;
     _vpostLCMInfo_t.ucMPU_Mode=ucMPU_Mode;
     _vpostLCMInfo_t.ucMPU_Color_Mode=ucMPU_Color_Mode;
}

void vpostVA_MPUInit(UINT8 ucLCM_NUM,UINT8 ucVASrc_Type,UINT8 ucVADisMode,UINT32 udisp_addr0,UINT8 ucscall_mode,UINT16 usPic_Width)
{
     UINT32 utmp;

     if (ucLCM_NUM==0)
       writew(REG_MISCR,0x00);
     else if (ucLCM_NUM==1)
       writew(REG_MISCR,0x01);
     vpostVA_DiSBuf0_Set(udisp_addr0);
//    vpostVA_DiSBuf1_Set(udisp_addr1);
     vpostDisplay_Mode(ucVADisMode);
     writew(REG_LCM_DCCS,readw(REG_LCM_DCCS)&0xfffff0ff); //VA init
     if (ucVASrc_Type==VA_RGB565)
         writew(REG_LCM_DCCS,readw(REG_LCM_DCCS)|0x00000400); //rgb565
     if (ucVASrc_Type==VA_YUV422)
         writew(REG_LCM_DCCS,readw(REG_LCM_DCCS)&0xfffff0ff); //VA_YUV422
     if (ucVASrc_Type==VA_YCbCr422)
         writew(REG_LCM_DCCS,readw(REG_LCM_DCCS)|0x00000100); //VA_YCbCr422
     if (ucVASrc_Type==VA_RGB888)
         writew(REG_LCM_DCCS,readw(REG_LCM_DCCS)|0x00000200); //VA_RGB888
     if (ucVASrc_Type==VA_RGB666)
         writew(REG_LCM_DCCS,readw(REG_LCM_DCCS)|0x00000300); //VA_RGB666
     if (ucVASrc_Type==VA_RGB444low)
         writew(REG_LCM_DCCS,readw(REG_LCM_DCCS)|0x00000500); //VA_RGB444low
     if (ucVASrc_Type==VA_RGB444high)
         writew(REG_LCM_DCCS,readw(REG_LCM_DCCS)|0x00000700); //VA_RGB444high

     writew(REG_LCM_DEV_CTRL,0x000000E0);
     if (_vpostLCMInfo_t.ucCmdLow==MPU_CmdHigh)
         writew(REG_LCM_DEV_CTRL,readw(REG_LCM_DEV_CTRL)&0x7fffffff);
     else if (_vpostLCMInfo_t.ucCmdLow==MPU_CmdLow)
         writew(REG_LCM_DEV_CTRL,readw(REG_LCM_DEV_CTRL)|0x80000000);
     if (_vpostLCMInfo_t.ucCmd16t18==MPU_Cm16t18_L0)
         writew(REG_LCM_DEV_CTRL,readw(REG_LCM_DEV_CTRL)&0xbfffffff);
     else if (_vpostLCMInfo_t.ucCmd16t18==MPU_Cm16t18_H0)
         writew(REG_LCM_DEV_CTRL,readw(REG_LCM_DEV_CTRL)|0x40000000);
     if (_vpostLCMInfo_t.ucCmdBusWidth==MPU_CmdBus_Width_8BIT)
         writew(REG_LCM_DEV_CTRL,readw(REG_LCM_DEV_CTRL)&0xdfffffff);
     else if (_vpostLCMInfo_t.ucCmdBusWidth==MPU_CmdBus_Width_16BIT)
         writew(REG_LCM_DEV_CTRL,readw(REG_LCM_DEV_CTRL)|0x20000000);
     if (_vpostLCMInfo_t.ucMPU_Mode==MPU_80_MODE)
         writew(REG_LCM_DEV_CTRL,readw(REG_LCM_DEV_CTRL)&0xf7ffffff);
     else if (_vpostLCMInfo_t.ucMPU_Mode==MPU_68_MODE)
         writew(REG_LCM_DEV_CTRL,readw(REG_LCM_DEV_CTRL)|0x08000000);
     if (_vpostLCMInfo_t.ucDataBusWidth==MPU_DataBus_Width_8BIT)
         writew(REG_LCM_DEV_CTRL,readw(REG_LCM_DEV_CTRL)&0xfbffffff);
     else if (_vpostLCMInfo_t.ucDataBusWidth==MPU_DataBus_Width_9BIT)
         writew(REG_LCM_DEV_CTRL,readw(REG_LCM_DEV_CTRL)&0xfbffffff);
     else if (_vpostLCMInfo_t.ucDataBusWidth==MPU_DataBus_Width_16BIT)
         writew(REG_LCM_DEV_CTRL,readw(REG_LCM_DEV_CTRL)|0x04000000);
     else if (_vpostLCMInfo_t.ucDataBusWidth==MPU_DataBus_Width_18BIT)
         writew(REG_LCM_DEV_CTRL,readw(REG_LCM_DEV_CTRL)|0x04000000);
     if (_vpostLCMInfo_t.ucMPU_Color_Mode==MPU_4096_Col_MODE)
         writew(REG_LCM_DEV_CTRL,readw(REG_LCM_DEV_CTRL)&0xfcffffff);
     else if (_vpostLCMInfo_t.ucMPU_Color_Mode==MPU_64k_Col_MODE)
     {
         writew(REG_LCM_DEV_CTRL,readw(REG_LCM_DEV_CTRL)&0xfcffffff);
         writew(REG_LCM_DEV_CTRL,readw(REG_LCM_DEV_CTRL)|0x01000000);
     }
     else if (_vpostLCMInfo_t.ucMPU_Color_Mode==MPU_256k_Col_MODE)
     {
         writew(REG_LCM_DEV_CTRL,readw(REG_LCM_DEV_CTRL)&0xfcffffff);
         writew(REG_LCM_DEV_CTRL,readw(REG_LCM_DEV_CTRL)|0x02000000);
     }
     else if (_vpostLCMInfo_t.ucMPU_Color_Mode==MPU_True_Col_MODE)
     {
         writew(REG_LCM_DEV_CTRL,readw(REG_LCM_DEV_CTRL)&0xfcffffff);
         writew(REG_LCM_DEV_CTRL,readw(REG_LCM_DEV_CTRL)|0x03000000);
     }

     if ((_vpostLCMInfo_t.ucDataBusWidth==MPU_DataBus_Width_8BIT)||(_vpostLCMInfo_t.ucDataBusWidth==MPU_DataBus_Width_9BIT))
        _vpostLCMInfo_t.usDevWidth=_vpostLCMInfo_t.usDevWidth<<1;
    
     utmp=_vpostLCMInfo_t.usDevHeigth+CRTC_Retrace_HOffset;
     utmp=((utmp<<16)|_vpostLCMInfo_t.usDevWidth)+CRTC_Retrace_Offset;
     writew(REG_LCM_CRTC_SIZE,utmp); //CRTC_SIZE
     utmp=_vpostLCMInfo_t.usDevHeigth;
     utmp=((utmp<<16)|_vpostLCMInfo_t.usDevWidth);
     writew(REG_LCM_CRTC_DEND,utmp); //CRTC_DEND
     utmp=_vpostLCMInfo_t.usDevWidth+15;
     utmp=((utmp<<16)|_vpostLCMInfo_t.usDevWidth+10);
     writew(REG_LCM_CRTC_HR,utmp); //CRTC_HR
     utmp=_vpostLCMInfo_t.usDevWidth+CRTC_Retrace_Offset-5;
     utmp=((utmp<<16)|(_vpostLCMInfo_t.usDevWidth+CRTC_Retrace_Offset-10));
     writew(REG_LCM_CRTC_HSYNC,utmp); //CRTC_HSYNC
     utmp=_vpostLCMInfo_t.usDevHeigth+CRTC_Retrace_HOffset/2;
     utmp=((utmp<<16)|_vpostLCMInfo_t.usDevHeigth);
     writew(REG_LCM_CRTC_VR,utmp); //CRTC_VR
     if ((ucVASrc_Type==VA_RGB888)||(ucVASrc_Type==VA_RGB666))
     {
        utmp=usPic_Width;
        utmp=((utmp<<16)|utmp);
        writew(REG_LCM_VA_FBCTRL,utmp); //VAFF~VA_STRIDE
     }else
     {
        utmp=usPic_Width/2;
        utmp=((utmp<<16)|utmp);
        writew(REG_LCM_VA_FBCTRL,utmp); //VAFF~VA_STRIDE
     }

     if ((_vpostLCMInfo_t.ucDataBusWidth==MPU_DataBus_Width_8BIT)||(_vpostLCMInfo_t.ucDataBusWidth==MPU_DataBus_Width_9BIT))
     {
         if (ucscall_mode==SCALE_MODE_INTERPOLATION)
            vpostVA_Stream_Scaling_Ctrl(1,0,2,0,SCALE_MODE_INTERPOLATION);
         if (ucscall_mode==SCALE_MODE_DUPLICATION)
            vpostVA_Stream_Scaling_Ctrl(1,0,2,0,SCALE_MODE_DUPLICATION);
     }else
     {
         if (ucscall_mode==SCALE_MODE_INTERPOLATION)
             vpostVA_Stream_Scaling_Ctrl(1,0,1,0,SCALE_MODE_INTERPOLATION);
         if (ucscall_mode==SCALE_MODE_DUPLICATION)
             vpostVA_Stream_Scaling_Ctrl(1,0,1,0,SCALE_MODE_DUPLICATION);
     }
}
#endif
/*
void vpostLCM_Select(UINT8 ucLCM_NUM)
{
	if (ucLCM_NUM==0)
       writew(REG_MISCR,0x00);
	else if (ucLCM_NUM==1)
       writew(REG_MISCR,0x01);
}
*/
void vpostVA_Enable(void)
{
    writew(REG_LCM_DCCS,readw(REG_LCM_DCCS)|0x00000008); //display_out-enable
    writew(REG_LCM_DCCS,readw(REG_LCM_DCCS)|0x00000002); //va-enable
}

void vpostVA_Disable(void)
{
    if ((readw(REG_LCM_DCCS)&0x00000002)==0x00000002)
    {
        writew(REG_LCM_DCCS,readw(REG_LCM_DCCS)&0xfffffffd); //va-disable
        if((readw(REG_LCM_DCCS)&0x00000080)==0x00000080) //rgb565,single
            while((readw(REG_LCM_DCCS)&0x00000002)==0x00000002);
        else 
            while((readw(REG_LCM_DCCS)&0x02000000)==0x02000000);
        writew(REG_LCM_DCCS,readw(REG_LCM_DCCS)&0xfffffff7); //display_out-disable
    }
}

void vpostVA_DiSBuf0_Set(UINT32 uSrc_Buffer0)
{
    writew(REG_LCM_VA_BADDR0,uSrc_Buffer0);
}

void vpostVA_DiSBuf1_Set(UINT32 uSrc_Buffer1)
{
    writew(REG_LCM_VA_BADDR1,uSrc_Buffer1);
}

void vpostCRTC_Setting(UINT8 ucDev_T,UINT8 ucVA_Type,UINT16 uswidth,UINT16 ushigth,UINT16 usPic_Width)
{

    UINT32 tmp;
#ifdef MPU_LCD
    if ((ucDev_T==TOPPOLY_80_8BIT)||(ucDev_T==TOPPOLY_80_9BIT)||(ucDev_T==TOPPOLY_68_8BIT)||(ucDev_T==TOPPOLY_68_9BIT))
        uswidth=uswidth<<1;
    
//    if ((ucDev_T==SAMSUNG_80_16BIT)||(ucDev_T==TOPPOLY_80_16BIT)||(ucDev_T==TOPPOLY_80_8BIT)||(ucDev_T==TOPPOLY_80_18BIT)||(ucDev_T==TOPPOLY_80_9BIT))
//    {
        tmp=ushigth+CRTC_Retrace_HOffset;
        tmp=((tmp<<16)|uswidth)+CRTC_Retrace_Offset;
        writew(REG_LCM_CRTC_SIZE,tmp); //CRTC_SIZE
        if (ucDev_T==SAMSUNG_80_16BIT)
            tmp=ushigth+1;
        else
            tmp=ushigth;
        tmp=((tmp<<16)|uswidth);
        writew(REG_LCM_CRTC_DEND,tmp); //CRTC_DEND
#if 1
        tmp=uswidth+15;
        tmp=((tmp<<16)|uswidth+10);
        writew(REG_LCM_CRTC_HR,tmp); //CRTC_HR
        tmp=uswidth+CRTC_Retrace_Offset-5;
        tmp=((tmp<<16)|(uswidth+CRTC_Retrace_Offset-10));
        writew(REG_LCM_CRTC_HSYNC,tmp); //CRTC_HSYNC
#else
        tmp=uswidth+CRTC_Retrace_Offset/2;
        tmp=((tmp<<16)|uswidth);
        writew(REG_LCM_CRTC_HR,tmp); //CRTC_HR
        tmp=uswidth+CRTC_Retrace_Offset/2+CRTC_Retrace_Offset/4;
        tmp=((tmp<<16)|(uswidth+CRTC_Retrace_Offset/4));
        writew(REG_LCM_CRTC_HSYNC,tmp); //CRTC_HSYNC
#endif
        tmp=ushigth+CRTC_Retrace_HOffset/2;
        tmp=((tmp<<16)|ushigth)+CRTC_Retrace_HOffset/4;
        writew(REG_LCM_CRTC_VR,tmp); //CRTC_VR
        if ((ucVA_Type==VA_RGB888)||(ucVA_Type==VA_RGB666))
        {
            tmp=usPic_Width;
            tmp=((tmp<<16)|tmp);
            writew(REG_LCM_VA_FBCTRL,tmp); //VAFF~VA_STRIDE
        }else
        {
            tmp=usPic_Width/2;
            tmp=((tmp<<16)|tmp);
            writew(REG_LCM_VA_FBCTRL,tmp); //VAFF~VA_STRIDE
        }
//    }
#endif

#ifdef SYNC_BASE

#define FULLSCREEN
//#define _480X400

	switch (ucDev_T) 
	{

    //if (ucDev_T==OPT_NTSC)
    case OPT_NTSC:
    {
	// VPLL=27MHz
        writew(REG_LCM_CRTC_SIZE,0x0106035a); //CRTC_SIZE ,262X858
        
#ifdef FULLSCREEN
        writew(REG_LCM_CRTC_DEND,0x00f202c7); //CRTC_DEND ,242X711 全屏
        writew(REG_LCM_CRTC_HR,0x02e002d8); //CRTC_HR,736~728
        writew(REG_LCM_CRTC_HSYNC,0x034502E2); //CRTC_HSYNC,837~728//0x034502d8
        writew(REG_LCM_CRTC_VR,0x00fa00f2); //CRTC_VR,250~244//0x00fa00f4
#elif defined _480X400
		writew(REG_LCM_CRTC_DEND,0x00C801E0); //CRTC_DEND ,200X320 480X200
        writew(REG_LCM_CRTC_HR,0x02e00240); //CRTC_HR,358~350
        writew(REG_LCM_CRTC_HSYNC,0x03450270); //CRTC_HSYNC,837~512 水平往中间移
        writew(REG_LCM_CRTC_VR,0x00fa00e0); //CRTC_VR,250~180, 垂直往下移
#else
        writew(REG_LCM_CRTC_DEND,0x00780140); //CRTC_DEND ,120X320 320X240
        writew(REG_LCM_CRTC_HR,0x02e00200); //CRTC_HR,358~350
        writew(REG_LCM_CRTC_HSYNC,0x03450200); //CRTC_HSYNC,837~512 水平往中间移
        writew(REG_LCM_CRTC_VR,0x00fa00b4); //CRTC_VR,250~180, 垂直往下移
#endif

        tmp=usPic_Width/2;
        tmp=((tmp<<16)|tmp);
        writew(REG_LCM_VA_FBCTRL,tmp); //VAFF~VA_STRIDE 
    }
    break;
    
    //if (ucDev_T==OPT_PAL)
    case OPT_PAL:
    {
	// VPLL=27MHz
        writew(REG_LCM_CRTC_SIZE,0x01380360); //CRTC_SIZE ,312X864
        
#ifdef FULLSCREEN        
		writew(REG_LCM_CRTC_DEND,0x011f02b9); //CRTC_DEND ,287X697 全屏
		writew(REG_LCM_CRTC_HR,0x02dd02cd); //CRTC_HR,733~717
		writew(REG_LCM_CRTC_HSYNC,0x033102d5); //CRTC_HSYNC,817~717//0x033102cd
		writew(REG_LCM_CRTC_VR,0x012b0124); //CRTC_VR,299~289//0x012b0121
#elif defined _480X400
		writew(REG_LCM_CRTC_DEND,0x00C801E0); //CRTC_DEND ,200X480 480X400
        writew(REG_LCM_CRTC_HR,0x021d020d); //CRTC_HR,358~350
        writew(REG_LCM_CRTC_HSYNC,0x0331025e); //CRTC_HSYNC,837~512 水平往中间移
        writew(REG_LCM_CRTC_VR,0x012b00F1); //CRTC_VR,250~180, 垂直往下移
#else
        writew(REG_LCM_CRTC_DEND,0x00780140); //CRTC_DEND ,120X320 320X240
        writew(REG_LCM_CRTC_HR,0x021d020d); //CRTC_HR,733~717
        writew(REG_LCM_CRTC_HSYNC,0x0331020d); //CRTC_HSYNC,817~525 水平往中间移
        writew(REG_LCM_CRTC_VR,0x012b00d1); //CRTC_VR,299~209 垂直往下移
#endif
        
        tmp=usPic_Width/2;
        tmp=((tmp<<16)|tmp);
        writew(REG_LCM_VA_FBCTRL,tmp); //VAFF~VA_STRIDE
    }
    break;
    
    //if (ucDev_T==OPT_TOPPOLY_TDA025)//clyu
    case OPT_TOPPOLY_TDA025:
    {
	// VPLL=27MHz
        
        writew(REG_LCM_CRTC_SIZE,0x0106030C); //VTT X HTT = 262 X 858 (0x0106035A)
        
   	    writew(REG_LCM_CRTC_DEND,0x00F00280); //CRTC_DEND ,242X711 全屏
       	writew(REG_LCM_CRTC_HR,0x029C0290); //CRTC_HR,736~728
        //writew(REG_LCM_CRTC_HSYNC,0x029B029A); //CRTC_HSYNC,837~728
        writew(REG_LCM_CRTC_HSYNC,0x029A0299); //CRTC_HSYNC,837~728
   	    writew(REG_LCM_CRTC_VR,0x00F200F1); //CRTC_VR,250~244

        tmp=usPic_Width/2;
        tmp=((tmp<<16)|tmp);
        writew(REG_LCM_VA_FBCTRL,tmp); //VAFF~VA_STRIDE		
  
    }
    break;
    
    //if (ucDev_T==OPT_NTSC_NONINT_PATCH)
    case OPT_NTSC_NONINT_PATCH:
    {
        writew(REG_LCM_CRTC_SIZE,0x0106035a); //CRTC_SIZE
        writew(REG_LCM_CRTC_DEND,0x00f202c7); //CRTC_DEND,242X711
        writew(REG_LCM_CRTC_HR,0x02e002d8); //CRTC_HR
        writew(REG_LCM_CRTC_HSYNC,0x034502d8); //CRTC_HSYNC
        writew(REG_LCM_CRTC_VR,0x00fa00f4); //CRTC_VR
        tmp=usPic_Width;
        tmp=((tmp<<16)|tmp);
        writew(REG_LCM_VA_FBCTRL,tmp); //VAFF~VA_STRIDE
    }
    break;

    //if (ucDev_T==OPT_EPSON)
    case OPT_EPSON:
    {
	//
	// resolution: 312x230 dots
	// 64 gray scales, 262144 colors (6 bits R/G/B)
	//
	// VPLL=12.273*4MHz
	// display clock=VPLL/4=12.273MHz
	// sync: low active
#if 0
        writew(REG_LCM_CRTC_SIZE,0x0107030c); //CRTC_SIZE
        writew(REG_LCM_CRTC_DEND,0x00e60270); //CRTC_DEND
        writew(REG_LCM_CRTC_HR,0x02c002b9); //CRTC_HR
        writew(REG_LCM_CRTC_HSYNC,0x02eb02b9); //CRTC_HSYNC
        writew(REG_LCM_CRTC_VR,0x00f200ef); //CRTC_VR
#else
        writew(REG_LCM_CRTC_SIZE,0x0107030c); //CRTC_SIZE,263X780
        writew(REG_LCM_CRTC_DEND,0x00e60270); //CRTC_DEND,230X624
        writew(REG_LCM_CRTC_HR,0x02c002b6); //CRTC_HR,704~695,adjust color
        writew(REG_LCM_CRTC_HSYNC,0x02eb02b7); //CRTC_HSYNC,747~695,adjust color
        writew(REG_LCM_CRTC_VR,0x00f200ef); //CRTC_VR,242~239
#endif
        if ((ucVA_Type==VA_RGB888)||(ucVA_Type==VA_RGB666))
        {
            tmp=usPic_Width;
            tmp=((tmp<<16)|tmp);
            writew(REG_LCM_VA_FBCTRL,tmp); //VAFF~VA_STRIDE
        }else
        {
            tmp=usPic_Width/2;
            tmp=((tmp<<16)|tmp);
            writew(REG_LCM_VA_FBCTRL,tmp); //VAFF~VA_STRIDE
        }
    }
	break;
	
    //if (ucDev_T==OPT_UNIPAC)
    case OPT_UNIPAC:
    {
	/*+++
	  MEMO of Unipac programming (reference clock = 13.5 MHz):
	  Use NTSC setting
	  1.8": HRS = 0x2d0
	    VRE = 0xfb
	    DCLK = 5.67 MHz (0x6b)
 	  2.5":
	   DCLK = 9.70 MHz (0xb6)
 	 4.0":
	    DCLK = 9.70 MHz (0xb6)
	---*/
	// VPLL=27MHz
	// display clock=VPLL/2=13.5MHz
        writew(REG_LCM_CRTC_SIZE,0x0106035a); //CRTC_SIZE,262X858
        writew(REG_LCM_CRTC_DEND,0x00f202c7); //CRTC_DEND,242X711
        writew(REG_LCM_CRTC_HR,0x030002d0); //CRTC_HR,768~720
        writew(REG_LCM_CRTC_HSYNC,0x034502d8); //CRTC_HSYNC,837~728
        writew(REG_LCM_CRTC_VR,0x00fa00f4); //CRTC_VR,250~244
        if ((ucVA_Type==VA_RGB888)||(ucVA_Type==VA_RGB666))
        {
            tmp=usPic_Width;
            tmp=((tmp<<16)|tmp);
            writew(REG_LCM_VA_FBCTRL,tmp); //VAFF~VA_STRIDE
        }else
        {
            tmp=usPic_Width/2;
            tmp=((tmp<<16)|tmp);
            writew(REG_LCM_VA_FBCTRL,tmp); //VAFF~VA_STRIDE
        }
    }
    break;
    
    //if (ucDev_T==OPT_CASIO)
    case OPT_CASIO:
    {
	/*+++
	    CASIO
	    VRE = 0xfb
	    DCLK = 6.75 MHz (0x6b)
	---*/
        writew(REG_LCM_CRTC_SIZE,0x00F201D2); //CRTC_SIZE,242X466
        writew(REG_LCM_CRTC_DEND,0x00F00163); //CRTC_DEND,240X355
        writew(REG_LCM_CRTC_HR,0x0177016D); //CRTC_HR,375~365
        writew(REG_LCM_CRTC_HSYNC,0x01D201C5); //CRTC_HSYNC,466~455
        writew(REG_LCM_CRTC_VR,0x00F100F0); //CRTC_VR,241~240
        if ((ucVA_Type==VA_RGB888)||(ucVA_Type==VA_RGB666))
        {
            tmp=usPic_Width;
            tmp=((tmp<<16)|tmp);
            writew(REG_LCM_VA_FBCTRL,tmp); //VAFF~VA_STRIDE
        }else
        {
            tmp=usPic_Width/2;
            tmp=((tmp<<16)|tmp);
            writew(REG_LCM_VA_FBCTRL,tmp); //VAFF~VA_STRIDE
        }
    }
    break;
    
    //if (ucDev_T==OPT_UNIPAC_NONINT)//clyu
    case OPT_UNIPAC_NONINT:
    {
	// VPLL=27MHz
		writew(REG_LCM_CRTC_SIZE,0x0106047A); //CRTC_SIZE ,262X1146
        writew(REG_LCM_CRTC_DEND,0x00E40375); //CRTC_DEND ,228X885 全屏
        writew(REG_LCM_CRTC_HR,0x0384037F); //CRTC_HR,900~895
        writew(REG_LCM_CRTC_HSYNC,0x03C603BD); //CRTC_HSYNC,966~957
        writew(REG_LCM_CRTC_VR,0x00EF00EE); //CRTC_VR,239~238
		
        tmp=usPic_Width/2;
        tmp=((tmp<<16)|tmp);
        writew(REG_LCM_VA_FBCTRL,tmp); //VAFF~VA_STRIDE 
    }
    break;
    
    //if (ucDev_T==OPT_TPG038D)//clyu
    case OPT_TPG038D:
    {
	// VPLL=27MHz
        
        writew(REG_LCM_CRTC_SIZE,0x0106035A); //VTT X HTT = 262 X 858 (0x0106035A)
        
        writew(REG_LCM_CRTC_DEND,0x00F002c7); //CRTC_DEND ,242X711 全屏
        writew(REG_LCM_CRTC_HR,0x02e002d8); //CRTC_HR,736~728
        writew(REG_LCM_CRTC_HSYNC,0x02E802CA); //CRTC_HSYNC,837~728
        writew(REG_LCM_CRTC_VR,0x00FA00E9); //CRTC_VR,250~244

        tmp=usPic_Width/2;
        tmp=((tmp<<16)|tmp);
        writew(REG_LCM_VA_FBCTRL,tmp); //VAFF~VA_STRIDE
        
        TPG038D_WriteLCMRegister(0x04,0x20);
		TPG038D_WriteLCMRegister(0x05,0x07);
		
    }
    break;
    
    
    default:
    	break;
    }
#endif
}

#ifdef SYNC_BASE
void vpostSetTVDispMode(UINT8 ucTVmode)
{
    if (ucTVmode==TV_INTERLACE)
    {
		if ((readw(REG_LCM_DEV_CTRL) & 0x00C00000) == 0)  //noninterlace
    	{
    	    writew(REG_LCM_DEV_CTRL,readw(REG_LCM_DEV_CTRL) | 0x00C00000); //8-bit,65536,packed YUV, CNLi
            writew(REG_LCM_VA_FBCTRL,((readw(REG_LCM_VA_FBCTRL) & 0x7FF)/2 ) | readw(REG_LCM_VA_FBCTRL)); //VAFF~VA_STRIDE
            writew(REG_LCM_OSD_FBCTRL,((readw(REG_LCM_OSD_FBCTRL) & 0x7FF)/2 ) | readw(REG_LCM_OSD_FBCTRL)); //OSDFF~OSD_STRIDE
    	}
    }else if (ucTVmode==TV_NONINTERLACE)
    {
    	if ((readw(REG_LCM_DEV_CTRL) & 0x00C00000) == 0x00C00000)  //interlace
    	{
    	    writew(REG_LCM_DEV_CTRL,readw(REG_LCM_DEV_CTRL) & ~0x00C00000); //8-bit,65536,packed YUV, CNLi
            writew(REG_LCM_VA_FBCTRL,((readw(REG_LCM_VA_FBCTRL) & 0x7FF) * 2 ) | (readw(REG_LCM_VA_FBCTRL) & 0xFFFFF800)); //VAFF~VA_STRIDE
            writew(REG_LCM_OSD_FBCTRL,((readw(REG_LCM_OSD_FBCTRL) & 0x7FF) * 2 ) | (readw(REG_LCM_OSD_FBCTRL) & 0xFFFFF800)); //OSDFF~OSD_STRIDE
    	}
    }
}
#endif

void vpostGet_LCM_Info(UINT8 ucDev_Type,UINT16 *usLCM_Width,UINT16 *usLCM_Height)
{
#ifdef MPU_LCD
//    if ((ucDev_Type==SAMSUNG_80_16BIT)||(ucDev_Type==TOPPOLY_80_16BIT)||(ucDev_Type==TOPPOLY_80_8BIT)||(ucDev_Type==TOPPOLY_80_18BIT)||(ucDev_Type==TOPPOLY_80_9BIT))
//    {
        *usLCM_Width=128;
        *usLCM_Height=160;
//    }
#endif
#ifdef SYNC_BASE
    if ((ucDev_Type==OPT_NTSC)||(ucDev_Type==OPT_NTSC_NONINT_PATCH)||(ucDev_Type==OPT_PAL))
    {
// VPLL=27MHz
        *usLCM_Width=711;
        *usLCM_Height=484;
    }

    if (ucDev_Type==OPT_EPSON)
    {
//
// resolution: 312x230 dots
// 64 gray scales, 262144 colors (6 bits R/G/B)
//
// VPLL=12.273*4MHz
// display clock=VPLL/4=12.273MHz
// sync: low active
        *usLCM_Width=624;
        *usLCM_Height=460;
    }

    if (ucDev_Type==OPT_UNIPAC)
    {
/*+++
  MEMO of Unipac programming (reference clock = 13.5 MHz):
  Use NTSC setting
  1.8": HRS = 0x2d0
    VRE = 0xfb
    DCLK = 5.67 MHz (0x6b)
  2.5":
    DCLK = 9.70 MHz (0xb6)
  4.0":
    DCLK = 9.70 MHz (0xb6)
---*/
// VPLL=27MHz
// display clock=VPLL/2=13.5MHz
        *usLCM_Width=711;
        *usLCM_Height=484;
    }
    if (ucDev_Type==OPT_UNIPAC_NONINT)//clyu
    {
		*usLCM_Width=295;
        *usLCM_Height=228;
    }
    if (ucDev_Type==OPT_TPG038D)//clyu
    {
		*usLCM_Width=320;
        *usLCM_Height=240;
    }
#endif
}
#if 1
void vpostVA_Trigger(void)
{
    if((readw(REG_LCM_DCCS)&0x00000080)==0x00000080) //rgb565,single
        while((readw(REG_LCM_DCCS)&0x00000002)==0x00000002);
    writew(REG_LCM_DCCS,readw(REG_LCM_DCCS)|0x00000008); //display_out-enable
    writew(REG_LCM_DCCS,readw(REG_LCM_DCCS)|0x00000002); //va-enable
}
#else
void vpostVA_Trigger(void)
{
    if((readw(REG_LCM_DCCS)&0x00000080)==0x00000080) //rgb565,single
        while((readw(REG_LCM_DCCS)&0x02000000)==0x02000000);
    writew(REG_LCM_DCCS,readw(REG_LCM_DCCS)|0x00000008); //display_out-enable
    writew(REG_LCM_DCCS,readw(REG_LCM_DCCS)|0x00000002); //va-enable
    writew(REG_LCM_DCCS,readw(REG_LCM_DCCS)&0xfffffffd); //va-disable
}
#endif

void vpostDual_Buffer_Switch(void)
{
    writew(REG_LCM_VA_FBCTRL,readw(REG_LCM_VA_FBCTRL)|0xc0000000); 
}

void vpostVA_MoveXY(UINT16 usxp,UINT16 usyp,UINT32 upic_addr,UINT16 usPic_Width)
{
    UINT32 tmpaddr;

    tmpaddr=(UINT32)usyp*usPic_Width+(UINT32)usxp<<2;
    tmpaddr=tmpaddr+upic_addr;
    vpostVA_DiSBuf0_Set(tmpaddr);

//    while((readw(REG_LCM_DCCS)&0x00000002)==0x00000002);
//    writew(REG_LCM_DCCS,readw(REG_LCM_DCCS)|0x00000002); //output-enable,video enable

}

void vpostVA_Stream_Scaling_Ctrl(UINT8 ucScall_V_Round,UINT16 usScall_V_Decimal,
                                  UINT8 ucScall_H_Round,UINT16 usScall_H_Decimal,UINT8 ucscall_mode)
{
    UINT32 tmp;
    if (ucscall_mode == SCALE_MODE_INTERPOLATION)
    {
        tmp = (((((UINT32)ucScall_V_Round<<10)+usScall_V_Decimal)<<1)<<2);
        tmp = (((tmp<<3)+ucScall_H_Round)<<10)+usScall_H_Decimal;
        writew(REG_LCM_VA_SCALE,tmp);
    }else if (ucscall_mode == SCALE_MODE_DUPLICATION)
    {
        tmp = ((((((UINT32)ucScall_V_Round<<10)+usScall_V_Decimal)+1)<<1)<<2);
        tmp = (((tmp<<3)+ucScall_H_Round)<<10)+usScall_H_Decimal;
        writew(REG_LCM_VA_SCALE,tmp);
    }
}

void vpostOSD_Enable(void)
{
     writew(REG_LCM_DCCS,readw(REG_LCM_DCCS)|0x00000004);//OSD enable
}

void vpostOSD_Disable(void)
{
     writew(REG_LCM_DCCS,readw(REG_LCM_DCCS)&0xfffffffb);//OSD disable
}


void vpostOSDSrc(UINT8 ucOSDSrc_Type)
{
 	unsigned int regDCCS;
 	
 	regDCCS = readw(REG_LCM_DCCS)&0xffff0fff; // Reg DCCS, OSD_SRC clear;
 	
 	switch(ucOSDSrc_Type)
 	{
 		case OSD_RGB565:
         	regDCCS |= 0x00004000;//OSD usSRc
         	break;
     	case OSD_YUV422:
         	break;//OSD usSRc
     	case OSD_YCbCr422:
         	regDCCS |= 0x00001000;//OSD usSRc
         	break;
     	case OSD_RGB888:
        	regDCCS |= 0x00002000;//OSD usSRc
         	break;
     	case OSD_RGB666:
        	regDCCS |= 0x00003000;//OSD usSRc
         	break;
     	case OSD_RGB444low:
        	regDCCS |= 0x00005000;//OSD usSRc
         	break;
     	case OSD_RGB444high:
        	regDCCS |= 0x00007000;//OSD usSRc
         	break;
     	case OSD_RGB332:
        	regDCCS |= 0x00006000;//OSD usSRc
         	break;	
 	
 		default: // YUV422
 			break;
 	}
 	writew(REG_LCM_DCCS, regDCCS);//OSD usSRc

}


void vpostOSD_Init(UINT8 ucDev_Type,UINT8 ucOSDSrc_Type,UINT16 usxst,UINT16 usyst,UINT16 usxed,UINT16 usyed,UINT32 uOSD_Src_Addr,UINT16 usPic_OSD_Width)
{
     //UINT32 tmp;
     UINT32 regOSD_FBCTRL;

	vpostOSDSrc(ucOSDSrc_Type); // Replaced

//	vpostOSDWindow(ucDev_Type);
	
//vpostOSDWindow(UINT8 ucDev_Type){

	switch(ucDev_Type)
	{
#ifdef MPU_LCD
	case TOPPOLY_80_8BIT:
	case TOPPOLY_80_9BIT:
	case TOPPOLY_68_8BIT:
	case TOPPOLY_68_9BIT:
		vpostOSD_Window_Set(usxst,usxed*2,usyst,usyed,uOSD_Src_Addr);
		break;
#endif
#ifdef SYNC_BASE
	case OPT_TOPPOLY_TDA025:
		vpostOSD_Window_Set(usxst,usxed*2,usyst,usyed*2,uOSD_Src_Addr);
		break;
	case OPT_EPSON:
	case OPT_UNIPAC:
		vpostOSD_Window_Set(usxst,usxed*2,usyst*2,usyed,uOSD_Src_Addr);
		break;
	case OPT_TPG038D:
		vpostOSD_Window_Set(usxst,usxed*2,usyst,usyed,uOSD_Src_Addr);
		break;
#endif
	default:
		vpostOSD_Window_Set(usxst,usxed,usyst,usyed,uOSD_Src_Addr);
		break;
	}	
	
//}


	switch(ucOSDSrc_Type){
		case OSD_RGB888:
		case OSD_RGB666:
			regOSD_FBCTRL = usPic_OSD_Width << 16 | usPic_OSD_Width;
			break;
		
		case OSD_RGB332:
			regOSD_FBCTRL = (usPic_OSD_Width/4) << 16 | (usPic_OSD_Width/4);
			break;
		default:
#ifdef SYNC_BASE
        if (ucDev_Type==OPT_NTSC_NONINT_PATCH)
             regOSD_FBCTRL = usPic_OSD_Width << 16 | usPic_OSD_Width;
        else
#endif		
			regOSD_FBCTRL = (usPic_OSD_Width/2) << 16 | (usPic_OSD_Width/2);
		break;
	}
	 writew(REG_LCM_OSD_FBCTRL, regOSD_FBCTRL); //OSDFF~OSD_STRIDE


	switch(ucDev_Type)
	{
#ifdef MPU_LCD
	case TOPPOLY_80_8BIT:
	case TOPPOLY_80_9BIT:
	case TOPPOLY_68_8BIT:
	case TOPPOLY_68_9BIT:
		vpostOSD_Scaling_Control(0,OSD_HUP_2X);
		break;
#endif
#ifdef SYNC_BASE
	case OPT_TOPPOLY_TDA025:
		vpostOSD_Scaling_Control(OSD_VUP_2X,OSD_HUP_2X);
		break;
	case OPT_EPSON:
	case OPT_UNIPAC:
		vpostOSD_Window_Set(usxst,usxed*2,usyst*2,usyed,uOSD_Src_Addr);
		break;
	case OPT_TPG038D:
		vpostOSD_Scaling_Control(0,OSD_HUP_2X);
		break;
#endif
	default:
		vpostOSD_Scaling_Control(0,0);;
		break;
	}	

       
}

void vpostOSD_DispBuf_Set(UINT32 uOSD_Buffer)
{
    writew(REG_LCM_OSD_BADDR,uOSD_Buffer);
}

void vpostOSD_Window_Set(UINT16 usxstart,UINT16 usxend,UINT16 usystart,UINT16 usyend,UINT32 uOSD_Src)
{
    UINT32 tmp;

    tmp = 0;
    tmp = ((((tmp|usystart)+1)<<16)|(usxstart+1));
    writew(REG_LCM_OSD_WIN_S,tmp);
    tmp = 0;
    tmp = ((((tmp|usyend))<<16)|(usxend));//clyu 20060119 usyend+1 --> usyend
    writew(REG_LCM_OSD_WIN_E,tmp);
    vpostOSD_DispBuf_Set(uOSD_Src);
}

void vpostOSD_Setting(UINT8 ucMatch_OSD_Mode,UINT8 ucUnMatch_OSD_Mode,UINT8 ucOSD_Syn_Weight)
{
//    writew(REG_LCM_OSD_OVERLAY,0x00000000);
//    writew(REG_LCM_OSD_OVERLAY,readw(REG_LCM_OSD_OVERLAY)&0xfffffeff);
    
    switch(ucMatch_OSD_Mode)
    {
    	case SHOW_MODE_VIDEO:
    		writew(REG_LCM_OSD_OVERLAY,readw(REG_LCM_OSD_OVERLAY)&0xfffffff0);
    		break;
    	case SHOW_MODE_OSD:
    		writew(REG_LCM_OSD_OVERLAY,readw(REG_LCM_OSD_OVERLAY)&0xfffffff0);
        	writew(REG_LCM_OSD_OVERLAY,readw(REG_LCM_OSD_OVERLAY)|0x00000004);
        	break;
    	case SHOW_MODE_SYNTHESIS:
    		writew(REG_LCM_OSD_OVERLAY,readw(REG_LCM_OSD_OVERLAY)&0xffffff03);
        	writew(REG_LCM_OSD_OVERLAY,readw(REG_LCM_OSD_OVERLAY)|((UINT32)ucOSD_Syn_Weight<<4));
       		writew(REG_LCM_OSD_OVERLAY,readw(REG_LCM_OSD_OVERLAY)|0x00000008);
       		break;
    	default:
    		break;
    }
    
    
     switch(ucUnMatch_OSD_Mode)
    {
    	case SHOW_MODE_VIDEO:
    		writew(REG_LCM_OSD_OVERLAY,readw(REG_LCM_OSD_OVERLAY)&0xfffffffc);
    		break;
    	case SHOW_MODE_OSD:
    		writew(REG_LCM_OSD_OVERLAY,readw(REG_LCM_OSD_OVERLAY)&0xfffffffc);
        	writew(REG_LCM_OSD_OVERLAY,readw(REG_LCM_OSD_OVERLAY)|0x00000001);
        	break;
    	case SHOW_MODE_SYNTHESIS:
    		writew(REG_LCM_OSD_OVERLAY,readw(REG_LCM_OSD_OVERLAY)&0xffffff0c);
       		writew(REG_LCM_OSD_OVERLAY,readw(REG_LCM_OSD_OVERLAY)|((UINT32)ucOSD_Syn_Weight<<4));
        	writew(REG_LCM_OSD_OVERLAY,readw(REG_LCM_OSD_OVERLAY)|0x00000002);
       		break;
    	default:
    		break;
    }
  
    
}

void vpostOSD_Scaling_Control(UINT8 ucScall_V_Round,UINT8 ucScall_H_Round)
{
    writew(REG_LCM_DCCS,readw(REG_LCM_DCCS)&0xfff0ffff);
    if (ucScall_V_Round==OSD_VUP_2X)
        writew(REG_LCM_DCCS,readw(REG_LCM_DCCS)|0x00010000);
    else if (ucScall_V_Round==OSD_VUP_4X)
        writew(REG_LCM_DCCS,readw(REG_LCM_DCCS)|0x00020000);
    if (ucScall_H_Round==OSD_HUP_2X)
        writew(REG_LCM_DCCS,readw(REG_LCM_DCCS)|0x00040000);
}

void vpostOSD_Blinking_Setting(UINT8 ucOSD_Blink_Vcnt)
{
    writew(REG_LCM_OSD_OVERLAY,readw(REG_LCM_OSD_OVERLAY)&0xfffffcff);
    writew(REG_LCM_OSD_OVERLAY,readw(REG_LCM_OSD_OVERLAY)|0x00000200);
    writew(REG_LCM_OSD_OVERLAY,readw(REG_LCM_OSD_OVERLAY)|((UINT32)(ucOSD_Blink_Vcnt)<<16));
}

void vpostColKey_Setting(UINT32 uCKey_Val)
{
    writew(REG_LCM_OSD_OVERLAY,readw(REG_LCM_OSD_OVERLAY)&0xfffffcff);
    writew(REG_LCM_OSD_OVERLAY,readw(REG_LCM_OSD_OVERLAY)|0x00000100);
    writew(REG_LCM_OSD_CKEY,uCKey_Val);
}

void vpostColMask_Setting(UINT32 uCMask_Val)
{
    writew(REG_LCM_OSD_CMASK,uCMask_Val);
}
#if 0
void vpostOSD_Blink_Display(UINT8 ucMatch_UnMatch_Type,UINT8 ucOSD_Blk_Vcnt,UINT8 ucOSD_Synthesis_Weight)
{
    if (ucMatch_UnMatch_Type==MATCH_V_UNMATCH_OSD)
        vpostOSD_Setting(SHOW_MODE_VIDEO,SHOW_MODE_OSD,ucOSD_Synthesis_Weight);
    if (ucMatch_UnMatch_Type==MATCH_V_UNMATCH_S)
        vpostOSD_Setting(SHOW_MODE_VIDEO,SHOW_MODE_SYNTHESIS,ucOSD_Synthesis_Weight);
    if (ucMatch_UnMatch_Type==MATCH_OSD_UNMATCH_V)
        vpostOSD_Setting(SHOW_MODE_OSD,SHOW_MODE_VIDEO,ucOSD_Synthesis_Weight);
    if (ucMatch_UnMatch_Type==MATCH_OSD_UNMATCH_S)
        vpostOSD_Setting(SHOW_MODE_OSD,SHOW_MODE_SYNTHESIS,ucOSD_Synthesis_Weight);
    if (ucMatch_UnMatch_Type==MATCH_S_UNMATCH_V)
        vpostOSD_Setting(SHOW_MODE_SYNTHESIS,SHOW_MODE_VIDEO,ucOSD_Synthesis_Weight);
    if (ucMatch_UnMatch_Type==MATCH_S_UNMATCH_OSD)
        vpostOSD_Setting(SHOW_MODE_SYNTHESIS,SHOW_MODE_OSD,ucOSD_Synthesis_Weight);
    if (ucMatch_UnMatch_Type==MATCH_OSD_UNMATCH_OSD)
        vpostOSD_Setting(SHOW_MODE_OSD,SHOW_MODE_OSD,ucOSD_Synthesis_Weight);
    vpostOSD_Blinking_Setting(ucOSD_Blk_Vcnt);
    vpostOSD_Enable();
}
#endif
void vpostOSD_ColKeyMask_Trigger(UINT8 ucMatch_UnMatch_Type,UINT32 uuCKey_Value,UINT32 uuCMask_Value,UINT8 ucOSD_Synthesis_Weight)
{

  
   switch(ucMatch_UnMatch_Type)
   {
   		case MATCH_V_UNMATCH_OSD:
        	vpostOSD_Setting(SHOW_MODE_VIDEO,SHOW_MODE_OSD,ucOSD_Synthesis_Weight);
        	break;
   		case MATCH_V_UNMATCH_S:
        	vpostOSD_Setting(SHOW_MODE_VIDEO,SHOW_MODE_SYNTHESIS,ucOSD_Synthesis_Weight);
        	break;
    	case MATCH_OSD_UNMATCH_V:
        	vpostOSD_Setting(SHOW_MODE_OSD,SHOW_MODE_VIDEO,ucOSD_Synthesis_Weight);
        	break;
   		case MATCH_OSD_UNMATCH_S:
        	vpostOSD_Setting(SHOW_MODE_OSD,SHOW_MODE_SYNTHESIS,ucOSD_Synthesis_Weight);
        	break;
    	case MATCH_S_UNMATCH_V:
        	vpostOSD_Setting(SHOW_MODE_SYNTHESIS,SHOW_MODE_VIDEO,ucOSD_Synthesis_Weight);
        	break;
   		case MATCH_S_UNMATCH_OSD:
        	vpostOSD_Setting(SHOW_MODE_SYNTHESIS,SHOW_MODE_OSD,ucOSD_Synthesis_Weight);
        	break;
    	case MATCH_OSD_UNMATCH_OSD:
        	vpostOSD_Setting(SHOW_MODE_OSD,SHOW_MODE_OSD,ucOSD_Synthesis_Weight);
        	break; 
   }
   
      
    vpostColKey_Setting(uuCKey_Value);
    vpostColMask_Setting(uuCMask_Value);
    vpostOSD_Enable();
}

#ifdef MPU_LCD
void vpostLCDWriteCMDAddr(UINT8 Dev_T,UINT16 uscmd)
{
    writew(REG_LCM_MPU_CMD,readw(REG_LCM_MPU_CMD)&0xbfff0000);         //RS=0
    writew(REG_LCM_MPU_CMD,readw(REG_LCM_MPU_CMD)&0xdfffffff);         //r/w
//    if ((Dev_T==TOPPOLY_80_16BIT)||(Dev_T==TOPPOLY_80_8BIT)||(Dev_T==TOPPOLY_80_18BIT)||(Dev_T==TOPPOLY_80_9BIT))
//        writew(REG_LCM_DEV_CTRL,readw(REG_LCM_DEV_CTRL)&0xdfffffff);         //8 bit
    if ((Dev_T==SAMSUNG_80_16BIT)||(Dev_T==SAMSUNG_80_18BIT)||(Dev_T==HITACHI_80_18BIT)||(Dev_T==NEC_80_18BIT))
        writew(REG_LCM_DEV_CTRL,readw(REG_LCM_DEV_CTRL)|0x20000000);         //16 bit
    else
        writew(REG_LCM_DEV_CTRL,readw(REG_LCM_DEV_CTRL)&0xdfffffff);         //8 bit
    writew(REG_LCM_DCCS,(readw(REG_LCM_DCCS)|0x00000020));       //CMD ON
    if (Dev_T==SAMSUNG_80_9BIT)
    {
        writew(REG_LCM_MPU_CMD,((readw(REG_LCM_MPU_CMD)&0xffff0000)|(UINT8)(uscmd>>8)));
        while(readw(REG_LCM_MPU_CMD)&0x80000000); //CNLi
        writew(REG_LCM_MPU_CMD,((readw(REG_LCM_MPU_CMD)&0xffff0000)|(UINT8)(uscmd&0x00ff)));
        while(readw(REG_LCM_MPU_CMD)&0x80000000); //CNLi
    }
    else
    {
        writew(REG_LCM_MPU_CMD,(readw(REG_LCM_MPU_CMD)|uscmd));
        while(readw(REG_LCM_MPU_CMD)&0x80000000); //CNLi
    }
    writew(REG_LCM_DCCS,(readw(REG_LCM_DCCS)&0xffffffdf));         //CMD OFF
}

void vpostLCDWriteCMDReg(UINT8 Dev_T,UINT16 uscmd)
{
    writew(REG_LCM_MPU_CMD,readw(REG_LCM_MPU_CMD)|0x40000000);         //RS=1
    writew(REG_LCM_MPU_CMD,readw(REG_LCM_MPU_CMD)&0xdfff0000);         //w
//    if ((Dev_T==TOPPOLY_80_16BIT)||(Dev_T==TOPPOLY_80_8BIT)||(Dev_T==TOPPOLY_80_18BIT)||(Dev_T==TOPPOLY_80_9BIT))
//        writew(REG_LCM_DEV_CTRL,readw(REG_LCM_DEV_CTRL)&0xdfffffff);         //8 bit
    if ((Dev_T==SAMSUNG_80_16BIT)||(Dev_T==SAMSUNG_80_18BIT)||(Dev_T==HITACHI_80_18BIT))
        writew(REG_LCM_DEV_CTRL,readw(REG_LCM_DEV_CTRL)|0x20000000);         //16 bit
    else
        writew(REG_LCM_DEV_CTRL,readw(REG_LCM_DEV_CTRL)&0xdfffffff);         //8 bit
    writew(REG_LCM_DCCS,readw(REG_LCM_DCCS)|0x00000020);       //CMD ON
    if (Dev_T==SAMSUNG_80_9BIT)
    {
        writew(REG_LCM_MPU_CMD,((readw(REG_LCM_MPU_CMD)&0xffff0000)|(UINT8)(uscmd>>8)));
        while(readw(REG_LCM_MPU_CMD)&0x80000000); //CNLi
        writew(REG_LCM_MPU_CMD,((readw(REG_LCM_MPU_CMD)&0xffff0000)|(UINT8)(uscmd&0x00ff)));
        while(readw(REG_LCM_MPU_CMD)&0x80000000); //CNLi
    }
    else
    {
        writew(REG_LCM_MPU_CMD,readw(REG_LCM_MPU_CMD)|uscmd);
        while(readw(REG_LCM_MPU_CMD)&0x80000000); //CNLi
    }
    writew(REG_LCM_DCCS,readw(REG_LCM_DCCS)&0xffffffdf);         //CMD OFF
}

UINT32 vpostLCDReadCMDReg(UINT8 Dev_T)
{
    UINT32 tmp;	
    writew(REG_LCM_MPU_CMD,readw(REG_LCM_MPU_CMD)|0x40000000);         //RS=1
//    if ((Dev_T==TOPPOLY_80_16BIT)||(Dev_T==TOPPOLY_80_8BIT)||(Dev_T==TOPPOLY_80_18BIT)||(Dev_T==TOPPOLY_80_9BIT))
//        writew(REG_LCM_DEV_CTRL,readw(REG_LCM_DEV_CTRL)&0xdfffffff);         //8 bit
    if ((Dev_T==SAMSUNG_80_16BIT)||(Dev_T==SAMSUNG_80_18BIT)||(Dev_T==HITACHI_80_18BIT))
        writew(REG_LCM_DEV_CTRL,readw(REG_LCM_DEV_CTRL)|0x20000000);         //16 bit
    else
        writew(REG_LCM_DEV_CTRL,readw(REG_LCM_DEV_CTRL)&0xdfffffff);         //8 bit
    writew(REG_LCM_DCCS,readw(REG_LCM_DCCS)|0x00000020);       //CMD ON
    writew(REG_LCM_MPU_CMD,readw(REG_LCM_MPU_CMD)|0x20000000);         //r
    while(readw(REG_LCM_MPU_CMD)&0x80000000); //CNLi
    tmp=readw(REG_LCM_MPU_CMD);    
    writew(REG_LCM_DCCS,readw(REG_LCM_DCCS)&0xffffffdf);         //CMD OFF
    return tmp;
}

void vpostHitachiInit(UINT8 Dev_T,UINT8 ROT90Flg)
{
#if 0
        vpostLCDWriteCMDAddr(Dev_T,0x00);	
        vpostLCDWriteCMDReg(Dev_T,0x0001);
        vpostdelay_long(8000);
        vpostLCDWriteCMDAddr(Dev_T,0x01);	
        vpostLCDWriteCMDReg(Dev_T,0x001B);	    
        vpostLCDWriteCMDAddr(Dev_T,0x12);	
        vpostLCDWriteCMDReg(Dev_T,0x0000);	    
        vpostLCDWriteCMDAddr(Dev_T,0x13);	
        vpostLCDWriteCMDReg(Dev_T,0x090C);		
        vpostLCDWriteCMDAddr(Dev_T,0x0A);	
        vpostLCDWriteCMDReg(Dev_T,0x0101);
        vpostdelay_long(8000);
        vpostLCDWriteCMDAddr(Dev_T,0x11);	
        vpostLCDWriteCMDReg(Dev_T,0x0404);	    	
        vpostLCDWriteCMDAddr(Dev_T,0x12);	
        vpostLCDWriteCMDReg(Dev_T,0x0004);	    
        vpostLCDWriteCMDAddr(Dev_T,0x13);	
        vpostLCDWriteCMDReg(Dev_T,0x0610);		
        vpostLCDWriteCMDAddr(Dev_T,0x0A);	
        vpostLCDWriteCMDReg(Dev_T,0x0101);
        vpostdelay_long(8000);
        vpostLCDWriteCMDAddr(Dev_T,0x07);	
        vpostLCDWriteCMDReg(Dev_T,0x0036);		
        vpostLCDWriteCMDAddr(Dev_T,0x0B);	
        vpostLCDWriteCMDReg(Dev_T,0x0000);	    	
        vpostLCDWriteCMDAddr(Dev_T,0x40);	
        vpostLCDWriteCMDReg(Dev_T,0x0000);	    	
        vpostLCDWriteCMDAddr(Dev_T,0x42);	
        vpostLCDWriteCMDReg(Dev_T,0xDB00);	    
        vpostLCDWriteCMDAddr(Dev_T,0x44);	
        vpostLCDWriteCMDReg(Dev_T,0xAF00);
        vpostLCDWriteCMDAddr(Dev_T,0x45);	
        vpostLCDWriteCMDReg(Dev_T,0xDB00);		
        vpostLCDWriteCMDAddr(Dev_T,0x12);	
        vpostLCDWriteCMDReg(Dev_T,0x0000);	    
        vpostLCDWriteCMDAddr(Dev_T,0x13);	
        vpostLCDWriteCMDReg(Dev_T,0x0100);	    
        vpostLCDWriteCMDAddr(Dev_T,0x0A);	
        vpostLCDWriteCMDReg(Dev_T,0x0101);
        vpostdelay_long(800);
        vpostLCDWriteCMDAddr(Dev_T,0x13);	
        vpostLCDWriteCMDReg(Dev_T,0x0610);	    
        vpostLCDWriteCMDAddr(Dev_T,0x0A);	
        vpostLCDWriteCMDReg(Dev_T,0x0106);
        vpostdelay_long(8000);
        vpostLCDWriteCMDAddr(Dev_T,0x30);	
        vpostLCDWriteCMDReg(Dev_T,0x0000);
        vpostLCDWriteCMDAddr(Dev_T,0x31);	
        vpostLCDWriteCMDReg(Dev_T,0x0103);
        vpostLCDWriteCMDAddr(Dev_T,0x32);	
        vpostLCDWriteCMDReg(Dev_T,0x0001);	    
        vpostLCDWriteCMDAddr(Dev_T,0x33);	
        vpostLCDWriteCMDReg(Dev_T,0x0700);	    
        vpostLCDWriteCMDAddr(Dev_T,0x34);	
        vpostLCDWriteCMDReg(Dev_T,0x0607);
        vpostLCDWriteCMDAddr(Dev_T,0x35);	
        vpostLCDWriteCMDReg(Dev_T,0x0406);
        vpostLCDWriteCMDAddr(Dev_T,0x36);	
        vpostLCDWriteCMDReg(Dev_T,0x0707);	    
        vpostLCDWriteCMDAddr(Dev_T,0x37);	
        vpostLCDWriteCMDReg(Dev_T,0x0007);	    
        vpostLCDWriteCMDAddr(Dev_T,0x3F);	
        vpostLCDWriteCMDReg(Dev_T,0x0000);
        vpostLCDWriteCMDAddr(Dev_T,0x21);	
        vpostLCDWriteCMDReg(Dev_T,0x0000);	    
        vpostLCDWriteCMDAddr(Dev_T,0x22);	

        vpostdelay_long(800);
#else
        vpostLCDWriteCMDAddr(Dev_T,0x00);	
        vpostLCDWriteCMDReg(Dev_T,0x0001);
        vpostdelay_long(8000);
        vpostLCDWriteCMDAddr(Dev_T,0x14);	
        vpostLCDWriteCMDReg(Dev_T,0x3800);
        vpostLCDWriteCMDAddr(Dev_T,0x0A);	
        vpostLCDWriteCMDReg(Dev_T,0x0102);
        vpostdelay_long(800);
        vpostLCDWriteCMDAddr(Dev_T,0x07);	
        vpostLCDWriteCMDReg(Dev_T,0x0001);
        vpostLCDWriteCMDAddr(Dev_T,0x10);	
        vpostLCDWriteCMDReg(Dev_T,0x0000);
        vpostLCDWriteCMDAddr(Dev_T,0x11);	
        vpostLCDWriteCMDReg(Dev_T,0x0404);
        vpostLCDWriteCMDAddr(Dev_T,0x12);	
        vpostLCDWriteCMDReg(Dev_T,0x0000);	    
        vpostLCDWriteCMDAddr(Dev_T,0x13);	
        vpostLCDWriteCMDReg(Dev_T,0x0003);
        vpostLCDWriteCMDAddr(Dev_T,0x14);	
        vpostLCDWriteCMDReg(Dev_T,0x0D0F);
        vpostLCDWriteCMDAddr(Dev_T,0x0A);	
        vpostLCDWriteCMDReg(Dev_T,0x0100);
        vpostdelay_long(800);
        vpostLCDWriteCMDAddr(Dev_T,0x0A);	
        vpostLCDWriteCMDReg(Dev_T,0x0101);
        vpostdelay_long(800);
        vpostLCDWriteCMDAddr(Dev_T,0x0A);	
        vpostLCDWriteCMDReg(Dev_T,0x0102);
        vpostdelay_long(800);
        vpostLCDWriteCMDAddr(Dev_T,0x12);	
        vpostLCDWriteCMDReg(Dev_T,0x0000);	    
        vpostLCDWriteCMDAddr(Dev_T,0x13);	
        vpostLCDWriteCMDReg(Dev_T,0x090C);
        vpostLCDWriteCMDAddr(Dev_T,0x0A);	
        vpostLCDWriteCMDReg(Dev_T,0x0101);
        vpostdelay_long(8000);
        vpostLCDWriteCMDAddr(Dev_T,0x13);	
        vpostLCDWriteCMDReg(Dev_T,0x0603);	    
        vpostLCDWriteCMDAddr(Dev_T,0x10);	
        vpostLCDWriteCMDReg(Dev_T,0x0A08);
        vpostLCDWriteCMDAddr(Dev_T,0x0A);	
        vpostLCDWriteCMDReg(Dev_T,0x0100);
        vpostdelay_long(30000);
        vpostLCDWriteCMDAddr(Dev_T,0x13);	
        vpostLCDWriteCMDReg(Dev_T,0x0610);	    
        vpostLCDWriteCMDAddr(Dev_T,0x0A);	
        vpostLCDWriteCMDReg(Dev_T,0x0101);
        vpostdelay_long(30000);
        vpostLCDWriteCMDAddr(Dev_T,0x14);	
        vpostLCDWriteCMDReg(Dev_T,0x3901);	    
        vpostLCDWriteCMDAddr(Dev_T,0x0A);	
        vpostLCDWriteCMDReg(Dev_T,0x0102);
        vpostdelay_long(800);
        vpostLCDWriteCMDAddr(Dev_T,0x14);	
        vpostLCDWriteCMDReg(Dev_T,0x2D0F);	    
        vpostLCDWriteCMDAddr(Dev_T,0x0A);	
        vpostLCDWriteCMDReg(Dev_T,0x0102);
        vpostdelay_long(800);
        vpostLCDWriteCMDAddr(Dev_T,0x13);	
        vpostLCDWriteCMDReg(Dev_T,0x0A00);	    
        vpostLCDWriteCMDAddr(Dev_T,0x0A);	
        vpostLCDWriteCMDReg(Dev_T,0x0101);
        vpostdelay_long(800);
        vpostLCDWriteCMDAddr(Dev_T,0x13);	
        vpostLCDWriteCMDReg(Dev_T,0x0610);		
        vpostLCDWriteCMDAddr(Dev_T,0x01);	
        vpostLCDWriteCMDReg(Dev_T,0x001B);	    	
        vpostLCDWriteCMDAddr(Dev_T,0x02);	
        vpostLCDWriteCMDReg(Dev_T,0x0700);	    	
        vpostLCDWriteCMDAddr(Dev_T,0x03);	
        if (ROT90Flg==1)
            vpostLCDWriteCMDReg(Dev_T,0x0038);		//k04264-1
        else
            vpostLCDWriteCMDReg(Dev_T,0x0230);	    
        vpostLCDWriteCMDAddr(Dev_T,0x04);	
        vpostLCDWriteCMDReg(Dev_T,0x0000);
        vpostLCDWriteCMDAddr(Dev_T,0x05);	
        vpostLCDWriteCMDReg(Dev_T,0x0000);		
        vpostLCDWriteCMDAddr(Dev_T,0x08);	
        vpostLCDWriteCMDReg(Dev_T,0x0808);	    
        vpostLCDWriteCMDAddr(Dev_T,0x23);	
        vpostLCDWriteCMDReg(Dev_T,0x0000);	    
        vpostLCDWriteCMDAddr(Dev_T,0x24);	
        vpostLCDWriteCMDReg(Dev_T,0x0000);
        vpostLCDWriteCMDAddr(Dev_T,0x0B);	
        vpostLCDWriteCMDReg(Dev_T,0x0000);	    
        vpostLCDWriteCMDAddr(Dev_T,0x0C);	
        vpostLCDWriteCMDReg(Dev_T,0x0000);
        vpostLCDWriteCMDAddr(Dev_T,0x40);	
        vpostLCDWriteCMDReg(Dev_T,0x0000);
        vpostLCDWriteCMDAddr(Dev_T,0x41);	
        vpostLCDWriteCMDReg(Dev_T,0x0000);
        vpostLCDWriteCMDAddr(Dev_T,0x42);	
        vpostLCDWriteCMDReg(Dev_T,0xDB00);	    
        vpostLCDWriteCMDAddr(Dev_T,0x43);	
        vpostLCDWriteCMDReg(Dev_T,0xEFEF);	    
        vpostLCDWriteCMDAddr(Dev_T,0x44);	
//        if (ROT90Flg==1)
//            vpostLCDWriteCMDReg(Dev_T,0xDB00);
//        else
            vpostLCDWriteCMDReg(Dev_T,0xAF00);
        vpostLCDWriteCMDAddr(Dev_T,0x45);
//        if (ROT90Flg==1)
//            vpostLCDWriteCMDReg(Dev_T,0xAF00);
//        else
            vpostLCDWriteCMDReg(Dev_T,0xDB00);
        vpostLCDWriteCMDAddr(Dev_T,0x12);	
        vpostLCDWriteCMDReg(Dev_T,0x0000);	    
        vpostLCDWriteCMDAddr(Dev_T,0x13);	
        vpostLCDWriteCMDReg(Dev_T,0x0100);	    
        vpostLCDWriteCMDAddr(Dev_T,0x0A);	
        vpostLCDWriteCMDReg(Dev_T,0x0101);
        vpostdelay_long(800);
        vpostLCDWriteCMDAddr(Dev_T,0x13);	
        vpostLCDWriteCMDReg(Dev_T,0x0610);	    
        vpostLCDWriteCMDAddr(Dev_T,0x0A);	
        vpostLCDWriteCMDReg(Dev_T,0x0106);
        vpostdelay_long(800);
        vpostLCDWriteCMDAddr(Dev_T,0x30);	
        vpostLCDWriteCMDReg(Dev_T,0x0000);
        vpostLCDWriteCMDAddr(Dev_T,0x31);	
        vpostLCDWriteCMDReg(Dev_T,0x0103);
        vpostLCDWriteCMDAddr(Dev_T,0x32);	
        vpostLCDWriteCMDReg(Dev_T,0x0001);	    
        vpostLCDWriteCMDAddr(Dev_T,0x33);	
        vpostLCDWriteCMDReg(Dev_T,0x0700);	    
        vpostLCDWriteCMDAddr(Dev_T,0x34);	
        vpostLCDWriteCMDReg(Dev_T,0x0607);
        vpostLCDWriteCMDAddr(Dev_T,0x35);	
        vpostLCDWriteCMDReg(Dev_T,0x0406);
        vpostLCDWriteCMDAddr(Dev_T,0x36);	
        vpostLCDWriteCMDReg(Dev_T,0x0707);	    
        vpostLCDWriteCMDAddr(Dev_T,0x37);	
        vpostLCDWriteCMDReg(Dev_T,0x0007);	    
        vpostLCDWriteCMDAddr(Dev_T,0x3F);	
        vpostLCDWriteCMDReg(Dev_T,0x0000);
        vpostLCDWriteCMDAddr(Dev_T,0x21);	
        vpostLCDWriteCMDReg(Dev_T,0x0000);	    
        vpostLCDWriteCMDAddr(Dev_T,0x22);
        vpostdelay_long(800);
        vpostHitachiDisplayOn(HITACHI_80_18BIT,ROT90Flg);
#endif
}

void vpostHitachiDisplayOn(UINT8 Dev_T,UINT8 ROT90Flg)
{
        vpostLCDWriteCMDAddr(Dev_T,0x07);	
        vpostLCDWriteCMDReg(Dev_T,0x0027);	    
        vpostLCDWriteCMDAddr(Dev_T,0x0A);	
        vpostLCDWriteCMDReg(Dev_T,0x0100);
        vpostdelay_long(800);
        vpostLCDWriteCMDAddr(Dev_T,0x07);	
        vpostLCDWriteCMDReg(Dev_T,0x0037);	    
        vpostLCDWriteCMDAddr(Dev_T,0x12);	
        vpostLCDWriteCMDReg(Dev_T,0x0004);	    
        vpostLCDWriteCMDAddr(Dev_T,0x0A);	
        vpostLCDWriteCMDReg(Dev_T,0x0101);
        vpostdelay_long(800);
        vpostLCDWriteCMDAddr(Dev_T,0x21);
//        if (ROT90Flg==1)	
//            vpostLCDWriteCMDReg(Dev_T,0xDB00);
//        else
            vpostLCDWriteCMDReg(Dev_T,0x0000);
        vpostLCDWriteCMDAddr(Dev_T,0x22);
}

void vpostInitialFunction1(UINT8 Dev_T,UINT8 ROT90Flg)
{
    if (Dev_T==SAMSUNG_80_16BIT)
    {
        vpostLCDWriteCMDAddr(Dev_T,0x01);
        vpostLCDWriteCMDReg(Dev_T,0x0115);

        vpostLCDWriteCMDAddr(Dev_T,0x02);	
        vpostLCDWriteCMDReg(Dev_T,0x0700);	        
        vpostLCDWriteCMDAddr(Dev_T,0x05);	
        if (ROT90Flg==1)
            vpostLCDWriteCMDReg(Dev_T,0x1028);				//k04264-1
        else
            vpostLCDWriteCMDReg(Dev_T,0x1230);
        vpostLCDWriteCMDAddr(Dev_T,0x06);	
        vpostLCDWriteCMDReg(Dev_T,0x0000);        	
        vpostLCDWriteCMDAddr(Dev_T,0x07);
        vpostLCDWriteCMDReg(Dev_T,0x0104);
        vpostLCDWriteCMDAddr(Dev_T,0x0b);
        vpostLCDWriteCMDReg(Dev_T,0x0000);
    }else if((Dev_T==SAMSUNG_80_18BIT)||(Dev_T==SAMSUNG_80_9BIT))
    {
        vpostLCDWriteCMDAddr(Dev_T,0x01);
        vpostLCDWriteCMDReg(Dev_T,0x200);
        vpostLCDWriteCMDAddr(Dev_T,0x02);	
        vpostLCDWriteCMDReg(Dev_T,0x0700);	        
        vpostLCDWriteCMDAddr(Dev_T,0x03);
        if (ROT90Flg==1)
//            vpostLCDWriteCMDReg(Dev_T,0x0028);				//k04264-1
            vpostLCDWriteCMDReg(Dev_T,0x0008);				//k04264-1
        else
            vpostLCDWriteCMDReg(Dev_T,0x0220);				//k04264-1
        vpostLCDWriteCMDAddr(Dev_T,0x04);	
        vpostLCDWriteCMDReg(Dev_T,0x0000);        	
        vpostLCDWriteCMDAddr(Dev_T,0x08);
        vpostLCDWriteCMDReg(Dev_T,0x0626);
        vpostLCDWriteCMDAddr(Dev_T,0x09);
        vpostLCDWriteCMDReg(Dev_T,0x0000);
        vpostLCDWriteCMDAddr(Dev_T,0x0b);	
        vpostLCDWriteCMDReg(Dev_T,0x0000);        	
        vpostLCDWriteCMDAddr(Dev_T,0x0c);
#if 0
        vpostLCDWriteCMDReg(Dev_T,0x0000);   //updated 05/08/16//hsu
#else
        vpostLCDWriteCMDReg(Dev_T,0x0001);
#endif
        vpostLCDWriteCMDAddr(Dev_T,0x0f);
        vpostLCDWriteCMDReg(Dev_T,0x0000);
        vpostdelay_long(400);
    }
}

void vpostInitialFunction2(UINT8 Dev_T)
{
    if (Dev_T==SAMSUNG_80_16BIT)
    {
        vpostLCDWriteCMDAddr(Dev_T,0x21);	
        vpostLCDWriteCMDReg(Dev_T,0x0100);	    
        vpostLCDWriteCMDAddr(Dev_T,0x30);	
        vpostLCDWriteCMDReg(Dev_T,0x0000);	    
        vpostLCDWriteCMDAddr(Dev_T,0x31);	
        vpostLCDWriteCMDReg(Dev_T,0x0000);		
        vpostLCDWriteCMDAddr(Dev_T,0x32);	
        vpostLCDWriteCMDReg(Dev_T,0x0000);		
        vpostLCDWriteCMDAddr(Dev_T,0x33);	
        vpostLCDWriteCMDReg(Dev_T,0x0000);	    	
        vpostLCDWriteCMDAddr(Dev_T,0x34);	
        vpostLCDWriteCMDReg(Dev_T,0x0000);	    
        vpostLCDWriteCMDAddr(Dev_T,0x35);	
        vpostLCDWriteCMDReg(Dev_T,0x0707);		
        vpostLCDWriteCMDAddr(Dev_T,0x36);	
        vpostLCDWriteCMDReg(Dev_T,0x0707);		
        vpostLCDWriteCMDAddr(Dev_T,0x37);	
        vpostLCDWriteCMDReg(Dev_T,0x0000);		
        vpostLCDWriteCMDAddr(Dev_T,0x0f);	
        vpostLCDWriteCMDReg(Dev_T,0x0000);	    	
        vpostLCDWriteCMDAddr(Dev_T,0x11);	
        vpostLCDWriteCMDReg(Dev_T,0x0000);	    	
        vpostLCDWriteCMDAddr(Dev_T,0x14);	
        vpostLCDWriteCMDReg(Dev_T,0x5c00);	    
        vpostLCDWriteCMDAddr(Dev_T,0x15);	
        vpostLCDWriteCMDReg(Dev_T,0xa05d);		
        vpostLCDWriteCMDAddr(Dev_T,0x16);	
        vpostLCDWriteCMDReg(Dev_T,0x7f00);		
        vpostLCDWriteCMDAddr(Dev_T,0x17);	
        vpostLCDWriteCMDReg(Dev_T,0xA000);	    
        vpostLCDWriteCMDAddr(Dev_T,0x3A);	
        vpostLCDWriteCMDReg(Dev_T,0x0000);	    
        vpostLCDWriteCMDAddr(Dev_T,0x3B);	
        vpostLCDWriteCMDReg(Dev_T,0x0000);
    }else if((Dev_T==SAMSUNG_80_18BIT)||(Dev_T==SAMSUNG_80_9BIT))
    {
        vpostLCDWriteCMDAddr(Dev_T,0x23);	
        vpostLCDWriteCMDReg(Dev_T,0x0000);	    
        vpostLCDWriteCMDAddr(Dev_T,0x24);	
        vpostLCDWriteCMDReg(Dev_T,0x0000);	    
        vpostLCDWriteCMDAddr(Dev_T,0x50);	
        vpostLCDWriteCMDReg(Dev_T,0x0000);		
        vpostLCDWriteCMDAddr(Dev_T,0x51);	
        vpostLCDWriteCMDReg(Dev_T,0x007f);		
        vpostLCDWriteCMDAddr(Dev_T,0x52);	
        vpostLCDWriteCMDReg(Dev_T,0x0000);	    	
        vpostLCDWriteCMDAddr(Dev_T,0x53);	
        vpostLCDWriteCMDReg(Dev_T,0x009f);	    
        vpostLCDWriteCMDAddr(Dev_T,0x60);	
        vpostLCDWriteCMDReg(Dev_T,0x9300);		
        vpostLCDWriteCMDAddr(Dev_T,0x61);	
        vpostLCDWriteCMDReg(Dev_T,0x0001);		
        vpostLCDWriteCMDAddr(Dev_T,0x68);	
        vpostLCDWriteCMDReg(Dev_T,0x0000);		
        vpostLCDWriteCMDAddr(Dev_T,0x69);	
        vpostLCDWriteCMDReg(Dev_T,0x009f);	    	
        vpostLCDWriteCMDAddr(Dev_T,0x6a);	
        vpostLCDWriteCMDReg(Dev_T,0x0000);	    	
        vpostLCDWriteCMDAddr(Dev_T,0x70);	
        vpostLCDWriteCMDReg(Dev_T,0x8b14);	    
        vpostLCDWriteCMDAddr(Dev_T,0x71);	
        vpostLCDWriteCMDReg(Dev_T,0x0001);		
        vpostLCDWriteCMDAddr(Dev_T,0x78);	
        vpostLCDWriteCMDReg(Dev_T,0x00a0);		
        vpostLCDWriteCMDAddr(Dev_T,0x79);	
        vpostLCDWriteCMDReg(Dev_T,0x00ff);	    
        vpostLCDWriteCMDAddr(Dev_T,0x7a);	
        vpostLCDWriteCMDReg(Dev_T,0x0000);	    
        vpostLCDWriteCMDAddr(Dev_T,0x80);	
        vpostLCDWriteCMDReg(Dev_T,0x0000);
        vpostLCDWriteCMDAddr(Dev_T,0x81);	
        vpostLCDWriteCMDReg(Dev_T,0x0000);	    
        vpostLCDWriteCMDAddr(Dev_T,0x82);	
        vpostLCDWriteCMDReg(Dev_T,0x0000);	    
        vpostLCDWriteCMDAddr(Dev_T,0x83);	
        vpostLCDWriteCMDReg(Dev_T,0x0000);
        vpostLCDWriteCMDAddr(Dev_T,0x84);	
        vpostLCDWriteCMDReg(Dev_T,0x0000);
        vpostLCDWriteCMDAddr(Dev_T,0x85);	
        vpostLCDWriteCMDReg(Dev_T,0x0000);	    
        vpostLCDWriteCMDAddr(Dev_T,0x86);	
        vpostLCDWriteCMDReg(Dev_T,0x0000);	    
        vpostLCDWriteCMDAddr(Dev_T,0x87);	
        vpostLCDWriteCMDReg(Dev_T,0x0000);
        vpostLCDWriteCMDAddr(Dev_T,0x88);	
        vpostLCDWriteCMDReg(Dev_T,0x0000);
        vpostdelay_long(8000);
        vpostLCDWriteCMDAddr(Dev_T,0x30);	
        vpostLCDWriteCMDReg(Dev_T,0x0700);	    
        vpostLCDWriteCMDAddr(Dev_T,0x31);	
        vpostLCDWriteCMDReg(Dev_T,0x0007);	    
        vpostLCDWriteCMDAddr(Dev_T,0x32);	
//        vpostLCDWriteCMDReg(Dev_T,0x0000);   //050627
        vpostLCDWriteCMDReg(Dev_T,0x0700);
        vpostLCDWriteCMDAddr(Dev_T,0x33);	
        vpostLCDWriteCMDReg(Dev_T,0x0100);	    
        vpostLCDWriteCMDAddr(Dev_T,0x34);	
        vpostLCDWriteCMDReg(Dev_T,0x0707);	    
        vpostLCDWriteCMDAddr(Dev_T,0x35);	
        vpostLCDWriteCMDReg(Dev_T,0x0007);
        vpostLCDWriteCMDAddr(Dev_T,0x36);	
        vpostLCDWriteCMDReg(Dev_T,0x0700);	    
        vpostLCDWriteCMDAddr(Dev_T,0x37);	
        vpostLCDWriteCMDReg(Dev_T,0x0001);	    
        vpostLCDWriteCMDAddr(Dev_T,0x38);	
        vpostLCDWriteCMDReg(Dev_T,0x1800);
        vpostLCDWriteCMDAddr(Dev_T,0x39);	
        vpostLCDWriteCMDReg(Dev_T,0x0007);
        vpostdelay_long(400);
        vpostLCDWriteCMDAddr(Dev_T,0x40);	
        vpostLCDWriteCMDReg(Dev_T,0x0700);	    
        vpostLCDWriteCMDAddr(Dev_T,0x41);	
        vpostLCDWriteCMDReg(Dev_T,0x0007);
        vpostLCDWriteCMDAddr(Dev_T,0x42);	
#if 0	
        vpostLCDWriteCMDReg(Dev_T,0x0000);
#else
        vpostLCDWriteCMDReg(Dev_T,0x0700);  //updated 05/08/16//hsu
#endif
        vpostLCDWriteCMDAddr(Dev_T,0x43);	
        vpostLCDWriteCMDReg(Dev_T,0x0100);	    
        vpostLCDWriteCMDAddr(Dev_T,0x44);	
        vpostLCDWriteCMDReg(Dev_T,0x0707);
        vpostLCDWriteCMDAddr(Dev_T,0x45);	
        vpostLCDWriteCMDReg(Dev_T,0x0007);
        vpostLCDWriteCMDAddr(Dev_T,0x46);	
        vpostLCDWriteCMDReg(Dev_T,0x0700);	    
        vpostLCDWriteCMDAddr(Dev_T,0x47);	
        vpostLCDWriteCMDReg(Dev_T,0x0001);
        vpostLCDWriteCMDAddr(Dev_T,0x48);	
        vpostLCDWriteCMDReg(Dev_T,0x1800);
        vpostLCDWriteCMDAddr(Dev_T,0x49);	
        vpostLCDWriteCMDReg(Dev_T,0x0007);
        vpostdelay_long(400);
    }
/*    
    vpostLCDWriteCMDAddr(Dev_T,0x16);	
    vpostLCDReadCMDReg(Dev_T);	    
    vpostLCDWriteCMDAddr(Dev_T,0x35);	
    vpostLCDReadCMDReg(Dev_T);	    
    vpostLCDWriteCMDAddr(Dev_T,0x17);	
    vpostLCDReadCMDReg(Dev_T);
    vpostLCDWriteCMDAddr(Dev_T,0x36);	
    vpostLCDReadCMDReg(Dev_T);*/
}

void vpostPowerSettingFunction(UINT8 Dev_T)
{
    if (Dev_T==SAMSUNG_80_16BIT)
    {
        vpostLCDWriteCMDAddr(Dev_T,0x0c);	
        vpostLCDWriteCMDReg(Dev_T,0x0000);	    
        vpostLCDWriteCMDAddr(Dev_T,0x0d);	
        vpostLCDWriteCMDReg(Dev_T,0x0401);	    
        vpostLCDWriteCMDAddr(Dev_T,0x0e);	
        vpostLCDWriteCMDReg(Dev_T,0x0d18);
        vpostdelay_long(4000);
        vpostLCDWriteCMDAddr(Dev_T,0x03);	
        vpostLCDWriteCMDReg(Dev_T,0x0214);	    
        vpostLCDWriteCMDAddr(Dev_T,0x04);	
        vpostLCDWriteCMDReg(Dev_T,0x8000);	
        vpostdelay_long(4000);	
        vpostLCDWriteCMDAddr(Dev_T,0x0e);	
        vpostLCDWriteCMDReg(Dev_T,0x2910);	
        vpostdelay_long(4000);
        vpostLCDWriteCMDAddr(Dev_T,0x0d);	
        vpostLCDWriteCMDReg(Dev_T,0x0512);
    }else if((Dev_T==SAMSUNG_80_18BIT)||(Dev_T==SAMSUNG_80_9BIT))
    {
        vpostLCDWriteCMDAddr(Dev_T,0x07);	
        vpostLCDWriteCMDReg(Dev_T,0x0008);	    
        vpostLCDWriteCMDAddr(Dev_T,0x11);	
        vpostLCDWriteCMDReg(Dev_T,0x0067);	    
        vpostLCDWriteCMDAddr(Dev_T,0x12);	
        vpostLCDWriteCMDReg(Dev_T,0x0f09);
        vpostLCDWriteCMDAddr(Dev_T,0x13);	
        vpostLCDWriteCMDReg(Dev_T,0x121b);	    
        vpostLCDWriteCMDAddr(Dev_T,0x10);	
        vpostLCDWriteCMDReg(Dev_T,0x0010);	    
        vpostLCDWriteCMDAddr(Dev_T,0x12);	
        vpostLCDWriteCMDReg(Dev_T,0x0f19);
        vpostdelay_long(8000);
#if 0
        vpostLCDWriteCMDAddr(Dev_T,0x13);	
        vpostLCDWriteCMDReg(Dev_T,0x321b);	    
        vpostLCDWriteCMDAddr(Dev_T,0x11);	
        vpostLCDWriteCMDReg(Dev_T,0x0297);	
        vpostLCDWriteCMDAddr(Dev_T,0x10);	
        vpostLCDWriteCMDReg(Dev_T,0x0210);	
        vpostLCDWriteCMDAddr(Dev_T,0x12);	
//        vpostLCDWriteCMDReg(Dev_T,0x3f19);
        vpostLCDWriteCMDReg(Dev_T,0x3f1e);
#else
#if 1
        vpostLCDWriteCMDAddr(Dev_T,0x13);   //updated 05/08/16//hsu
        vpostLCDWriteCMDReg(Dev_T,0x321b);	    
        vpostLCDWriteCMDAddr(Dev_T,0x11);	
        vpostLCDWriteCMDReg(Dev_T,0x0297);	
        vpostLCDWriteCMDAddr(Dev_T,0x10);	
        vpostLCDWriteCMDReg(Dev_T,0x0210);	
        vpostLCDWriteCMDAddr(Dev_T,0x12);	
        vpostLCDWriteCMDReg(Dev_T,0x3f19);  //hsu
#else
        vpostLCDWriteCMDAddr(Dev_T,0x13);	
        vpostLCDWriteCMDReg(Dev_T,0x371f);	    
        vpostLCDWriteCMDAddr(Dev_T,0x11);	
        vpostLCDWriteCMDReg(Dev_T,0x0217);	
        vpostLCDWriteCMDAddr(Dev_T,0x10);	
        vpostLCDWriteCMDReg(Dev_T,0x0110);	
        vpostLCDWriteCMDAddr(Dev_T,0x12);	
//        vpostLCDWriteCMDReg(Dev_T,0x3f19);
        vpostLCDWriteCMDReg(Dev_T,0x3f1e);
#endif
#endif
        vpostdelay_long(12000);
    }
}
void vpostDisplayOffFunction(UINT8 Dev_T)
{
    vpostLCDWriteCMDAddr(Dev_T,0x07);	
    vpostLCDWriteCMDReg(Dev_T,0x0136);	    
    vpostdelay_long(4000);
    vpostLCDWriteCMDAddr(Dev_T,0x07);	
    vpostLCDWriteCMDReg(Dev_T,0x0126);	    
    vpostdelay_long(4000);
    vpostLCDWriteCMDAddr(Dev_T,0x07);	
    vpostLCDWriteCMDReg(Dev_T,0x0004);	    
}
void vpostDisplayOnFunction(UINT8 Dev_T,UINT8 ROT90Flg)
{
    if (Dev_T==SAMSUNG_80_16BIT)
    {
        vpostLCDWriteCMDAddr(Dev_T,0x07);	
        vpostLCDWriteCMDReg(Dev_T,0x0105);
        vpostdelay_long(4000);
        vpostLCDWriteCMDAddr(Dev_T,0x07);	
        vpostLCDWriteCMDReg(Dev_T,0x0125);	    
        vpostLCDWriteCMDAddr(Dev_T,0x07);	
        vpostLCDWriteCMDReg(Dev_T,0x0127);	
        vpostdelay_long(4000);
        vpostLCDWriteCMDAddr(Dev_T,0x07);	
        vpostLCDWriteCMDReg(Dev_T,0x0137);	    
        vpostLCDWriteCMDAddr(Dev_T,0x21);    
        vpostLCDWriteCMDReg(Dev_T,0x0000);		
        vpostLCDWriteCMDAddr(Dev_T,0x22);
    }else if((Dev_T==SAMSUNG_80_18BIT)||(Dev_T==SAMSUNG_80_9BIT))
    {
        vpostLCDWriteCMDAddr(Dev_T,0x07);	
        vpostLCDWriteCMDReg(Dev_T,0x0301);
        vpostdelay_long(12000);
        vpostLCDWriteCMDAddr(Dev_T,0x07);	
        vpostLCDWriteCMDReg(Dev_T,0x0321);	    
        vpostLCDWriteCMDAddr(Dev_T,0x07);	
        vpostLCDWriteCMDReg(Dev_T,0x0323);	
        vpostdelay_long(12000);
        vpostLCDWriteCMDAddr(Dev_T,0x07);	
        vpostLCDWriteCMDReg(Dev_T,0x0333);
        vpostdelay_long(8000);    
        vpostLCDWriteCMDAddr(Dev_T,0x20);
        if (ROT90Flg==0)
            vpostLCDWriteCMDReg(Dev_T,0x007f);
        else
            vpostLCDWriteCMDReg(Dev_T,0x007f);
//        vpostLCDWriteCMDReg(Dev_T,0x007f);
        vpostLCDWriteCMDAddr(Dev_T,0x21);
        if (ROT90Flg==0)
            vpostLCDWriteCMDReg(Dev_T,0x0000);
        else
            vpostLCDWriteCMDReg(Dev_T,0x009f);
        vpostLCDWriteCMDAddr(Dev_T,0x22);
    }
}

void vpostNECInit(UINT8 Dev_T,UINT8 ROT90Flg)
{
    if (ROT90Flg==1)	
        vpostLCDWriteCMDAddr(Dev_T,0x0140);
    else
        vpostLCDWriteCMDAddr(Dev_T,0x0100);
    vpostLCDWriteCMDAddr(Dev_T,0x0301);
    vpostdelay_long(1000);

    vpostLCDWriteCMDAddr(Dev_T,0x2200);
    if (ROT90Flg==1)	
    {
        vpostLCDWriteCMDAddr(Dev_T,0x0143);
	vpostLCDWriteCMDAddr(Dev_T,0x0140);
    }
    else
    {
	vpostLCDWriteCMDAddr(Dev_T,0x0103);
	vpostLCDWriteCMDAddr(Dev_T,0x0100);	
    }
    vpostLCDWriteCMDAddr(Dev_T,0x2201);	

    vpostLCDWriteCMDAddr(Dev_T,0x2200);	
    vpostdelay_long(2000);
    if (ROT90Flg==1)		
	vpostLCDWriteCMDAddr(Dev_T,0x0142);
    else	
	vpostLCDWriteCMDAddr(Dev_T,0x0102);	
    vpostdelay_long(1000);	
    vpostLCDWriteCMDAddr(Dev_T,0x4E20);	
    vpostdelay_long(1000);	
    vpostLCDWriteCMDAddr(Dev_T,0x4E60);	
    vpostdelay_long(1000);	

    vpostLCDWriteCMDAddr(Dev_T,0x4101);	
    vpostdelay_long(1000);
    vpostLCDWriteCMDAddr(Dev_T,0x2677);	
    vpostdelay_long(1000);		
    vpostLCDWriteCMDAddr(Dev_T,0x27CB);	
    vpostdelay_long(1000);		
    vpostLCDWriteCMDAddr(Dev_T,0x285D);	
    vpostdelay_long(1000);		
    vpostLCDWriteCMDAddr(Dev_T,0x2965);	
    vpostdelay_long(1000);	
    vpostLCDWriteCMDAddr(Dev_T,0x2A40);	
    vpostdelay_long(1000);		
    vpostLCDWriteCMDAddr(Dev_T,0x3C15);	
    vpostdelay_long(1000);		
    vpostLCDWriteCMDAddr(Dev_T,0x3D15);	
    vpostdelay_long(1000);		
    vpostLCDWriteCMDAddr(Dev_T,0x3E96);	
    vpostdelay_long(1000);		
    vpostLCDWriteCMDAddr(Dev_T,0x3FE4);	
    vpostdelay_long(1000);		
    vpostLCDWriteCMDAddr(Dev_T,0x402E);	
    vpostdelay_long(1000);	

    vpostLCDWriteCMDAddr(Dev_T,0x0000);	
    vpostLCDWriteCMDAddr(Dev_T,0x0200);
    if (ROT90Flg==1)	
	vpostLCDWriteCMDAddr(Dev_T,0x0514); // rotation (b2)
    else
	vpostLCDWriteCMDAddr(Dev_T,0x0510); /* Bit 7 -> H 18 bit mode , L -> 16 bit mode */

    vpostLCDWriteCMDAddr(Dev_T,0x1A87);
    vpostLCDWriteCMDAddr(Dev_T,0x2B10);
    vpostLCDWriteCMDAddr(Dev_T,0x2C01);
    vpostLCDWriteCMDAddr(Dev_T,0x2D20);
    vpostLCDWriteCMDAddr(Dev_T,0x2E20);
    vpostLCDWriteCMDAddr(Dev_T,0x2F01);
    vpostLCDWriteCMDAddr(Dev_T,0x3011);
    vpostLCDWriteCMDAddr(Dev_T,0x31DE);
    vpostLCDWriteCMDAddr(Dev_T,0x3207);
    vpostLCDWriteCMDAddr(Dev_T,0x3300);
    vpostLCDWriteCMDAddr(Dev_T,0x3411);
    vpostLCDWriteCMDAddr(Dev_T,0x35ED);
    vpostLCDWriteCMDAddr(Dev_T,0x3607);
    vpostLCDWriteCMDAddr(Dev_T,0x3700);
    vpostLCDWriteCMDAddr(Dev_T,0x3802);
    vpostLCDWriteCMDAddr(Dev_T,0x3B11);
    vpostLCDWriteCMDAddr(Dev_T,0x4200);
    vpostLCDWriteCMDAddr(Dev_T,0x4400);
    vpostLCDWriteCMDAddr(Dev_T,0x4820);
    vpostLCDWriteCMDAddr(Dev_T,0x4902);
    vpostLCDWriteCMDAddr(Dev_T,0x4B05);
    vpostLCDWriteCMDAddr(Dev_T,0x4C01);
    vpostLCDWriteCMDAddr(Dev_T,0x4E60);
    vpostLCDWriteCMDAddr(Dev_T,0x4F01);
    vpostLCDWriteCMDAddr(Dev_T,0x5028);
    vpostLCDWriteCMDAddr(Dev_T,0x5303);
    vpostLCDWriteCMDAddr(Dev_T,0x540E);
    vpostLCDWriteCMDAddr(Dev_T,0x550F);
    vpostLCDWriteCMDAddr(Dev_T,0x5618);
    vpostLCDWriteCMDAddr(Dev_T,0x5719);
    vpostLCDWriteCMDAddr(Dev_T,0x5822);
    vpostLCDWriteCMDAddr(Dev_T,0x5D78);
    vpostLCDWriteCMDAddr(Dev_T,0x4D2C);	
    if (ROT90Flg==1)		
	vpostLCDWriteCMDAddr(Dev_T,0x0594); // rotation
    else
	vpostLCDWriteCMDAddr(Dev_T,0x0590);

    vpostLCDWriteCMDAddr(Dev_T,0x0000);	
    if (ROT90Flg==1)
    {
	vpostLCDWriteCMDAddr(Dev_T,0x0600);		//Xaddr register set 
	vpostLCDWriteCMDAddr(Dev_T,0); 
	vpostLCDWriteCMDAddr(Dev_T,0x0700);		//Yaddr register set
	vpostLCDWriteCMDAddr(Dev_T,0); 
	vpostLCDWriteCMDAddr(Dev_T,0x0800);		//min Xaddr register set 
	vpostLCDWriteCMDAddr(Dev_T,0); 
	vpostLCDWriteCMDAddr(Dev_T,0x0900);		//max Xaddr register set
	vpostLCDWriteCMDAddr(Dev_T,239); 
	vpostLCDWriteCMDAddr(Dev_T,0x0A00);		//min Yaddr register set 
	vpostLCDWriteCMDAddr(Dev_T,0); 
	vpostLCDWriteCMDAddr(Dev_T,0x0B00);		//max Yaddr register set
	vpostLCDWriteCMDAddr(Dev_T,319);   
    }
    else
    {	
	vpostLCDWriteCMDAddr(Dev_T,0x0600);		//Xaddr register set 
	vpostLCDWriteCMDAddr(Dev_T,0); 
	vpostLCDWriteCMDAddr(Dev_T,0x0700);		//Yaddr register set
	vpostLCDWriteCMDAddr(Dev_T,0); 
	vpostLCDWriteCMDAddr(Dev_T,0x0800);		//min Xaddr register set 
	vpostLCDWriteCMDAddr(Dev_T,0); 
	vpostLCDWriteCMDAddr(Dev_T,0x0900);		//max Xaddr register set
	vpostLCDWriteCMDAddr(Dev_T,239); 
	vpostLCDWriteCMDAddr(Dev_T,0x0A00);		//min Yaddr register set 
	vpostLCDWriteCMDAddr(Dev_T,0); 
	vpostLCDWriteCMDAddr(Dev_T,0x0B00);		//max Yaddr register set
	vpostLCDWriteCMDAddr(Dev_T,319);
    }
}

void vpostSetupMpuLCD(UINT8 ucDev_Type,UINT8 ROT90)
{
    writew(REG_LCM_DCCS,readw(REG_LCM_DCCS)&0xfffffff5); //output-disable,video disable
    writew(REG_LCM_DCCS,readw(REG_LCM_DCCS)|0x00000008); //display_out-enable
    vpostdelay_long(4000);
    if (ucDev_Type==HITACHI_80_18BIT)
        vpostHitachiInit(ucDev_Type,ROT90);
    else if (ucDev_Type==NEC_80_18BIT)
        vpostNECInit(ucDev_Type,ROT90);
    else if ((ucDev_Type==SAMSUNG_80_16BIT)||(ucDev_Type==SAMSUNG_80_18BIT)||(ucDev_Type==SAMSUNG_80_9BIT))
    {
        vpostInitialFunction1(ucDev_Type,ROT90);
        vpostPowerSettingFunction(ucDev_Type);
        vpostInitialFunction2(ucDev_Type);
        vpostDisplayOnFunction(ucDev_Type,ROT90);
//        vpostDisplayOffFunction(ucDev_Type);
//        StandByMode(ucDev_Type);
    }
//    if ((ucDev_Type==TOPPOLY_80_16BIT)||(ucDev_Type==TOPPOLY_80_8BIT)||(ucDev_Type==TOPPOLY_80_18BIT)||(ucDev_Type==TOPPOLY_80_9BIT))
    else
    {
        vpostLCDWriteCMDAddr(ucDev_Type,0x10);    
        vpostLCDWriteCMDReg(ucDev_Type,0x00);
        vpostLCDWriteCMDAddr(ucDev_Type,0x11);  
        vpostLCDWriteCMDReg(ucDev_Type,0x00);    

        vpostLCDWriteCMDAddr(ucDev_Type,0x12);
        vpostLCDWriteCMDReg(ucDev_Type,0x00);    
        vpostLCDWriteCMDAddr(ucDev_Type,0x13);    
        vpostLCDWriteCMDReg(ucDev_Type,0x00);    

        vpostLCDWriteCMDAddr(ucDev_Type,0x14);        
        vpostLCDWriteCMDReg(ucDev_Type,0x00);    
        vpostLCDWriteCMDAddr(ucDev_Type,0x15);    
        vpostLCDWriteCMDReg(ucDev_Type,0x7f);
        vpostLCDWriteCMDAddr(ucDev_Type,0x16);
        vpostLCDWriteCMDReg(ucDev_Type,0x9f);
    
//        vpostLCDWriteCMDAddr(ucDev_Type,0x01);      //8 color mode   
//        vpostLCDWriteCMDReg(ucDev_Type,0x21);    
        vpostLCDWriteCMDAddr(ucDev_Type,0x01);   
        vpostLCDWriteCMDReg(ucDev_Type,0x00);        
        vpostLCDWriteCMDAddr(ucDev_Type,0x02);         
        if ((ucDev_Type==TOPPOLY_80_16BIT)||(ucDev_Type==TOPPOLY_68_16BIT))
            vpostLCDWriteCMDReg(ucDev_Type,0x66);    
        if ((ucDev_Type==TOPPOLY_80_8BIT)||(ucDev_Type==TOPPOLY_68_8BIT))
            vpostLCDWriteCMDReg(ucDev_Type,0x60); 
        if ((ucDev_Type==TOPPOLY_80_9BIT)||(ucDev_Type==TOPPOLY_68_9BIT))
            vpostLCDWriteCMDReg(ucDev_Type,0x68); 
        if ((ucDev_Type==TOPPOLY_80_18BIT)||(ucDev_Type==TOPPOLY_68_18BIT))
            vpostLCDWriteCMDReg(ucDev_Type,0x6e); 
/*
        vpostLCDWriteCMDAddr(ucDev_Type,0x15);    
        vpostLCDReadCMDReg(ucDev_Type);
        vpostLCDWriteCMDAddr(ucDev_Type,0x16);
        vpostLCDReadCMDReg(ucDev_Type);
        vpostLCDWriteCMDAddr(ucDev_Type,0x02);    
        vpostLCDReadCMDReg(ucDev_Type);
*/
    }
}
#endif

#ifdef notdef // Added by JLLin
#if 0
int vpostCheck_CRC(UINT16 usSR,UINT16 usSG,UINT16 usSB)
{
    UINT16 sigR, sigG, sigB,tmpv;

    writew(REG_LCM_DCCS,readw(REG_LCM_DCCS)|0x00000001); //rgb565,single
    writew(REG_LCM_DCCS,readw(REG_LCM_DCCS)&0xfffffffe); //rgb565,single
    tmpv=0x1000;
    while(tmpv-->0);

    vpostOSD_Disable();
    vpostVA_Disable();
    writew(REG_LCM_DCCS,readw(REG_LCM_DCCS)|0x00000080); //rgb565,single
    writew(REG_LCM_DCCS,readw(REG_LCM_DCCS)|0x00000008); //display_out-enable
/*--- R ---*/
    writew(REG_LCM_VA_TEST,0x00000002);//r
    writew(REG_LCM_VA_TEST,0x0000000a);
    writew(REG_LCM_VA_TEST,0x0000000b);
    while((readw(REG_LCM_VA_TEST) & 0x00000080)==0x00000000);	// wait	for CRC	ready
    writew(REG_LCM_VA_TEST,0x0000000a);
    writew(REG_LCM_DCCS,readw(REG_LCM_DCCS)|0x00000002); //va-enable
//    while((readw(REG_LCM_DCCS) & 0x10000000)!=0x00000000);// wait for vertical retrace
    while((readw(REG_LCM_VA_TEST) & 0x00000080)==0x00000080);	// wait	for CRC	ready
    while((readw(REG_LCM_DCCS)&0x00000002)==0x00000002);
    sigR = (UINT16)(readw(REG_LCM_VA_TEST)>>16);

/*--- G ---*/
    writew(REG_LCM_DCCS,readw(REG_LCM_DCCS)|0x00000001); //rgb565,single
    writew(REG_LCM_DCCS,readw(REG_LCM_DCCS)&0xfffffffe); //rgb565,single
    tmpv=0x1000;
    while(tmpv-->0);


    writew(REG_LCM_VA_TEST,0x00000004);
    writew(REG_LCM_VA_TEST,0x0000000c);
    writew(REG_LCM_VA_TEST,0x0000000d);
    while((readw(REG_LCM_VA_TEST) & 0x00000080)==0x00000000);	// wait	for CRC	ready
    writew(REG_LCM_VA_TEST,0x0000000c);
    writew(REG_LCM_DCCS,readw(REG_LCM_DCCS)|0x00000002); //va-enable
//    while((readw(REG_LCM_DCCS) & 0x10000000)!=0x00000000);// wait for vertical retrace
    while((readw(REG_LCM_VA_TEST) & 0x00000080)==0x00000080);	// wait	for CRC	ready
    while((readw(REG_LCM_DCCS)&0x00000002)==0x00000002);
    sigG = (UINT16)(readw(REG_LCM_VA_TEST)>>16);

/*--- B ---*/
    writew(REG_LCM_DCCS,readw(REG_LCM_DCCS)|0x00000001); //rgb565,single
    writew(REG_LCM_DCCS,readw(REG_LCM_DCCS)&0xfffffffe); //rgb565,single
    tmpv=0x1000;
    while(tmpv-->0);


    writew(REG_LCM_VA_TEST,0x00000006);
    writew(REG_LCM_VA_TEST,0x0000000e);
    writew(REG_LCM_VA_TEST,0x0000000f);
    while((readw(REG_LCM_VA_TEST) & 0x00000080)==0x00000000);	// wait	for CRC	ready
    writew(REG_LCM_VA_TEST,0x0000000e);
    writew(REG_LCM_DCCS,readw(REG_LCM_DCCS)|0x00000002); //va-enable
//    while((readw(REG_LCM_DCCS) & 0x10000000)!=0x00000000);// wait for vertical retrace
    while((readw(REG_LCM_VA_TEST) & 0x00000080)==0x00000080);	// wait	for CRC	ready
    while((readw(REG_LCM_DCCS)&0x00000002)==0x00000002);
    sigB = (UINT16)(readw(REG_LCM_VA_TEST)>>16);

    if ((sigR==usSR)&&(sigG==usSG)&&(sigB==usSB))
       return 0;
    else
       return LCM_ERR_CRC;
}
#else
int vpostCheck_CRC(UINT16 usSR,UINT16 usSG,UINT16 usSB)
{
    UINT16 sigR, sigG, sigB,tmpv;

    writew(REG_LCM_DCCS,readw(REG_LCM_DCCS)|0x00000001); //rgb565,single
    writew(REG_LCM_DCCS,readw(REG_LCM_DCCS)&0xfffffffe); //rgb565,single
    tmpv=0x1000;
    while(tmpv-->0);

    vpostOSD_Disable();
    vpostVA_Disable();
    writew(REG_LCM_DCCS,readw(REG_LCM_DCCS)|0x00000080); //rgb565,single
    writew(REG_LCM_DCCS,readw(REG_LCM_DCCS)|0x00000008); //display_out-enable
/*--- R ---*/
    writew(REG_LCM_VA_TEST,0x00000002);//r
    writew(REG_LCM_VA_TEST,0x0000000a);
    writew(REG_LCM_VA_TEST,0x0000000b);
    while((readw(REG_LCM_VA_TEST) & 0x00000080)==0x00000000);	// wait	for CRC	ready
    writew(REG_LCM_VA_TEST,0x0000000a);
    writew(REG_LCM_DCCS,readw(REG_LCM_DCCS)|0x00000002); //va-enable
//    while((readw(REG_LCM_DCCS) & 0x10000000)!=0x00000000);// wait for vertical retrace
    while((readw(REG_LCM_VA_TEST) & 0x00000080)==0x00000080);	// wait	for CRC	ready
    while((readw(REG_LCM_DCCS)&0x00000002)==0x00000002);
    sigR = (UINT16)(readw(REG_LCM_VA_TEST)>>16);

/*--- G ---*/
    writew(REG_LCM_DCCS,readw(REG_LCM_DCCS)|0x00000001); //rgb565,single
    writew(REG_LCM_DCCS,readw(REG_LCM_DCCS)&0xfffffffe); //rgb565,single
    tmpv=0x1000;
    while(tmpv-->0);


    writew(REG_LCM_VA_TEST,0x00000004);
    writew(REG_LCM_VA_TEST,0x0000000c);
    writew(REG_LCM_VA_TEST,0x0000000d);
    while((readw(REG_LCM_VA_TEST) & 0x00000080)==0x00000000);	// wait	for CRC	ready
    writew(REG_LCM_VA_TEST,0x0000000c);
    writew(REG_LCM_DCCS,readw(REG_LCM_DCCS)|0x00000002); //va-enable
//    while((readw(REG_LCM_DCCS) & 0x10000000)!=0x00000000);// wait for vertical retrace
    while((readw(REG_LCM_VA_TEST) & 0x00000080)==0x00000080);	// wait	for CRC	ready
    while((readw(REG_LCM_DCCS)&0x00000002)==0x00000002);
    sigG = (UINT16)(readw(REG_LCM_VA_TEST)>>16);

/*--- B ---*/
    writew(REG_LCM_DCCS,readw(REG_LCM_DCCS)|0x00000001); //rgb565,single
    writew(REG_LCM_DCCS,readw(REG_LCM_DCCS)&0xfffffffe); //rgb565,single
    tmpv=0x1000;
    while(tmpv-->0);


    writew(REG_LCM_VA_TEST,0x00000006);
    writew(REG_LCM_VA_TEST,0x0000000e);
    writew(REG_LCM_VA_TEST,0x0000000f);
    while((readw(REG_LCM_VA_TEST) & 0x00000080)==0x00000000);	// wait	for CRC	ready
    writew(REG_LCM_VA_TEST,0x0000000e);
    writew(REG_LCM_DCCS,readw(REG_LCM_DCCS)|0x00000002); //va-enable
//    while((readw(REG_LCM_DCCS) & 0x10000000)!=0x00000000);// wait for vertical retrace
    while((readw(REG_LCM_VA_TEST) & 0x00000080)==0x00000080);	// wait	for CRC	ready
    while((readw(REG_LCM_DCCS)&0x00000002)==0x00000002);
    sigB = (UINT16)(readw(REG_LCM_VA_TEST)>>16);

    if ((sigR==usSR)&&(sigG==usSG)&&(sigB==usSB))
       return 0;
    else
       return LCM_ERR_CRC;
}

#endif

#endif


/* sjlu 20050512*/
#define SDA	(0x1<<1)
#define SCLK	(0x1)
#define CS		(1<<2)
#define READSTATUS() ((inpw(REG_SerialBusCR)&0x38)>>3)

static void Delay(int nCnt)
{
	int volatile loop;
	for (loop=0; loop<nCnt*100; loop++);
}

void TPG038D_WriteLCMRegister(UINT8 addr,UINT8 data)
{
	UINT16 cmd;
	int i;
	
	sysDisableCache();
 	sysInvalidCache();

	/* for demo board */
	
	outpw(REG_PADC0, inpw(REG_PADC0) | (7<<1));
	
	cmd = ((addr & 0x3f)<<10) | (0<<9)| data;
	outpw(REG_SerialBusCR,READSTATUS() &~SCLK | CS);//cs high, sclk low
	Delay(1);
	outpw(REG_SerialBusCR,READSTATUS() & ~CS);//cs low
	Delay(1);
	for (i=15;i>=0;i--){
		if (i==8)
		{
			outpw(REG_SerialBusCR,READSTATUS() & ~SCLK);//sclk low
			Delay(0x1);
			outpw(REG_SerialBusCR,READSTATUS() | SCLK);//sclk high
			Delay(0x1);
		}
		else if  ((1<<i)&cmd){//cmd bit is high
			outpw(REG_SerialBusCR,READSTATUS() & ~SCLK | SDA);//sclk low, SDA ready
			Delay(0x1);
			outpw(REG_SerialBusCR,READSTATUS() | SCLK | SDA);//sclk high
			Delay(0x1);
			
		}
		else{
			outpw(REG_SerialBusCR,READSTATUS() & ~SDA & ~SCLK);//sclk low
			Delay(0x1);
			outpw(REG_SerialBusCR,READSTATUS() & ~SDA | SCLK);//sclk high
			Delay(0x1);
		}
		
	}


	outpw(REG_SerialBusCR,READSTATUS() & ~SCLK);//sclk low
	Delay(2);
	outpw(REG_SerialBusCR,READSTATUS() | CS);//cs high, for latch
	
	//outpw(REG_PADC0, inpw(REG_PADC0) & ~(7<<1));//clyu collision with RTC
	
	sysEnableCache(CACHE_WRITE_THROUGH);

}
static UINT8 ReadLCMRegister(UINT8 addr)
{
	UINT16 cmd;
	UINT8 data;
	int i;
	
	outpw(REG_PADC0, inpw(REG_PADC0) | (7<<1));
	
	cmd = ((addr & 0x3f)<<10) | (1<<9);
	outpw(REG_SerialBusCR,READSTATUS() &~SCLK | CS);//cs high, sclk low
	Delay(1);
	outpw(REG_SerialBusCR,READSTATUS() & ~CS);//cs low
	Delay(1);
	for (i=15;i>=0;i--){
		if (i>8){//address
			if  ((1<<i)&cmd){//cmd bit is high
				outpw(REG_SerialBusCR,READSTATUS() & ~SCLK | SDA);//sclk low, SDA ready
				Delay(0x1);
				outpw(REG_SerialBusCR,READSTATUS() | SCLK | SDA);//sclk high
				Delay(0x1);
			}	
			else{
				outpw(REG_SerialBusCR,READSTATUS() & ~SDA & ~SCLK);//sclk low
				Delay(0x1);
				outpw(REG_SerialBusCR,READSTATUS() & ~SDA | SCLK);//sclk high
				Delay(0x1);
			}
			outpw(REG_SerialBusCR,READSTATUS() & ~SDA);

			
		}else if (i==8){
			outpw(REG_SerialBusCR,READSTATUS() & ~SCLK);//sclk low
			Delay(0x1);
			outpw(REG_SerialBusCR,READSTATUS() | SCLK);//sclk high
			Delay(0x1);
		}else		
		{
			outpw(REG_SerialBusCR,READSTATUS() & ~SCLK);//sclk low
			Delay(0x1);
			data = data | ((READSTATUS() && SDA) << i);
			outpw(REG_SerialBusCR,READSTATUS() | SCLK);//sclk high
			Delay(0x1);
		}
		
	}
	outpw(REG_SerialBusCR,READSTATUS() & ~SCLK);//sclk low
	Delay(2);
	outpw(REG_SerialBusCR,READSTATUS() | CS);//cs high, for latch
	
	//outpw(REG_PADC0, inpw(REG_PADC0) & ~(7<<1));//clyu collision with RTC
	return data;
}

#if 1
void swDelay(UINT32 uloop)
{
	UINT8 volatile i;
	if(uloop == 0)
		for(i=0; i<=200; i++);
	else
		for(i=0; i<=uloop; i++);
}

static void swSDOLevel(UINT8 level)
{
    UINT32 volatile GPIOData;

    GPIOData = inpw(REG_GPIOA_DAT);      
    if(level)
        GPIOData |=  _LCM_SDO;		//SDO HIGH
    else
        GPIOData &= ~_LCM_SDO;		//SDO LOW          
    outpw(REG_GPIOA_DAT, GPIOData);	
}

/*---------------------------------*/
static void swSDALevel(UINT8 level)
{
    UINT32 volatile GPIOData;
    
    
#ifdef CTRL_USE_GPIO
    
    GPIOData = inpw(REG_GPIO_DAT);  
    if(level)
        GPIOData |=  _802_SDA;		//SDA HIGH
    else
        GPIOData &= ~_802_SDA;		//SDA LOW          
    outpw(REG_GPIO_DAT, GPIOData);		
    
#else
  
    GPIOData = inpw(REG_GPIOA_DAT);  
    if(level)
        GPIOData |=  _802_SDA;		//SDA HIGH
    else
        GPIOData &= ~_802_SDA;		//SDA LOW          
    outpw(REG_GPIOA_DAT, GPIOData);		
    
#endif
}
/*--------------------------------*/

static void swSCLLevel(UINT8 level)
{
    UINT32 volatile GPIOData;
    
#ifdef CTRL_USE_GPIO
    
    GPIOData = inpw(REG_GPIO_DAT);  
    if(level)
        GPIOData |=  _802_SCL;		//SCLK HIGH
    else
        GPIOData &= ~_802_SCL;		//SCLK LOW    
    outpw(REG_GPIO_DAT, GPIOData);	
    	
#else
    
    GPIOData = inpw(REG_GPIOA_DAT);  
    if(level)
        GPIOData |=  _802_SCL;		//SCLK HIGH
    else
        GPIOData &= ~_802_SCL;		//SCLK LOW    
    outpw(REG_GPIOA_DAT, GPIOData);		
    
#endif
}

static UINT32 g_uiIbitEnable;

int I2CRDWRBegin(int flag)
{
#if 1
	cyg_scheduler_lock();
	g_uiIbitEnable = sysGetIBitState();
	sysSetLocalInterrupt(DISABLE_IRQ);
#endif
	return 0;
}

int I2CRDWREnd(int flag)
{
#if 1
	g_uiIbitEnable ? sysSetLocalInterrupt(ENABLE_IRQ) : sysSetLocalInterrupt(DISABLE_IRQ);
	cyg_scheduler_unlock();
#endif
	return 0;
}

void Write_Tda025_Reg(UINT8 reg, UINT8 value)
{
    UINT8 i,TransReg;
    
    UINT32 volatile tmp, gpioOutputEnable;
    
    I2CRDWRBegin(0);
    //set I2C pins and LCS2 are GPIO pins, (LCS2#:Normal Operation), (SDO/SDA/SCK, GPIOA[4/3/2])
    tmp = inpw(REG_PADC0);
    gpioOutputEnable = inpw(REG_GPIO_OE);
    outpw(REG_PADC0, inpw(REG_PADC0)&(~0x0020000E));
    outpw(REG_GPIO_OE, inpw(REG_GPIO_OE)&~(_802_SDA | _802_SCL));
    
    TransReg = reg << 2;
    TransReg &= ~0x02;		//Set Write
    TransReg &= ~0x01;		//Set HiZ		?? Tooply spec
	
    swSDOLevel(1);			//SDO->HIGH
    swSDALevel(0);			//SDA->Low
    swSCLLevel(0);			//SCK->Low 
    swDelay(0);    

	swSDOLevel(0);			//SEN->Low  Data setup star-up
	swDelay(0);

    for(i=0; i<8; i++)		//fill Register Number
    {
		if(TransReg&0x80)
		    swSDALevel(1);	//Data->High 
		else
		    swSDALevel(0);	//Data->Low     
		swDelay(0);
		swSCLLevel(1);		//SCK->High 
		swDelay(0);
		swSCLLevel(0);		//SCK->Low 	
		TransReg <<= 1;
    }
	
    for(i=0; i<8; i++)		//fill Register Value
    {
		if(value&0x80)
		    swSDALevel(1);	//Data->High 
		else
		    swSDALevel(0);	//Data->Low     
		swDelay(0);
		swSCLLevel(1);		//SCK->High 
		swDelay(0);
		swSCLLevel(0);		//SCK->Low 		
		value <<= 1;
    }	

    swDelay(0);
    swSDOLevel(1);			//SDO->HIGH	
    swSDALevel(0);			//Data->Low	
    
	outpw(REG_PADC0, tmp);//clyu for collision with RTC
	outpw(REG_GPIO_OE, gpioOutputEnable);
	
	I2CRDWREnd(0);
    
}


#endif

#define F_SDA		(1<<5)
#define F_SCLK		(1<<1)
#define F_CS		(1<<4)

static void writeTda025Reg(UINT8 addr,UINT8 data)
{
	UINT16 cmd;
	int i;
	
	outpw(REG_PADC0, inpw(REG_PADC0) & ~(7<<1));//switch PAD to GPIO
	outpw(REG_GPIOA_OE, inpw(REG_GPIOA_OE) & ~(1<<4));//GPIOA[4]:CS,
	outpw(REG_GPIO_OE,inpw(REG_GPIO_OE)&~(1<<1) &~(1<<5));//GPIO[1]:SCLK,GPIO[5]:SDA
	
	cmd = ((addr & 0x3f)<<10) | (0<<9)| data;
	//cs high, sclk low
	outpw(REG_GPIOA_DAT,inpw(REG_GPIOA_DAT) | F_CS);
	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~F_SCLK & ~F_SDA);
	
	
	cyg_thread_delay(1);
	outpw(REG_GPIOA_DAT,inpw(REG_GPIOA_DAT) & ~F_CS);//cs low
	cyg_thread_delay(1);
	for (i=15;i>=0;i--)
	{
		if  ((1<<i)&cmd)
		{	//cmd bit is high
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT)| F_SDA);
		}
		else
		{
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT)& ~F_SDA);
		}
		cyg_thread_delay(0x1);
		outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT)  | F_SCLK);
		cyg_thread_delay(0x1);
		outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT)  & ~F_SCLK);
	}



	//outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~F_SCLK);//sclk low
	cyg_thread_delay(1);
	outpw(REG_GPIOA_DAT,inpw(REG_GPIOA_DAT) | F_CS);//cs high, for latch
	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT)& ~F_SDA );
	cyg_thread_delay(1);
	
}

void vpostInitAtFirst(void)
{
	outpw(REG_LCM_DCCS, inpw(REG_LCM_DCCS) & 0xffff00ff | 0x00004400);
	writeTda025Reg(0x02, 0x02); // through mode
	//sysprintf("Tda025 [%x]\n", readTda025Reg(0x02));

	// set the VPOST registers
	outpw(REG_MISCR, 0x00);
	outpw(REG_LCM_MPU_CMD, 0x00000000);
	outpw(REG_LCM_INT_CS, 0xC0000003);
	outpw(REG_LCM_CRTC_SIZE, 0x01060493);
	outpw(REG_LCM_CRTC_DEND, 0x00F003C0);
	outpw(REG_LCM_CRTC_HR, 0x03E003D0);
	outpw(REG_LCM_CRTC_HSYNC, 0x04100401);
	outpw(REG_LCM_CRTC_VR, 0x00F900F8);

	outpw(REG_LCM_VA_FBCTRL, 0x00A000A0);
	outpw(REG_LCM_VA_SCALE, 0x04000C20);
	outpw(REG_LCM_VA_TEST, 0x00010000);

	// trigger
	outpw(REG_LCM_DEV_CTRL, 0x13000082);
	outpw(REG_LCM_DCCS, 0x0005441A);
}

void InitialTda025LCM(void)
{
	UINT32 volatile tmp;
    //set I2C pins and LCS2 are GPIO pins, (LCS2#:Normal Operation), (SDO/SDA/SCK, GPIOA[4/3/2])
    tmp = inpw(REG_PADC0);
    outpw(REG_PADC0, inpw(REG_PADC0)&(~0x0020000E));

   //set GPIOA-04 is High
    outpw(REG_GPIOA_DAT, inpw(REG_GPIOA_DAT)|(0x00000010));	
   //set GPIOA-04 are Output Mode
    outpw(REG_GPIOA_OE, inpw(REG_GPIOA_OE)&(~0x00000010));

	//use 802_SCK/802_SDA, GPIO-01/GPIO-05
    outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT)|(0x00000022));	
    outpw(REG_GPIO_OE, inpw(REG_GPIO_OE)&(~0x00000022));

	//set TOPPLY LCD SHDB pin is High
   //set GPIOB16 is High
    outpw(REG_GPIOB_DAT, inpw(REG_GPIOB_DAT)|(0x00010000));	
   //set GPIOB16 is Output Mode
    outpw(REG_GPIOB_OE, inpw(REG_GPIOB_OE)&(~0x00010000));
    //set GPIOB16 is High
    outpw(REG_GPIOB_DAT, inpw(REG_GPIOB_DAT)|(0x00010000));

    //Write_LCD_Reg(0x02,0x00); // RGB Dummy(4/4/4/0),Valid(1440):0x08(Width:1280=>0x00)
    Write_Tda025_Reg(0x02, 0x01); // YUV422, 1280
    Write_Tda025_Reg(0x03, 0x30);			
    Write_Tda025_Reg(0x04, 0x0F);
    Write_Tda025_Reg(0x05, 0x13);	
    Write_Tda025_Reg(0x06, 0x18); // Shift-H, default(0x18)		   
    Write_Tda025_Reg(0x07, 0x08); // Shift-V, default(0x08)	      
    Write_Tda025_Reg(0x08, 0x00);	    
    Write_Tda025_Reg(0x09, 0x20);	
    Write_Tda025_Reg(0x0A, 0x20);	     
    Write_Tda025_Reg(0x0B, 0x20);	       
    Write_Tda025_Reg(0x0C, 0x10);      
    Write_Tda025_Reg(0x10, 0x3A);
    Write_Tda025_Reg(0x11, 0x3A);	
    Write_Tda025_Reg(0x12, 0x1D);	   
    Write_Tda025_Reg(0x13, 0x17);	  
    Write_Tda025_Reg(0x14, 0x98);	  
    Write_Tda025_Reg(0x15, 0x9A);		      
    Write_Tda025_Reg(0x16, 0xA9);	       
    Write_Tda025_Reg(0x17, 0x99);	
    Write_Tda025_Reg(0x18, 0x08);
    
    outpw(REG_PADC0, tmp);//clyu for collision with RTC
}

void RGBHardWareRegistersInit(void)
{
	
	outpw(REG_LCM_DCCS, inpw(REG_LCM_DCCS)&0xffff00ff | 0x00004400);
	Write_Tda025_Reg(0x02, 0x02); // THROUGH_MODE
	
	//diag_printf("Read_Tda025_Reg 0x02 = %d\n",Read_Tda025_Reg(0x02));
	outpw(REG_LCM_MPU_CMD,           0x00000000);
	outpw(REG_LCM_INT_CS,     0xC0000003);
	outpw(REG_LCM_CRTC_SIZE,  0x01060493);
	outpw(REG_LCM_CRTC_DEND,  0x00F003C0);
	outpw(REG_LCM_CRTC_HR,          0x03E003D0);
	outpw(REG_LCM_CRTC_HSYNC, 0x04100401);
	outpw(REG_LCM_CRTC_VR,           0x00F900F8);
	outpw(REG_LCM_VA_FBCTRL,   0x00a000a0);
	outpw(REG_LCM_VA_SCALE,    0x04000c20);
	outpw(REG_LCM_VA_TEST,     0x00010000);
	//trigger
	outpw(REG_LCM_DEV_CTRL,   0x13000082);
	outpw(REG_LCM_DCCS, 0x0005441A); // only use VPOST buffer
	
}


#if 0
void LCD_Delay(void)
{
   UINT8 volatile i;
   for(i=0;i<=10;i++);
}

static void SDOLevel(UINT8 level)
{
    UINT32 GPIOData;

    GPIOData = readw(REG_GPIOA_DAT);
    if(level)
    {
        GPIOData |=  0x0000010;			//SDO HIGH
    }
    else
    {
        GPIOData &= ~ 0x0000010;		//SDO LOW    
    }
    writew(REG_GPIOA_DAT,GPIOData);	
}

static void SDALevel(UINT8 level)
{
    UINT32 GPIOData;

    GPIOData = readw(REG_GPIOA_DAT);	
    if(level)
    {
        GPIOData |=  0x0000008;			//SDA HIGH
    }
    else
    {
        GPIOData &= ~ 0x0000008;		//SDA LOW    
    }
    writew(REG_GPIOA_DAT,GPIOData);		
}

static void SCKLevel(UINT8 level)
{
    UINT32 GPIOData;

    GPIOData = readw(REG_GPIOA_DAT);	
    if(level)
    {
        GPIOData |=  0x0000004;			//SCL HIGH
    }
    else
    {
        GPIOData &= ~ 0x0000004;		//SCL LOW    
    }
    writew(REG_GPIO_DAT,GPIOData);		
}

void Write_Tda025_Reg(UINT8 reg, UINT8 value)
{
    UINT8 i,TransReg;
    TransReg = reg << 2;
    TransReg &= ~0x02;		//Set Write
//	TransReg |= 0x01;		//Set HiZ		?? Tooply spec
    TransReg &= ~0x01;		//Set HiZ		?? Tooply spec

    SDOLevel(1);			//SDO->HIGH
    SDALevel(0);			//SDA->Low
    SCKLevel(0);			//SCK->Low 
    LCD_Delay();    

	SDOLevel(0);			//SEN->Low  Data setup star-up
	LCD_Delay();

    for(i=0; i<8; i++)		//fill Register Number
    {
		if(TransReg&0x80)
		{
		    SDALevel(1);	//Data->High 
		}
		else
		{
		    SDALevel(0);	//Data->Low     
		}
		LCD_Delay();
		SCKLevel(1);		//SCK->High 
		LCD_Delay();
		SCKLevel(0);		//SCK->Low 	
		TransReg <<= 1;
    }
	
    for(i=0; i<8; i++)		//fill Register Value
    {
		if(value&0x80)
		{
		    SDALevel(1);	//Data->High 
		}
		else
		{
		    SDALevel(0);	//Data->Low     
		}    
			LCD_Delay();
			SCKLevel(1);	//SCK->High 
			LCD_Delay();
			SCKLevel(0);	//SCK->Low 		
			value <<= 1;
    }	

    LCD_Delay();
    SDOLevel(1);			//SDO->HIGH	
    SDALevel(0);			//Data->Low	          
}

UINT8 Read_Tda025_Reg(UINT8 reg)
{
    UINT8 i,TransReg,value;
    TransReg = reg << 2;
    TransReg |= 0x02;		//Set Read
//	TransReg |= 0x01;		//Set HiZ		?? Tooply spec
    TransReg &= ~0x01;		//Set HiZ		?? Tooply spec
    
    SDOLevel(1);			//SDO->HIGH
    SDALevel(0);			//SDA->Low
    SCKLevel(0);			//SCK->Low 
    LCD_Delay();    

    SDOLevel(0);			//SEN->Low  Data setup star-up
    LCD_Delay();

    for(i=0; i<8; i++)		//fill Register Number
    {
		if(TransReg&0x80)
		{
		    SDALevel(1);	//Data->High 
		}
		else
		{
		    SDALevel(0);	//Data->Low     
		}
		LCD_Delay();
		SCKLevel(1);		//SCK->High 
		LCD_Delay();
		SCKLevel(0);		//SCK->Low 	
		TransReg <<= 1;
    }

    //change SDA direction to Input
    writew(REG_GPIOA_OE,readw(REG_GPIOA_OE)|(0x0000008));
	
    for(i=0; i<8; i++)		//fill Register Value
    {
		if(readw(REG_GPIOA_DAT)&(0x0000008))    
		{
		    value |= 0x01;
		}
		else
		{
		    value &= ~0x01;
		}    
		LCD_Delay();
		SCKLevel(1);		//SCK->High 
		LCD_Delay();
		SCKLevel(0);		//SCK->Low 		
		value <<= 1;
    }	

    LCD_Delay();
    SDOLevel(1);			//SDO->HIGH	
    
    //change SDA direction to Output
    writew(REG_GPIOA_OE,readw(REG_GPIOA_OE)&(~0x0000008));
	
    SDALevel(0);			//Data->Low	 

    return value;
}

static void InitialTda025LCM(void)
{
	UINT32 volatile tmp;
    //set I2C pins and LCS2 are GPIO pins, (LCS2#:Normal Operation), (SDO/SDA/SCK, GPIOA[4/3/2])
    tmp = readw(REG_PADC0);
    writew(REG_PADC0,readw(REG_PADC0)&(~0x0020000E));

#if 1
	//set GPIOA-04 is High
    outpw(REG_GPIOA_DAT, inpw(REG_GPIOA_DAT)|(0x00000010));	
   //set GPIOA-04 are Output Mode
    outpw(REG_GPIOA_OE, inpw(REG_GPIOA_OE)&(~0x00000010));

	//use 802_SCK/802_SDA, GPIO-01/GPIO-05
    outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT)|(0x00000022));	
    outpw(REG_GPIO_OE, inpw(REG_GPIO_OE)&(~0x00000022));
#else
   //set GPIOA02,GPIOA03 are Low ,GPIOA04 is High
    writew(REG_GPIOA_DAT,readw(REG_GPIOA_DAT)|(0x00000010));	
   //set GPIOA02,GPIOA03,GPIOA04 are Output Mode
    writew(REG_GPIOA_OE,readw(REG_GPIOA_OE)&(~0x0000001C));
#endif
	//set TOPPLY LCD SHDB pin is High
   //set GPIOB16 is High
    writew(REG_GPIOB_DAT,readw(REG_GPIOB_DAT)|(0x00010000));	
   //set GPIOB16 is Output Mode
    writew(REG_GPIOB_OE,readw(REG_GPIOB_OE)&(~0x00010000));
    //set GPIOB16 is High
    writew(REG_GPIOB_DAT,readw(REG_GPIOB_DAT)|(0x00010000));

    //Write_Tda025_Reg(0x02,0x00); // RGB Dummy(4/4/4/0),Valid(1440):0x08(Width:1280=>0x00)
    //Write_Tda025_Reg(0x02,0x01); // YUV422, 1280

   	Write_Tda025_Reg(0x02,0x01); // YUV422, 1280
    Write_Tda025_Reg(0x03,0x30);			
    Write_Tda025_Reg(0x04,0x0F);
    Write_Tda025_Reg(0x05,0x13);	
    Write_Tda025_Reg(0x06,0x18); // Shift-H, default(0x18)		   
    Write_Tda025_Reg(0x07,0x08); // Shift-V, default(0x08)	      
    Write_Tda025_Reg(0x08,0x00);	    
    Write_Tda025_Reg(0x09,0x20);	
    Write_Tda025_Reg(0x0A,0x20);	     
    Write_Tda025_Reg(0x0B,0x20);	       
    Write_Tda025_Reg(0x0C,0x10);      
    Write_Tda025_Reg(0x10,0x3A);
    Write_Tda025_Reg(0x11,0x3A);	
    Write_Tda025_Reg(0x12,0x1D);	   
    Write_Tda025_Reg(0x13,0x17);	  
    Write_Tda025_Reg(0x14,0x98);	  
    Write_Tda025_Reg(0x15,0x9A);		      
    Write_Tda025_Reg(0x16,0xA9);	       
    Write_Tda025_Reg(0x17,0x99);	
    Write_Tda025_Reg(0x18,0x08);
    
    writew(REG_PADC0,tmp);//clyu for collision with RTC
}

#endif