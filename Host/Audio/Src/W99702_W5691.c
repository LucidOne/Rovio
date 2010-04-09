/****************************************************************************
 * 
 * FILENAME
 *     W99702_W5691.c
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
#include "W5691.h"

#ifdef HAVE_W5691

//#define SOFT_MODE


#define Play_FIFO_Trigger_Length		(128)

static AUDIO_T	_tW5691;
static INT 		_W5691_Playing = 0;
static INT		_Change_Volume = 0;
static UINT8	g_byInterruptMask;
static volatile INT	_bPlayDmaToggle =0;
static UINT8	g_byEQVol;
static UINT8 g_byHPLeftVol;
static UINT8 g_byHPRightVol;

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

/*----------------disable interrupt-----------------------------------------*/
static void ComDrv_DisableInterrupt(void)
{
	if (g_byInterruptMask == 1)
	{
#ifdef SOFT_MODE
		soft_write_command_byte_reg(0x0f);
		//soft_write_data_reg(0x1f);
#else
		write_intermediate_reg(0x0f,0);
		
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
		soft_write_command_byte_reg(0x8f);
		//soft_write_data_reg(0x1f);
#else		
		write_intermediate_reg(0x8f,0);
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
	return MW_NO_ERROR;
}
/*--------------------------------------------------------------------------*/
/*----- Receive Data from W5691 -----*/
static UINT8 ComDrv_ReceiveSimpleData(
	UINT8	byReg
	)
{
	UINT8 byData;

	/* select Interrupt Status Register and disable interrupt */
	ComDrv_DisableInterrupt();
#ifdef SOFT_MODE
	soft_write_command_byte_reg(byReg);
	byData = soft_read_data_reg();
#else
	/* read interrupt nStatus */
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
	}
	soft_write_command_byte_reg(byFIFOReg);	
#else
	outpw(REG_ACTL_M80ADDR,byFIFOReg);
#endif	
	
	
	
	if (byFIFOReg == CMD_COM_ID_GEN_FIFO) //for sync with w5691's F/W
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
	if (byFIFOReg == CMD_COM_ID_SPE_FIFO)
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
	return MW_NO_ERROR;
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
/*----- set melody FIFO interrupt trigger level -----*/
static short ComDrv_SetMelodyFIFOInterruptPoint(
	UINT8	byIntPoint	/* interrupt point */
	)
{
	UINT8	byRegValue;

	if (byIntPoint > 7)
		return MW_ERROR_ARGUMENT;

	/* Select FIFO Status register and disable interrupt */
	ComDrv_DisableInterrupt();

	byRegValue = ComDrv_ReceiveSimpleData(CMD_COM_ID_FIFO_ST_REG);

	byRegValue = (byIntPoint & 0x07) | (byRegValue & 0xF8);


	ComDrv_SendSimpleCommand(CMD_COM_ID_FIFO_ST_REG,byRegValue);
	

	/* enable interrupt */
	ComDrv_EnableInterrupt();

	return MW_NO_ERROR;
}
/*----- Set P-FIFO interrupt trigger level -----*/
static short ComDrv_SetSpeechFIFOInterruptPoint(
	UINT8	byIntPoint	/* interrupt point */
	)
{
	UINT8	byRegValue;

	if (byIntPoint > 7)
		return MW_ERROR_ARGUMENT;

	/* Select FIFO Status register and disable interrupt */
	ComDrv_DisableInterrupt();
	
	byRegValue=ComDrv_ReceiveSimpleData(CMD_COM_ID_FIFO_ST_REG);

	byRegValue = ((byIntPoint & 0x07) << 4) | (byRegValue & 0x8F);

	
	ComDrv_SendSimpleCommand(CMD_COM_ID_FIFO_ST_REG,byRegValue);
	

	/* enable interrupt */
	ComDrv_EnableInterrupt();

	return MW_NO_ERROR;
}/*----- end of Set P-FIFO interrupt trigger level -----*/

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

/*----- Power up -----*/
static short ComDrv_PowerUp(
	UINT8	bySpeakerVolume	/* 0x00 ~ 0x1F */
	)
{
	ComDrv_DisableInterrupt();
	
	
	/* Set AP2 to 0,  Set DP1 to 0 */
	
#ifdef SOFT_MODE	
	soft_write_command_byte_reg(CMD_COM_ID_CLK_CTRL);
	soft_write_data_reg(0);
#else	
	ComDrv_SendSimpleCommand(CMD_COM_ID_CLK_CTRL, 0);
#endif	
	//soft_read_status_flag_reg();
	/* Set DP0 to 0, wake up uC */
	
	
#ifdef SOFT_MODE		
	soft_write_command_byte_reg(CMD_COM_ID_WAKE_UP);
	soft_write_data_reg(0);
#else	
	ComDrv_SendSimpleCommand(CMD_COM_ID_WAKE_UP, 0);	
#endif
	//soft_read_status_flag_reg();
	
	/* Reset W5691 */
	ComDrv_SetInterruptMask(0);
//	Delay(0x100);
	ComDrv_SendSimpleCommand(CMD_COM_ID_FIFO_RT_REG, FIFO_RT_REG_RST);
#ifdef SOFT_MODE
	while((soft_read_status_flag_reg() & CMD_STA_BUSY) != 0);
#else
	Delay(50000);//wait for the busy bit;
#endif	

	ComDrv_EnableInterruptBits(IRQ_EN_REG_FIE0 | IRQ_EN_REG_FIE1 |
							   IRQ_EN_REG_FIE2 | IRQ_EN_REG_FIE3);
				   

		

	
	
	

	return MW_NO_ERROR;
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
	write_intermediate_reg(CMD_COM_ID_SPK_VOL_REG,(SPK_VOL_REG_VDS << 6) | 0);
#endif
	/* Increment speaker volume register to wanted value one by one */
	ComDrv_VolumeUp(bySpeakerVolume);
	_tW5691.sPlayVolume = bySpeakerVolume;
	
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
	/* Wait 40 ms */
//	ComDrv_Wait(40);
	Delay(40000);
	ComDrv_EnableInterrupt();
	return MW_NO_ERROR;
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
	ComDrv_SendSimpleCommand(CMD_COM_ID_ANALOG_CTRL,ANALOG_CTRL_REG_AP0 | ANALOG_CTRL_REG_AP1);


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

	for (nTimeOutCount=0;nTimeOutCount<10000;nTimeOutCount++){
		if ((ComDrv_ReceiveSimpleData(CMD_COM_ID_WAKE_UP) & WAKE_UP_REG_DP0) != 0)
			break;
	}
	if (nTimeOutCount==10000){
		_error_msg("ComDrv_PowerDown - wait for W5691 uC enther STOP mode timeout\n");
		return ERR_W5691_STOP_TIMEOUT;
	}
		

	/* Set DP1 to 1, Set AP2 to 1 */
	ComDrv_SendSimpleCommand(CMD_COM_ID_CLK_CTRL,CLK_CTRL_REG_DP1 | CLK_CTRL_REG_AP2);

	ComDrv_EnableInterrupt();

	return MW_NO_ERROR;
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
static short ComDrv_SendCommand(enum E_CommandType eCommand)
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
		return ComDrv_PowerUp(0x1F);
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
		_tW5691.sPlayVolume = ComDrv_VolumeDown();
		ComDrv_EnableInterrupt();
		return ComDrv_SendFIFOData(CMD_COM_ID_GEN_FIFO, abyCmdPauseMelody, 
								   sizeof(abyCmdPauseMelody));
	case eCOMMAND_PAUSE_SPEECH_SYNTHESIZER:
		ComDrv_DisableInterrupt();
		_tW5691.sPlayVolume = ComDrv_VolumeDown();
		ComDrv_EnableInterrupt();
		return ComDrv_SendFIFOData(CMD_COM_ID_GEN_FIFO, abyCmdPauseSpeech, 
								   sizeof(abyCmdPauseSpeech));
	case eCOMMAND_RESUME_MELODY_SYNTHESIZER:
		ComDrv_DisableInterrupt();
		ComDrv_VolumeUp(_tW5691.sPlayVolume);
		ComDrv_EnableInterrupt();
		return ComDrv_SendFIFOData(CMD_COM_ID_GEN_FIFO, abyCmdResumeMelody, 
								   sizeof(abyCmdResumeMelody));
	case eCOMMAND_RESUME_SPEECH_SYNTHESIZER:
		ComDrv_DisableInterrupt();
		ComDrv_VolumeUp(_tW5691.sPlayVolume);
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
		return MW_ERROR_ARGUMENT;
	}
	return MW_NO_ERROR;
}/*----- end of send command -----*/


