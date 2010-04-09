/****************************************************************************
 * 
 * FILENAME
 *     W99702_W56964.c
 *
 * VERSION
 *     0.1 
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
 *     2004.07.12		Created by Shih-Jen Lu
 *
 *
 * REMARK
 *     None
 *
 **************************************************************************/
#ifdef ECOS
#include "stdio.h"
#include "stdlib.h"
#include "drv_api.h"
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

#include "wbio.h"
#include "wblib.h"
#include "W99702_Audio_Regs.h"
#include "W99702_Audio.h"
#include "W56964.h"


#ifdef HAVE_W56964

#define SOFT_MODE

 


static AUDIO_T	_tW56964;
static INT 		_W56964_Playing = 0;
static UINT8	g_byInterruptMask;
static volatile INT	_bPlayDmaToggle =0;
static UINT8	g_byEQVol;
static UINT8 g_byHPLeftVol;
static UINT8 g_byHPRightVol;
static INT g_wS_FIFOSize = 512;
static INT g_wS_FIFOThreshold = 256;
static INT g_wP_FIFOSize = 256;
static INT g_wP_FIFOThreshold = 128;
static INT g_w_FIFOSize = 256;
static INT g_w_FIFOThreshold = 128;




typedef void (*AU_W56964_FUNC_T)();
static AU_W56964_FUNC_T _OLD_GPIO_ISR;


static void Delay(int nCnt)
{
	volatile int  loop;
	for (loop=0; loop<nCnt; loop++);
}

#ifdef SOFT_MODE
/*-----  software control functions  -----*/
static UINT8 soft_read_status_flag_reg()
{
	UINT8			data8;
	outpw(REG_ACTL_M80DATA0, (SOFT_CON_REQ | SOFT_CON_W | SOFT_CON_R | SOFT_CON_A1));//cs low
	Delay(1);
	outpw(REG_ACTL_M80DATA0, (SOFT_CON_REQ | SOFT_CON_W | SOFT_CON_R ));//A0 low
	Delay(1);
	outpw(REG_ACTL_M80DATA0, (SOFT_CON_REQ | SOFT_CON_W));//read low
	Delay(1);	
	data8 =	inpw(REG_ACTL_M80DATA0) & 0xff;
	Delay(1);
	outpw(REG_ACTL_M80DATA0, (SOFT_CON_REQ | SOFT_CON_W | SOFT_CON_R));//read high
	Delay(1);
	outpw(REG_ACTL_M80DATA0, (SOFT_CON_REQ | SOFT_CON_W | SOFT_CON_R | SOFT_CON_A1));//A0 high
	Delay(1);
	outpw(REG_ACTL_M80DATA0, (SOFT_CON_CS | SOFT_CON_REL | SOFT_CON_W | SOFT_CON_R));//cs high
	return data8;
}


static void soft_write_command_byte_reg(UINT8 data)
{

	outpw(REG_ACTL_M80DATA0, (SOFT_CON_REQ | SOFT_CON_W | SOFT_CON_R | SOFT_CON_A1 |data));//cs low
	Delay(1);	
	outpw(REG_ACTL_M80DATA0, (SOFT_CON_REQ | SOFT_CON_W | SOFT_CON_R |data));//A0 low
	Delay(1);
	outpw(REG_ACTL_M80DATA0, (SOFT_CON_REQ | SOFT_CON_R | data)  );//write low
	Delay(1);	
	outpw(REG_ACTL_M80DATA0, (SOFT_CON_REQ | SOFT_CON_R | SOFT_CON_W | data)  );//write high
	Delay(1);	
	outpw(REG_ACTL_M80DATA0, (SOFT_CON_REQ | SOFT_CON_R | SOFT_CON_W | SOFT_CON_A1| data)  );//A0 high
	Delay(1);	
	outpw(REG_ACTL_M80DATA0, (SOFT_CON_CS | SOFT_CON_REL | SOFT_CON_W | SOFT_CON_R | SOFT_CON_A1));//cs high
	
}


static UINT8 soft_read_data_reg()
{
	UINT8			data8;
	outpw(REG_ACTL_M80DATA0, (SOFT_CON_REQ | SOFT_CON_W | SOFT_CON_R));//cs low
	Delay(1);
	outpw(REG_ACTL_M80DATA0, (SOFT_CON_A1 | SOFT_CON_REQ | SOFT_CON_W | SOFT_CON_R));//A0 high
	Delay(1);
	outpw(REG_ACTL_M80DATA0, (SOFT_CON_A1 | SOFT_CON_REQ | SOFT_CON_W));//read low
	Delay(1);
	data8 = inpw(REG_ACTL_M80DATA0) & 0xff;
	Delay(1);	
	outpw(REG_ACTL_M80DATA0, (SOFT_CON_A1 | SOFT_CON_REQ | SOFT_CON_W | SOFT_CON_R));//read high
	Delay(1);	
	outpw(REG_ACTL_M80DATA0, (SOFT_CON_REQ | SOFT_CON_W | SOFT_CON_R));//A0 low
	Delay(1);
	outpw(REG_ACTL_M80DATA0, (SOFT_CON_CS | SOFT_CON_REL | SOFT_CON_W | SOFT_CON_R));//cs high
	Delay(1);	
	return data8;
}


static void soft_write_data_reg(UINT8 data)
{
	
	outpw(REG_ACTL_M80DATA0, (SOFT_CON_REQ | SOFT_CON_W | SOFT_CON_R | data));//cs low
	Delay(1);	
	outpw(REG_ACTL_M80DATA0, (SOFT_CON_REQ | SOFT_CON_W | SOFT_CON_R | SOFT_CON_A1 | data));//A0 high
	Delay(1);	
	outpw(REG_ACTL_M80DATA0, (SOFT_CON_A1 | SOFT_CON_REQ | SOFT_CON_R |  data));//write low
	Delay(1);	
	outpw(REG_ACTL_M80DATA0, (SOFT_CON_A1 | SOFT_CON_REQ | SOFT_CON_W| SOFT_CON_R | data));//write high
	Delay(1);	
	outpw(REG_ACTL_M80DATA0, (SOFT_CON_REQ | SOFT_CON_W| SOFT_CON_R | data));//A0 low
	Delay(1);
	outpw(REG_ACTL_M80DATA0, (SOFT_CON_CS | SOFT_CON_REL | SOFT_CON_W | SOFT_CON_R));//cs high
	Delay(1);	
}
/*-----  end of software control functions  -----*/
#endif
static INT read_intermediate_reg(INT nRegId)
{
	INT nTimeOutCount;
	outpw(REG_ACTL_M80ADDR, nRegId);
	outpw(REG_ACTL_M80CON, inpw(REG_ACTL_M80CON) | R_IF11_ACT);

	
	for(nTimeOutCount=0;nTimeOutCount<10000;nTimeOutCount++){
		if ((inpw(REG_ACTL_M80CON) & R_IF11_ACT) == 0 )
			break;
	}
	
	if (nTimeOutCount == 10000){
			_error_msg("read_intermediate_reg - M80 read intermediate register timeout\n");
			return ERR_M80_R_INTERMEDIATEREG_TIMEOUT;
	}

	return ((inpw(REG_ACTL_M80SIZE)>>16) & 0xff);
}

static INT write_intermediate_reg(INT nRegId, UINT8 data)
{
	INT nTimeOutCount;
	outpw(REG_ACTL_M80ADDR, nRegId);
	outpw(REG_ACTL_M80SIZE, 0x1);
	outpw(REG_ACTL_M80DATA0, data);
	outpw(REG_ACTL_M80CON, inpw(REG_ACTL_M80CON) | W_IF12_ACT);
	for(nTimeOutCount=0;nTimeOutCount<10000;nTimeOutCount++){
		if ((inpw(REG_ACTL_M80CON) & W_IF12_ACT) == 0 )
			break;
	}
	
	if (nTimeOutCount == 10000){
			_error_msg("write_intermediate_reg - M80 write intermediate register timeout\n");
			return ERR_M80_W_INTERMEDIATEREG_TIMEOUT;
	}
	return 0;
}


