#ifdef ECOS
#include "stdlib.h"
#else
#include <stdlib.h>
#include <string.h>
#endif

#include "w99702_reg.h"
#include "wblib.h"
#include "fmi.h"
#include "wb_fmi.h"

#ifdef SM_DEVICE

UINT32 volatile _fmi_uSM_DataReady=0, _fmi_uSM_2NANDZone=0, _fmi_uSM_ID;
BOOL volatile _fmi_bIsSamSungNAND=FALSE, _fmi_bIsSMDefaultCheck=TRUE, _fmi_bIs2NAND=FALSE;
BOOL volatile _fmi_bIsCheckNAND=FALSE, _fmi_bIsReserved=FALSE, _fmi_bIsPartitionDisk=FALSE;

// SM functions
VOID fmiSM_INTHandler(VOID)
{
	UINT32 isr;

	isr = inpw(REG_SMISR);
	if (isr & 0x01)
	{
		_fmi_uSM_DataReady = 1;
		outpw(REG_SMISR, isr|0x01);
	}

	if (isr & 0x02)
	{
		_fmi_uSM_DataReady = 1;
		outpw(REG_SMISR, isr|0x02);
	}
}

// for reserved area used
UINT32 _fmi_uReservedBaseSector=0;	// logical sector
UINT32 _fmi_uSMReservedAreaSize=0, _fmi_uSMReservedBlock=0, _fmi_uSMReservedSector=0;
BOOL volatile _fmi_bIs2Disk=FALSE;

// global variables
UINT32 _fmi_uF4cycle=0;		// 0:3-cycle address; 1:4-cycle address
UINT32 _fmi_u2KPageSize=0;		// 0:page size 512 byte; 1:page size 2K byte
UINT32 _fmi_uSectorPerFlash=0, _fmi_uBlockPerFlash=0, _fmi_uPagePerBlock=0, _fmi_uSectorPerBlock=0;
UCHAR _fmi_ucBaseAddr1, _fmi_ucBaseAddr2;