/*--------------------------------------------------------------------------*/
static short ComDrv_SendCommandWithData(
	enum E_CommandType	eCommand,
	const UINT8*			pbyData,
	UINT16				wDataSize
	)
{
	UINT8 UINT8mp = 0;
	switch(eCommand)
	{
	case eCOMMAND_POWER_UP:
		return ComDrv_PowerUp(*pbyData);
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
	return MW_ERROR_ARGUMENT;
}

/*----- send PCM data -----*/
static short SendSpeechDataISR()
{
	if(_bPlayDmaToggle==0)
	{
		_bPlayDmaToggle=1;
		_tW5691.bPlayLastBlock = _tW5691.fnPlayCallBack((UINT8 *)_uAuPlayBuffAddr,
										Play_FIFO_Trigger_Length);
		ComDrv_SendCommandWithData(eCOMMAND_SEND_SPEECH_DATA, 
								   (UINT8 *)_uAuPlayBuffAddr, Play_FIFO_Trigger_Length);
	}
	else{
		_bPlayDmaToggle=0;
		_tW5691.bPlayLastBlock = _tW5691.fnPlayCallBack((UINT8 *)(_uAuPlayBuffAddr + Play_FIFO_Trigger_Length), 
									Play_FIFO_Trigger_Length);
		ComDrv_SendCommandWithData(eCOMMAND_SEND_SPEECH_DATA, 
								   (UINT8 *)(_uAuPlayBuffAddr + Play_FIFO_Trigger_Length), Play_FIFO_Trigger_Length);
	}
	
	
	return 1;
}/*----- end of send PCM data -----*/
/*----- SEND SPEECH DATA ISR -----*/
static short InterruptSpeechFIFO()
{	
	
	SendSpeechDataISR();
	
	
	return MW_NO_ERROR;
}/*----- END OF SEND SPEECH DATA ISR -----*/
/*----- receive data -----*/
static short ComDrv_ReceiveData(
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
		return MW_ERROR_ARGUMENT;
	}
	return MW_NO_ERROR;
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
		

	return MW_NO_ERROR;
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
	short nRet = MW_NO_ERROR;
	UINT8 i;

	nRet = ComDrv_ReceiveData(eCOMMAND_GET_INTERRUPT_STATUS, &byStatus);
	if (nRet != MW_NO_ERROR)
		return nRet;


	nRet = ComDrv_ReceiveData(eCOMMAND_GET_INTERRUPT_ENABLE, &byEnable);
	if (nRet != MW_NO_ERROR)
		return nRet;
		


	if ((byStatus & byEnable) == 0x00)
	{
		ComDrv_SendSimpleCommand(CMD_COM_ID_INT_ST_REG, byStatus);
		return MW_NO_ERROR;
	}
	byStatus &= byEnable;

	for(i = 0; i <8 ; i++)
	{
		byTemp = abyIntType[i];
		if (byStatus & byTemp)
		{
			switch(byTemp)
			{

			case IRQ_STA_REG_PFIS:
				while(1)
				{
					ComDrv_SendSimpleCommand(CMD_COM_ID_INT_ST_REG, 
												  IRQ_STA_REG_PFIS);

#ifndef	_LOCAL_TEST
					byFIFOStatus = 0;
					ComDrv_ReceiveData(eCOMMAND_GET_FIFO_STATUS, 
									   &byFIFOStatus);
					if ((byFIFOStatus & FIFO_STAT_REG_PLET) == 0)
						break;
#endif

					if ((nRet = InterruptSpeechFIFO()) < 0)
						return nRet;

					ComDrv_ReceiveData(eCOMMAND_GET_INTERRUPT_ENABLE,
									   &byEnable);
					if ((byEnable & IRQ_EN_REG_PFIE) == 0)
						break;
				}
				return MW_NO_ERROR;

			case IRQ_STA_REG_FIS1:
				{
					UINT8 byData = 0;
					
					

					//Delay(10000);
					byData=ComDrv_ReceiveSimpleData(CMD_COM_ID_X_BUF);				   



					if (byData != 0x55){

						return MW_NO_ERROR;
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

	return MW_NO_ERROR;
}

#ifdef ECOS
cyg_uint32  w5691_dma_isr(cyg_vector_t vector, cyg_addrword_t data)
#else
static void  w5691_dma_isr()
#endif
{
	ComDrv_DisableInterrupt();
	ComDrv_SetInterruptMask(0);
	
	InterruptHandlerInternal();
	
	//count++;
	
	ComDrv_SetInterruptMask(1);
	ComDrv_EnableInterrupt();
	return ;
}

static short DevDrv_PowerUp(UINT8 byEqualizerVolume)
{ 
	short nRet = 0;
	UINT8 bySpeakerVolume = byEqualizerVolume;

	nRet = ComDrv_SendCommandWithData(eCOMMAND_POWER_UP, &bySpeakerVolume, 1); 
	
	return nRet;
}
static short DevDrv_PowerDown(void)
{ 
	short nRet = 0;
	nRet = ComDrv_SendCommand(eCOMMAND_POWER_DOWN);
	
	return nRet;
}


/*----- INITIALIZE W5691-----*/
static short ComDrv_Initialize(void)
{
	/* Set PLL */
	ComDrv_SendSimpleCommand(CMD_COM_ID_PLL_ST_REG_I, W5691_PLL_ADJUST_N);
	ComDrv_SendSimpleCommand(CMD_COM_ID_PLL_ST_REG_II, W5691_PLL_ADJUST_M);


	

	g_byInterruptMask = 1;
	ComDrv_DisableInterrupt();
	//ComDrv_SetInterruptMask(0);
	/* Set AP2 to 0,  Set DP1 to 0 */
	ComDrv_SendSimpleCommand(CMD_COM_ID_CLK_CTRL,0);

	/* Set DP0 to 0, wake up uC */
	ComDrv_SendSimpleCommand(CMD_COM_ID_WAKE_UP,0);
	Delay(500000);

	/* Reset W5691 */
	ComDrv_SendSimpleCommand(CMD_COM_ID_FIFO_RT_REG, FIFO_RT_REG_RST);
	
#ifdef SOFT_MODE
	
	
	while((soft_read_status_flag_reg() & CMD_STA_BUSY) != 0);
		
	
#else
	Delay(500000);//wait for the busy bit;
#endif
	
	/* Set DP0 to 1 */
	ComDrv_SendSimpleCommand(CMD_COM_ID_WAKE_UP,WAKE_UP_REG_DP0);
	/* Set AP2 to 1,  Set DP1 to 1 */
	ComDrv_SendSimpleCommand(CMD_COM_ID_WAKE_UP,CLK_CTRL_REG_DP1|CLK_CTRL_REG_AP2);
	

	ComDrv_SendSimpleCommand(CMD_COM_ID_PLL_ST_REG_I, W5691_PLL_ADJUST_N);
	ComDrv_SendSimpleCommand(CMD_COM_ID_PLL_ST_REG_II, W5691_PLL_ADJUST_M);	

	

	
	g_byEQVol = 0x1F;
	g_byHPLeftVol = 0x1F;
	g_byHPRightVol = 0x1F;
	
	ComDrv_EnableInterrupt();
	
	return MW_NO_ERROR;
}/*----- END OF COMDRV INITIALIZE -----*/



/* DevDrv_Initialize*/
static short DevDrv_Initialize(void)
{

	short nRet = 0;

	nRet = ComDrv_Initialize();
	if (nRet < 0)
		return nRet;
	
	DevDrv_PowerUp(0);
	DevDrv_PowerDown();
	return 0;
}
static void Initialize(void)
{
	_tW5691.sPlayVolume = 0x1F;
	DevDrv_Initialize();
	
}

/*----- power up analog -----*/
static short DevDrv_PowerUpAnalog(UINT8 byEqualizerVolume)
{ 
	UINT8 bySpeakerVolume = byEqualizerVolume;
	return ComDrv_SendCommandWithData(eCOMMAND_POWER_UP_ANALOG, &bySpeakerVolume, 1); 
}/*----- end of power up analog -----*/


/*----- initialize audio interface and w5691 -----*/
static INT  w5691_init()
{
	

	/* reset W5691 */

	outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & ~0x100000);	/* GPIO 20 */
	outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) | 0x100000);
	Delay(100);
	outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) & ~0x100000);
	Delay(100);
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

	//initialize w5691
	
	
	Initialize();// set PLL  ComDrv_SendSimpleCommand

	
	
	DevDrv_PowerUp(_tW5691.sPlayVolume);
	


	DevDrv_PowerUpAnalog(_tW5691.sPlayVolume);
	
	
	ComDrv_SendCommand(eCOMMAND_SETUP_SPEECH_INTERRUPT);
	ComDrv_SendCommand(eCOMMAND_CLEAR_SPEECH_BUFFER);
	

	
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
INT  w5691StartPlay(AU_CB_FUNC_T *fnCallBack, INT nSamplingRate, 
						INT nChannels, INT pcm_format)
{


    UINT16	wTMV;
   	UINT8	byTMB = 0;
   	UINT8   byClockBase = 1;

	
	UINT8	eFormat = eSPEECH_FORMAT_SIGN_16BIT_PCM;
	INT 	nStatus;
	UINT8 byStartSpchCmd[8] = { 0x03, 0x03, 0x00, 0x00, 0x00, 0x00 , 0x7f, 0x40 };//GFIFO start play command
	
	if (_W5691_Playing)
		return ERR_W5691_PLAY_ACTIVE;
		
	if (_W5691_Playing == 0)
	{
		memset((CHAR *)&_tW5691, 0, sizeof(_tW5691));
		nStatus = w5691_init();
		if (nStatus < 0)
			return nStatus;	
	}
	

	
#ifdef ECOS
	cyg_interrupt_disable();
	sysSetInterruptType(GPIO_INT_NUM, LOW_LEVEL_SENSITIVE);
	cyg_interrupt_create(GPIO_INT_NUM, 1, 0, w5691_dma_isr, NULL, 
					&_tW5691.int_handle_play, &_tW5691.int_holder_play);
	cyg_interrupt_attach(_tW5691.int_handle_play);
	cyg_interrupt_unmask(AU_PLAY_INT_NUM);
	cyg_interrupt_enable();
#else	

	sysSetInterruptType(GPIO_INT_NUM, LOW_LEVEL_SENSITIVE);
	/* Install ISR */
	sysInstallISR(IRQ_LEVEL_1, GPIO_INT_NUM, (PVOID)w5691_dma_isr);
    /* enable CPSR I bit */
    sysSetLocalInterrupt(ENABLE_IRQ);
	/* Set AIC into SW mode */
//	sysSetAIC2SWMode();
	sysEnableInterrupt(GPIO_INT_NUM);
#endif
	outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) | (1<<USED_GPIO_NUM));
	
	_tW5691.nPlaySamplingRate = nSamplingRate;
	_tW5691.fnPlayCallBack = fnCallBack;
	
	wTMV = (UINT16)(W5691_SYSTEM_CLOCK / _tW5691.nPlaySamplingRate) / byClockBase;
	while(wTMV > 511)
	{
		byClockBase <<= 1;
		wTMV >>= 1;
		byTMB++;
	}

	byStartSpchCmd[2] = (UINT8)eFormat | eSPEECH_FCH_PFIFO ;
	byStartSpchCmd[3] = (UINT8)(wTMV & 0xFF);
	byStartSpchCmd[4] = (UINT8)(wTMV >> 8);
	byStartSpchCmd[5] = byTMB;

	


	/* call back to fill DMA buffer*/
	_tW5691.bPlayLastBlock = _tW5691.fnPlayCallBack((UINT8 *)_uAuPlayBuffAddr,
										Play_FIFO_Trigger_Length*2);

	/* send PCM data to w5691*/									
	ComDrv_SendCommandWithData(eCOMMAND_SEND_SPEECH_DATA, 
								   (UINT8 *)_uAuPlayBuffAddr, Play_FIFO_Trigger_Length*2);


	ComDrv_SendCommand(eCOMMAND_ENABLE_SPEECH_INTERRUPT);