/*----------------disable interrupt-----------------------------------------*/
static void ComDrv_DisableInterrupt(void)
{
	if (g_byInterruptMask == 1)
	{
#ifdef SOFT_MODE
		soft_write_command_byte_reg(0x0);
#else
		write_intermediate_reg(0x0,0);
		
#endif
	}
		
}
/*--------------------------------------------------------------------------*/
/*--------------------- enable interrupt -----------------------------------*/
static void ComDrv_EnableInterrupt(void)
{
	if (g_byInterruptMask == 1)
	{
#ifdef SOFT_MODE
		//soft_write_command_byte_reg(CMD_COM_IRQE);		
		soft_write_command_byte_reg(0x80);
		//soft_write_data_reg(0x1f);
#else		
		write_intermediate_reg(0x80,0);
#endif
	}
}
/*--------------------------------------------------------------------------*/
/*------------------ send 1 byte command -----------------------------------*/
static short ComDrv_SendSimpleCommand(
	UINT8	byReg,
	UINT8	byData
	
	)
{
	
	/* select register and disable interrupt */
	ComDrv_DisableInterrupt();
	
#ifdef SOFT_MODE
	soft_write_command_byte_reg(byReg);
	soft_write_data_reg(byData);
#else
	write_intermediate_reg(byReg,byData);
#endif	
	/* enable interrupt */
	ComDrv_EnableInterrupt();
	return 0;
}
/*--------------------------------------------------------------------------*/
INT ComDrv_ClearFIFO(short nChannelID)
{
	switch(nChannelID)
	{
	case 0:	// S-FIFO
		return ComDrv_SendSimpleCommand(CMD_COM_ID_FIFO_RT_REG, FIFO_RT_REG_SFRT);
	case 1:	// P-FIFO
		return ComDrv_SendSimpleCommand(CMD_COM_ID_FIFO_RT_REG, FIFO_RT_REG_PFRT);
	default:
		return ERR_W56964_ERROR_ARGUMENT;
	}
}
/*--------------------------------------------------------------------------*/
/*----- Receive Data from W5691 -----*/
static UINT8 ComDrv_ReceiveSimpleData(
	UINT8	byReg
	)
{
	UINT8 byData;

	ComDrv_DisableInterrupt();
#ifdef SOFT_MODE
	soft_write_command_byte_reg(byReg);
	byData = soft_read_data_reg();
#else
	byData = read_intermediate_reg(byReg);
#endif
	/* enable interrupt */
	ComDrv_EnableInterrupt();
	return byData;
}/*----- END OF Receive Data from W5691 -----*/
/*----- send fifo data -----*/
static short ComDrv_SendFIFOData(
	UINT8		byFIFOReg,
	const UINT8*	pbyData,
	UINT16		wDataSize
	)
{


	/* select Sequencer FIFO and disable interrupt */
	ComDrv_DisableInterrupt();
#ifdef SOFT_MODE
	if (byFIFOReg == CMD_COM_ID_GEN_FIFO)
	{	/* wait for G-FIFO empty */
		while((soft_read_status_flag_reg() & CMD_STA_G_EMP) == 0x00);
#if 0		
		{
			int i ;
			for (i=0;i<wDataSize;i++){
				_debug_msg("0x%x | ",pbyData[i]);
			}
			_debug_msg("\n");
		}
#endif		
	}
	soft_write_command_byte_reg(byFIFOReg);	
#else
	outpw(REG_ACTL_M80ADDR,byFIFOReg);
#endif	
	
	
	
	if (byFIFOReg == CMD_COM_ID_GEN_FIFO) //for sync with w569's F/W
	{
#ifdef SOFT_MODE
		outpw(REG_ACTL_M80CON, inpw(REG_ACTL_M80CON) | W_GFIFO);
		soft_write_data_reg(0x55);
#endif
	}	
	
	//----- send one UINT8 data into FIFO -----//

#ifdef SOFT_MODE
		
		while(wDataSize > 0)
		{
			soft_write_data_reg(*pbyData++);
			wDataSize--;
		}
		
#else
	if (byFIFOReg == CMD_COM_ID_SPE_FIFO || byFIFOReg == CMD_COM_ID_SEQ_FIFO)
	{	
		
		outpw(REG_ACTL_PDSTB, (UINT) pbyData |  0x10000000);
		outpw(REG_ACTL_PDST_LENGTH,wDataSize);
		outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) | W5691_PLAY);
		outpw(REG_ACTL_M80SIZE,(wDataSize&0x7f) | ((wDataSize&0xFF80)<<1));//bothered by MA3, should shift left 1
		outpw(REG_ACTL_M80CON,inpw(REG_ACTL_M80CON) | W_IF13_ACT);
		while((inpw(REG_ACTL_M80CON) & W_IF13_ACT) != 0);
		outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) & ~W5691_PLAY );
	}
#endif
	
	if (byFIFOReg == CMD_COM_ID_GEN_FIFO)
	{
		ComDrv_SendSimpleCommand(CMD_COM_ID_WAKE_UP,WAKE_UP_REG_INT);
	}
	
	/* enable interrupt */
	ComDrv_EnableInterrupt();
	return 0;
}


/*----- volume down -----*/
static UINT8 ComDrv_VolumeDown(void)
{
	UINT8 byData;
	UINT8 byRet;

	/* Decrease speaker volume register to 00H one by one */
#ifdef SOFT_MODE
	soft_write_command_byte_reg(CMD_COM_ID_SPK_VOL_REG);
	byData = soft_read_data_reg();
#else	
	byData = read_intermediate_reg(CMD_COM_ID_SPK_VOL_REG);
#endif	
	byRet = byData & 0x1F;
	while((byData & 0x1F) > 0){
#ifdef SOFT_MODE
		soft_write_data_reg(--byData);
#else
		write_intermediate_reg(CMD_COM_ID_SPK_VOL_REG,--byData);
#endif
	}
	return byRet;
}
/*----- volume up -----*/
static void ComDrv_VolumeUp(UINT8 bySpeakerVolume)
{
	UINT8 byValue;

	/* Decrease speaker volume register to 00H one by one */
	
	for(byValue = 1; byValue <= bySpeakerVolume; byValue++)
	{
#ifdef SOFT_MODE
		soft_write_command_byte_reg(CMD_COM_ID_SPK_VOL_REG);
		soft_write_data_reg((SPK_VOL_REG_VDS << 6) | byValue);
#else		
		write_intermediate_reg(CMD_COM_ID_SPK_VOL_REG,(SPK_VOL_REG_VDS << 6) | byValue);
#endif
	}
}
INT ComDrv_SetFIFOSize(INT wS_FIFOSize, INT wP_FIFOSize, INT wS_FIFOThreshold, INT wP_FIFOThreshold)
{
	if (((wS_FIFOSize == 256) && (wP_FIFOSize == 256)) ||
		((wS_FIFOSize == 256) && (wP_FIFOSize == 384)) ||
		((wS_FIFOSize == 512) && (wP_FIFOSize == 0)))
	{
		g_wS_FIFOSize = wS_FIFOSize;
		g_wP_FIFOSize = wP_FIFOSize;
	}
	else
		return ERR_W56964_ERROR_ARGUMENT;

	switch (wS_FIFOSize)
	{
	case 256:
		if ((wS_FIFOThreshold == 32) ||
			(wS_FIFOThreshold == 64) ||
			(wS_FIFOThreshold == 96) ||
			(wS_FIFOThreshold == 128) ||
			(wS_FIFOThreshold == 160) ||
			(wS_FIFOThreshold == 192) ||
			(wS_FIFOThreshold == 224) )
			break;
		else
			return ERR_W56964_ERROR_ARGUMENT;
	case 512:
		if ((wS_FIFOThreshold == 64)  ||
			(wS_FIFOThreshold == 128) ||
			(wS_FIFOThreshold == 192) ||
			(wS_FIFOThreshold == 256) ||
			(wS_FIFOThreshold == 320) ||
			(wS_FIFOThreshold == 384) ||
			(wS_FIFOThreshold == 448) )
			break;
		else
			return ERR_W56964_ERROR_ARGUMENT;
	}

	switch (wP_FIFOSize)
	{
	case 0:
		if (wP_FIFOThreshold == 0)
			break;
		else
			return ERR_W56964_ERROR_ARGUMENT;
	case 256:
		if ((wP_FIFOThreshold == 32)  ||
			(wP_FIFOThreshold == 64)  ||
			(wP_FIFOThreshold == 96)  ||
			(wP_FIFOThreshold == 128) ||
			(wP_FIFOThreshold == 160) ||
			(wP_FIFOThreshold == 192) ||
			(wP_FIFOThreshold == 224) )
			break;
		else
			return ERR_W56964_ERROR_ARGUMENT;
	case 384:
		if ((wP_FIFOThreshold == 64)  ||
			(wP_FIFOThreshold == 128) ||
			(wP_FIFOThreshold == 192) ||
			(wP_FIFOThreshold == 256) ||
			(wP_FIFOThreshold == 320))
			break;
		else
			return ERR_W56964_ERROR_ARGUMENT;
	}

	g_wS_FIFOThreshold = wS_FIFOThreshold;
	g_wP_FIFOThreshold = wP_FIFOThreshold;
	return 0;
}
/*----- set melody FIFO interrupt trigger level -----*/
static short ComDrv_SetMelodyFIFOInterruptPoint(
	UINT8	byIntPoint	/* interrupt point */
	)
{
	UINT8	byRegValue;

	/* Select FIFO Status register and disable interrupt */
	ComDrv_DisableInterrupt();

	byRegValue = ComDrv_ReceiveSimpleData(CMD_COM_ID_FIFO_ST_REG);

	byRegValue = byRegValue & 0xf0;
	
	byRegValue = byRegValue | byIntPoint;


	ComDrv_SendSimpleCommand(CMD_COM_ID_FIFO_ST_REG,byRegValue);
	
		/* enable interrupt */
	ComDrv_EnableInterrupt();

	return 0;
}
/*----- Set P-FIFO interrupt trigger level -----*/
static short ComDrv_SetSpeechFIFOInterruptPoint(
	UINT8	byIntPoint	/* interrupt point */
	)
{
	UINT8	byRegValue;


	/* Select FIFO Status register and disable interrupt */
	ComDrv_DisableInterrupt();
	
	byRegValue = ComDrv_ReceiveSimpleData(CMD_COM_ID_FIFO_ST_REG);

	byRegValue = byRegValue & 0x0f;
	
	byRegValue = byRegValue | (byIntPoint<<4);

	
	ComDrv_SendSimpleCommand(CMD_COM_ID_FIFO_ST_REG,byRegValue);
	

	/* enable interrupt */
	ComDrv_EnableInterrupt();

	return 0;
}/*----- end of Set P-FIFO interrupt trigger level -----*/
INT ComDrv_SetFIFOInterruptPoint(short nChannelID)
{
	
	switch(nChannelID)
	{
	case 0:	// S-FIFO
		g_w_FIFOThreshold = g_wS_FIFOThreshold;
		g_w_FIFOSize = g_wS_FIFOSize;
		if (g_wS_FIFOSize == 512)
			return ComDrv_SetMelodyFIFOInterruptPoint(0x08 | (UINT8)(g_wS_FIFOThreshold >> 6));
		else
			return ComDrv_SetMelodyFIFOInterruptPoint((UINT8)(g_wS_FIFOThreshold >> 5));
	case 1:	// P-FIFO
		g_w_FIFOThreshold = g_wP_FIFOThreshold;
		g_w_FIFOSize = g_wP_FIFOSize;
		if (g_wP_FIFOSize == 384)
			return ComDrv_SetSpeechFIFOInterruptPoint(0x08 | (UINT8)(g_wP_FIFOThreshold >> 6));
		else
			return ComDrv_SetSpeechFIFOInterruptPoint((UINT8)(g_wP_FIFOThreshold >> 5));
	default:
		return ERR_W56964_ERROR_ARGUMENT;
	}
}