UINT8	*_fmi_pCIS;
__align(32) static UCHAR _fmi_CIS[512] = {
0x01,0x03,0xd9,0x01,0xff,0x18,0x02,0xdf,0x01,0x20,0x04,0x00,0x00,0x00,0x00,0x21,
0x02,0x04,0x01,0x22,0x02,0x01,0x01,0x22,0x03,0x02,0x04,0x07,0x1a,0x05,0x01,0x03,
0x00,0x02,0x0f,0x1b,0x08,0xc0,0xc0,0xa1,0x01,0x55,0x08,0x00,0x20,0x1b,0x0a,0xc1,
0x41,0x99,0x01,0x55,0x64,0xf0,0xff,0xff,0x20,0x1b,0x0c,0x82,0x41,0x18,0xea,0x61,
0xf0,0x01,0x07,0xf6,0x03,0x01,0xee,0x1b,0x0c,0x83,0x41,0x18,0xea,0x61,0x70,0x01,
0x07,0x76,0x03,0x01,0xee,0x15,0x14,0x05,0x00,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
0x00,0x20,0x20,0x20,0x20,0x00,0x30,0x2e,0x30,0x00,0xff,0x14,0x00,0xff,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x01,0x03,0xd9,0x01,0xff,0x18,0x02,0xdf,0x01,0x20,0x04,0x00,0x00,0x00,0x00,0x21,
0x02,0x04,0x01,0x22,0x02,0x01,0x01,0x22,0x03,0x02,0x04,0x07,0x1a,0x05,0x01,0x03,
0x00,0x02,0x0f,0x1b,0x08,0xc0,0xc0,0xa1,0x01,0x55,0x08,0x00,0x20,0x1b,0x0a,0xc1,
0x41,0x99,0x01,0x55,0x64,0xf0,0xff,0xff,0x20,0x1b,0x0c,0x82,0x41,0x18,0xea,0x61,
0xf0,0x01,0x07,0xf6,0x03,0x01,0xee,0x1b,0x0c,0x83,0x41,0x18,0xea,0x61,0x70,0x01,
0x07,0x76,0x03,0x01,0xee,0x15,0x14,0x05,0x00,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
0x00,0x20,0x20,0x20,0x20,0x00,0x30,0x2e,0x30,0x00,0xff,0x14,0x00,0xff,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

// for reserved area
extern UCHAR _fmi_ucSMBuffer[];
extern UINT8 *_fmi_pSMBuffer;

INT fmiSetSMTiming(UINT8 nALE, UINT8 nHI, UINT8 nLO)
{
	UINT32 type;

	type = ((nALE & 0x3) << 16) | ((nHI & 0xf) << 12) | ((nLO & 0xf) << 8);
	outpw(REG_SMCR, (inpw(REG_SMCR) & 0xfffc00ff) | type);
	return FMI_NO_ERR;
}

INT fmiSetSMTimePulse(UINT32 type)
{
	outpw(REG_SMCR, (inpw(REG_SMCR) & 0xfffc00ff) | type);
	return FMI_NO_ERR;
}

INT fmiSM_Reset(VOID)
{
	unsigned int volatile tick;
	unsigned int volatile i;

	if (!_fmi_bIsSMDefaultCheck)
	{
		tick = sysGetTicks(TIMER0);
		while(!(inpw(REG_SMISR) & 0x10))	// rb# status
		{
			if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
				return FMI_TIMEOUT;

			if (_fmi_bIsSMInsert == FALSE)
				return FMI_NO_SM_CARD;
		}
	}

	outpw(REG_SMCMD, 0xff);
	for (i=0x100; i>0; i--);

	if (_fmi_bIsSMDefaultCheck)
	{
		tick = sysGetTicks(TIMER0);
		while(!(inpw(REG_SMISR) & 0x10))	// rb# status
		{
			if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
				return FMI_TIMEOUT;

			if (_fmi_bIsSMInsert == FALSE)
				return FMI_NO_SM_CARD;
		}
	}
	return FMI_NO_ERR;
}


VOID fmiSM_Initial(UINT32 uChipSel)
{
	fmiLock();
	
	// multi PAD control
	outpw(0x7ff00020, (inpw(0x7ff00020)&0x9fffffff)|0x80000000);

	if (uChipSel == 7)
	{
		outpw(REG_SMCR, (inpw(REG_SMCR)&0xffffff00)|0x7c);
		fmiUnLock();
		return;
	}

	//outpw(REG_SMCR, 0x3ff7c);
	if (_fmi_u2KPageSize)
	{
		if (uChipSel == 1)
			outpw(REG_SMCR, (inpw(REG_SMCR)&0xffffff00)|0x9c);	// psize:2048; ecc, wp# set 1; dma r/w disable
		else
			outpw(REG_SMCR, (inpw(REG_SMCR)&0xffffff00)|0x8c);	// psize:2048; ecc, wp# set 1; dma r/w disable
	}
	else
	{
		if (uChipSel == 1)
			outpw(REG_SMCR, (inpw(REG_SMCR)&0xffffff00)|0x1c);	// psize:512; ecc, wp# set 1; dma r/w disable
		else
			outpw(REG_SMCR, (inpw(REG_SMCR)&0xffffff00)|0x0c);	// psize:512; ecc, wp# set 1; dma r/w disable
	}

#ifdef _USE_IRQ
	if (_fmi_uFirst_L2P == 0)
		outpw(REG_SMIER, 0x03);
#endif
	
	fmiUnLock();
}


INT fmiSM_ReadID(UINT32 uChipSel)
{
	UINT32 tempID[2];
	unsigned int volatile tick, status;

	if (uChipSel == 1)
		outpw(REG_SMCR, (inpw(REG_SMCR)&0xffffff80)|0x1c);	// psize:512; ecc, wp# set 1; dma r/w disable
	else
		outpw(REG_SMCR, (inpw(REG_SMCR)&0xffffff80)|0x0c);	// psize:512; ecc, wp# set 1; dma r/w disable

	status = fmiSM_Reset();
	if (status != FMI_NO_ERR)
		return status;

	if (!_fmi_bIsSMDefaultCheck)
	{
		tick = sysGetTicks(TIMER0);
		while(!(inpw(REG_SMISR) & 0x10))	// rb# status
		{
			if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
				return FMI_TIMEOUT;

			if (_fmi_bIsSMInsert == FALSE)
				return FMI_NO_SM_CARD;
		}
	}
	outpw(REG_SMCMD, 0x90);		// read ID command
	outpw(REG_SMADDR, 0x00);	// address 0x00

	tick = sysGetTicks(TIMER0);
	while(!(inpw(REG_SMISR) & 0x10))	// rb# status
	{
		if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
			return FMI_TIMEOUT;

		if (_fmi_bIsSMInsert == FALSE)
			return FMI_NO_SM_CARD;
	}

	tempID[0] = inpw(REG_SMDATA);
	tempID[1] = inpw(REG_SMDATA);

#if 0
	// 0x98:toshiba; 0xec:samsung; 0x20:ST
	if ((tempID[0] != 0x98) && (tempID[0] != 0xec) && (tempID[0] != 0x20))
	{
#ifdef DEBUG
		printf("SM ID error [%x][%x]\n", tempID[0], tempID[1]);
#endif
		return FMI_SM_ID_ERROR;
	}

	if (tempID[0] == 0xec)
		_fmi_bIsSamSungNAND = TRUE;
	else
		_fmi_bIsSamSungNAND = FALSE;
#endif

	switch (tempID[1])
	{
		case 0x79:	// 128M / 512byte
			if (!_fmi_bIsCheckNAND)
			{
#ifdef _USE_TWO_NAND_
				if (_fmi_bIs2NAND)
				{
					_fmi_uSectorPerFlash += 255744;
					_fmi_uBlockPerFlash += 8191;
				}
				else
				{
					_fmi_uSectorPerFlash = 255744;
					_fmi_uBlockPerFlash = 8191;
				}
				if (uChipSel == 0)
					_fmi_uSM_2NANDZone = 8;
#else
				_fmi_uSectorPerFlash = 255744;
				_fmi_uBlockPerFlash = 8191;
#endif
				_fmi_uPagePerBlock = 32;
				_fmi_uSectorPerBlock = 32;
			}
			_fmi_uF4cycle = 1;
			_fmi_u2KPageSize = 0;
			break;

		case 0x76:	// 64M / 512byte
			if (!_fmi_bIsCheckNAND)
			{
#ifdef _USE_TWO_NAND_
				if (_fmi_bIs2NAND)
				{
					_fmi_uSectorPerFlash += 127872;
					_fmi_uBlockPerFlash += 4095;
				}
				else
				{
					_fmi_uSectorPerFlash = 127872;
					_fmi_uBlockPerFlash = 4095;
				}
				if (uChipSel == 0)
					_fmi_uSM_2NANDZone = 4;
#else
				_fmi_uSectorPerFlash = 127872;
				_fmi_uBlockPerFlash = 4095;
#endif
				_fmi_uPagePerBlock = 32;
				_fmi_uSectorPerBlock = 32;
			}
			_fmi_uF4cycle = 1;
			_fmi_u2KPageSize = 0;
			break;

		case 0x75:	// 32M / 512byte
			if (!_fmi_bIsCheckNAND)
			{
#ifdef _USE_TWO_NAND_
				if (_fmi_bIs2NAND)
				{
					_fmi_uSectorPerFlash += 63936;
					_fmi_uBlockPerFlash += 2047;
				}
				else
				{
					_fmi_uSectorPerFlash = 63936;
					_fmi_uBlockPerFlash = 2047;
				}
				if (uChipSel == 0)
					_fmi_uSM_2NANDZone = 2;
#else
				_fmi_uSectorPerFlash = 63936;
				_fmi_uBlockPerFlash = 2047;
#endif
				_fmi_uPagePerBlock = 32;
				_fmi_uSectorPerBlock = 32;
			}
			_fmi_uF4cycle = 0;
			_fmi_u2KPageSize = 0;
			break;

		case 0xf1:	// 128M / 2kbyte
		case 0xa1:
			if (!_fmi_bIsCheckNAND)
			{
#ifdef _USE_TWO_NAND_
				if (_fmi_bIs2NAND)
				{
					_fmi_uSectorPerFlash += 255744;
					_fmi_uBlockPerFlash += 1023;
				}
				else
				{
					_fmi_uSectorPerFlash = 255744;
					_fmi_uBlockPerFlash = 1023;
				}
				if (uChipSel == 0)
					_fmi_uSM_2NANDZone = 1;
#else
				_fmi_uSectorPerFlash = 255744;
				_fmi_uBlockPerFlash = 1023;
#endif
				_fmi_uPagePerBlock = 64;
				_fmi_uSectorPerBlock = 256;
			}
			_fmi_uF4cycle = 0;
			_fmi_u2KPageSize = 1;
			break;

		case 0x73:	// 16M / 512byte
			if (!_fmi_bIsCheckNAND)
			{
#ifdef _USE_TWO_NAND_
				if (_fmi_bIs2NAND)
				{
					_fmi_uSectorPerFlash += 31968;	// max. sector no. = 999 * 32
					_fmi_uBlockPerFlash += 1023;
				}
				else
				{
					_fmi_uSectorPerFlash = 31968;	// max. sector no. = 999 * 32
					_fmi_uBlockPerFlash = 1023;
				}
				if (uChipSel == 0)
					_fmi_uSM_2NANDZone = 1;
#else
				_fmi_uSectorPerFlash = 31968;	// max. sector no. = 999 * 32
				_fmi_uBlockPerFlash = 1023;
#endif
				_fmi_uPagePerBlock = 32;
				_fmi_uSectorPerBlock = 32;
			}
			_fmi_uF4cycle = 0;
			_fmi_u2KPageSize = 0;
			break;

		case 0xf2:	// 64M / 2kbyte
		case 0xa2:
			if (!_fmi_bIsCheckNAND)
			{
#ifdef _USE_TWO_NAND_
				if (_fmi_bIs2NAND)
				{
					_fmi_uSectorPerFlash += 127744;
					_fmi_uBlockPerFlash += 511;
				}
				else
				{
					_fmi_uSectorPerFlash = 127744;
					_fmi_uBlockPerFlash = 511;
				}
				if (uChipSel == 0)
					_fmi_uSM_2NANDZone = 1;
#else
				_fmi_uSectorPerFlash = 127744;
				_fmi_uBlockPerFlash = 511;
#endif
				_fmi_uPagePerBlock = 64;
				_fmi_uSectorPerBlock = 256;
			}
			_fmi_uF4cycle = 0;
			_fmi_u2KPageSize = 1;
			break;

		case 0xda:	// 256M / 2kbyte
		case 0xaa:
			if (!_fmi_bIsCheckNAND)
			{
#ifdef _USE_TWO_NAND_
				if (_fmi_bIs2NAND)
				{
					_fmi_uSectorPerFlash += 511488;
					_fmi_uBlockPerFlash += 2047;
				}
				else
				{
					_fmi_uSectorPerFlash = 511488;
					_fmi_uBlockPerFlash = 2047;
				}
				if (uChipSel == 0)
					_fmi_uSM_2NANDZone = 2;
#else
				_fmi_uSectorPerFlash = 511488;
				_fmi_uBlockPerFlash = 2047;
#endif
				_fmi_uPagePerBlock = 64;
				_fmi_uSectorPerBlock = 256;
			}
			_fmi_uF4cycle = 1;
			_fmi_u2KPageSize = 1;
			break;

		case 0xdc:	// 512M / 2kbyte
		case 0xac:
			if (!_fmi_bIsCheckNAND)
			{
#ifdef _USE_TWO_NAND_
				if (_fmi_bIs2NAND)
				{
					_fmi_uSectorPerFlash += 1022976;
					_fmi_uBlockPerFlash += 4095;
				}
				else
				{
					_fmi_uSectorPerFlash = 1022976;
					_fmi_uBlockPerFlash = 4095;
				}
				if (uChipSel == 0)
					_fmi_uSM_2NANDZone = 4;
#else
				_fmi_uSectorPerFlash = 1022976;
				_fmi_uBlockPerFlash = 4095;
#endif
				_fmi_uPagePerBlock = 64;
				_fmi_uSectorPerBlock = 256;
			}
			_fmi_uF4cycle = 1;
			_fmi_u2KPageSize = 1;
			break;

		case 0xd3:	// 1024M / 2kbyte
		case 0xa3:
			if (!_fmi_bIsCheckNAND)
			{
#ifdef _USE_TWO_NAND_
				if (_fmi_bIs2NAND)
				{
					_fmi_uSectorPerFlash += 2045952;
					_fmi_uBlockPerFlash += 8191;
				}
				else
				{
					_fmi_uSectorPerFlash = 2045952;
					_fmi_uBlockPerFlash = 8191;
				}
				if (uChipSel == 0)
					_fmi_uSM_2NANDZone = 8;
#else
				_fmi_uSectorPerFlash = 2045952;
				_fmi_uBlockPerFlash = 8191;
#endif
				_fmi_uPagePerBlock = 64;
				_fmi_uSectorPerBlock = 256;
			}
			_fmi_uF4cycle = 1;
			_fmi_u2KPageSize = 1;
			break;

		case 0xd5:	// 2048M / 2kbyte
			if (!_fmi_bIsCheckNAND)
			{
#ifdef _USE_TWO_NAND_
				if (_fmi_bIs2NAND)
				{
					_fmi_uSectorPerFlash += 4091904;
					_fmi_uBlockPerFlash += 16383;
				}
				else
				{
					_fmi_uSectorPerFlash = 4091904;
					_fmi_uBlockPerFlash = 16383;
				}
				if (uChipSel == 0)
					_fmi_uSM_2NANDZone = 16;
#else
				_fmi_uSectorPerFlash = 4091904;
				_fmi_uBlockPerFlash = 16383;
#endif
				_fmi_uPagePerBlock = 64;
				_fmi_uSectorPerBlock = 256;
			}
			_fmi_uF4cycle = 1;
			_fmi_u2KPageSize = 1;
			break;

		default:
#ifdef DEBUG
			printf("SM ID not support!![%x][%x]\n", tempID[0], tempID[1]);
#endif
			return FMI_SM_ID_ERROR;
	}

	if (!_fmi_bIsCheckNAND)
	{
		if (_fmi_bIsReserved)
		{
			_fmi_uSectorPerFlash -= _fmi_uSMReservedSector;
			_fmi_uBlockPerFlash -= _fmi_uSMReservedBlock;
			_fmi_uReservedBaseSector = _fmi_uSectorPerFlash;
		}
	}

#ifdef DEBUG
	printf("SM ID [%x][%x]\n", tempID[0], tempID[1]);
#endif
	return FMI_NO_ERR;
}


INT fmiSM_BlockErase(UINT32 uBlock)
{
	UINT32 page_no;
	unsigned int volatile tick;
	
	fmiLock();
	
	page_no = uBlock * _fmi_uPagePerBlock;		// get page address

	if (!_fmi_bIsSMDefaultCheck)
	{
		tick = sysGetTicks(TIMER0);
		while(!(inpw(REG_SMISR) & 0x10))	// rb# status
		{
			if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
			{
				fmiUnLock();
				return FMI_TIMEOUT;
			}

			if (_fmi_bIsSMInsert == FALSE)
			{
				fmiUnLock();
				return FMI_NO_SM_CARD;
			}
		}

		outpw(REG_SMCMD, 0x70);		// status read command
		if (inpw(REG_SMDATA) & 0x01)	// 1:fail; 0:pass
		{
			fmiCIS_Write(page_no);
			fmiUnLock();
			return FMI_SM_STATE_ERROR;
		}
	}

	outpw(REG_SMCMD, 0x60);		// erase setup command

	outpw(REG_SMMADDR, (page_no & 0xff));		// PA0 - PA7
	if (_fmi_uF4cycle == 0)
		outpw(REG_SMADDR, ((page_no  >> 8) & 0xff));		// PA8 - PA15
	else
	{
		outpw(REG_SMMADDR, ((page_no  >> 8) & 0xff));		// PA8 - PA15
		outpw(REG_SMADDR, ((page_no  >> 16) & 0xff));		// PA16 - PA17
	}
	
	outpw(REG_SMCMD, 0xd0);		// erase command
	if (_fmi_bIsSMDefaultCheck)
	{
		tick = sysGetTicks(TIMER0);
		while(!(inpw(REG_SMISR) & 0x10))	// rb# status
		{
			if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
			{
				fmiUnLock();
				return FMI_TIMEOUT;
			}

			if (_fmi_bIsSMInsert == FALSE)
			{
				fmiUnLock();
				return FMI_NO_SM_CARD;
			}
		}

		outpw(REG_SMCMD, 0x70);		// status read command
		if (inpw(REG_SMDATA) & 0x01)	// 1:fail; 0:pass
		{
			fmiCIS_Write(page_no);
			fmiUnLock();
			return FMI_SM_STATE_ERROR;
		}
	}
	
	fmiUnLock();
	return FMI_NO_ERR;
}


VOID fmiSM_GetBlockAddr(UINT32 uBlock)
{
	UINT32 BA, parity;

	BA = 0;
	parity = 1;		// even parity

	// check BA0 ~ BA9
	if (uBlock & 0x01)	BA++;
	if (uBlock & 0x02)	BA++;
	if (uBlock & 0x04)	BA++;
	if (uBlock & 0x08)	BA++;
	if (uBlock & 0x10)	BA++;
	if (uBlock & 0x20)	BA++;
	if (uBlock & 0x40)	BA++;
	if (uBlock & 0x80)	BA++;
	if (uBlock & 0x100)	BA++;
	if (uBlock & 0x200)	BA++;

	if (BA & 0x01)		// odd parity?
		parity = 0;

	_fmi_ucBaseAddr1 = (uBlock >> 7) | 0x10;
	_fmi_ucBaseAddr2 = (uBlock << 1) | parity;
}


INT fmiBuffer2SM(UINT32 uSector, UINT32 uLBlock, UINT8 ucColAddr, UINT8 ncBufNo)
{
	UINT32 tmp;
	unsigned int volatile tick;

	fmiSM_GetBlockAddr(uLBlock);

	if (ncBufNo == 0)
	{
		outpw(REG_FMICR, (inpw(REG_FMICR) & 0xfffff8ff)|0x200);		// buffer0 to SM
		// set the spare area configuration
		tmp = (_fmi_ucBaseAddr2 << 24) | (_fmi_ucBaseAddr1 << 16) | 0xffff;
		outpw(REG_SMRA_1, tmp);
		tmp = (_fmi_ucBaseAddr1 << 24) | 0xffffff;
		outpw(REG_SMRA_2, tmp);
		tmp = 0xffffff00 | _fmi_ucBaseAddr2;
		outpw(REG_SMRA_3, tmp);
	}
	else if (ncBufNo == 1)
	{
		outpw(REG_FMICR, (inpw(REG_FMICR) & 0xfffff8ff)|0x600);		// buffer1 to SM
		// set the spare area configuration
		tmp = (_fmi_ucBaseAddr2 << 24) | (_fmi_ucBaseAddr1 << 16) | 0xffff;
		outpw(REG_SMRA_5, tmp);
		tmp = (_fmi_ucBaseAddr1 << 24) | 0xffffff;
		outpw(REG_SMRA_6, tmp);
		tmp = 0xffffff00 | _fmi_ucBaseAddr2;
		outpw(REG_SMRA_7, tmp);
	}

	if (!_fmi_bIsSMDefaultCheck)
	{
		tick = sysGetTicks(TIMER0);
		while(!(inpw(REG_SMISR) & 0x10))	// rb# status
		{
			if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
				return FMI_TIMEOUT;

			if (_fmi_bIsSMInsert == FALSE)
				return FMI_NO_SM_CARD;
		}

		outpw(REG_SMCMD, 0x70);		// status read command
		if (inpw(REG_SMDATA) & 0x01)	// 1:fail; 0:pass
			return FMI_SM_STATE_ERROR;
	}
	// send command
	outpw(REG_SMCMD, 0x80);		// serial data input command
	outpw(REG_SMMADDR, ucColAddr);	// CA0 - CA7
	outpw(REG_SMMADDR, uSector & 0xff);	// PA0 - PA7
	if (_fmi_uF4cycle == 0)
		outpw(REG_SMADDR, (uSector >> 8) & 0xff);		// PA8 - PA15
	else
	{
		outpw(REG_SMMADDR, (uSector >> 8) & 0xff);		// PA8 - PA15
		outpw(REG_SMADDR, (uSector >> 16) & 0xff);		// PA16 - PA17
	}

#ifdef _USE_IRQ
	_fmi_uSM_DataReady = 0;
#endif

	outpw(REG_SMCR, inpw(REG_SMCR) | 0x01);

	tick = sysGetTicks(TIMER0);
#ifdef _USE_IRQ
	while(_fmi_uSM_DataReady == 0)
	{
		if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
			return FMI_TIMEOUT;

		if (_fmi_bIsSMInsert == FALSE)
			return FMI_NO_SM_CARD;
	}
#else
	while(inpw(REG_SMCR) & 0x01)
	{
		if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
			return FMI_TIMEOUT;

		if (_fmi_bIsSMInsert == FALSE)
			return FMI_NO_SM_CARD;
	}
#endif

	outpw(REG_SMCMD, 0x10);		// auto program command

	if (_fmi_bIsSMDefaultCheck)
	{
		tick = sysGetTicks(TIMER0);
		while(!(inpw(REG_SMISR) & 0x10))	// rb# status
		{
			if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
				return FMI_TIMEOUT;

			if (_fmi_bIsSMInsert == FALSE)
				return FMI_NO_SM_CARD;
		}

		outpw(REG_SMCMD, 0x70);		// status read command
		if (inpw(REG_SMDATA) & 0x01)	// 1:fail; 0:pass
			return FMI_SM_STATE_ERROR;
	}
	return FMI_NO_ERR;
}


INT fmiSM2Buffer(UINT32 uSector, UINT8 ucColAddr, UINT8 ncBufNo)
{
#ifdef _MP_
	unsigned int volatile state;
#endif
	unsigned int volatile tick;

	if (ncBufNo == 0)
		outpw(REG_FMICR, (inpw(REG_FMICR) & 0xffffff8f)|0x20);		// SM to buffer0
	if (ncBufNo == 1)
		outpw(REG_FMICR, (inpw(REG_FMICR) & 0xffffff8f)|0x60);		// SM to buffer1

	if (!_fmi_bIsSMDefaultCheck)
	{
#ifdef _MP_
		tick = sysGetTicks(TIMER0);
		while(1)	// rb# interrupt status
		{
			state = inpw(REG_SMISR);
			if (state & 0x04)
			{
				outpw(REG_SMISR, (state & 0xfffffffb)|0x04);
				break;
			}

			if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
				return FMI_TIMEOUT;

			if (_fmi_bIsSMInsert == FALSE)
				return FMI_NO_SM_CARD;
		}
#endif

		tick = sysGetTicks(TIMER0);
		while(!(inpw(REG_SMISR) & 0x10))	// rb# status
		{
			if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
				return FMI_TIMEOUT;

			if (_fmi_bIsSMInsert == FALSE)
				return FMI_NO_SM_CARD;
		}
	}

	outpw(REG_SMCMD, 0x00);		// read command
	outpw(REG_SMMADDR, ucColAddr);	// CA0 - CA7
	outpw(REG_SMMADDR, uSector & 0xff);	// PA0 - PA7
	if (_fmi_uF4cycle == 0)
		outpw(REG_SMADDR, (uSector >> 8) & 0xff);		// PA8 - PA15
	else
	{
		outpw(REG_SMMADDR, (uSector >> 8) & 0xff);		// PA8 - PA15
		outpw(REG_SMADDR, (uSector >> 16) & 0xff);		// PA16 - PA17
	}

#ifdef _MP_
	tick = sysGetTicks(TIMER0);
	while(1)	// rb# interrupt status
	{
		state = inpw(REG_SMISR);
		if (state & 0x04)
		{
			outpw(REG_SMISR, (state & 0xfffffffb)|0x04);
			break;
		}
		if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
			return FMI_TIMEOUT;
		if (_fmi_bIsSMInsert == FALSE)
			return FMI_NO_SM_CARD;
	}
#endif

	tick = sysGetTicks(TIMER0);
	while(!(inpw(REG_SMISR) & 0x10))	// rb# status
	{
		if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
			return FMI_TIMEOUT;
		if (_fmi_bIsSMInsert == FALSE)
			return FMI_NO_SM_CARD;
	}

#ifdef _USE_IRQ
	_fmi_uSM_DataReady = 0;
#endif

	outpw(REG_SMCR, inpw(REG_SMCR) | 0x02);

	tick = sysGetTicks(TIMER0);
#ifdef _USE_IRQ
	while(_fmi_uSM_DataReady == 0)
	{
		if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
			return FMI_TIMEOUT;

		if (_fmi_bIsSMInsert == FALSE)
			return FMI_NO_SM_CARD;
	}
#else
	while(inpw(REG_SMCR) & 0x02)
	{
		if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
			return FMI_TIMEOUT;

		if (_fmi_bIsSMInsert == FALSE)
			return FMI_NO_SM_CARD;
	}
#endif

	return FMI_NO_ERR;
}


INT fmiBuffer2SMM(UINT32 uSector, UINT8 ucColAddr, UINT8 ncBufNo, UINT32 mark)
{
	UINT32 volatile temp;

	if (ncBufNo == 0)
	{
		outpw(REG_FMICR, (inpw(REG_FMICR) & 0xfffff8ff)|0x200);		// buffer0 to SM
		// set the spare area configuration
		outpw(REG_SMRA_0, mark);
		temp = (_fmi_ucBaseAddr2 << 24) | (_fmi_ucBaseAddr1 << 16) | 0xffff;
		outpw(REG_SMRA_1, temp);
		temp = (_fmi_ucBaseAddr1 << 24) | 0xffffff;
		outpw(REG_SMRA_2, temp);
		temp = 0xffffff00 | _fmi_ucBaseAddr2;
		outpw(REG_SMRA_3, temp);
	}
	else if (ncBufNo == 1)
	{
		outpw(REG_FMICR, (inpw(REG_FMICR) & 0xfffff8ff)|0x600);		// buffer1 to SM
		// set the spare area configuration
		outpw(REG_SMRA_4, mark);
		temp = (_fmi_ucBaseAddr2 << 24) | (_fmi_ucBaseAddr1 << 16) | 0xffff;
		outpw(REG_SMRA_5, temp);
		temp = (_fmi_ucBaseAddr1 << 24) | 0xffffff;
		outpw(REG_SMRA_6, temp);
		temp = 0xffffff00 | _fmi_ucBaseAddr2;
		outpw(REG_SMRA_7, temp);
	}

	// send command
	outpw(REG_SMCMD, 0x80);		// serial data input command
	outpw(REG_SMMADDR, ucColAddr);	// CA0 - CA7
	outpw(REG_SMMADDR, uSector & 0xff);	// PA0 - PA7
	if (_fmi_uF4cycle == 0)
		outpw(REG_SMADDR, (uSector >> 8) & 0xff);		// PA8 - PA15
	else
	{
		outpw(REG_SMMADDR, (uSector >> 8) & 0xff);		// PA8 - PA15
		outpw(REG_SMADDR, (uSector >> 16) & 0xff);		// PA16 - PA17
	}

	return FMI_NO_ERR;
}


INT fmiSM2BufferM(UINT32 uSector, UINT8 ucColAddr, UINT8 ncBufNo)
{
#ifdef _MP_
	unsigned int volatile state;
#endif
	unsigned int volatile tick;

	while(!(inpw(REG_SMISR) & 0x10));	// rb# status
	if (ncBufNo == 0)
		outpw(REG_FMICR, (inpw(REG_FMICR) & 0xffffff8f)|0x20);		// SM to buffer0
	if (ncBufNo == 1)
		outpw(REG_FMICR, (inpw(REG_FMICR) & 0xffffff8f)|0x60);		// SM to buffer1

	if (!_fmi_bIsSMDefaultCheck)
	{
#ifdef _MP_
		tick = sysGetTicks(TIMER0);
		while(1)	// rb# interrupt status
		{
			state = inpw(REG_SMISR);
			if (state & 0x04)
			{
				outpw(REG_SMISR, (state & 0xfffffffb)|0x04);
				break;
			}

			if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
				return FMI_TIMEOUT;

			if (_fmi_bIsSMInsert == FALSE)
				return FMI_NO_SM_CARD;
		}
#endif
		tick = sysGetTicks(TIMER0);
		while(!(inpw(REG_SMISR) & 0x10))	// rb# status
		{
			if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
				return FMI_TIMEOUT;

			if (_fmi_bIsSMInsert == FALSE)
				return FMI_NO_SM_CARD;
		}
	}

	// for test
	tick = sysGetTicks(TIMER0);
	while(!(inpw(REG_SMISR) & 0x10))	// rb# status
	{
		if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
			return FMI_TIMEOUT;
	}

	outpw(REG_SMCMD, 0x00);		// read command
	outpw(REG_SMMADDR, ucColAddr);	// CA0 - CA7
	outpw(REG_SMMADDR, uSector & 0xff);	// PA0 - PA7
	if (_fmi_uF4cycle == 0)
		outpw(REG_SMADDR, (uSector >> 8) & 0xff);		// PA8 - PA15
	else
	{
		outpw(REG_SMMADDR, (uSector >> 8) & 0xff);		// PA8 - PA15
		outpw(REG_SMADDR, (uSector >> 16) & 0xff);		// PA16 - PA17
	}

#ifdef _MP_
	tick = sysGetTicks(TIMER0);
	while(1)	// rb# interrupt status
	{
		state = inpw(REG_SMISR);
		if (state & 0x04)
		{
			outpw(REG_SMISR, (state & 0xfffffffb)|0x04);
			break;
		}
		if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
			return FMI_TIMEOUT;
		if (_fmi_bIsSMInsert == FALSE)
			return FMI_NO_SM_CARD;
	}
#endif
	tick = sysGetTicks(TIMER0);
	while(!(inpw(REG_SMISR) & 0x10))	// rb# status
	{
		if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
			return FMI_TIMEOUT;
		if (_fmi_bIsSMInsert == FALSE)
			return FMI_NO_SM_CARD;
	}
	return FMI_NO_ERR;
}


INT fmiSM_Read_512(UINT32 uSector, UINT32 uBufcnt, UINT32 uDAddr)
{
	int volatile status=0;
	unsigned int volatile tick;
	UINT32 count, rbuf, wbuf;
	UINT32 rSec, wSec, waddr;
	UINT8 *ptr;

	count = uBufcnt;
	rSec = uSector;
	wSec = 0;
	rbuf = wbuf = 0;

#ifdef _USE_IRQ
	_fmi_uSM_DataReady = 0;
#endif
	fmiSM2BufferM(rSec, 0, rbuf);
	outpw(REG_SMCR, inpw(REG_SMCR) | 0x02);
	rbuf = 1;
	rSec++;
	count--;
	tick = sysGetTicks(TIMER0);
#ifdef _USE_IRQ
	while(_fmi_uSM_DataReady == 0)
	{
		if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
			return FMI_TIMEOUT;

		if (_fmi_bIsSMInsert == FALSE)
			return FMI_NO_SM_CARD;
	}
#else
	while(inpw(REG_SMCR) & 0x02)
	{
		if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
			return FMI_TIMEOUT;

		if (_fmi_bIsSMInsert == FALSE)
			return FMI_NO_SM_CARD;
	}
#endif
	// check ECC
	if ((inpw(REG_SMISR) & 0x40) == 0)
	{
		ptr = (UINT8 *)REG_FB0_0;
		status = fmiSM_ECC_Correct(0, ptr);
		if (status != FMI_NO_ERR)
		{
#ifdef DEBUG
			printf("1.ECC check Fail [%d], status[0x%x]!\n", rSec-1, inpw(REG_SMISR));
#endif
			return FMI_SM_ECC_ERROR;
		}
	}

	while(count>0)
	{
		waddr = uDAddr + (wSec << 9);
		fmiSDRAM_Write(waddr, wbuf);
		fmiSM2BufferM(rSec, 0, rbuf);
#ifdef _USE_IRQ
		_fmi_uSM_DataReady = _fmi_uDataReady = 0;
#endif
		outpw(REG_SMCR, inpw(REG_SMCR) | 0x02);
		outpw(REG_FMICR, inpw(REG_FMICR) | 0x04);	// enable DMA
		tick = sysGetTicks(TIMER0);
#ifdef _USE_IRQ
		while((_fmi_uSM_DataReady == 0) || (_fmi_uDataReady == 0))
		{
			if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
				return FMI_TIMEOUT;

			if (_fmi_bIsSMInsert == FALSE)
				return FMI_NO_SM_CARD;
		}
#else
		while(inpw(REG_FMICR) & 0x04);
		while(inpw(REG_SMCR) & 0x02)
		{
			if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
				return FMI_TIMEOUT;

			if (_fmi_bIsSMInsert == FALSE)
				return FMI_NO_SM_CARD;
		}
#endif
		// check ECC
		if ((inpw(REG_SMISR) & 0x40) == 0)
		{
			if (rbuf == 0)
				ptr = (UINT8 *)REG_FB0_0;
			else
				ptr = (UINT8 *)REG_FB1_0;
			status = fmiSM_ECC_Correct(rbuf, ptr);
			if (status != FMI_NO_ERR)
			{
#ifdef DEBUG
				printf("2.ECC check Fail [%d], status[0x%x]!\n", rSec, inpw(REG_SMISR));
#endif
				return FMI_SM_ECC_ERROR;
			}
		}
		rSec++;
		wSec++;
		rbuf = ~rbuf & 0x01;
		wbuf = ~wbuf & 0x01;
		count--;
	}

	waddr = uDAddr + (wSec << 9);
	fmiSDRAM_Write(waddr, wbuf);
#ifdef _USE_IRQ
	_fmi_uDataReady = 0;
#endif
	outpw(REG_FMICR, inpw(REG_FMICR) | 0x04);	// enable DMA
#ifdef _USE_IRQ
	tick = sysGetTicks(TIMER0);
	while(_fmi_uDataReady == 0)
	{
		if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
			return FMI_TIMEOUT;
	}
#else
	while(inpw(REG_FMICR) & 0x04);
#endif

	return FMI_NO_ERR;
}


INT fmiSM_Write_512(UINT32 uSector, UINT32 uBufcnt, UINT32 uSAddr, UINT32 mark)
{
	unsigned int volatile tick;
	UINT32 count, rbuf, wbuf;
	UINT32 rSec, wSec, raddr;

	count = uBufcnt;
	rSec = 0;
	wSec = uSector;
	rbuf = wbuf = 0;

#ifdef _USE_IRQ
	_fmi_uDataReady = 0;
#endif
	raddr = uSAddr;
	fmiSDRAM_Read(raddr, rbuf);
	outpw(REG_FMICR, inpw(REG_FMICR) | 0x08);	// enable DMA
	rbuf = 1;
	rSec++;
	count--;

#ifdef _USE_IRQ
	tick = sysGetTicks(TIMER0);
	while(_fmi_uDataReady == 0)
	{
		if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
			return FMI_TIMEOUT;
	}
#else
	while(inpw(REG_FMICR) & 0x08);
#endif

	while(count>0)
	{
		if (!_fmi_bIsSMDefaultCheck)
		{
			tick = sysGetTicks(TIMER0);
			while(!(inpw(REG_SMISR) & 0x10))	// rb# status
			{
				if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
					return FMI_TIMEOUT;

				if (_fmi_bIsSMInsert == FALSE)
					return FMI_NO_SM_CARD;
			}

			outpw(REG_SMCMD, 0x70);		// status read command
			if (inpw(REG_SMDATA) & 0x01)	// 1:fail; 0:pass
			{
#ifdef DEBUG
				printf("1.data error [%d]!!\n", wSec);
#endif
				return FMI_SM_STATE_ERROR;
			}
		}

		raddr = uSAddr + (rSec << 9);
		fmiBuffer2SMM(wSec, 0, wbuf, mark);
		fmiSDRAM_Read(raddr, rbuf);

#ifdef _USE_IRQ
		_fmi_uSM_DataReady = _fmi_uDataReady = 0;
#endif
		outpw(REG_SMCR, inpw(REG_SMCR) | 0x01);
		outpw(REG_FMICR, inpw(REG_FMICR) | 0x08);	// enable DMA
		tick = sysGetTicks(TIMER0);
#ifdef _USE_IRQ
		while((_fmi_uSM_DataReady == 0) || (_fmi_uDataReady == 0))
		{
			if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
				return FMI_TIMEOUT;

			if (_fmi_bIsSMInsert == FALSE)
				return FMI_NO_SM_CARD;
		}
#else
		while(inpw(REG_FMICR) & 0x08);
		while(inpw(REG_SMCR) & 0x01)
		{
			if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
				return FMI_TIMEOUT;

			if (_fmi_bIsSMInsert == FALSE)
				return FMI_NO_SM_CARD;
		}
#endif
		outpw(REG_SMCMD, 0x10);		// auto program command

		if (_fmi_bIsSMDefaultCheck)
		{
			tick = sysGetTicks(TIMER0);
			while(!(inpw(REG_SMISR) & 0x10))	// rb# status
			{
				if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
					return FMI_TIMEOUT;

				if (_fmi_bIsSMInsert == FALSE)
					return FMI_NO_SM_CARD;
			}

			outpw(REG_SMCMD, 0x70);		// status read command
			if (inpw(REG_SMDATA) & 0x01)	// 1:fail; 0:pass
			{
#ifdef DEBUG
				printf("1.data error [%d]!!\n", wSec);
#endif
				return FMI_SM_STATE_ERROR;
			}
		}

		rSec++;
		wSec++;
		rbuf = ~rbuf & 0x01;
		wbuf = ~wbuf & 0x01;
		count--;
	}

	if (!_fmi_bIsSMDefaultCheck)
	{
		tick = sysGetTicks(TIMER0);
		while(!(inpw(REG_SMISR) & 0x10))	// rb# status
		{
			if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
				return FMI_TIMEOUT;

			if (_fmi_bIsSMInsert == FALSE)
				return FMI_NO_SM_CARD;
		}

		outpw(REG_SMCMD, 0x70);		// status read command
		if (inpw(REG_SMDATA) & 0x01)	// 1:fail; 0:pass
		{
#ifdef DEBUG
			printf("2.data error [%d]!!\n", wSec);
#endif
			return FMI_SM_STATE_ERROR;
		}
	}

	fmiBuffer2SMM(wSec, 0, wbuf, mark);
#ifdef _USE_IRQ
	_fmi_uSM_DataReady = 0;
#endif
	outpw(REG_SMCR, inpw(REG_SMCR) | 0x01);
	tick = sysGetTicks(TIMER0);
#ifdef _USE_IRQ
	while(_fmi_uSM_DataReady == 0)
	{
		if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
			return FMI_TIMEOUT;

		if (_fmi_bIsSMInsert == FALSE)
			return FMI_NO_SM_CARD;
	}
#else
	while(inpw(REG_SMCR) & 0x01)
	{
		if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
			return FMI_TIMEOUT;

		if (_fmi_bIsSMInsert == FALSE)
			return FMI_NO_SM_CARD;
	}
#endif
	outpw(REG_SMCMD, 0x10);		// auto program command

	if (_fmi_bIsSMDefaultCheck)
	{
		tick = sysGetTicks(TIMER0);
		while(!(inpw(REG_SMISR) & 0x10))	// rb# status
		{
			if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
				return FMI_TIMEOUT;

			if (_fmi_bIsSMInsert == FALSE)
				return FMI_NO_SM_CARD;
		}

		outpw(REG_SMCMD, 0x70);		// status read command
		if (inpw(REG_SMDATA) & 0x01)	// 1:fail; 0:pass
		{
#ifdef DEBUG
			printf("2.data error [%d]!!\n", wSec);
#endif
			return FMI_SM_STATE_ERROR;
		}
	}

	return FMI_NO_ERR;
}


INT fmiSM2BufferM_1(UINT32 uPage, UINT8 ucColumn, UINT8 ncBufNo)
{
#ifdef _MP_
	unsigned int volatile state;
#endif
	UINT32 colAddr = ucColumn * 512;
	//UINT32 volatile status;

	//status = fmiSM_Reset();
	//if (status != FMI_NO_ERR)
	//	return status;

	if (ncBufNo == 0)
		outpw(REG_FMICR, (inpw(REG_FMICR) & 0xffffff8f)|0x20);		// SM to buffer0
	if (ncBufNo == 1)
		outpw(REG_FMICR, (inpw(REG_FMICR) & 0xffffff8f)|0x60);		// SM to buffer1

	if (!_fmi_bIsSMDefaultCheck)
		while(!(inpw(REG_SMISR) & 0x10));	// rb# status

	// for test
	while(!(inpw(REG_SMISR) & 0x10));	// rb# status

	outpw(REG_SMCMD, 0x00);		// read command
	outpw(REG_SMMADDR, colAddr);	// CA0 - CA7
	outpw(REG_SMMADDR, (colAddr >> 8) & 0x0f);	// CA8 - CA11
	outpw(REG_SMMADDR, uPage & 0xff);	// PA0 - PA7
	if (_fmi_uF4cycle == 0)
		outpw(REG_SMADDR, (uPage >> 8) & 0xff);		// PA8 - PA15
	else
	{
		outpw(REG_SMMADDR, (uPage >> 8) & 0xff);		// PA8 - PA15
		outpw(REG_SMADDR, (uPage >> 16) & 0xff);		// PA16 - PA17
	}
	outpw(REG_SMCMD, 0x30);		// read command

#ifdef _MP_
	while(1)	// rb# interrupt status
	{
		state = inpw(REG_SMISR);
		if (state & 0x04)
		{
			outpw(REG_SMISR, (state & 0xfffffffb)|0x04);
			break;
		}
	}
#endif

	while(!(inpw(REG_SMISR) & 0x10));	// rb# status

	return FMI_NO_ERR;
}


VOID fmiSM_Read_1(UINT32 uPage, UINT32 uColumn, UINT32 uCount, UINT32 uDAddr)
{
	UINT32 cut, rbuf, wbuf;
	UINT32 rSec, wSec, waddr;

	cut = uCount;
	rSec = uColumn;
	wSec = 0;
	rbuf = wbuf = 0;

#ifdef _USE_IRQ
	_fmi_uSM_DataReady = 0;
#endif
	fmiSM2BufferM_1(uPage, rSec, rbuf);
	outpw(REG_SMCR, inpw(REG_SMCR) | 0x02);
	rbuf = 1;
	rSec++;
	cut--;
#ifdef _USE_IRQ
	while(_fmi_uSM_DataReady == 0);
#else
	while(inpw(REG_SMCR) & 0x02);
#endif

	while(cut>0)
	{
		waddr = uDAddr + (wSec << 9);
		fmiSDRAM_Write(waddr, wbuf);
		fmiSM2BufferM_1(uPage, rSec, rbuf);
#ifdef _USE_IRQ
		_fmi_uSM_DataReady = _fmi_uDataReady = 0;
#endif
		outpw(REG_SMCR, inpw(REG_SMCR) | 0x02);
		outpw(REG_FMICR, inpw(REG_FMICR) | 0x04);	// enable DMA
#ifdef _USE_IRQ
		while((_fmi_uSM_DataReady == 0) || (_fmi_uDataReady == 0));
#else
		while(inpw(REG_FMICR) & 0x04);
		while(inpw(REG_SMCR) & 0x02);
#endif
		rSec++;
		wSec++;
		rbuf = ~rbuf & 0x01;
		wbuf = ~wbuf & 0x01;
		cut--;
	}
	waddr = uDAddr + (wSec << 9);
	fmiSDRAM_Write(waddr, wbuf);
#ifdef _USE_IRQ
	_fmi_uDataReady = 0;
#endif
	outpw(REG_FMICR, inpw(REG_FMICR) | 0x04);	// enable DMA
#ifdef _USE_IRQ
	while(_fmi_uDataReady == 0);
#else
	while(inpw(REG_FMICR) & 0x04);
#endif
}


INT fmiSM_Read_2K(UINT32 uPage, UINT8 ucColAddr, UINT32 uDAddr)
{
#ifdef _MP_
	unsigned int volatile state, tick;
#endif
	UINT32 volatile status, ecc_ra;

	//status = fmiSM_Reset();
	//if (status != FMI_NO_ERR)
	//	return status;

	if (!_fmi_bIsSMDefaultCheck)
		while(!(inpw(REG_SMISR) & 0x10));	// rb# status

	outpw(REG_FMIDSA, uDAddr);	// set DMA transfer starting address
	outpw(REG_FMICR, (inpw(REG_FMICR) & 0xfffff8ff));	// buffer to sdram
	outpw(REG_FMICR, (inpw(REG_FMICR) & 0xffffff8f)|0x20);		// SM to buffer0

	// for test
	while(!(inpw(REG_SMISR) & 0x10));	// rb# status

	outpw(REG_SMCMD, 0x00);		// read command
	outpw(REG_SMMADDR, ucColAddr);	// CA0 - CA7
	outpw(REG_SMMADDR, (ucColAddr >> 8) & 0x0f);	// CA8 - CA11
	outpw(REG_SMMADDR, uPage & 0xff);	// PA0 - PA7
	if (_fmi_uF4cycle == 0)
		outpw(REG_SMADDR, (uPage >> 8) & 0xff);		// PA8 - PA15
	else
	{
		outpw(REG_SMMADDR, (uPage >> 8) & 0xff);		// PA8 - PA15
		outpw(REG_SMADDR, (uPage >> 16) & 0xff);		// PA16 - PA17
	}
	outpw(REG_SMCMD, 0x30);		// read command

#ifdef _MP_
	tick = sysGetTicks(TIMER0);
	while(1)	// rb# interrupt status
	{
		state = inpw(REG_SMISR);
		if (state & 0x04)
		{
			outpw(REG_SMISR, (state & 0xfffffffb)|0x04);
			break;
		}

		if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
			return FMI_TIMEOUT;

		if (_fmi_bIsSMInsert == FALSE)
			return FMI_NO_SM_CARD;
	}
#endif

	while(!(inpw(REG_SMISR) & 0x10));	// rb# status

#ifdef _USE_IRQ
	_fmi_uSM_DataReady = 0;
#endif

	outpw(REG_SMCR, inpw(REG_SMCR) | 0x02);

#ifdef _USE_IRQ
	while(_fmi_uSM_DataReady == 0);
#else
	while(inpw(REG_SMCR) & 0x02);
#endif

	// check ECC
	ecc_ra = ((inpw(REG_SMRA_3) << 8)|(inpw(REG_SMRA_2) >> 24)) & 0xffffff;
	if (inpw(REG_SMECC0) != ecc_ra)
	{
		status = fmiSM_ECC_Correct(0, (UINT8 *)uDAddr);
		if (status != FMI_NO_ERR)
		{
#ifdef DEBUG
			printf("Read_2K: area-1 ECC check Fail!!\n");
#endif
			return FMI_SM_ECC_ERROR;
		}
	}

	ecc_ra = ((inpw(REG_SMRA_7) << 8)|(inpw(REG_SMRA_6) >> 24)) & 0xffffff;
	if (inpw(REG_SMECC0) != ecc_ra)
	{
		status = fmiSM_ECC_Correct(1, (UINT8 *)uDAddr);
		if (status != FMI_NO_ERR)
		{
#ifdef DEBUG
			printf("Read_2K: area-2 ECC check Fail!!\n");
#endif
			return FMI_SM_ECC_ERROR;
		}
	}

	ecc_ra = ((inpw(REG_SMRA_11) << 8)|(inpw(REG_SMRA_10) >> 24)) & 0xffffff;
	if (inpw(REG_SMECC0) != ecc_ra)
	{
		status = fmiSM_ECC_Correct(2, (UINT8 *)uDAddr);
		if (status != FMI_NO_ERR)
		{
#ifdef DEBUG
			printf("Read_2K: area-3 ECC check Fail!!\n");
#endif
			return FMI_SM_ECC_ERROR;
		}
	}

	ecc_ra = ((inpw(REG_SMRA_15) << 8)|(inpw(REG_SMRA_14) >> 24)) & 0xffffff;
	if (inpw(REG_SMECC0) != ecc_ra)
	{
		status = fmiSM_ECC_Correct(3, (UINT8 *)uDAddr);
		if (status != FMI_NO_ERR)
		{
#ifdef DEBUG
			printf("Read_2K: area-4 ECC check Fail!!\n");
#endif
			return FMI_SM_ECC_ERROR;
		}
	}
	return FMI_NO_ERR;
}


INT fmiSM_Write_2K(UINT32 uSector, UINT32 ucColAddr, UINT32 uSAddr, UINT32 mark)
{
#ifdef _MP_
	unsigned int volatile state, tick;
#endif
	UINT32 volatile temp;

	outpw(REG_FMIDSA, uSAddr);	// set DMA transfer starting address
	outpw(REG_FMICR, (inpw(REG_FMICR) & 0xffffff8f));	// sdram to buffer
	outpw(REG_FMICR, (inpw(REG_FMICR) & 0xfffff8ff)|0x200);		// buffer0 to SM

	// set the spare area configuration
	/* set page used flag */
	//tmp = 0xfefe00 | 0x000000ff | (mark<<24);
	outpw(REG_SMRA_0, mark);
	/* set block address */
	temp = (_fmi_ucBaseAddr2 << 16) | (_fmi_ucBaseAddr1 << 8) | 0xff0000ff;
	outpw(REG_SMRA_2, temp);
	temp = (_fmi_ucBaseAddr2 << 24) | (_fmi_ucBaseAddr1 << 16) | 0xffff;
	outpw(REG_SMRA_3, temp);

	temp = (_fmi_ucBaseAddr2 << 16) | (_fmi_ucBaseAddr1 << 8) | 0xff0000ff;
	outpw(REG_SMRA_6, temp);
	temp = (_fmi_ucBaseAddr2 << 24) | (_fmi_ucBaseAddr1 << 16) | 0xffff;
	outpw(REG_SMRA_7, temp);

	temp = (_fmi_ucBaseAddr2 << 16) | (_fmi_ucBaseAddr1 << 8) | 0xff0000ff;
	outpw(REG_SMRA_10, temp);
	temp = (_fmi_ucBaseAddr2 << 24) | (_fmi_ucBaseAddr1 << 16) | 0xffff;
	outpw(REG_SMRA_11, temp);

	temp = (_fmi_ucBaseAddr2 << 16) | (_fmi_ucBaseAddr1 << 8) | 0xff0000ff;
	outpw(REG_SMRA_14, temp);
	temp = (_fmi_ucBaseAddr2 << 24) | (_fmi_ucBaseAddr1 << 16) | 0xffff;
	outpw(REG_SMRA_15, temp);

	if (!_fmi_bIsSMDefaultCheck)
	{
		while(!(inpw(REG_SMISR) & 0x10));	// rb# status

		outpw(REG_SMCMD, 0x70);		// status read command
		if (inpw(REG_SMDATA) & 0x01)	// 1:fail; 0:pass
		{
#ifdef DEBUG
			printf("SM_Write_2K: data error!!\n");
#endif
			return FMI_SM_STATE_ERROR;
		}
	}
	// send command
	outpw(REG_SMCMD, 0x80);		// serial data input command
	outpw(REG_SMMADDR, ucColAddr);	// CA0 - CA7
	outpw(REG_SMMADDR, (ucColAddr >> 8) & 0x0f);	// CA8 - CA11
	outpw(REG_SMMADDR, uSector & 0xff);	// PA0 - PA7
	if (_fmi_uF4cycle == 0)
		outpw(REG_SMADDR, (uSector >> 8) & 0xff);		// PA8 - PA15
	else
	{
		outpw(REG_SMMADDR, (uSector >> 8) & 0xff);		// PA8 - PA15
		outpw(REG_SMADDR, (uSector >> 16) & 0xff);		// PA16 - PA17
	}

#ifdef _USE_IRQ
	_fmi_uSM_DataReady = 0;
#endif

	outpw(REG_SMCR, inpw(REG_SMCR) | 0x01);

#ifdef _USE_IRQ
	while(_fmi_uSM_DataReady == 0);
#else
	while(inpw(REG_SMCR) & 0x01);
#endif

	outpw(REG_SMCMD, 0x10);		// auto program command
	if (_fmi_bIsSMDefaultCheck)
	{
#ifdef _MP_
		tick = sysGetTicks(TIMER0);
		while(1)	// rb# interrupt status
		{
			state = inpw(REG_SMISR);
			if (state & 0x04)
			{
				outpw(REG_SMISR, (state & 0xfffffffb)|0x04);
				break;
			}

			if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
				return FMI_TIMEOUT;

			if (_fmi_bIsSMInsert == FALSE)
				return FMI_NO_SM_CARD;
		}
#endif
		while(!(inpw(REG_SMISR) & 0x10));	// rb# status

		outpw(REG_SMCMD, 0x70);		// status read command
		if (inpw(REG_SMDATA) & 0x01)	// 1:fail; 0:pass
		{
#ifdef DEBUG
			printf("SM_Write_2K: data error!!\n");
#endif
			return FMI_SM_STATE_ERROR;
		}
	}

	return FMI_NO_ERR;
}


INT fmiSM_SetMarkFlag(UINT32 uSector, UINT8 ncData)
{
	UINT32 ucColAddr, status;

	//status = fmiSM_Reset();
	//if (status != FMI_NO_ERR)
	//	return status;

	if (!_fmi_bIsSMDefaultCheck)
	{
		while(!(inpw(REG_SMISR) & 0x10));	// rb# status

		outpw(REG_SMCMD, 0x70);		// status read command
		if (inpw(REG_SMDATA) & 0x01)	// 1:fail; 0:pass
		{
#ifdef DEBUG
			printf("fmiSM_SetMarkFlag: data error!!\n");
#endif
			return FMI_SM_STATE_ERROR;
		}
	}

	if (_fmi_u2KPageSize)
	{
		// send command
		ucColAddr = 2049;
		outpw(REG_SMCMD, 0x80);		// serial data input command
		outpw(REG_SMMADDR, ucColAddr);	// CA0 - CA7
		outpw(REG_SMMADDR, (ucColAddr >> 8) & 0x0f);	// CA8 - CA11
		outpw(REG_SMMADDR, uSector & 0xff);	// PA0 - PA7
		if (_fmi_uF4cycle == 0)
			outpw(REG_SMADDR, (uSector >> 8) & 0xff);		// PA8 - PA15
		else
		{
			outpw(REG_SMMADDR, (uSector >> 8) & 0xff);		// PA8 - PA15
			outpw(REG_SMADDR, (uSector >> 16) & 0xff);		// PA16 - PA17
		}
	}
	else
	{
		ucColAddr = 1;
		outpw(REG_SMCMD, 0x50);		// read RA command
		outpw(REG_SMCMD, 0x80);		// serial data input command
		outpw(REG_SMMADDR, ucColAddr);	// CA0 - CA7
		outpw(REG_SMMADDR, uSector & 0xff);	// PA0 - PA7
		if (_fmi_uF4cycle == 0)
			outpw(REG_SMADDR, (uSector >> 8) & 0xff);		// PA8 - PA15
		else
		{
			outpw(REG_SMMADDR, (uSector >> 8) & 0xff);		// PA8 - PA15
			outpw(REG_SMADDR, (uSector >> 16) & 0xff);		// PA16 - PA17
		}
	}

	/* mark data */
	outpw(REG_SMDATA, ncData);
	outpw(REG_SMDATA, ncData);

	outpw(REG_SMCMD, 0x10);		// auto program command

	if (_fmi_bIsSMDefaultCheck)
	{
		while(!(inpw(REG_SMISR) & 0x10));	// rb# status

		outpw(REG_SMCMD, 0x70);		// status read command
		if (inpw(REG_SMDATA) & 0x01)	// 1:fail; 0:pass
		{
#ifdef DEBUG
			printf("fmiSM_SetMarkFlag: data error!!\n");
#endif
			return FMI_SM_STATE_ERROR;
		}
	}

	status = fmiSM_Reset();
	if (status != FMI_NO_ERR)
		return status;

	return FMI_NO_ERR;
}

INT fmiSML2PWriteProtect(UINT32 uSector, UINT32 data)
{
	UINT32 ucColAddr, status;

	//status = fmiSM_Reset();
	//if (status != FMI_NO_ERR)
	//	return status;

	if (!_fmi_bIsSMDefaultCheck)
	{
		while(!(inpw(REG_SMISR) & 0x10));	// rb# status

		outpw(REG_SMCMD, 0x70);		// status read command
		if (inpw(REG_SMDATA) & 0x01)	// 1:fail; 0:pass
		{
#ifdef DEBUG
			printf("fmiSML2PWriteProtect: data error!!\n");
#endif
			return FMI_SM_STATE_ERROR;
		}
	}

	if (_fmi_u2KPageSize)
	{
		// send command
		ucColAddr = 2051;
		outpw(REG_SMCMD, 0x80);		// serial data input command
		outpw(REG_SMMADDR, ucColAddr);	// CA0 - CA7
		outpw(REG_SMMADDR, (ucColAddr >> 8) & 0x0f);	// CA8 - CA11
		outpw(REG_SMMADDR, uSector & 0xff);	// PA0 - PA7
		if (_fmi_uF4cycle == 0)
			outpw(REG_SMADDR, (uSector >> 8) & 0xff);		// PA8 - PA15
		else
		{
			outpw(REG_SMMADDR, (uSector >> 8) & 0xff);		// PA8 - PA15
			outpw(REG_SMADDR, (uSector >> 16) & 0xff);		// PA16 - PA17
		}
	}
	else
	{
		ucColAddr = 3;
		outpw(REG_SMCMD, 0x50);		// read RA command
		outpw(REG_SMCMD, 0x80);		// serial data input command
		outpw(REG_SMMADDR, ucColAddr);	// CA0 - CA7
		outpw(REG_SMMADDR, uSector & 0xff);	// PA0 - PA7
		if (_fmi_uF4cycle == 0)
			outpw(REG_SMADDR, (uSector >> 8) & 0xff);		// PA8 - PA15
		else
		{
			outpw(REG_SMMADDR, (uSector >> 8) & 0xff);		// PA8 - PA15
			outpw(REG_SMADDR, (uSector >> 16) & 0xff);		// PA16 - PA17
		}
	}

	/* mark data */
	outpw(REG_SMDATA, data);	// block update finish, bit7,6 should be 0

	outpw(REG_SMCMD, 0x10);		// auto program command

	if (_fmi_bIsSMDefaultCheck)
	{
		while(!(inpw(REG_SMISR) & 0x10));	// rb# status

		outpw(REG_SMCMD, 0x70);		// status read command
		if (inpw(REG_SMDATA) & 0x01)	// 1:fail; 0:pass
		{
#ifdef DEBUG
			printf("fmiSML2PWriteProtect: data error!!\n");
#endif
			return FMI_SM_STATE_ERROR;
		}
	}

	status = fmiSM_Reset();
	if (status != FMI_NO_ERR)
		return status;

	return FMI_NO_ERR;
}


INT fmiSM_WritePage0(UINT32 uSector, UINT32 data)
{
	UINT32 ucColAddr, status;

	//status = fmiSM_Reset();
	//if (status != FMI_NO_ERR)
	//	return status;

	if (!_fmi_bIsSMDefaultCheck)
	{
		while(!(inpw(REG_SMISR) & 0x10));	// rb# status

		outpw(REG_SMCMD, 0x70);		// status read command
		if (inpw(REG_SMDATA) & 0x01)	// 1:fail; 0:pass
		{
#ifdef DEBUG
			printf("fmiSM_WritePage0: data error!!\n");
#endif
			return FMI_SM_STATE_ERROR;
		}
	}

	if (_fmi_u2KPageSize)
	{
		// send command
		ucColAddr = 2048;
		outpw(REG_SMCMD, 0x80);		// serial data input command
		outpw(REG_SMMADDR, ucColAddr);	// CA0 - CA7
		outpw(REG_SMMADDR, (ucColAddr >> 8) & 0x0f);	// CA8 - CA11
		outpw(REG_SMMADDR, uSector & 0xff);	// PA0 - PA7
		if (_fmi_uF4cycle == 0)
			outpw(REG_SMADDR, (uSector >> 8) & 0xff);		// PA8 - PA15
		else
		{
			outpw(REG_SMMADDR, (uSector >> 8) & 0xff);		// PA8 - PA15
			outpw(REG_SMADDR, (uSector >> 16) & 0xff);		// PA16 - PA17
		}

		// data
		outpw(REG_SMDATA, 0xff);	// 2048
		outpw(REG_SMDATA, (data >> 8) & 0xff);	// 2049
		outpw(REG_SMDATA, (data >> 16) & 0xff);	// 2050
		outpw(REG_SMDATA, (data >> 24) & 0xff);	// 2051
		outpw(REG_SMDATA, 0xff);	// 2052
		outpw(REG_SMDATA, 0xff);	// 2053
		outpw(REG_SMDATA, 0xff);	// 2054
		outpw(REG_SMDATA, 0xff);	// 2055
		outpw(REG_SMDATA, 0xff);	// 2056
	}
	else
	{
		ucColAddr = 0;
		outpw(REG_SMCMD, 0x50);		// read RA command
		outpw(REG_SMCMD, 0x80);		// serial data input command
		outpw(REG_SMMADDR, ucColAddr);	// CA0 - CA7
		outpw(REG_SMMADDR, uSector & 0xff);	// PA0 - PA7
		if (_fmi_uF4cycle == 0)
			outpw(REG_SMADDR, (uSector >> 8) & 0xff);		// PA8 - PA15
		else
		{
			outpw(REG_SMMADDR, (uSector >> 8) & 0xff);		// PA8 - PA15
			outpw(REG_SMADDR, (uSector >> 16) & 0xff);		// PA16 - PA17
		}

		// data
		outpw(REG_SMDATA, data & 0xff);	// 512
		outpw(REG_SMDATA, (data >> 8) & 0xff);	// 513
		outpw(REG_SMDATA, (data >> 16) & 0xff);	// 514
		outpw(REG_SMDATA, (data >> 24) & 0xff);	// 515
		outpw(REG_SMDATA, 0xff);	// 516
		outpw(REG_SMDATA, 0xff);	// 517
	}

	/* mark data */
	outpw(REG_SMDATA, _fmi_ucBaseAddr1);
	outpw(REG_SMDATA, _fmi_ucBaseAddr2);

	outpw(REG_SMCMD, 0x10);		// auto program command

	if (_fmi_bIsSMDefaultCheck)
	{
		while(!(inpw(REG_SMISR) & 0x10));	// rb# status

		outpw(REG_SMCMD, 0x70);		// status read command
		if (inpw(REG_SMDATA) & 0x01)	// 1:fail; 0:pass
		{
#ifdef DEBUG
			printf("fmiSM_WritePage0: data error!!\n");
#endif
			return FMI_SM_STATE_ERROR;
		}
	}

	status = fmiSM_Reset();
	if (status != FMI_NO_ERR)
		return status;

	return FMI_NO_ERR;
}


INT fmiSM_Write_2K_CB(UINT32 uSpage, UINT32 uDpage)
{
//	INT volatile status;
//	status = fmiSM_Reset();
//	if (status != FMI_NO_ERR)
//		return status;

	if (!_fmi_bIsSMDefaultCheck)
	{
		while(!(inpw(REG_SMISR) & 0x10));	// rb# status

		outpw(REG_SMCMD, 0x70);		// status read command
		if (inpw(REG_SMDATA) & 0x01)	// 1:fail; 0:pass
		{
#ifdef DEBUG
			printf("SM_Write_2K_CB: data error!!\n");
#endif
			return FMI_SM_STATE_ERROR;
		}
	}

	// copy-back command
	outpw(REG_SMCMD, 0x00);		// read command
	outpw(REG_SMMADDR, 0);	// CA0 - CA7
	outpw(REG_SMMADDR, 0);	// CA8 - CA11
	outpw(REG_SMMADDR, uSpage & 0xff);	// PA0 - PA7
	if (_fmi_uF4cycle == 0)
		outpw(REG_SMADDR, (uSpage >> 8) & 0xff);		// PA8 - PA15
	else
	{
		outpw(REG_SMMADDR, (uSpage >> 8) & 0xff);		// PA8 - PA15
		outpw(REG_SMADDR, (uSpage >> 16) & 0xff);		// PA16 - PA17
	}
	outpw(REG_SMCMD, 0x35);		// read command
	while(!(inpw(REG_SMISR) & 0x10));	// rb# status

	// send command
	outpw(REG_SMCMD, 0x85);		// serial data input command
	outpw(REG_SMMADDR, 0);	// CA0 - CA7
	outpw(REG_SMMADDR, 0);	// CA8 - CA11
	outpw(REG_SMMADDR, uDpage & 0xff);	// PA0 - PA7
	if (_fmi_uF4cycle == 0)
		outpw(REG_SMADDR, (uDpage >> 8) & 0xff);		// PA8 - PA15
	else
	{
		outpw(REG_SMMADDR, (uDpage >> 8) & 0xff);		// PA8 - PA15
		outpw(REG_SMADDR, (uDpage >> 16) & 0xff);		// PA16 - PA17
	}
	outpw(REG_SMCMD, 0x10);		// auto program command

	if (_fmi_bIsSMDefaultCheck)
	{
		while(!(inpw(REG_SMISR) & 0x10));	// rb# status

		outpw(REG_SMCMD, 0x70);		// status read command
		if (inpw(REG_SMDATA) & 0x01)	// 1:fail; 0:pass
		{
#ifdef DEBUG
			printf("SM_Write_2K_CB: data error!!\n");
#endif
			return FMI_SM_STATE_ERROR;
		}
	}
	return FMI_NO_ERR;
}


INT fmiCIS_Write(UINT32 uPage)
{
	unsigned int volatile tick;

	_fmi_pCIS = (UINT8 *)((UINT32)_fmi_CIS | 0x10000000);

	if (!_fmi_bIsSMDefaultCheck)
	{
		tick = sysGetTicks(TIMER0);
		while(!(inpw(REG_SMISR) & 0x10))	// rb# status
		{
			if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
				return FMI_TIMEOUT;

			if (_fmi_bIsSMInsert == FALSE)
				return FMI_NO_SM_CARD;
		}

		outpw(REG_SMCMD, 0x70);		// status read command
		if (inpw(REG_SMDATA) & 0x01)	// 1:fail; 0:pass
		{
#ifdef DEBUG
			printf("write CIS fail!!\n");
#endif
			return FMI_SM_STATE_ERROR;
		}
	}

	// set the spare area configuration
	if (_fmi_u2KPageSize == 0)
	{
		fmiSDRAM2Buffer((UINT32)_fmi_pCIS, 0);
		outpw(REG_FMICR, (inpw(REG_FMICR) & 0xfffff8ff)|0x200);		// buffer0 to SM

		outpw(REG_SMRA_0, 0xfffefeff);
		outpw(REG_SMRA_1, 0x0000ffff);
		outpw(REG_SMRA_2, 0x00ffffff);
		outpw(REG_SMRA_3, 0xffffff00);

		// send command
		outpw(REG_SMCMD, 0x80);		// serial data input command
		outpw(REG_SMMADDR, 0x00);	// CA0 - CA7
		outpw(REG_SMMADDR, uPage & 0xff);	// PA0 - PA7
		if (_fmi_uF4cycle == 0)
			outpw(REG_SMADDR, (uPage >> 8) & 0xff);		// PA8 - PA15
		else
		{
			outpw(REG_SMMADDR, (uPage >> 8) & 0xff);		// PA8 - PA15
			outpw(REG_SMADDR, (uPage >> 16) & 0xff);		// PA16 - PA17
		}
	}
	else
	{
		outpw(REG_FMIDSA, (UINT32)_fmi_pCIS);	// set DMA transfer starting address
		outpw(REG_FMICR, (inpw(REG_FMICR) & 0xffffff8f));	// sdram to buffer
		outpw(REG_FMICR, (inpw(REG_FMICR) & 0xfffff8ff)|0x200);		// buffer0 to SM

		outpw(REG_SMRA_0, 0xfffefeff);
		outpw(REG_SMRA_2, 0xff0000ff);
		outpw(REG_SMRA_3, 0xffff);
		outpw(REG_SMRA_6, 0xff0000ff);
		outpw(REG_SMRA_7, 0xffff);

		// send command
		outpw(REG_SMCMD, 0x80);		// serial data input command
		outpw(REG_SMMADDR, 0x00);	// CA0 - CA7
		outpw(REG_SMMADDR, 0x00);
		outpw(REG_SMMADDR, (uPage & 0xff));
		if (_fmi_uF4cycle == 0)
			outpw(REG_SMADDR, (uPage >> 8) & 0xff);		// PA8 - PA15
		else
		{
			outpw(REG_SMMADDR, (uPage >> 8) & 0xff);		// PA8 - PA15
			outpw(REG_SMADDR, (uPage >> 16) & 0xff);		// PA16 - PA17
		}
	}

#ifdef _USE_IRQ
	_fmi_uSM_DataReady = 0;
#endif

	outpw(REG_SMCR, inpw(REG_SMCR) | 0x01);

	tick = sysGetTicks(TIMER0);
#ifdef _USE_IRQ
	while(_fmi_uSM_DataReady == 0)
	{
		if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
			return FMI_TIMEOUT;

		if (_fmi_bIsSMInsert == FALSE)
			return FMI_NO_SM_CARD;
	}
#else
	while(inpw(REG_SMCR) & 0x01)
	{
		if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
			return FMI_TIMEOUT;

		if (_fmi_bIsSMInsert == FALSE)
			return FMI_NO_SM_CARD;
	}
#endif

	outpw(REG_SMCMD, 0x10);		// auto program command

	if (_fmi_bIsSMDefaultCheck)
	{
		tick = sysGetTicks(TIMER0);
		while(!(inpw(REG_SMISR) & 0x10))	// rb# status
		{
			if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
				return FMI_TIMEOUT;

			if (_fmi_bIsSMInsert == FALSE)
				return FMI_NO_SM_CARD;
		}

		outpw(REG_SMCMD, 0x70);		// status read command
		if (inpw(REG_SMDATA) & 0x01)	// 1:fail; 0:pass
		{
#ifdef DEBUG
			printf("write CIS fail!!\n");
#endif
			return FMI_SM_STATE_ERROR;
		}
	}

	return FMI_NO_ERR;
}

extern UINT32 _fmi_uFirst_L2P;
VOID fmiSM_ChipErase(VOID)
{
	UINT32 i, status;

#ifdef _USE_TWO_NAND_
	UINT32 volatile uEraseBlock;

	// multi PAD control
	outpw(0x7ff00020, (inpw(0x7ff00020)&0x9fffffff)|0x80000000);

	if (_fmi_bIs2NAND)
	{
		uEraseBlock = _fmi_uSM_2NANDZone * 1024;
		// erase all chip CS0
		fmiSM_Initial(0);
		for (i=0; i<uEraseBlock; i++)
		{
			status = fmiSM_BlockErase(i);
#ifdef DEBUG
			if (status != FMI_NO_ERR)
				printf("CS0 SM block erase fail <%d>!!\n", i);
#endif
		}
		fmiCIS_Write(0);

		uEraseBlock = _fmi_uBlockPerFlash - uEraseBlock + 1;
		// erase all chip CS1
		fmiSM_Initial(1);
		for (i=0; i<=uEraseBlock; i++)
		{
			status = fmiSM_BlockErase(i);
#ifdef DEBUG
			if (status != FMI_NO_ERR)
				printf("CS1 SM block erase fail <%d>!!\n", i);
#endif
		}
	}
	else
	{
		fmiSM_Initial(0);
		// erase all chip
		for (i=0; i<=_fmi_uBlockPerFlash; i++)
		{
			status = fmiSM_BlockErase(i);
#ifdef DEBUG
			if (status != FMI_NO_ERR)
				printf("status[0x%x] SM block erase fail <%d>!!\n", status, i);
#endif
		}
		fmiCIS_Write(0);
	}

#else // one NAND

	// multi PAD control
	outpw(0x7ff00020, (inpw(0x7ff00020)&0x9fffffff)|0x80000000);

	fmiSM_Initial(0);
	// erase all chip
	for (i=0; i<=_fmi_uBlockPerFlash; i++)
	{
		status = fmiSM_BlockErase(i);
#ifdef DEBUG
		if (status != FMI_NO_ERR)
			printf("status[0x%x] SM block erase fail <%d>!!\n", status, i);
#endif
	}
	fmiCIS_Write(0);

#endif // two NAND

	_fmi_uFirst_L2P = 0;
	fmiSM_L2PTable_Init(0);
}


VOID fmiGet_SM_info(DISK_DATA_T *_info)
{
//	_info->diskSize =  _fmi_uSectorPerFlash * 512 / 1024;
	_info->diskSize =  _fmi_uSectorPerFlash >> 1;
	_info->sectorSize = 512;
	_info->totalSectorN = _fmi_uSectorPerFlash;

	_info->vendor[0] = '\0';
	_info->product[0] = '\0';
	_info->serial[0] = '\0';
}

VOID fmiSMDefaultCheck(UINT32 state)
{
	_fmi_bIsSMDefaultCheck = state;
}

INT fmiCheckSMAvailable(UINT32 uChipSel)
{
	outpw(REG_FMICR, 0x01);		// enable FMI
	// [2004/10/27] modify multi PAD control
	outpw(0x7ff00020, (inpw(0x7ff00020)&0x9fffffff)|0x80000000);

	_fmi_bIsSMInsert = TRUE;	// card insert
	_fmi_bIsCheckNAND = TRUE;
	if (fmiSM_ReadID(uChipSel) != FMI_NO_ERR)
	{
		_fmi_bIsCheckNAND = FALSE;
		return FALSE;
	}
	_fmi_bIsCheckNAND = FALSE;
	return TRUE;
}

INT fmiSetReservedAreaSizebyKB(UINT32 uSize)
{
	unsigned int volatile tick, status, uPage=4;
	unsigned int *ptr;

	// [2004/10/27] modify multi PAD control
	outpw(0x7ff00020, (inpw(0x7ff00020)&0x9fffffff)|0x80000000);

	_fmi_pSMBuffer = (UINT8 *)((UINT32)_fmi_ucSMBuffer | 0x10000000);
	ptr = (unsigned int *)_fmi_ucSMBuffer;

	_fmi_uSectorPerFlash = 0;
	_fmi_uBlockPerFlash = 0;

	_fmi_bIsSMInsert = TRUE;	// card insert
	if ((status = fmiSM_ReadID(0)) != FMI_NO_ERR)
		return status;

	// check the NAND size, reserved area must less than NAND size
	if (_fmi_bIsReserved == TRUE)
	{
		if ((_fmi_uSectorPerFlash + _fmi_uSMReservedSector) < (uSize << 1))
			return FMI_SM_OVER_NAND_SIZE;
	}
	else
	{
		if (_fmi_uSectorPerFlash < (uSize << 1))
			return FMI_SM_OVER_NAND_SIZE;
	}

	// erase all Nand flash
	// mark for USB upgrade
	//fmiSM_ChipErase();

	_fmi_uSMReservedAreaSize = uSize;
	_fmi_uSMReservedSector = uSize * 2;	/* (uSize * 1024 / 512) */
	if (_fmi_u2KPageSize)
		_fmi_uSMReservedBlock = _fmi_uSMReservedSector / 256;
	else
		_fmi_uSMReservedBlock = _fmi_uSMReservedSector / 32;

#ifdef DEBUG
	printf("set reserved area: size[%d]KB, block[%d], sector[%d]\n", _fmi_uSMReservedAreaSize,
	_fmi_uSMReservedBlock, _fmi_uSMReservedSector);
#endif

	status = fmiSM_Reset();
	if (status != FMI_NO_ERR)
		return status;

	if (!_fmi_bIsSMDefaultCheck)
	{
		tick = sysGetTicks(TIMER0);
		while(!(inpw(REG_SMISR) & 0x10))	// rb# status
		{
			if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
				return FMI_TIMEOUT;

			if (_fmi_bIsSMInsert == FALSE)
				return FMI_NO_SM_CARD;
		}

		outpw(REG_SMCMD, 0x70);		// status read command
		if (inpw(REG_SMDATA) & 0x01)	// 1:fail; 0:pass
		{
#ifdef DEBUG
			printf("write reserved area fail!!\n");
#endif
			return FMI_SM_RA_ERROR;
		}
	}

	// set the spare area configuration
	if (_fmi_u2KPageSize == 0)	// 512
	{
		memset(_fmi_pSMBuffer, 0xff, 512);
		*(ptr+0) = 0x574255aa;
		*(ptr+1) = uSize;
		*(ptr+2) = _fmi_uSMReservedSector;
		*(ptr+3) = _fmi_uSMReservedBlock;
		*(ptr+4) = _fmi_bIsPartitionDisk;
		*(ptr+5) = 0x57429702;

		fmiSDRAM2Buffer((UINT32)_fmi_pSMBuffer, 0);
		outpw(REG_FMICR, (inpw(REG_FMICR) & 0xfffff8ff)|0x200);		// buffer0 to SM

		outpw(REG_SMRA_0, 0xfffefeff);
		outpw(REG_SMRA_1, 0x0000ffff);
		outpw(REG_SMRA_2, 0x00ffffff);
		outpw(REG_SMRA_3, 0xffffff00);

		// send command
		outpw(REG_SMCMD, 0x80);		// serial data input command
		outpw(REG_SMMADDR, 0x00);	// CA0 - CA7
		outpw(REG_SMMADDR, uPage & 0xff);	// PA0 - PA7
		if (_fmi_uF4cycle == 0)
			outpw(REG_SMADDR, (uPage >> 8) & 0xff);		// PA8 - PA15
		else
		{
			outpw(REG_SMMADDR, (uPage >> 8) & 0xff);		// PA8 - PA15
			outpw(REG_SMADDR, (uPage >> 16) & 0xff);		// PA16 - PA17
		}
	}
	else	// 2048
	{
		memset(_fmi_pSMBuffer, 0xff, 2048);
		*(ptr+0) = 0x574255aa;
		*(ptr+1) = uSize;
		*(ptr+2) = _fmi_uSMReservedSector;
		*(ptr+3) = _fmi_uSMReservedBlock;
		*(ptr+4) = _fmi_bIsPartitionDisk;
		*(ptr+5) = 0x57429702;

		outpw(REG_FMIDSA, (UINT32)_fmi_pSMBuffer);	// set DMA transfer starting address
		outpw(REG_FMICR, (inpw(REG_FMICR) & 0xffffff8f));	// sdram to buffer
		outpw(REG_FMICR, (inpw(REG_FMICR) & 0xfffff8ff)|0x200);		// buffer0 to SM

		outpw(REG_SMRA_0, 0xfffefeff);
		outpw(REG_SMRA_2, 0xff0000ff);
		outpw(REG_SMRA_3, 0x0000ffff);
		outpw(REG_SMRA_6, 0xff0000ff);
		outpw(REG_SMRA_7, 0x0000ffff);

		// send command
		outpw(REG_SMCMD, 0x80);		// serial data input command
		outpw(REG_SMMADDR, 0x00);	// CA0 - CA7
		outpw(REG_SMMADDR, 0x00);
		outpw(REG_SMMADDR, (uPage & 0xff));
		if (_fmi_uF4cycle == 0)
			outpw(REG_SMADDR, (uPage >> 8) & 0xff);		// PA8 - PA15
		else
		{
			outpw(REG_SMMADDR, (uPage >> 8) & 0xff);		// PA8 - PA15
			outpw(REG_SMADDR, (uPage >> 16) & 0xff);		// PA16 - PA17
		}
	}

#ifdef _USE_IRQ
	_fmi_uSM_DataReady = 0;
#endif

	outpw(REG_SMCR, inpw(REG_SMCR) | 0x01);

	tick = sysGetTicks(TIMER0);
#ifdef _USE_IRQ
	while(_fmi_uSM_DataReady == 0)
	{
		if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
			return FMI_TIMEOUT;

		if (_fmi_bIsSMInsert == FALSE)
			return FMI_NO_SM_CARD;
	}
#else
	while(inpw(REG_SMCR) & 0x01)
	{
		if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
			return FMI_TIMEOUT;

		if (_fmi_bIsSMInsert == FALSE)
			return FMI_NO_SM_CARD;
	}
#endif

	outpw(REG_SMCMD, 0x10);		// auto program command


	if (_fmi_bIsSMDefaultCheck)
	{
		tick = sysGetTicks(TIMER0);
		while(!(inpw(REG_SMISR) & 0x10))	// rb# status
		{
			if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
				return FMI_TIMEOUT;

			if (_fmi_bIsSMInsert == FALSE)
				return FMI_NO_SM_CARD;
		}

		outpw(REG_SMCMD, 0x70);		// status read command
		if (inpw(REG_SMDATA) & 0x01)	// 1:fail; 0:pass
		{
#ifdef DEBUG
			printf("write reserved area fail!!\n");
#endif
			return FMI_SM_RA_ERROR;
		}
	}

	_fmi_uSectorPerFlash = 0;
	_fmi_uBlockPerFlash = 0;

	return FMI_NO_ERR;
}

BOOL fmiCheckReservedArea(UINT32 uChipSel)
{
	UINT32 volatile bmark, emark;

	fmiCheckSMAvailable(uChipSel);
	fmiSM_Initial(uChipSel);
	if (_fmi_u2KPageSize == 0)
		fmiSM2Buffer(4, 0, 0);
	else
	{
		outpw(REG_SMCR, inpw(REG_SMCR) & 0xffffff7f);	// set page size = 512 bytes
#ifdef _USE_IRQ
		_fmi_uSM_DataReady = 0;
#endif
		fmiSM2BufferM_1(4, 0, 0);
		outpw(REG_SMCR, inpw(REG_SMCR) | 0x02);
#ifdef _USE_IRQ
		while(_fmi_uSM_DataReady == 0);
#else
		while(inpw(REG_SMCR) & 0x02);
#endif
		outpw(REG_SMCR, (inpw(REG_SMCR) & 0xffffff7f) | 0x80);	// set page size = 2048 bytes
	}

	bmark = inpw(REG_FB0_0);
	emark = inpw(REG_FB0_0+0x14);

//printf("bmark[%x], emark[%x]\n", bmark, emark);
	if ((bmark == 0x574255aa) && (emark == 0x57429702))
	{
		_fmi_uSMReservedAreaSize = inpw(REG_FB0_0+0x4);
		_fmi_uSMReservedSector = inpw(REG_FB0_0+0x8);
		_fmi_uSMReservedBlock = inpw(REG_FB0_0+0xc);
		_fmi_bIs2Disk = inpw(REG_FB0_0+0x10);

#ifdef DEBUG
	printf("check reserved area: size[%d]KB, block[%d], sector[%d], Is2Disk[%d]\n", _fmi_uSMReservedAreaSize,
	_fmi_uSMReservedBlock, _fmi_uSMReservedSector, _fmi_bIs2Disk);
#endif

		return TRUE;
	}

	return FALSE;
}

UINT32 fmiGetReservedAreaSizebyKB(VOID)
{
	fmiInitReservedArea();

	return _fmi_uSMReservedAreaSize;
}

INT fmiReadSMID(UINT32 uChipSel)
{
	fmiCheckSMAvailable(uChipSel);
	return _fmi_uSM_ID;
}

INT fmiSetSMPartitionSizebyKB(UINT32 uSize)
{
	INT volatile status=0;

	_fmi_bIsPartitionDisk = TRUE;
	status = fmiSetReservedAreaSizebyKB(uSize);
	if (status != FMI_NO_ERR)
		return status;

	return FMI_NO_ERR;
}

INT fmiSetSerialNumber(char *pString)
{
	unsigned int volatile status, uPage=2;
	unsigned char *ptr;
	int volatile i;

	// [2004/10/27] modify multi PAD control
	outpw(0x7ff00020, (inpw(0x7ff00020)&0x9fffffff)|0x80000000);

	ptr = _fmi_pSMBuffer = (UINT8 *)((UINT32)_fmi_ucSMBuffer | 0x10000000);

	if (_fmi_u2KPageSize == 0)	// 512
	{
		status = fmiSM_Read_512(0, _fmi_uPagePerBlock, (UINT32)_fmi_pSMBuffer);
		ptr = ptr + uPage * 512;
		if (*ptr != 0xff)
			fmiSM_BlockErase(0);

		for (i=0; i<15; i++)
			*(ptr+i) = *(pString+i);

		fmiSM_Write_512(0, _fmi_uPagePerBlock, (UINT32)_fmi_pSMBuffer, 0x0);

	}
	else	// 2048
	{
		fmiSM_Read_2K(uPage, 0, (UINT32)_fmi_pSMBuffer);
		if (*ptr != 0xff)
		{
			for (i=0; i<6; i++)
				fmiSM_Read_2K(i, 0, (UINT32)_fmi_pSMBuffer+i*2048);
			fmiSM_BlockErase(0);
			ptr = ptr + uPage * 2048;

			for (i=0; i<15; i++)
				*(ptr+i) = *(pString+i);

			for (i=0; i<6; i++)
				fmiSM_Write_2K(i, 0, (UINT32)_fmi_pSMBuffer+i*2048, 0x0);
		}
		else
		{
			for (i=0; i<15; i++)
				*(ptr+i) = *(pString+i);

			fmiSM_Write_2K(uPage, 0, (UINT32)_fmi_pSMBuffer, 0x0);
		}
	}

	return FMI_NO_ERR;
}

INT fmiGetSerialNumber(char *pString)
{
	unsigned int volatile status, uPage=2;
	int volatile i;

	// [2004/10/27] modify multi PAD control
	outpw(0x7ff00020, (inpw(0x7ff00020)&0x9fffffff)|0x80000000);

	_fmi_pSMBuffer = (UINT8 *)((UINT32)_fmi_ucSMBuffer | 0x10000000);

	status = fmiSM_Reset();
	if (status != FMI_NO_ERR)
		return status;

	if (_fmi_u2KPageSize == 0)	// 512
		status = fmiSM_Read_512(uPage, 1, (UINT32)_fmi_pSMBuffer);
	else	// 2048
		fmiSM_Read_2K(uPage, 0, (UINT32)_fmi_pSMBuffer);

	for (i=0; i<15; i++)
		*(pString+i) = *(_fmi_pSMBuffer+i);
	*(pString+15) = '\0';

	return FMI_NO_ERR;
}

#if 0
INT fmiSetSMInformation(FMI_SM_INFO_T *info)
{
	_fmi_uPagePerBlock = info->uPagePerBlock;
	_fmi_uBlockPerFlash = info->uBlockPerFlash - 1;

	if (info->uPageSize == 512)
	{
		_fmi_uSectorPerBlock = info->uPagePerBlock;
		if (info->uBlockPerFlash >= 4096)
			_fmi_uF4cycle = 1;
		else
			_fmi_uF4cycle = 0;
		_fmi_u2KPageSize = 0;
	}
	else if (info->uPageSize == 2048)
	{
		_fmi_uSectorPerBlock = info->uPagePerBlock * 4;
		if (info->uBlockPerFlash >= 2048)
			_fmi_uF4cycle = 1;
		else
			_fmi_uF4cycle = 0;
		_fmi_u2KPageSize = 1;
	}
	else
		return FMI_SM_ID_ERROR;

	_fmi_uSectorPerFlash = (info->uBlockPerFlash/1024) * 999 * _fmi_uSectorPerBlock;

	return FMI_NO_ERR;
}
#endif

#endif	// SM_DEVICE