#ifdef SOFT_MODE
	ComDrv_SendFIFOData(CMD_COM_ID_GEN_FIFO,byStartSpchCmd,6);
#else
	outpw(REG_ACTL_M80SIZE, 6+1);
	outpw(REG_ACTL_M80DATA0, 0x55 | byStartSpchCmd[0]<<8 | (byStartSpchCmd[1]<<16) | (byStartSpchCmd[2]<<24) );
	outpw(REG_ACTL_M80DATA1, byStartSpchCmd[3] | (byStartSpchCmd[4]<<8) | (byStartSpchCmd[5]<<16));
	outpw(REG_ACTL_M80ADDR, CMD_COM_ID_GEN_FIFO);
	outpw(REG_ACTL_M80CON, inpw(REG_ACTL_M80CON) | W_GFIFO | W_IF12_ACT);
	while((inpw(REG_ACTL_M80CON)&W_IF12_ACT) != 0);
	outpw(REG_ACTL_M80CON, inpw(REG_ACTL_M80CON) & ~W_GFIFO );
	ComDrv_SendSimpleCommand(CMD_COM_ID_WAKE_UP,WAKE_UP_REG_INT);	
#endif	

	Delay(0x5000);
	
	ComDrv_SetInterruptMask(1);
#ifdef SOFT_MODE	
	soft_read_status_flag_reg();
#else	
	ComDrv_DisableInterrupt();
#endif	
	ComDrv_EnableInterrupt();
	_W5691_Playing=1;
	

	return 0;
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      w5691StopPlay                                                    */
/*                                                                       */
/* DESCRIPTION                                                           */
/*      Stop W5691 playback immdediately.                                 */
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
INT  w5691StopPlay()
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
	_W5691_Playing = 0;
	return 0;
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      w5691SetPlayVolume                                              */
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
INT  w5691SetPlayVolume(UINT8 ucLeftVol, UINT8 ucRightVol)//w5691:0 (mute) ~ 0x1f (max)
{
	_tW5691.sPlayVolume = (ucLeftVol << 8) | ucRightVol;
	ComDrv_SendCommandWithData(eCOMMAND_SET_EQ_VOLUME, &ucLeftVol, 1); 
	_Change_Volume = 1;
	
	return 0;
}


#endif	/* HAVE_W5691 */