/*----- set interrupt mask -----*/ 
static void ComDrv_SetInterruptMask(UINT8 byMask)
{ 
	g_byInterruptMask = byMask; 
}/*----- end of set interrupt mask -----*/

/*----- set interrupt bits -----*/
static void ComDrv_EnableInterruptBits(UINT8 byBits)
{
	UINT8 byData;

	ComDrv_DisableInterrupt();
#ifdef SOFT_MODE
	soft_write_command_byte_reg(CMD_COM_ID_INT_EN_REG);
	byData = soft_read_data_reg();
	byData |= byBits;
	/* Set enable setting */
	soft_write_data_reg(byData);
#else	
	/* Get original enable setting */
	byData = read_intermediate_reg(CMD_COM_ID_INT_EN_REG);
	byData = byData | byBits;
	/* Set enable setting */
	write_intermediate_reg(CMD_COM_ID_INT_EN_REG,byData);
#endif
	/* enable interrupt */
	ComDrv_EnableInterrupt();
}/*----- end of set interrupt bits -----*/

/*----- enable headphone -----*/
void ComDrv_EnableHeadphoneOutput(void)
{

	UINT8 byData;
	UINT8 abyData[3] = { 0x03, 0x1E, 0x00};

	ComDrv_DisableInterrupt();
	byData = ComDrv_ReceiveSimpleData(CMD_COM_ID_HEADPHONE_VOL_L_REG);
    byData = byData & 0x3F;		//reset bit 6 & 7;
	byData |= (HEADPHONE_MONO << 7);
	
	ComDrv_SendSimpleCommand(CMD_COM_ID_HEADPHONE_VOL_L_REG,byData);

#ifdef SOFT_MODE
	ComDrv_SendFIFOData(CMD_COM_ID_GEN_FIFO, abyData, 3);
	while((soft_read_status_flag_reg() & CMD_STA_G_EMP) == 0x00);
	//while((soft_read_status_flag_reg() & CMD_STA_BUSY ) != 0x00)
#else
	outpw(REG_ACTL_M80SIZE, 3+1);
	outpw(REG_ACTL_M80DATA0, 0x55 | abyData[0]<<8 | (abyData[1]<<16) | (abyData[2]<<24) );
	outpw(REG_ACTL_M80ADDR, CMD_COM_ID_GEN_FIFO);
	//outpw(REG_ACTL_M80CON, 0x25210);
	outpw(REG_ACTL_M80CON, inpw(REG_ACTL_M80CON) | W_GFIFO | W_IF12_ACT);
	while((inpw(REG_ACTL_M80CON)&W_IF12_ACT) != 0);
	//outpw(REG_ACTL_M80CON, 0x5200);
	outpw(REG_ACTL_M80CON, inpw(REG_ACTL_M80CON) & ~W_GFIFO );
	ComDrv_SendSimpleCommand(CMD_COM_ID_WAKE_UP,WAKE_UP_REG_INT);	
#endif
	


	ComDrv_EnableInterrupt();


}
/*--------------------------------------------------------------------------*/
/*----- Power up -----*/
static INT ComDrv_PowerUp()
{
#ifdef SOFT_MODE
	INT nTimeOutCount;
#endif
	ComDrv_DisableInterrupt();
	
	
	/* Set AP2 to 0,  Set DP1 to 0 */
	ComDrv_SendSimpleCommand(CMD_COM_ID_CLK_CTRL, 0);
	
	/* Set DP0 to 0, wake up uC */
	ComDrv_SendSimpleCommand(CMD_COM_ID_WAKE_UP, 0);	
	
	/* Reset W5691 */
	ComDrv_SetInterruptMask(0);
//	Delay(0x100);
	ComDrv_SendSimpleCommand(CMD_COM_ID_FIFO_RT_REG, FIFO_RT_REG_RST);
#ifdef SOFT_MODE
	for (nTimeOutCount=0;nTimeOutCount<10000;nTimeOutCount++){
		if((soft_read_status_flag_reg() & CMD_STA_BUSY) == 0)
			break;
	}
	if (nTimeOutCount == 10000)
	{
		_error_msg("ComDrv_PowerUp - W56964 reset error\n");
		return ERR_W56964_RESET_TIMEOUT;
	}
#else
	Delay(50000);//wait for the busy bit;
#endif	

	ComDrv_EnableInterruptBits(IRQ_EN_REG_FIE0 | IRQ_EN_REG_FIE1 |
							   IRQ_EN_REG_FIE2 | IRQ_EN_REG_FIE3);
				   

	ComDrv_EnableHeadphoneOutput();

		

	
	
	

	return 0;
}/*----- end of Power up -----*/

/*----- Power up analog -----*/
static short ComDrv_PowerUpAnalog(
	UINT8	bySpeakerVolume	/* 0x00 ~ 0x1F */
	)
{
	ComDrv_DisableInterrupt();
	/* Make sure speaker volume register is 00H */
	
#ifdef SOFT_MODE
	soft_write_command_byte_reg(CMD_COM_ID_SPK_VOL_REG);
	soft_write_data_reg(SPK_VOL_REG_VDS << 6);
#else	
	write_intermediate_reg(CMD_COM_ID_SPK_VOL_REG, (SPK_VOL_REG_VDS << 6) | 0);
#endif

#ifdef SOFT_MODE
	soft_write_command_byte_reg(CMD_COM_ID_ANALOG_CTRL);
	soft_write_data_reg(ANALOG_CTRL_REG_AP1 | ANALOG_CTRL_REG_AP3 | 
						  ANALOG_CTRL_REG_AP4 | ANALOG_CTRL_REG_AP5 |
						  ANALOG_CTRL_REG_AP6);
#else	
	/* Set AP0 and AP1 to 0 */
	write_intermediate_reg(CMD_COM_ID_ANALOG_CTRL,ANALOG_CTRL_REG_AP1 | ANALOG_CTRL_REG_AP3 | 
						  ANALOG_CTRL_REG_AP4 | ANALOG_CTRL_REG_AP5 |
						  ANALOG_CTRL_REG_AP6);
#endif
	
	Delay(1000);//delay
	
	/* AP1 = 0 */
#ifdef SOFT_MODE
	soft_write_data_reg(0);
#else
	write_intermediate_reg(CMD_COM_ID_ANALOG_CTRL,0);
#endif
	/* Increment speaker volume register to wanted value one by one */
	ComDrv_VolumeUp(bySpeakerVolume);
	_tW56964.sPlayVolume = bySpeakerVolume;

	ComDrv_EnableInterrupt();
	return 0;
}
/*--------------------------------------------------------------------------*/
static INT ComDrv_PowerDown(void)
{
	INT nTimeOutCount;
	const UINT8 abyStopMode[2] = { 0x03, 0x17 };
	ComDrv_DisableInterrupt();
	
	ComDrv_VolumeDown();

	/* Set AP1 to 1 */
	ComDrv_SendSimpleCommand(CMD_COM_ID_ANALOG_CTRL,ANALOG_CTRL_REG_AP1);

	/* Set AP0 to 1 */
	ComDrv_SendSimpleCommand(CMD_COM_ID_ANALOG_CTRL, ANALOG_CTRL_REG_AP0 | ANALOG_CTRL_REG_AP1 |
							  ANALOG_CTRL_REG_AP3 | ANALOG_CTRL_REG_AP4 |
							  ANALOG_CTRL_REG_AP5 | ANALOG_CTRL_REG_AP6 |
							  ANALOG_CTRL_REG_AP7);


	/* Let uC enter STOP mode */
#ifdef SOFT_MODE
	ComDrv_SendFIFOData(CMD_COM_ID_GEN_FIFO, abyStopMode, 2);
#else
	outpw(REG_ACTL_M80SIZE, 2+1);
	outpw(REG_ACTL_M80DATA0, 0x55 | abyStopMode[0]<<8 | (abyStopMode[1]<<16));
	outpw(REG_ACTL_M80ADDR, CMD_COM_ID_GEN_FIFO);
	//outpw(REG_ACTL_M80CON, 0x25210);
	outpw(REG_ACTL_M80CON, inpw(REG_ACTL_M80CON) | W_GFIFO | W_IF12_ACT);
	while((inpw(REG_ACTL_M80CON)&W_IF12_ACT) != 0);
	//outpw(REG_ACTL_M80CON, 0x5200);
	outpw(REG_ACTL_M80CON, inpw(REG_ACTL_M80CON) & ~W_GFIFO );
	ComDrv_SendSimpleCommand(CMD_COM_ID_WAKE_UP,WAKE_UP_REG_INT);
#endif		


	// wait W5691 enter STOP mode
	for (nTimeOutCount = 0; nTimeOutCount < 10000; nTimeOutCount++){
		if ((ComDrv_ReceiveSimpleData(CMD_COM_ID_WAKE_UP) & WAKE_UP_REG_DP0) != 0) 
			break;
	}
	
	if (nTimeOutCount == 10000)
	{
		_error_msg("ComDrv_PowerDown - W56964 power down error\n");
		return ERR_W56964_STOP_TIMEOUT;
	}
	

	/* Set DP1 to 1, Set AP2 to 1 */
	ComDrv_SendSimpleCommand(CMD_COM_ID_CLK_CTRL,CLK_CTRL_REG_DP1 | CLK_CTRL_REG_AP2);

	ComDrv_EnableInterrupt();

	return 0;
}
/*----- clear interrupt bits -----*/
static void ComDrv_DisableInterruptBits(UINT8 byBits)
{
	UINT8 byData;
	ComDrv_DisableInterrupt();
#ifdef SOFT_MODE
	soft_write_command_byte_reg(CMD_COM_ID_INT_EN_REG);
	byData = soft_read_data_reg();
	byData &= (~byBits);
	/* Set enable setting */
	soft_write_data_reg(byData);
#else	
	byData = read_intermediate_reg(CMD_COM_ID_INT_EN_REG);
	byData &= (~byBits);
	/* Set enable setting */
	write_intermediate_reg(CMD_COM_ID_INT_EN_REG,byData);
#endif	
	/* enable interrupt */
	ComDrv_EnableInterrupt();
}/*----- end of clear interrupt bits -----*/

/*----- send command -----*/
static INT ComDrv_SendCommand(enum E_CommandType eCommand)
{
	const UINT8 abyCmdStartMelody[] = { 0x03, 0x02, 0x00 };
	const UINT8 abyCmdStopMelody[] = { 0x03, 0x04, 0x01 };
	const UINT8 abyCmdPauseMelody[] = { 0x03, 0x13, 0x01 };
	const UINT8 abyCmdPauseSpeech[] = { 0x03, 0x13, 0x02 };
	const UINT8 abyCmdResumeMelody[] = { 0x03, 0x14, 0x01 };
	const UINT8 abyCmdResumeSpeech[] = { 0x03, 0x14, 0x02 };
	const UINT8 abyCmdSpeechEOS[] = { 0x03, 0x0F, 0x02 }; 
	const UINT8 abyCmdStopSpeech[] = { 0x03, 0x04, 0x02 };

	//short nRet = 0;
	switch(eCommand)
	{
	case eCOMMAND_POWER_UP:
		return ComDrv_PowerUp();
	case eCOMMAND_POWER_DOWN:
		return ComDrv_PowerDown();
	case eCOMMAND_CLEAR_MELODY_BUFFER:
		return ComDrv_SendSimpleCommand(CMD_COM_ID_FIFO_RT_REG, FIFO_RT_REG_SFRT);
	case eCOMMAND_CLEAR_SPEECH_BUFFER:
		return ComDrv_SendSimpleCommand(CMD_COM_ID_FIFO_RT_REG, FIFO_RT_REG_PFRT);
	case eCOMMAND_SETUP_MELODY_INTERRUPT:
		return ComDrv_SetMelodyFIFOInterruptPoint(4);
	case eCOMMAND_SETUP_SPEECH_INTERRUPT:
		return ComDrv_SetSpeechFIFOInterruptPoint(4);
	case eCOMMAND_START_MELODY_SYNTHESIZER:
		return ComDrv_SendFIFOData(CMD_COM_ID_GEN_FIFO, abyCmdStartMelody, 
								   sizeof(abyCmdStartMelody));
	case eCOMMAND_STOP_MELODY_SYNTHESIZER:
		return ComDrv_SendFIFOData(CMD_COM_ID_GEN_FIFO, abyCmdStopMelody, 
								   sizeof(abyCmdStopMelody));
	case eCOMMAND_STOP_SPEECH_SYNTHESIZER:
		return ComDrv_SendFIFOData(CMD_COM_ID_GEN_FIFO, abyCmdStopSpeech, 
								   sizeof(abyCmdStopSpeech));
	case eCOMMAND_PAUSE_MELODY_SYNTHESIZER:
		ComDrv_DisableInterrupt();
		_tW56964.sPlayVolume = ComDrv_VolumeDown();
		ComDrv_EnableInterrupt();
		return ComDrv_SendFIFOData(CMD_COM_ID_GEN_FIFO, abyCmdPauseMelody, 
								   sizeof(abyCmdPauseMelody));
	case eCOMMAND_PAUSE_SPEECH_SYNTHESIZER:
		ComDrv_DisableInterrupt();
		_tW56964.sPlayVolume = ComDrv_VolumeDown();
		ComDrv_EnableInterrupt();
		return ComDrv_SendFIFOData(CMD_COM_ID_GEN_FIFO, abyCmdPauseSpeech, 
								   sizeof(abyCmdPauseSpeech));
	case eCOMMAND_RESUME_MELODY_SYNTHESIZER:
		ComDrv_DisableInterrupt();
		ComDrv_VolumeUp(_tW56964.sPlayVolume);
		ComDrv_EnableInterrupt();
		return ComDrv_SendFIFOData(CMD_COM_ID_GEN_FIFO, abyCmdResumeMelody, 
								   sizeof(abyCmdResumeMelody));
	case eCOMMAND_RESUME_SPEECH_SYNTHESIZER:
		ComDrv_DisableInterrupt();
		ComDrv_VolumeUp(_tW56964.sPlayVolume);
		ComDrv_EnableInterrupt();
		return ComDrv_SendFIFOData(CMD_COM_ID_GEN_FIFO, abyCmdResumeSpeech, 
								   sizeof(abyCmdResumeSpeech));
	case eCOMMAND_SPEECH_END_OF_SEQUENCE:

		return ComDrv_SendFIFOData(CMD_COM_ID_GEN_FIFO, abyCmdSpeechEOS, 
								   sizeof(abyCmdSpeechEOS));
	case eCOMMAND_ENABLE_MELODY_INTERRUPT:
		ComDrv_EnableInterruptBits(IRQ_EN_REG_SFIE);
		break;
	case eCOMMAND_ENABLE_SPEECH_INTERRUPT:
		
		ComDrv_EnableInterruptBits(IRQ_EN_REG_PFIE);
		
		break;
	case eCOMMAND_DISABLE_MELODY_INTERRUPT:
		ComDrv_DisableInterruptBits(IRQ_EN_REG_SFIE);
		break;
	case eCOMMAND_DISABLE_SPEECH_INTERRUPT:
		ComDrv_DisableInterruptBits(IRQ_EN_REG_PFIE);
		break;
	default:
		return ERR_W56964_ERROR_ARGUMENT;
	}
	return 0;
}/*----- end of send command -----*/

/*--------------------------------------------------------------------------*/
INT ComDrv_EnableFIFOInterrupt(short nChannelID)
{
	switch(nChannelID)
	{
	case 0:	// S-FIFO
		ComDrv_EnableInterruptBits(IRQ_EN_REG_SFIE);
		break;
	case 1:	// P-FIFO
		ComDrv_EnableInterruptBits(IRQ_EN_REG_PFIE);
		break;
	default:
		return ERR_W56964_ERROR_ARGUMENT;
	}
	return 0;
}
/*--------------------------------------------------------------------------*/
INT ComDrv_DisableFIFOInterrupt(short nChannelID)
{
	switch(nChannelID)
	{
	case 0:	// S-FIFO
		ComDrv_DisableInterruptBits(IRQ_EN_REG_SFIE);
		break;
	case 1:	// P-FIFO
		ComDrv_DisableInterruptBits(IRQ_EN_REG_PFIE);
		break;
	default:
		return ERR_W56964_ERROR_ARGUMENT;
	}
	return 0;
}
/*--------------------------------------------------------------------------*/
static INT ComDrv_SendCommandWithData(
	enum E_CommandType	eCommand,
	const UINT8*			pbyData,
	UINT16				wDataSize
	)
{
	UINT8 UINT8mp = 0;
	switch(eCommand)
	{
	case eCOMMAND_POWER_UP:
		return ComDrv_PowerUp();
	case eCOMMAND_POWER_UP_ANALOG:
		return ComDrv_PowerUpAnalog(*pbyData);
	case eCOMMAND_SEND_COMMAND_DATA:
		return ComDrv_SendFIFOData(CMD_COM_ID_GEN_FIFO, pbyData, wDataSize);
	case eCOMMAND_SEND_MELODY_DATA:
		return ComDrv_SendFIFOData(CMD_COM_ID_SEQ_FIFO, pbyData, wDataSize);
	case eCOMMAND_SEND_SPEECH_DATA:
		return ComDrv_SendFIFOData(CMD_COM_ID_SPE_FIFO, pbyData, wDataSize);
	case eCOMMAND_SET_EQ_VOLUME:
		return ComDrv_SendSimpleCommand(CMD_COM_ID_SPK_VOL_REG, 
										((UINT8)SPK_VOL_REG_VDS << 6) | *pbyData);
	case eCOMMAND_SET_LED_ON_OFF_IN_POWER_DOWN_MODE:
		UINT8mp = ComDrv_ReceiveSimpleData(CMD_COM_ID_OP_CTRL) & 0x01;
		UINT8mp |= ((~(*pbyData)) << 1);  //ctwu1 - active high
		//UINT8mp |= ((*pbyData) << 1);  // - active low
		return ComDrv_SendSimpleCommand(CMD_COM_ID_OP_CTRL, UINT8mp);
	case eCOMMAND_SET_MOTOR_ON_OFF_IN_POWER_DOWN_MODE:
		//UINT8mp = ComDrv_ReceiveSimpleData(CMD_COM_ID_OP_CTRL) & 0x0E;
		UINT8mp |= (~(*pbyData)); //ctwu1 - active high
		//UINT8mp |= ((*pbyData) << 1);  //active low
		return ComDrv_SendSimpleCommand(CMD_COM_ID_OP_CTRL, UINT8mp);
	}
	return ERR_W56964_ERROR_ARGUMENT;
}
//-------------------------------------------------------------------------
INT DevDrv_SendFIFOChannelData(
	short		nChannelID,
	UINT8*	pbyData,
	UINT16		wDataSize
	)
{
	switch(nChannelID)
	{
	case 0:
		return ComDrv_SendFIFOData(CMD_COM_ID_SEQ_FIFO, pbyData, wDataSize);
	case 1:
		return ComDrv_SendFIFOData(CMD_COM_ID_SPE_FIFO, pbyData, wDataSize);
	default:
		return ERR_W56964_ERROR_ARGUMENT;
	}
}
/*----- send PCM data -----*/
static short SendSpeechDataISR(SHORT channelID)
{
	if(_bPlayDmaToggle==0)
	{
		_bPlayDmaToggle=1;
		_tW56964.bPlayLastBlock = _tW56964.fnPlayCallBack((UINT8 *)_uAuPlayBuffAddr,
										g_w_FIFOThreshold);
		DevDrv_SendFIFOChannelData(channelID,(UINT8 *)_uAuPlayBuffAddr,g_w_FIFOThreshold);
								   
	}
	else{
		_bPlayDmaToggle=0;
		_tW56964.bPlayLastBlock = _tW56964.fnPlayCallBack((UINT8 *)(_uAuPlayBuffAddr + g_w_FIFOThreshold), 
									g_w_FIFOSize - g_w_FIFOThreshold);
		DevDrv_SendFIFOChannelData(channelID,(UINT8 *)(_uAuPlayBuffAddr + g_w_FIFOThreshold),g_w_FIFOSize - g_w_FIFOThreshold);
	}
	
	
	return 1;
}/*----- end of send PCM data -----*/
/*--------------------------------------------------------------------------*/
static short InterruptSequencerFIFO()
{
	return SendSpeechDataISR(0);
}
/*--------------------------------------------------------------------------*/

/*----- SEND SPEECH DATA ISR -----*/
static short InterruptSpeechFIFO()
{	
	return SendSpeechDataISR(1);
}/*----- END OF SEND SPEECH DATA ISR -----*/
/*----- receive data -----*/
static INT ComDrv_ReceiveData(
	enum E_CommandType	eCommand,
	UINT8*				pbyData
	)
{
	switch(eCommand)
	{
	case eCOMMAND_GET_INTERRUPT_STATUS:
		*pbyData = ComDrv_ReceiveSimpleData(CMD_COM_ID_INT_ST_REG);
		break;
	case eCOMMAND_GET_INTERRUPT_ENABLE:
		*pbyData = ComDrv_ReceiveSimpleData(CMD_COM_ID_INT_EN_REG);
		break;
	case eCOMMAND_GET_FIFO_STATUS:
		*pbyData = ComDrv_ReceiveSimpleData(CMD_COM_ID_FIFO_STAT_REG);
		break;
	case eCOMMAND_GET_X_BUFFER_DATA:
#ifdef SOFT_MODE
		while((soft_read_status_flag_reg() & CMD_STA_X_RDY) == 0);
#else
		Delay(10);		
#endif
	case eCOMMAND_GET_X_BUFFER_DATA_NO_WAIT:
		ComDrv_DisableInterrupt();
#ifdef SOFT_MODE
		soft_write_command_byte_reg(CMD_COM_ID_X_BUF);
		*pbyData = soft_read_data_reg();
#else
		*pbyData = read_intermediate_reg(CMD_COM_ID_X_BUF);
#endif		
		ComDrv_EnableInterrupt();
		break;
	case eCOMMAND_GET_Y_BUFFER_DATA:
#ifdef SOFT_MODE
		while((soft_read_status_flag_reg() & CMD_STA_Y_RDY) == 0);
#else
		Delay(1000);
#endif		
	case eCOMMAND_GET_Y_BUFFER_DATA_NO_WAIT:
		ComDrv_DisableInterrupt();
#ifdef SOFT_MODE
		soft_write_command_byte_reg(CMD_COM_ID_Y_BUF);
		*pbyData = soft_read_data_reg();
#else
		*pbyData = read_intermediate_reg(CMD_COM_ID_Y_BUF);
#endif
		ComDrv_EnableInterrupt();
		break;
	default:
		return ERR_W56964_ERROR_ARGUMENT;
	}
	return 0;
}/*----- end of receive data -----*/

/*----- INTERRUPT FIRMWARE -----*/
static short InterruptFirmware1()
{
	UINT8  byData = 0;




	byData = ComDrv_ReceiveSimpleData(CMD_COM_ID_X_BUF);
	

	if (byData & 0x80)
	{	// Notify for the sound being audible or not
		switch(byData)
		{
		case 0x81:	// audible
			_debug_msg("---> Sound is audible <----\n");
			Delay(10);
			break;
		case 0x82:	// silence

			break;
		}
	}else {

	}
		

	return 0;
}/*----- END OF INTERRUPT FIRMWARE -----*/
/*----- INTERRUPT HANDLER -----*/
static short InterruptHandlerInternal(void)
{
	const UINT8	abyIntType[8] =
	{
		IRQ_STA_REG_FIS0,		/* F/W interrupt #0 */
		IRQ_STA_REG_FIS1,		/* F/W interrupt #1 */
		IRQ_STA_REG_TBIS,		/* Timer B */
		IRQ_STA_REG_SFIS,		/* Sequencer FIFO */
		IRQ_STA_REG_FIS2,		/* F/W interrupt #2 */
		IRQ_STA_REG_FIS3,		/* F/W interrupt #3 */
		IRQ_STA_REG_TAIS,		/* Timer A */
		IRQ_STA_REG_PFIS		/* Speech FIFO */
	};
	
	UINT8 byStatus = 0;
	UINT8 byEnable = 0;
	UINT8 byTemp = 0;
	UINT8 byFIFOStatus = 0;
	short nRet = 0;
	UINT8 i;

	nRet = ComDrv_ReceiveData(eCOMMAND_GET_INTERRUPT_STATUS, &byStatus);
	if (nRet != 0)
		return nRet;


	nRet = ComDrv_ReceiveData(eCOMMAND_GET_INTERRUPT_ENABLE, &byEnable);
	if (nRet != 0)
		return nRet;
		


	if ((byStatus & byEnable) == 0x00)
	{
		ComDrv_SendSimpleCommand(CMD_COM_ID_INT_ST_REG, byStatus);
		return 0;
	}
	byStatus &= byEnable;

	for(i = 0; i <8 ; i++)
	{
		byTemp = abyIntType[i];
		if (byStatus & byTemp)
		{
			switch(byTemp)
			{
			case IRQ_STA_REG_SFIS:
				while(1)
				{
					ComDrv_SendSimpleCommand(CMD_COM_ID_INT_ST_REG, 
											 IRQ_STA_REG_SFIS);
											 
					byFIFOStatus = 0;
					ComDrv_ReceiveData(eCOMMAND_GET_FIFO_STATUS, 
									   &byFIFOStatus);
					if ((byFIFOStatus & FIFO_STAT_REG_SLET) == 0)
						break;
						
					if ((nRet = InterruptSequencerFIFO()) < 0)
						return nRet;
					
					ComDrv_ReceiveData(eCOMMAND_GET_INTERRUPT_ENABLE,
									   &byEnable);
									   
					if ((byEnable & IRQ_EN_REG_SFIE) == 0)
						break;
				}
				return 0;
			case IRQ_STA_REG_PFIS:
				while(1)
				{
					ComDrv_SendSimpleCommand(CMD_COM_ID_INT_ST_REG, 
												  IRQ_STA_REG_PFIS);

					byFIFOStatus = 0;
					ComDrv_ReceiveData(eCOMMAND_GET_FIFO_STATUS, 
									   &byFIFOStatus);
					if ((byFIFOStatus & FIFO_STAT_REG_PLET) == 0)
						break;
					
					

					if ((nRet = InterruptSpeechFIFO()) < 0)
						return nRet;

					ComDrv_ReceiveData(eCOMMAND_GET_INTERRUPT_ENABLE,
									   &byEnable);
					if ((byEnable & IRQ_EN_REG_PFIE) == 0)
						break;
				}
				return 0;

			case IRQ_STA_REG_FIS1:
				{
					UINT8 byData = 0;
					
					

					//Delay(10000);
					byData=ComDrv_ReceiveSimpleData(CMD_COM_ID_X_BUF);				   



					if (byData != 0x55){

						return 0;
					}
						
				}
				break;
			}

			ComDrv_SendSimpleCommand(CMD_COM_ID_INT_ST_REG, byTemp);
			switch(byTemp)
			{

			case IRQ_STA_REG_FIS1:		/* F/W interrupt #1 */
				nRet = InterruptFirmware1();
				break;
			case IRQ_STA_REG_PFIS:		/* Speech FIFO */
				nRet = InterruptSpeechFIFO();
				break;
			}
			if (nRet < 0)
				return nRet;
		}
	}

	return 0;
}

int counter =0 ;
#ifdef ECOS
cyg_uint32  w56964_dma_isr(cyg_vector_t vector, cyg_addrword_t data)
#else
static void  w56964_dma_isr()
#endif
{
	UINT8	byStatus;
#if 1

	if (inpw(REG_GPIO_IS) & (1<<USED_GPIO_NUM)){	//GPIO15
		if ((inpw(REG_GPIO_STS) & (1<<USED_GPIO_NUM))==0){
			ComDrv_DisableInterrupt();
			ComDrv_SetInterruptMask(0);
		
			InterruptHandlerInternal();
		//ComDrv_ReceiveData(eCOMMAND_GET_INTERRUPT_STATUS, &byStatus);
		//_debug_msg("a 0x%x, 0x%x\n",soft_read_status_flag_reg(),byStatus);
		//_debug_msg("%d\n",counter++);
			counter++;
//		_debug_msg("low\n");
			outpw(REG_GPIO_IS, 1<<USED_GPIO_NUM);
			ComDrv_SetInterruptMask(1);
			ComDrv_EnableInterrupt();
			
		}
		else{
			ComDrv_ReceiveData(eCOMMAND_GET_INTERRUPT_STATUS, &byStatus);
			outpw(REG_GPIO_IS, 1<<USED_GPIO_NUM);
		}
		//_debug_msg("high\n");
	
		
		
		//ComDrv_ReceiveData(eCOMMAND_GET_INTERRUPT_STATUS, &byStatus);
		//_debug_msg("c 0x%x, 0x%x\n",soft_read_status_flag_reg(),byStatus);
		//_debug_msg("b 0x%x \n",soft_read_status_flag_reg());
	}
	else
		(*_OLD_GPIO_ISR)();
#endif	
#if 0
	ComDrv_DisableInterrupt();
	ComDrv_SetInterruptMask(0);
	InterruptHandlerInternal();
	ComDrv_SetInterruptMask(1);
	ComDrv_EnableInterrupt();
#endif	
	return ;
}

static short DevDrv_PowerUp()
{ 
	short nRet = 0;
	
	nRet = ComDrv_SendCommand(eCOMMAND_POWER_UP);
	
	return nRet;
}
static short DevDrv_PowerDown(void)
{ 
	short nRet = 0;
	nRet = ComDrv_SendCommand(eCOMMAND_POWER_DOWN);
	
	return nRet;
}
/*----- set headphone left volume -----*/
void ComDrv_SetHeadphoneLeftVol(UINT8 byVol)
{

	if (byVol > 0x1F)	//only 5 bits are valid : 0 ~ 1F
		byVol = 0x1F;

	g_byHPLeftVol = byVol;
	#if 0
	byData = ComDrv_ReceiveSimpleData(CMD_COM_ID_HEADPHONE_VOL_L_REG);
	byData = byData & 0xE0;
	byData = byData | byVol;
	#endif
	ComDrv_SendSimpleCommand(CMD_COM_ID_HEADPHONE_VOL_L_REG,byVol);
	ComDrv_EnableInterrupt();

}
/*----- set headphone right volume -----*/
void ComDrv_SetHeadphoneRightVol(UINT8 byVol)
{
	UINT8 byData;
	if (byVol > 0x1F)	//only 5 bits are valid : 0 ~ 1F
		byVol = 0x1F;
	g_byHPRightVol = byVol;
	byData = ComDrv_ReceiveSimpleData(CMD_COM_ID_HEADPHONE_VOL_R_REG);
	byData = byData & 0xE0;
	byData = byData | byVol;
	ComDrv_SendSimpleCommand(CMD_COM_ID_HEADPHONE_VOL_R_REG,byData);

}
/*--------------------------------------------------------------------------*/
/*----- SET EQ VOLUME -----*/
short ComDrv_SetEQVol(UINT8 byVol)
{

	UINT8 byData;
	if (byVol > 0x1F)	//only 5 bits are valid : 0 ~ 1F
		byVol = 0x1F;
		

	g_byEQVol=byVol;
	byData = ComDrv_ReceiveSimpleData(CMD_COM_ID_EQ_VOL_REG);
	byData = byData & 0xE0;
	byData = byData | byVol;
	ComDrv_SendSimpleCommand(CMD_COM_ID_EQ_VOL_REG, byData);
	

	return 0;
}/*----- END OF SET EQ VOLUME -----*/

/*----- INITIALIZE W56964 -----*/
static INT ComDrv_Initialize(void)
{
#ifdef SOFT_MODE
	INT nTimeOutCount;
#endif
	g_byInterruptMask = 1;
	ComDrv_DisableInterrupt();
	
	ComDrv_SendSimpleCommand(CMD_COM_ID_PLL_ST_REG_I, W56964_PLL_ADJUST_N);
	ComDrv_SendSimpleCommand(CMD_COM_ID_PLL_ST_REG_II, W56964_PLL_ADJUST_M |0x80);
	
	if(ComDrv_ReceiveSimpleData(CMD_COM_ID_PLL_ST_REG_I) != (W56964_PLL_ADJUST_N)
		|| ComDrv_ReceiveSimpleData(CMD_COM_ID_PLL_ST_REG_II) != (W56964_PLL_ADJUST_M | 0x80))
	{
		_error_msg("W56964 register R/W error");
		return ERR_W56964_ERROR_REGRW;
	}
	
		

	
	//ComDrv_SetInterruptMask(0);
	/* Set AP2 to 0,  Set DP1 to 0 */
	ComDrv_SendSimpleCommand(CMD_COM_ID_CLK_CTRL,0);

	/* Set DP0 to 0, wake up uC */
	ComDrv_SendSimpleCommand(CMD_COM_ID_WAKE_UP,0);
	
	/* Reset W56964 */
	ComDrv_SendSimpleCommand(CMD_COM_ID_FIFO_RT_REG, FIFO_RT_REG_RST);
	
#ifdef SOFT_MODE
	
	for (nTimeOutCount=0;nTimeOutCount < 10000;nTimeOutCount++){
		if((soft_read_status_flag_reg() & CMD_STA_BUSY) == 0)
			break;
	}
	if (nTimeOutCount == 10000)
	{
		_error_msg("ComDrv_Initialize - W56964 reset error\n");
		return ERR_W56964_RESET_TIMEOUT;
	}
#else
	Delay(50000);//wait for the busy bit;
#endif


	ComDrv_PowerDown();
	
	/* Set DP0 to 1 */
	ComDrv_SendSimpleCommand(CMD_COM_ID_WAKE_UP,WAKE_UP_REG_DP0);
	/* Set AP2 to 1,  Set DP1 to 1 */
	ComDrv_SendSimpleCommand(CMD_COM_ID_WAKE_UP,CLK_CTRL_REG_DP1|CLK_CTRL_REG_AP2);
	

	ComDrv_SendSimpleCommand(CMD_COM_ID_PLL_ST_REG_I, W56964_PLL_ADJUST_N | 0x80);
	ComDrv_SendSimpleCommand(CMD_COM_ID_PLL_ST_REG_II, W56964_PLL_ADJUST_M | 0x80);

	

	
	g_byEQVol = 0x1F;
	g_byHPLeftVol = 0x1F;
	g_byHPRightVol = 0x1F;
	
	ComDrv_EnableInterrupt();

	return 0;
}/*----- END OF COMDRV INITIALIZE -----*/
/* DevDrv_Initialize*/
static short DevDrv_Initialize(void)
{

	short nRet = 0;

	nRet = ComDrv_Initialize();
	if (nRet < 0)
		return nRet;
	
	DevDrv_PowerUp();
	DevDrv_PowerDown();
	return 0;
}
static void Initialize(void)
{
	_tW56964.sPlayVolume = 0x1F;
	DevDrv_Initialize();
	
}

/*----- power up analog -----*/
static short DevDrv_PowerUpAnalog(UINT8 byEqualizerVolume)
{ 
	UINT8 bySpeakerVolume = byEqualizerVolume;
	return ComDrv_SendCommandWithData(eCOMMAND_POWER_UP_ANALOG, &bySpeakerVolume, 1); 
}/*----- end of power up analog -----*/


/*----- initialize audio interface and w5691 -----*/
static INT  w56964_init()
{
	

	/* reset W569X */

	outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & ~0x100000);	/* GPIO 20 */
	outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) | 0x100000);
	Delay(0x1000);
	outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) & ~0x100000);
	Delay(0x1000);
	outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) | 0x100000);

	
	/* enable audio controller interface */

	outpw(REG_ACTL_CON, inpw(REG_ACTL_CON) | AUDIO_EN | AUDCLK_EN | M80_EN | PFIFO_EN | RFIFO_EN | T_DMA_IRQ | R_DMA_IRQ | DMA_EN);			 
	
	Delay(100);
	
	/* reset Audio Controller */
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) | ACTL_RESET_BIT);
	Delay(100);
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) & ~ACTL_RESET_BIT );
	Delay(100);
	

	//reset M80 Interface 
	outpw(REG_ACTL_RESET,inpw(REG_ACTL_RESET) | M80_RESET );
	outpw(REG_ACTL_RESET,inpw(REG_ACTL_RESET) & ~M80_RESET);
	//outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) | PLAY_RIGHT_CHNNEL | PLAY_LEFT_CHNNEL);
	
	

	
	outpw(REG_ACTL_M80CON,CLK_DIV | MA3_W5691);//select w5691
	
	
	
	//enable audio clock, play FIFO, DMA, M80 and Audio controller
	
	outpw(REG_ACTL_CON,inpw(REG_ACTL_CON) | AUDIO_EN |PFIFO_EN |
			DMA_EN | AUDCLK_EN | M80_EN );
	

	//outpw(REG_ACTL_CON,0x9);
#ifdef	SOFT_MODE
	//software control mode
	outpw(REG_ACTL_M80CON, inpw(REG_ACTL_M80CON) | SOFT_CON);
	outpw(REG_ACTL_M80DATA0, (SOFT_CON_CS | SOFT_CON_REL | SOFT_CON_W | SOFT_CON_R));
#endif		


	Initialize();// set PLL  ComDrv_SendSimpleCommand

	
	
	DevDrv_PowerUp();
	


	DevDrv_PowerUpAnalog(0x1f);
	ComDrv_SetHeadphoneLeftVol(0x1f);
	ComDrv_SetHeadphoneRightVol(0x1f);
	
	
	
	
	
	return 0;
}/*----- end of initialize audio interface and w5691 -----*/


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      w5691StartPlay                                                    */
/*                                                                       */
/* DESCRIPTION                                                           */
/*      Start W5691 playback.                                             */
/*                                                                       */
/* INPUTS                                                                */
/*      fnCallBack     client program provided callback function. The audio */
/*                  driver will call back to get next block of PCM data  */
/*      nSamplingRate  the playback sampling rate. Supported sampling    */
/*                  rate are 48000, 44100, 32000, 24000, 22050, 16000,   */
/*                  11025, and 8000 Hz                                   */
/*      nChannels	number of playback nChannels                          */
/*					1: single channel, otherwise: double nChannels        */
/*                                                                       */
/* OUTPUTS                                                               */
/*      None                                                             */
/*                                                                       */
/* RETURN                                                                */
/*      0           Success                                              */
/*		Otherwise	error												 */
/*                                                                       */
/*************************************************************************/
INT  w56964StartPlay(AU_CB_FUNC_T *fnCallBack, INT nSamplingRate, 
						INT nChannels, INT pcm_format)
{


	UINT16 wDFA = 0;
	UINT8	byTemp;
	SHORT ChannelID = 1;//0:S-FIFO; 1:P-FIFO


	
	UINT8	eFormat = eSPEECH_FORMAT_SIGN_16BIT_PCM;
	
	INT 	nStatus;
	UINT8 byStartSpchCmd[8] = { 0x03, 0x03, 0x00, 0x00, 0x00, 0x00 , 0x7f, 0x40 };//GFIFO start play command
	
	if (_W56964_Playing)
		return ERR_W56964_PLAY_ACTIVE;
	
	
		
	if (_W56964_Playing == 0)
	{
		memset((CHAR *)&_tW56964, 0, sizeof(_tW56964));
		nStatus = w56964_init();
		if (nStatus < 0)
			return nStatus;	
	}
	

	
#ifdef ECOS
	cyg_interrupt_disable();
	sysSetInterruptType(GPIO_INT_NUM, LOW_LEVEL_SENSITIVE);
	cyg_interrupt_create(GPIO_INT_NUM, 1, 0, w56964_dma_isr, NULL, 
					&_tW56964.int_handle_play, &_tW56964.int_holder_play);
	cyg_interrupt_attach(_tW56964.int_handle_play);
	cyg_interrupt_unmask(AU_PLAY_INT_NUM);
	cyg_interrupt_enable();
#else	
#if 1
	//sysSetInterruptType(GPIO_INT_NUM, LOW_LEVEL_SENSITIVE);
	/* Install ISR */
	_OLD_GPIO_ISR = (AU_W56964_FUNC_T)sysInstallISR(IRQ_LEVEL_1, IRQ_GPIO, (PVOID)w56964_dma_isr);
    /* enable CPSR I bit */
    sysSetLocalInterrupt(ENABLE_IRQ);
	/* Set AIC into SW mode */
	//sysSetAIC2SWMode();
	sysEnableInterrupt(IRQ_GPIO);
#endif
#endif

	outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) | (1<<USED_GPIO_NUM));
	outpw(REG_GPIO_IE, inpw(REG_GPIO_IE) | (1<<USED_GPIO_NUM));
	
	_tW56964.nPlaySamplingRate = nSamplingRate;
	_tW56964.fnPlayCallBack = fnCallBack;
	

	wDFA = (UINT16)((float)_tW56964.nPlaySamplingRate / ALLOWED_SAMPLE_RATE * 0x400);
	wDFA >>= 1;//for 32 polyphony
	if (nChannels==1)
		byStartSpchCmd[2] = (UINT8)eFormat;
	else
		byStartSpchCmd[2] = 0x09 | eSPEECH_MS_STEREO;
		
	if (ChannelID==0){
		byStartSpchCmd[2] |= eSPEECH_FCH_SFIFO;
	}
	byStartSpchCmd[3] = (UINT8)(wDFA & 0xFF);
	byStartSpchCmd[4] = (UINT8)(wDFA >> 8);
	byStartSpchCmd[5] = 0;

	if ((inpw(REG_GPIO_STS) & (1<<USED_GPIO_NUM))==0){
		_debug_msg("low\n");
	}else{
		_debug_msg("high\n");
	}
	
	
#ifdef SOFT_MODE
	ComDrv_SendFIFOData(CMD_COM_ID_GEN_FIFO,byStartSpchCmd,sizeof(byStartSpchCmd));
	
#else
	outpw(REG_ACTL_M80SIZE, sizeof(byStartSpchCmd)+1);
	outpw(REG_ACTL_M80DATA0, 0x55 | byStartSpchCmd[0]<<8 | (byStartSpchCmd[1]<<16) | (byStartSpchCmd[2]<<24) );
	outpw(REG_ACTL_M80DATA1, byStartSpchCmd[3] | (byStartSpchCmd[4]<<8) | (byStartSpchCmd[5]<<16) | (byStartSpchCmd[6]<<24));
	outpw(REG_ACTL_M80DATA2, byStartSpchCmd[7]);
	outpw(REG_ACTL_M80ADDR, CMD_COM_ID_GEN_FIFO);
	outpw(REG_ACTL_M80CON, inpw(REG_ACTL_M80CON) | W_GFIFO | W_IF12_ACT);
	while((inpw(REG_ACTL_M80CON)&W_IF12_ACT) != 0);
	outpw(REG_ACTL_M80CON, inpw(REG_ACTL_M80CON) & ~W_GFIFO );
	ComDrv_SendSimpleCommand(CMD_COM_ID_WAKE_UP,WAKE_UP_REG_INT);	
#endif
		
	ComDrv_ReceiveData(eCOMMAND_GET_Y_BUFFER_DATA, &byTemp);
	ComDrv_SetFIFOSize(256,384,128,192);
	ComDrv_SetFIFOInterruptPoint(ChannelID);
	
	ComDrv_DisableFIFOInterrupt(ChannelID);
	
	ComDrv_ClearFIFO(ChannelID);
	
	
	/* call back to fill DMA buffer*/
	_tW56964.bPlayLastBlock = _tW56964.fnPlayCallBack((UINT8 *)_uAuPlayBuffAddr,
										g_w_FIFOSize);
	DevDrv_SendFIFOChannelData(ChannelID,(UINT8 *)_uAuPlayBuffAddr,g_w_FIFOSize);
	
	ComDrv_EnableFIFOInterrupt(ChannelID);

	ComDrv_ReceiveData(eCOMMAND_GET_Y_BUFFER_DATA, &byTemp);

//	while((soft_read_status_flag_reg() & CMD_STA_G_EMP) == 0x00);

	Delay(0x5000);
	

	_W56964_Playing=1;
	ComDrv_SetInterruptMask(1);

	ComDrv_EnableInterrupt();
	
	//Delay(0x5000);
	//ComDrv_ReceiveData(eCOMMAND_GET_Y_BUFFER_DATA, &byTemp);
	#if 1

	#endif
//	_debug_msg("0x%x\n",soft_read_status_flag_reg());
	
	//_debug_msg("status:0x%x\n",ComDrv_ReceiveSimpleData(0x04));

{	
	
	int i;

	for (i=0;i<0x11;i++){
		_debug_msg("reg %d = 0x%x\n",i,ComDrv_ReceiveSimpleData(i));
	}
}
	

	while(1)
	{
		//_debug_msg("%d\n",counter);
#if 0
		//_debug_msg("0x%x\n",soft_read_status_flag_reg());
	if (ChannelID){
		//_debug_msg("0x%x\n",ComDrv_ReceiveSimpleData(CMD_COM_ID_INT_ST_REG));
		if ((ComDrv_ReceiveSimpleData(CMD_COM_ID_INT_ST_REG) & IRQ_STA_REG_PFIS) != 0){
			w56964_dma_isr();
		}
	}
	else
	{
		if ((ComDrv_ReceiveSimpleData(CMD_COM_ID_INT_ST_REG) & IRQ_STA_REG_SFIS) != 0){
			w56964_dma_isr();
		}
	}
#endif	
		

	}

	return 0;
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      w56964StopPlay                                                    */
/*                                                                       */
/* DESCRIPTION                                                           */
/*      Stop W569 playback immdediately.                                 */
/*                                                                       */
/* INPUTS                                                                */
/*      None    	                                                     */
/*                                                                       */
/* OUTPUTS                                                               */
/*      None                                                             */
/*                                                                       */
/* RETURN                                                                */
/*      0           Success                                              */
/*		Otherwise	error												 */
/*                                                                       */
/*************************************************************************/
INT  w56964StopPlay()
{

#ifdef SOFT_MODE
	INT bytes;
#endif	
	const UINT8 abyCmdStopSpeech[] = { 0x03, 0x04, 0x02 };
	
#ifdef ECOS
	cyg_interrupt_mask(GPIO_INT_NUM);
	cyg_interrupt_detach(_tW5691.int_handle_play);
#else
	sysDisableInterrupt(GPIO_INT_NUM);
#endif
#ifdef SOFT_MODE
	
	//outpw(REG_ACTL_M80CON, 0x25202);
	outpw(REG_ACTL_M80CON, inpw(REG_ACTL_M80CON) | W_GFIFO );
	soft_write_command_byte_reg(0x08);
	soft_write_data_reg(0x55);
	for (bytes=0;bytes<3;bytes++){
		soft_write_data_reg(abyCmdStopSpeech[bytes]);
	}
	//outpw(REG_ACTL_M80CON, 0x5202);
	outpw(REG_ACTL_M80CON, inpw(REG_ACTL_M80CON) & ~W_GFIFO );
#else
	outpw(REG_ACTL_M80SIZE, sizeof(abyCmdStopSpeech)+1);
	outpw(REG_ACTL_M80DATA0, 0x55 | abyCmdStopSpeech[0]<<8 | (abyCmdStopSpeech[1]<<16) | (abyCmdStopSpeech[2]<<24) );
	outpw(REG_ACTL_M80ADDR, 0x08);
	//outpw(REG_ACTL_M80CON, 0x25210);
	outpw(REG_ACTL_M80CON, inpw(REG_ACTL_M80CON) | W_GFIFO | W_IF12_ACT);
	while((inpw(REG_ACTL_M80CON)&W_IF12_ACT) != 0);
	//outpw(REG_ACTL_M80CON, 0x5200);
	outpw(REG_ACTL_M80CON, inpw(REG_ACTL_M80CON) & ~W_GFIFO );
#endif
	
	ComDrv_SendCommand(eCOMMAND_DISABLE_SPEECH_INTERRUPT);
	
	DevDrv_PowerDown();
	_W56964_Playing = 0;
	return 0;
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      w56964SetPlayVolume                                              */
/*                                                                       */
/* DESCRIPTION                                                           */
/*      Set W5691 left and right channel play volume.                    */
/*                                                                       */
/* INPUTS                                                                */
/*      ucLeftVol    play volume of left channel                          */
/*      ucRightVol   play volume of left channel                          */
/*                  0:  mute                                             */
/*                  1:  minimal volume                                   */
/*                  31: maxmum volume                                    */
/*                                                                       */
/* OUTPUTS                                                               */
/*      None                                                             */
/*                                                                       */
/* RETURN                                                                */
/*      0           Success                                              */
/*		Otherwise	error												 */
/*                                                                       */
/*************************************************************************/
INT  w56964SetPlayVolume(UINT8 ucLeftVol, UINT8 ucRightVol)//w569x:0 (mute) ~ 0x1f (max)
{
	_tW56964.sPlayVolume = (ucLeftVol << 8) | ucRightVol;
	
	ComDrv_SendCommandWithData(eCOMMAND_SET_EQ_VOLUME, &ucLeftVol, 1); 
	//_Change_Volume = 1;
	
	ComDrv_SetHeadphoneLeftVol(ucLeftVol);
	ComDrv_SetHeadphoneLeftVol(ucRightVol);
	
	return 0;
}


#endif	/* HAVE_W56964 */
