#ifdef ECOS
#include "string.h"
#else
#include <string.h>
#endif

#include "w99702_reg.h"
#include "fmi.h"
#include "wb_fmi.h"

#ifdef CF_DEVICE

UINT32 volatile _fmi_uCF_DataReady = 0;

#define CF_HW_MODE

UCHAR IDBuffer[512];

// CF functions
INT fmiCF_INTHandler(VOID)
{
	unsigned int isr, val;

	isr = inpw(REG_CFISR);
	if (isr & 0x01)
	{
		_fmi_uCF_DataReady = 1;
		outpw(REG_CFISR, isr|0x01);

#ifdef CF_HW_MODE
		do {
			outpw(REG_CFCR, (inpw(REG_CFCR)&0xffffff03)|0x7c);
			val = inpw(REG_CFCSR);
			if (_fmi_bIsCFInsert == FALSE)
				return FMI_NO_CF_CARD;
		} while (val & CF_STATE_BUSY);
#else
		do {
			outpw(REG_CFDCR, 0x07);
			outpw(REG_CFCR, (inpw(REG_CFCR)&0xfffffffb)|0x04);
			val = inpw(REG_CFDR);
			if (_fmi_bIsCFInsert == FALSE)
				return FMI_NO_CF_CARD;
		} while (val & CF_STATE_BUSY);
#endif
	}

	if (isr & 0x02)
	{
		_fmi_uCF_DataReady = 1;
		outpw(REG_CFISR, isr|0x02);

#ifdef CF_HW_MODE
		do {
			outpw(REG_CFCR, (inpw(REG_CFCR)&0xffffff03)|0x7c);
			val = inpw(REG_CFCSR);
			if (_fmi_bIsCFInsert == FALSE)
				return FMI_NO_CF_CARD;
		} while (val & CF_STATE_BUSY);
#else
		do {
			outpw(REG_CFDCR, 0x07);
			outpw(REG_CFCR, (inpw(REG_CFCR)&0xfffffffb)|0x04);
			val = inpw(REG_CFDR);
			if (_fmi_bIsCFInsert == FALSE)
				return FMI_NO_CF_CARD;
		} while (val & CF_STATE_BUSY);
#endif
	}
	return FMI_NO_ERR;
}


VOID fmiCF_Reset(VOID)
{
	outpw(REG_CFCR, inpw(REG_CFCR) & 0xfffeffff);	// reset=0
	outpw(REG_CFCR, inpw(REG_CFCR) | 0x10000);		// reset=1
}


#ifdef CF_HW_MODE
INT fmiCF_Initial(VOID)
{
	// [2004/10/27] marked. This timing is for emulation
	//outpw(REG_CFTCR, 0x240007);
	outpw(REG_CFCR, 0x10008);	// csel=0, reset=1, dma r/w disable, transfer size=512
#ifdef _USE_IRQ
	outpw(REG_CFIER, 0x03);
#endif
	return FMI_NO_ERR;
}

INT fmiCF_Identify(UINT8 ncBufNo)
{
	unsigned int val;

	if (ncBufNo == 0)
		outpw(REG_FMICR, (inpw(REG_FMICR) & 0xffffff8f)|0x10);
	if (ncBufNo == 1)
		outpw(REG_FMICR, (inpw(REG_FMICR) & 0xffffff8f)|0x50);

	do {
		outpw(REG_CFCR, (inpw(REG_CFCR)&0xffffff03)|0x7c);
		val = inpw(REG_CFCSR);
		if (_fmi_bIsCFInsert == FALSE)
			return FMI_NO_CF_CARD;
	} while (val & CF_STATE_BUSY);

	outpw(REG_CFSCHR, 0xe0);	// LBA mode, LBA27~24=0

	do {
		outpw(REG_CFCR, (inpw(REG_CFCR)&0xffffff03)|0x7c);
		val = inpw(REG_CFCSR);
		if (_fmi_bIsCFInsert == FALSE)
			return FMI_NO_CF_CARD;
	} while ((!(val & CF_STATE_RDY)) || (val & CF_STATE_BUSY));

	outpw(REG_CFCSR, 0xec);		// identify command

	do {
		outpw(REG_CFCR, (inpw(REG_CFCR)&0xffffff03)|0x7c);
		val = inpw(REG_CFCSR);
		if (_fmi_bIsCFInsert == FALSE)
			return FMI_NO_CF_CARD;
	} while ((!(val & CF_STATE_DRQ)) || (val & CF_STATE_BUSY));

	outpw(REG_CFDCR, 0x00);
	outpw(REG_CFCR, inpw(REG_CFCR)|0x02);	// CF->buffer
	while (inpw(REG_CFCR) & 0x02)
	{
		if (_fmi_bIsCFInsert == FALSE)
			return FMI_NO_CF_CARD;
	}
	fmiBuffer2SDRAM((unsigned int)IDBuffer, ncBufNo);

	outpw(REG_CFCR, (inpw(REG_CFCR)&0xffffff03)|0x7c);
	val = inpw(REG_CFCSR);
	if (val & CF_STATE_ERR)
		return FMI_CF_STATE_ERROR;

	return FMI_NO_ERR;
}


INT fmiCF_Multiple(INT nScount)
{
	unsigned int val;

	do {
		outpw(REG_CFCR, (inpw(REG_CFCR)&0xffffff03)|0x7c);
		val = inpw(REG_CFCSR);
		if (_fmi_bIsCFInsert == FALSE)
			return FMI_NO_CF_CARD;
	} while (val & CF_STATE_BUSY);

	outpw(REG_CFSCR, nScount);	// sector count

	do {
		outpw(REG_CFCR, (inpw(REG_CFCR)&0xffffff03)|0x7c);
		val = inpw(REG_CFCSR);
		if (_fmi_bIsCFInsert == FALSE)
			return FMI_NO_CF_CARD;
	} while ((!(val & CF_STATE_RDY)) || (val & CF_STATE_BUSY));

	outpw(REG_CFCSR, 0xc6);		// multiple command

	do {
		outpw(REG_CFCR, (inpw(REG_CFCR)&0xffffff03)|0x7c);
		val = inpw(REG_CFCSR);
		if (_fmi_bIsCFInsert == FALSE)
			return FMI_NO_CF_CARD;
	} while ((!(val & CF_STATE_RDY)) || (val & CF_STATE_BUSY));

	outpw(REG_CFCR, (inpw(REG_CFCR)&0xffffff03)|0x7c);
	val = inpw(REG_CFCSR);
	if (val & CF_STATE_ERR)
		return FMI_CF_STATE_ERROR;
	return FMI_NO_ERR;
}


INT fmiBuffer2CF(UINT32 uSector, UINT8 ncBufNo)
{
	unsigned int val;

	if (ncBufNo == 0)
		outpw(REG_FMICR, (inpw(REG_FMICR) & 0xfffff8ff)|0x100);
	else if (ncBufNo == 1)
		outpw(REG_FMICR, (inpw(REG_FMICR) & 0xfffff8ff)|0x500);

	do {
		outpw(REG_CFCR, (inpw(REG_CFCR)&0xffffff03)|0x7c);
		val = inpw(REG_CFCSR);
		if (_fmi_bIsCFInsert == FALSE)
			return FMI_NO_CF_CARD;
	} while (val & CF_STATE_BUSY);

	outpw(REG_CFSCHR, 0xe0);	// LBA mode, LBA27~24=0

	do {
		outpw(REG_CFCR, (inpw(REG_CFCR)&0xffffff03)|0x7c);
		val = inpw(REG_CFCSR);
		if (_fmi_bIsCFInsert == FALSE)
			return FMI_NO_CF_CARD;
	} while ((!(val & CF_STATE_RDY)) || (val & CF_STATE_BUSY));

	outpw(REG_CFSCR, 1);							// sector count
	outpw(REG_CFSNR, uSector & 0xff);				// sector number
	outpw(REG_CFCLR, (uSector & 0xff00) >> 8);		// sector number
	outpw(REG_CFCHR, (uSector & 0xff0000) >> 16);	// sector number

	outpw(REG_CFCSR, 0x30);		// write sector command

	do {
		outpw(REG_CFCR, (inpw(REG_CFCR)&0xffffff03)|0x7c);
		val = inpw(REG_CFCSR);
		if (_fmi_bIsCFInsert == FALSE)
			return FMI_NO_CF_CARD;
	} while ((!(val & CF_STATE_DRQ)) || (val & CF_STATE_BUSY));

#ifdef _USE_IRQ
	_fmi_uCF_DataReady = 0;
#endif

	outpw(REG_CFDCR, 0x00);
	outpw(REG_CFCR, inpw(REG_CFCR)|0x01);	// buffer->CF
#ifdef _USE_IRQ
	while(_fmi_uCF_DataReady == 0)
	{
		if (_fmi_bIsCFInsert == FALSE)
			return FMI_NO_CF_CARD;
	}
#else
	while (inpw(REG_CFCR) & 0x01)
	{
		if (_fmi_bIsCFInsert == FALSE)
			return FMI_NO_CF_CARD;
	}
#endif

	outpw(REG_CFCR, (inpw(REG_CFCR)&0xffffff0b)|0x7c);
	val = inpw(REG_CFCSR);
	if (val & CF_STATE_ERR)
		return FMI_CF_STATE_ERROR;
	return FMI_NO_ERR;
}


INT fmiCF2Buffer(UINT32 uSector, UINT8 ncBufNo)
{
	unsigned int val;

	if (ncBufNo == 0)
		outpw(REG_FMICR, (inpw(REG_FMICR) & 0xffffff8f)|0x10);
	else if (ncBufNo == 1)
		outpw(REG_FMICR, (inpw(REG_FMICR) & 0xffffff8f)|0x50);

	do {
		outpw(REG_CFCR, (inpw(REG_CFCR)&0xffffff03)|0x7c);
		val = inpw(REG_CFCSR);
		if (_fmi_bIsCFInsert == FALSE)
			return FMI_NO_CF_CARD;
	} while (val & CF_STATE_BUSY);

	outpw(REG_CFSCHR, 0xe0);	// LBA mode, LBA27~24=0

	do {
		outpw(REG_CFCR, (inpw(REG_CFCR)&0xffffff03)|0x7c);
		val = inpw(REG_CFCSR);
		if (_fmi_bIsCFInsert == FALSE)
			return FMI_NO_CF_CARD;
	} while ((!(val & CF_STATE_RDY)) || (val & CF_STATE_BUSY));

	outpw(REG_CFSCR, 1);							// sector count
	outpw(REG_CFSNR, uSector & 0xff);				// sector number
	outpw(REG_CFCLR, (uSector & 0xff00) >> 8);		// sector number
	outpw(REG_CFCHR, (uSector & 0xff0000) >> 16);	// sector number

	outpw(REG_CFSCHR, 0xe0);	// LBA mode, LBA27~24=0
	outpw(REG_CFCSR, 0x20);		// read sector command

	do {
		outpw(REG_CFCR, (inpw(REG_CFCR)&0xffffff03)|0x7c);
		val = inpw(REG_CFCSR);
		if (_fmi_bIsCFInsert == FALSE)
			return FMI_NO_CF_CARD;
	} while ((!(val & CF_STATE_DRQ)) || (val & CF_STATE_BUSY));

#ifdef _USE_IRQ
	_fmi_uCF_DataReady = 0;
#endif

	outpw(REG_CFDCR, 0x00);
	outpw(REG_CFCR, inpw(REG_CFCR)|0x02);	// CF->buffer
#ifdef _USE_IRQ
	while(_fmi_uCF_DataReady == 0)
	{
		if (_fmi_bIsCFInsert == FALSE)
			return FMI_NO_CF_CARD;
	}
#else
	while (inpw(REG_CFCR) & 0x02)
	{
		if (_fmi_bIsCFInsert == FALSE)
			return FMI_NO_CF_CARD;
	}
#endif
	outpw(REG_CFCR, (inpw(REG_CFCR)&0xffffff0b)|0x7c);
	val = inpw(REG_CFCSR);
	if (val & CF_STATE_ERR)
		return FMI_CF_STATE_ERROR;
	return FMI_NO_ERR;
}


INT fmiBuffer2CFM(UINT32 uSector, UINT32 uBufcnt, UINT8 ncBufNo)
{
	unsigned int val;

	if (ncBufNo == 0)
		outpw(REG_FMICR, (inpw(REG_FMICR) & 0xfffff8ff)|0x100);
	else if (ncBufNo == 1)
		outpw(REG_FMICR, (inpw(REG_FMICR) & 0xfffff8ff)|0x500);

	do {
		outpw(REG_CFCR, (inpw(REG_CFCR)&0xffffff03)|0x7c);
		val = inpw(REG_CFCSR);
		if (_fmi_bIsCFInsert == FALSE)
			return FMI_NO_CF_CARD;
	} while (val & CF_STATE_BUSY);

	outpw(REG_CFSCHR, 0xe0);	// LBA mode, LBA27~24=0

	do {
		outpw(REG_CFCR, (inpw(REG_CFCR)&0xffffff03)|0x7c);
		val = inpw(REG_CFCSR);
		if (_fmi_bIsCFInsert == FALSE)
			return FMI_NO_CF_CARD;
	} while ((!(val & CF_STATE_RDY)) || (val & CF_STATE_BUSY));

	outpw(REG_CFSCR, uBufcnt);						// sector count
	outpw(REG_CFSNR, uSector & 0xff);				// sector number
	outpw(REG_CFCLR, (uSector & 0xff00) >> 8);		// sector number
	outpw(REG_CFCHR, (uSector & 0xff0000) >> 16);	// sector number

	outpw(REG_CFCSR, 0xc5);		// write multiple command

	return FMI_NO_ERR;
}


INT fmiCF2BufferM(UINT32 uSector, UINT32 uBufcnt, UINT8 ncBufNo)
{
	unsigned int val;

	if (ncBufNo == 0)
		outpw(REG_FMICR, (inpw(REG_FMICR) & 0xffffff8f)|0x10);
	else if (ncBufNo == 1)
		outpw(REG_FMICR, (inpw(REG_FMICR) & 0xffffff8f)|0x50);

	do {
		outpw(REG_CFCR, (inpw(REG_CFCR)&0xffffff03)|0x7c);
		val = inpw(REG_CFCSR);
		if (_fmi_bIsCFInsert == FALSE)
			return FMI_NO_CF_CARD;
	} while (val & CF_STATE_BUSY);

	outpw(REG_CFSCHR, 0xe0);	// LBA mode, LBA27~24=0

	do {
		outpw(REG_CFCR, (inpw(REG_CFCR)&0xffffff03)|0x7c);
		val = inpw(REG_CFCSR);
		if (_fmi_bIsCFInsert == FALSE)
			return FMI_NO_CF_CARD;
	} while ((!(val & CF_STATE_RDY)) || (val & CF_STATE_BUSY));

	outpw(REG_CFSCR, uBufcnt);						// sector count
	outpw(REG_CFSNR, uSector & 0xff);				// sector number
	outpw(REG_CFCLR, (uSector & 0xff00) >> 8);		// sector number
	outpw(REG_CFCHR, (uSector & 0xff0000) >> 16);	// sector number

	outpw(REG_CFSCHR, 0xe0);	// LBA mode, LBA27~24=0
	outpw(REG_CFCSR, 0xc4);		// read multiple command

	return FMI_NO_ERR;
}


INT fmiCF_Read(UINT32 uSector, UINT32 uBufcnt, UINT32 uDAddr)
{
	unsigned int count, rbuf, wbuf;
	unsigned int rSec, wSec, waddr;
	unsigned int val;
	unsigned int multiCount=0;

	// [2004/10/27] multi PAD control to CF host
	outpw(0x7ff00020, (inpw(0x7ff00020)&0x1dffffff)|0x02000000);

	count = uBufcnt;
	rSec = uSector;
	wSec = 0;
	rbuf = wbuf = 0;

	// the first sector
#ifdef _USE_IRQ
	_fmi_uCF_DataReady = 0;
#endif
	fmiCF2BufferM(rSec, uBufcnt, rbuf);
	multiCount++;
	do {
		outpw(REG_CFCR, (inpw(REG_CFCR)&0xffffff03)|0x7c);
		val = inpw(REG_CFCSR);
		if (_fmi_bIsCFInsert == FALSE)
			return FMI_NO_CF_CARD;
	} while ((!(val & CF_STATE_DRQ)) || (val & CF_STATE_BUSY));

	outpw(REG_CFDCR, 0x00);
	outpw(REG_CFCR, inpw(REG_CFCR)|0x02);	// CF->buffer

	rbuf = 1;
	rSec++;
	count--;

#ifdef _USE_IRQ
	while(_fmi_uCF_DataReady == 0)
	{
		if (_fmi_bIsCFInsert == FALSE)
			return FMI_NO_CF_CARD;
	}
#else
	while (inpw(REG_CFCR) & 0x02)
	{
		if (_fmi_bIsCFInsert == FALSE)
			return FMI_NO_CF_CARD;
	}
#endif

	outpw(REG_CFCR, (inpw(REG_CFCR)&0xffffff0b)|0x7c);
	val = inpw(REG_CFCSR);
	if (val & CF_STATE_ERR)
	{
#ifdef DEBUG
		printf("read data error !!\n");
#endif
		return FMI_CF_STATE_ERROR;
	}

	while(count>0)
	{
		if (multiCount > 0xff)
		{
			fmiCF2BufferM(rSec, uBufcnt, rbuf);
			multiCount = 0;
		}

		if (rbuf == 0)
			outpw(REG_FMICR, (inpw(REG_FMICR) & 0xffffff8f)|0x10);
		else
			outpw(REG_FMICR, (inpw(REG_FMICR) & 0xffffff8f)|0x50);
		multiCount++;

		waddr = uDAddr + (wSec << 9);
		fmiSDRAM_Write(waddr, wbuf);

		do {
			outpw(REG_CFCR, (inpw(REG_CFCR)&0xffffff03)|0x7c);
			val = inpw(REG_CFCSR);
			if (_fmi_bIsCFInsert == FALSE)
				return FMI_NO_CF_CARD;
		} while ((!(val & CF_STATE_DRQ)) || (val & CF_STATE_BUSY));

#ifdef _USE_IRQ
		_fmi_uCF_DataReady = _fmi_uDataReady = 0;
#endif
		outpw(REG_CFDCR, 0x00);
		outpw(REG_CFCR, inpw(REG_CFCR)|0x02);	// CF->buffer
		outpw(REG_FMICR, inpw(REG_FMICR) | 0x04);	// enable DMA
#ifdef _USE_IRQ
		while ((_fmi_uCF_DataReady == 0) || (_fmi_uDataReady == 0))
		{
			if (_fmi_bIsCFInsert == FALSE)
				return FMI_NO_CF_CARD;
		}
#else
		while (inpw(REG_CFCR) & 0x02)
		{
			if (_fmi_bIsCFInsert == FALSE)
				return FMI_NO_CF_CARD;
		}
		while (inpw(REG_FMICR) & 0x04);
#endif

		outpw(REG_CFCR, (inpw(REG_CFCR)&0xffffff0b)|0x7c);
		val = inpw(REG_CFCSR);
		if (val & CF_STATE_ERR)
		{
#ifdef DEBUG
			printf("read data error !!\n");
#endif
			return FMI_CF_STATE_ERROR;
		}

		rSec++;
		wSec++;
		rbuf = ~rbuf & 0x01;
		wbuf = ~wbuf & 0x01;
		count--;
	}

	// the last sector
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

	return FMI_NO_ERR;
}


INT fmiCF_Write(UINT32 uSector, UINT32 uBufcnt, UINT32 uSAddr)
{
	unsigned int count, rbuf, wbuf;
	unsigned int rSec, wSec, raddr;
	unsigned int val;
	unsigned int firstcmd=1, multiCount=0;

	// [2004/10/27] multi PAD control to CF host
	outpw(0x7ff00020, (inpw(0x7ff00020)&0x1dffffff)|0x02000000);

	count = uBufcnt;
	rSec = 0;
	wSec = uSector;
	rbuf = wbuf = 0;

	// the first sector
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
	while(_fmi_uDataReady == 0);
#else
	while(inpw(REG_FMICR) & 0x08);
#endif

	while(count>0)
	{
		if (firstcmd == 1)
		{
			fmiBuffer2CFM(wSec, uBufcnt, wbuf);
			firstcmd = 0;
		}
		else
		{
			if (wbuf == 0)
				outpw(REG_FMICR, (inpw(REG_FMICR) & 0xfffff8ff)|0x100);	// buffer 0
			else
				outpw(REG_FMICR, (inpw(REG_FMICR) & 0xfffff8ff)|0x500);		// buffer 1
		}
		multiCount++;

		raddr = uSAddr + (rSec << 9);
		fmiSDRAM_Read(raddr, rbuf);

		do {
			outpw(REG_CFCR, (inpw(REG_CFCR)&0xffffff03)|0x7c);
			val = inpw(REG_CFCSR);
			if (_fmi_bIsCFInsert == FALSE)
				return FMI_NO_CF_CARD;
		} while ((!(val & CF_STATE_DRQ)) || (val & CF_STATE_BUSY));

#ifdef _USE_IRQ
		_fmi_uCF_DataReady = _fmi_uDataReady = 0;
#endif
		outpw(REG_CFDCR, 0x00);
		outpw(REG_CFCR, inpw(REG_CFCR)|0x01);	// buffer->CF
		outpw(REG_FMICR, inpw(REG_FMICR) | 0x08);	// enable DMA
#ifdef _USE_IRQ
		while ((_fmi_uDataReady == 0) || (_fmi_uCF_DataReady == 0))
		{
			if (_fmi_bIsCFInsert == FALSE)
				return FMI_NO_CF_CARD;
		}
#else
		while (inpw(REG_CFCR) & 0x01)
		{
			if (_fmi_bIsCFInsert == FALSE)
				return FMI_NO_CF_CARD;
		}
		while (inpw(REG_FMICR) & 0x08);
#endif
		outpw(REG_CFCR, (inpw(REG_CFCR)&0xffffff0b)|0x7c);
		val = inpw(REG_CFCSR);
		if (val & CF_STATE_ERR)
		{
#ifdef DEBUG
			printf("write data error !!\n");
#endif
			return FMI_CF_STATE_ERROR;
		}

		if (multiCount > 0xff)
		{
			firstcmd = 1;
			multiCount = 0;
		}

		rSec++;
		wSec++;
		rbuf = ~rbuf & 0x01;
		wbuf = ~wbuf & 0x01;
		count--;
	}

	// the last sector
	if (firstcmd == 1)
	{
		fmiBuffer2CFM(wSec, uBufcnt, wbuf);
		firstcmd = 0;
	}
	else
	{
		if (wbuf == 0)
			outpw(REG_FMICR, (inpw(REG_FMICR) & 0xfffff8ff)|0x100);	// buffer 0
		else
			outpw(REG_FMICR, (inpw(REG_FMICR) & 0xfffff8ff)|0x500);		// buffer 1
		multiCount++;
	}

	do {
		outpw(REG_CFCR, (inpw(REG_CFCR)&0xffffff03)|0x7c);
		val = inpw(REG_CFCSR);
		if (_fmi_bIsCFInsert == FALSE)
			return FMI_NO_CF_CARD;
	} while ((!(val & CF_STATE_DRQ)) || (val & CF_STATE_BUSY));

#ifdef _USE_IRQ
	_fmi_uCF_DataReady = 0;
#endif
	outpw(REG_CFDCR, 0x00);
	outpw(REG_CFCR, inpw(REG_CFCR)|0x01);	// buffer->CF
#ifdef _USE_IRQ
	while (_fmi_uCF_DataReady == 0)
	{
		if (_fmi_bIsCFInsert == FALSE)
			return FMI_NO_CF_CARD;
	}
#else
	while (inpw(REG_CFCR) & 0x01)
	{
		if (_fmi_bIsCFInsert == FALSE)
			return FMI_NO_CF_CARD;
	}
#endif
	outpw(REG_CFCR, (inpw(REG_CFCR)&0xffffff0b)|0x7c);
	val = inpw(REG_CFCSR);
	if (val & CF_STATE_ERR)
	{
#ifdef DEBUG
		printf("write data error !!\n");
#endif
		return FMI_CF_STATE_ERROR;
	}

	return FMI_NO_ERR;
}



#else	// software mode
INT fmiCF_Initial(VOID)
{
	outpw(REG_CFCR, 0x10108);	// csel=0, reset=1, dma r/w disable, transfer size=512
	outpw(REG_CFDAR, 0x05);		// ce2_=1, ce1_=0, reg_=1
#ifdef _USE_IRQ
	outpw(REG_CFIER, 0x03);
#endif
	return FMI_NO_ERR;
}


INT fmiCF_Identify(UINT8 ncBufNo)
{
	unsigned int val;

	if (ncBufNo == 0)
		outpw(REG_FMICR, (inpw(REG_FMICR) & 0xffffff8f)|0x10);
	else if (ncBufNo == 1)
		outpw(REG_FMICR, (inpw(REG_FMICR) & 0xffffff8f)|0x50);

	do {
		outpw(REG_CFDCR, 0x07);
		outpw(REG_CFCR, (inpw(REG_CFCR)&0xfffffffb)|0x04);
		val = inpw(REG_CFDR);
		if (_fmi_bIsCFInsert == FALSE)
			return FMI_NO_CF_CARD;
	} while (val & CF_STATE_BUSY);

	outpw(REG_CFDCR, 0x06);
	outpw(REG_CFDR, 0xe0);	// LBA mode, LBA27~24=0

	do {
		outpw(REG_CFDCR, 0x07);
		outpw(REG_CFCR, (inpw(REG_CFCR)&0xfffffffb)|0x04);
		val = inpw(REG_CFDR);
		if (_fmi_bIsCFInsert == FALSE)
			return FMI_NO_CF_CARD;
	} while ((!(val & CF_STATE_RDY)) || (val & CF_STATE_BUSY));

	outpw(REG_CFDCR, 0x07);
	outpw(REG_CFDR, 0xec);	// identify command

	do {
		outpw(REG_CFDCR, 0x07);
		outpw(REG_CFCR, (inpw(REG_CFCR)&0xfffffffb)|0x04);
		val = inpw(REG_CFDR);
		if (_fmi_bIsCFInsert == FALSE)
			return FMI_NO_CF_CARD;
	} while ((!(val & CF_STATE_DRQ)) || (val & CF_STATE_BUSY));

	outpw(REG_CFDCR, 0x00);
	outpw(REG_CFCR, inpw(REG_CFCR)|0x02);	// CF->buffer
	while (inpw(REG_CFCR) & 0x02)
	{
		if (_fmi_bIsCFInsert == FALSE)
			return FMI_NO_CF_CARD;
	}
	fmiBuffer2SDRAM((unsigned int)IDBuffer, ncBufNo);

	outpw(REG_CFDCR, 0x07);
	outpw(REG_CFCR, (inpw(REG_CFCR)&0xfffffffb)|0x04);
	val = inpw(REG_CFDR);
	if (val & CF_STATE_ERR)
		return FMI_CF_STATE_ERROR;

	return FMI_NO_ERR;
}


INT fmiCF_Multiple(INT nScount)
{
	unsigned int val;

	do {
		outpw(REG_CFDCR, 0x07);
		outpw(REG_CFCR, (inpw(REG_CFCR)&0xfffffffb)|0x04);
		val = inpw(REG_CFDR);
		if (_fmi_bIsCFInsert == FALSE)
			return FMI_NO_CF_CARD;
	} while (val & CF_STATE_BUSY);

	outpw(REG_CFDCR, 0x02);
	outpw(REG_CFDR, nScount);	// sector count

	do {
		outpw(REG_CFDCR, 0x07);
		outpw(REG_CFCR, (inpw(REG_CFCR)&0xfffffffb)|0x04);
		val = inpw(REG_CFDR);
		if (_fmi_bIsCFInsert == FALSE)
			return FMI_NO_CF_CARD;
	} while ((!(val & CF_STATE_RDY)) || (val & CF_STATE_BUSY));

	outpw(REG_CFDCR, 0x07);
	outpw(REG_CFDR, 0xc6);	// multiple command

	do {
		outpw(REG_CFDCR, 0x07);
		outpw(REG_CFCR, (inpw(REG_CFCR)&0xfffffffb)|0x04);
		val = inpw(REG_CFDR);
		if (_fmi_bIsCFInsert == FALSE)
			return FMI_NO_CF_CARD;
	} while ((!(val & CF_STATE_RDY)) || (val & CF_STATE_BUSY));

	outpw(REG_CFDCR, 0x07);
	outpw(REG_CFCR, (inpw(REG_CFCR)&0xfffffffb)|0x04);
	val = inpw(REG_CFDR);
	if (val & CF_STATE_ERR)
		return FMI_CF_STATE_ERROR;

	return FMI_NO_ERR;
}


INT fmiBuffer2CF(UINT32 uSector, UINT8 ncBufNo)
{
	unsigned int val;

	if (ncBufNo == 0)
		outpw(REG_FMICR, (inpw(REG_FMICR) & 0xfffff8ff)|0x100);
	else if (ncBufNo == 1)
		outpw(REG_FMICR, (inpw(REG_FMICR) & 0xfffff8ff)|0x500);

	do {
		outpw(REG_CFDCR, 0x07);
		outpw(REG_CFCR, (inpw(REG_CFCR)&0xfffffffb)|0x04);
		val = inpw(REG_CFDR);
		if (_fmi_bIsCFInsert == FALSE)
			return FMI_NO_CF_CARD;
	} while (val & CF_STATE_BUSY);

	outpw(REG_CFDCR, 0x06);
	outpw(REG_CFDR, 0xe0);	// LBA mode, LBA27~24=0

	do {
		outpw(REG_CFDCR, 0x07);
		outpw(REG_CFCR, (inpw(REG_CFCR)&0xfffffffb)|0x04);
		val = inpw(REG_CFDR);
		if (_fmi_bIsCFInsert == FALSE)
			return FMI_NO_CF_CARD;
	} while ((!(val & CF_STATE_RDY)) || (val & CF_STATE_BUSY));

	outpw(REG_CFDCR, 0x02);
	outpw(REG_CFDR, 0x01);				// sector count
	outpw(REG_CFDCR, 0x03);
	outpw(REG_CFDR, uSector & 0xff);		// sector NO.
	outpw(REG_CFDCR, 0x04);
	outpw(REG_CFDR, (uSector & 0xff00) >> 8);	// cylinder low
	outpw(REG_CFDCR, 0x05);
	outpw(REG_CFDR, (uSector & 0xff0000) >> 16);	// cylinder high

	outpw(REG_CFDCR, 0x07);
	outpw(REG_CFDR, 0x30);	// write sector command

	do {
		outpw(REG_CFDCR, 0x07);
		outpw(REG_CFCR, (inpw(REG_CFCR)&0xfffffffb)|0x04);
		val = inpw(REG_CFDR);
		if (_fmi_bIsCFInsert == FALSE)
			return FMI_NO_CF_CARD;
	} while ((!(val & CF_STATE_DRQ)) || (val & CF_STATE_BUSY));

#ifdef _USE_IRQ
	_fmi_uCF_DataReady = 0;
#endif

	outpw(REG_CFDCR, 0x0);
	outpw(REG_CFCR, inpw(REG_CFCR)|0x01);	// buffer->CF

#ifdef _USE_IRQ
	while(_fmi_uCF_DataReady == 0)
	{
		if (_fmi_bIsCFInsert == FALSE)
			return FMI_NO_CF_CARD;
	}
#else
	while (inpw(REG_CFCR) & 0x01)
	{
		if (_fmi_bIsCFInsert == FALSE)
			return FMI_NO_CF_CARD;
	}
#endif

	outpw(REG_CFDCR, 0x07);
	outpw(REG_CFCR, (inpw(REG_CFCR)&0xfffffffb)|0x04);
	val = inpw(REG_CFDR);
	if (val & CF_STATE_ERR)
		return FMI_CF_STATE_ERROR;

	return FMI_NO_ERR;
}


INT fmiCF2Buffer(UINT32 uSector, UINT8 ncBufNo)
{
	unsigned int val;

	if (ncBufNo == 0)
		outpw(REG_FMICR, (inpw(REG_FMICR) & 0xffffff8f)|0x10);
	else if (ncBufNo == 1)
		outpw(REG_FMICR, (inpw(REG_FMICR) & 0xffffff8f)|0x50);

	do {
		outpw(REG_CFDCR, 0x07);
		outpw(REG_CFCR, (inpw(REG_CFCR)&0xfffffffb)|0x04);
		val = inpw(REG_CFDR);
		if (_fmi_bIsCFInsert == FALSE)
			return FMI_NO_CF_CARD;
	} while (val & CF_STATE_BUSY);

	outpw(REG_CFDCR, 0x06);
	outpw(REG_CFDR, 0xe0);	// LBA mode, LBA27~24=0

	do {
		outpw(REG_CFDCR, 0x07);
		outpw(REG_CFCR, (inpw(REG_CFCR)&0xfffffffb)|0x04);
		val = inpw(REG_CFDR);
		if (_fmi_bIsCFInsert == FALSE)
			return FMI_NO_CF_CARD;
	} while ((!(val & CF_STATE_RDY)) || (val & CF_STATE_BUSY));

	outpw(REG_CFDCR, 0x02);
	outpw(REG_CFDR, 0x01);				// sector count
	outpw(REG_CFDCR, 0x03);
	outpw(REG_CFDR, uSector & 0xff);		// sector NO.
	outpw(REG_CFDCR, 0x04);
	outpw(REG_CFDR, (uSector & 0xff00) >> 8);	// cylinder low
	outpw(REG_CFDCR, 0x05);
	outpw(REG_CFDR, (uSector & 0xff0000) >> 16);	// cylinder high
	outpw(REG_CFDCR, 0x06);
	outpw(REG_CFDR, 0xe0);	// LBA mode, LBA27~24=0

	outpw(REG_CFDCR, 0x07);
	outpw(REG_CFDR, 0x20);	// read sector command

	do {
		outpw(REG_CFDCR, 0x07);
		outpw(REG_CFCR, (inpw(REG_CFCR)&0xfffffffb)|0x04);
		val = inpw(REG_CFDR);
		if (_fmi_bIsCFInsert == FALSE)
			return FMI_NO_CF_CARD;
	} while ((!(val & CF_STATE_DRQ)) || (val & CF_STATE_BUSY));

#ifdef _USE_IRQ
	_fmi_uCF_DataReady = 0;
#endif

	outpw(REG_CFDCR, 0x0);
	outpw(REG_CFCR, inpw(REG_CFCR)|0x02);	// CF->buffer

#ifdef _USE_IRQ
	while(_fmi_uCF_DataReady == 0)
	{
		if (_fmi_bIsCFInsert == FALSE)
			return FMI_NO_CF_CARD;
	}
#else
	while (inpw(REG_CFCR) & 0x02)
	{
		if (_fmi_bIsCFInsert == FALSE)
			return FMI_NO_CF_CARD;
	}
#endif

	outpw(REG_CFDCR, 0x07);
	outpw(REG_CFCR, (inpw(REG_CFCR)&0xfffffffb)|0x04);
	val = inpw(REG_CFDR);
	if (val & CF_STATE_ERR)
		return FMI_CF_STATE_ERROR;

	return FMI_NO_ERR;
}


INT fmiBuffer2CFM(UINT32 uSector, UINT32 uBufcnt, UINT8 ncBufNo)
{
	unsigned int val;

	if (ncBufNo == 0)
		outpw(REG_FMICR, (inpw(REG_FMICR) & 0xfffff8ff)|0x100);
	else if (ncBufNo == 1)
		outpw(REG_FMICR, (inpw(REG_FMICR) & 0xfffff8ff)|0x500);

	do {
		outpw(REG_CFDCR, 0x07);
		outpw(REG_CFCR, (inpw(REG_CFCR)&0xfffffffb)|0x04);
		val = inpw(REG_CFDR);
		if (_fmi_bIsCFInsert == FALSE)
			return FMI_NO_CF_CARD;
	} while (val & CF_STATE_BUSY);

	outpw(REG_CFDCR, 0x06);
	outpw(REG_CFDR, 0xe0);	// LBA mode, LBA27~24=0

	do {
		outpw(REG_CFDCR, 0x07);
		outpw(REG_CFCR, (inpw(REG_CFCR)&0xfffffffb)|0x04);
		val = inpw(REG_CFDR);
		if (_fmi_bIsCFInsert == FALSE)
			return FMI_NO_CF_CARD;
	} while ((!(val & CF_STATE_RDY)) || (val & CF_STATE_BUSY));

	outpw(REG_CFDCR, 0x02);
	outpw(REG_CFDR, uBufcnt);			// sector count
	outpw(REG_CFDCR, 0x03);
	outpw(REG_CFDR, uSector & 0xff);		// sector NO.
	outpw(REG_CFDCR, 0x04);
	outpw(REG_CFDR, (uSector & 0xff00) >> 8);	// cylinder low
	outpw(REG_CFDCR, 0x05);
	outpw(REG_CFDR, (uSector & 0xff0000) >> 16);	// cylinder high

	outpw(REG_CFDCR, 0x07);
	outpw(REG_CFDR, 0xc5);	// write multiple command

	return FMI_NO_ERR;
}


INT fmiCF2BufferM(UINT32 uSector, UINT32 uBufcnt, UINT8 ncBufNo)
{
	unsigned int val;

	if (ncBufNo == 0)
		outpw(REG_FMICR, (inpw(REG_FMICR) & 0xffffff8f)|0x10);
	else if (ncBufNo == 1)
		outpw(REG_FMICR, (inpw(REG_FMICR) & 0xffffff8f)|0x50);

	do {
		outpw(REG_CFDCR, 0x07);
		outpw(REG_CFCR, (inpw(REG_CFCR)&0xfffffffb)|0x04);
		val = inpw(REG_CFDR);
		if (_fmi_bIsCFInsert == FALSE)
			return FMI_NO_CF_CARD;
	} while (val & CF_STATE_BUSY);

	outpw(REG_CFDCR, 0x06);
	outpw(REG_CFDR, 0xe0);	// LBA mode, LBA27~24=0

	do {
		outpw(REG_CFDCR, 0x07);
		outpw(REG_CFCR, (inpw(REG_CFCR)&0xfffffffb)|0x04);
		val = inpw(REG_CFDR);
		if (_fmi_bIsCFInsert == FALSE)
			return FMI_NO_CF_CARD;
	} while ((!(val & CF_STATE_RDY)) || (val & CF_STATE_BUSY));

	outpw(REG_CFDCR, 0x02);
	outpw(REG_CFDR, uBufcnt);			// sector count
	outpw(REG_CFDCR, 0x03);
	outpw(REG_CFDR, uSector & 0xff);		// sector NO.
	outpw(REG_CFDCR, 0x04);
	outpw(REG_CFDR, (uSector & 0xff00) >> 8);	// cylinder low
	outpw(REG_CFDCR, 0x05);
	outpw(REG_CFDR, (uSector & 0xff0000) >> 16);	// cylinder high
	outpw(REG_CFDCR, 0x06);
	outpw(REG_CFDR, 0xe0);	// LBA mode, LBA27~24=0

	outpw(REG_CFDCR, 0x07);
	outpw(REG_CFDR, 0xc4);	// read multiple command

	return FMI_NO_ERR;
}


INT fmiCF_Read(UINT32 uSector, UINT32 uBufcnt, UINT32 uDAddr)
{
	unsigned int count, rbuf, wbuf;
	unsigned int rSec, wSec, waddr;
	unsigned int val;

	count = uBufcnt;
	rSec = uSector;
	wSec = 0;
	rbuf = wbuf = 0;

	// the first sector
#ifdef _USE_IRQ
	_fmi_uCF_DataReady = 0;
#endif
	fmiCF2BufferM(rSec, uBufcnt, rbuf);

	do {
		outpw(REG_CFDCR, 0x07);
		outpw(REG_CFCR, (inpw(REG_CFCR)&0xfffffffb)|0x04);
		val = inpw(REG_CFDR);
		if (_fmi_bIsCFInsert == FALSE)
			return FMI_NO_CF_CARD;
	} while ((!(val & CF_STATE_DRQ)) || (val & CF_STATE_BUSY));

	outpw(REG_CFDCR, 0x00);
	outpw(REG_CFCR, inpw(REG_CFCR)|0x02);	// CF->buffer

	rbuf = 1;
	rSec++;
	count--;

#ifdef _USE_IRQ
	while(_fmi_uCF_DataReady == 0)
	{
		if (_fmi_bIsCFInsert == FALSE)
			return FMI_NO_CF_CARD;
	}
#else
	while (inpw(REG_CFCR) & 0x02)
	{
		if (_fmi_bIsCFInsert == FALSE)
			return FMI_NO_CF_CARD;
	}
#endif

	outpw(REG_CFDCR, 0x07);
	outpw(REG_CFCR, (inpw(REG_CFCR)&0xfffffffb)|0x04);
	val = inpw(REG_CFDR);
	if (val & CF_STATE_ERR)
	{
#ifdef DEBUG
		printf("read data error !!\n");
#endif
		return FMI_CF_STATE_ERROR;
	}

	while(count>0)
	{
		if (rbuf == 0)
			outpw(REG_FMICR, (inpw(REG_FMICR) & 0xffffff8f)|0x10);
		else
			outpw(REG_FMICR, (inpw(REG_FMICR) & 0xffffff8f)|0x50);

		waddr = uDAddr + (wSec << 9);
		fmiSDRAM_Write(waddr, wbuf);

		do {
			outpw(REG_CFDCR, 0x07);
			outpw(REG_CFCR, (inpw(REG_CFCR)&0xfffffffb)|0x04);
			val = inpw(REG_CFDR);
			if (_fmi_bIsCFInsert == FALSE)
				return FMI_NO_CF_CARD;
		} while ((!(val & CF_STATE_DRQ)) || (val & CF_STATE_BUSY));

#ifdef _USE_IRQ
		_fmi_uCF_DataReady = _fmi_uDataReady = 0;
#endif
		outpw(REG_CFDCR, 0x00);
		outpw(REG_CFCR, inpw(REG_CFCR)|0x02);	// CF->buffer
		outpw(REG_FMICR, inpw(REG_FMICR) | 0x04);	// enable DMA
#ifdef _USE_IRQ
		while ((_fmi_uCF_DataReady == 0) || (_fmi_uDataReady == 0))
		{
			if (_fmi_bIsCFInsert == FALSE)
				return FMI_NO_CF_CARD;
		}
#else
		while (inpw(REG_CFCR) & 0x02)
		{
			if (_fmi_bIsCFInsert == FALSE)
				return FMI_NO_CF_CARD;
		}
		while (inpw(REG_FMICR) & 0x04);
#endif

		outpw(REG_CFDCR, 0x07);
		outpw(REG_CFCR, (inpw(REG_CFCR)&0xfffffffb)|0x04);
		val = inpw(REG_CFDR);
		if (val & CF_STATE_ERR)
		{
#ifdef DEBUG
			printf("read data error !!\n");
#endif
			return FMI_CF_STATE_ERROR;
		}

		rSec++;
		wSec++;
		rbuf = ~rbuf & 0x01;
		wbuf = ~wbuf & 0x01;
		count--;
	}

	// the last sector
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

	return FMI_NO_ERR;
}


INT fmiCF_Write(UINT32 uSector, UINT32 uBufcnt, UINT32 uSAddr)
{
	unsigned int count, rbuf, wbuf;
	unsigned int rSec, wSec, raddr;
	unsigned int val;
	unsigned int firstcmd=1, multiCount=0;

	count = uBufcnt;
	rSec = 0;
	wSec = uSector;
	rbuf = wbuf = 0;

	// the first sector
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
	while(_fmi_uDataReady == 0);
#else
	while(inpw(REG_FMICR) & 0x08);
#endif

	while(count>0)
	{
		if (firstcmd == 1)
		{
			fmiBuffer2CFM(wSec, uBufcnt, wbuf);
			firstcmd = 0;
		}
		else
		{
			if (wbuf == 0)
				outpw(REG_FMICR, (inpw(REG_FMICR) & 0xfffff8ff)|0x100);	// buffer 0
			else
				outpw(REG_FMICR, (inpw(REG_FMICR) & 0xfffff8ff)|0x500);		// buffer 1
		}
		multiCount++;

		raddr = uSAddr + (rSec << 9);
		fmiSDRAM_Read(raddr, rbuf);

		do {
			outpw(REG_CFDCR, 0x07);
			outpw(REG_CFCR, (inpw(REG_CFCR)&0xfffffffb)|0x04);
			val = inpw(REG_CFDR);
			if (_fmi_bIsCFInsert == FALSE)
				return FMI_NO_CF_CARD;
		} while ((!(val & CF_STATE_DRQ)) || (val & CF_STATE_BUSY));

#ifdef _USE_IRQ
		_fmi_uCF_DataReady = _fmi_uDataReady = 0;
#endif
		outpw(REG_CFDCR, 0x00);
		outpw(REG_CFCR, inpw(REG_CFCR)|0x01);	// buffer->CF
		outpw(REG_FMICR, inpw(REG_FMICR) | 0x08);	// enable DMA
#ifdef _USE_IRQ
		while ((_fmi_uDataReady == 0) || (_fmi_uCF_DataReady == 0))
		{
			if (_fmi_bIsCFInsert == FALSE)
				return FMI_NO_CF_CARD;
		}
#else
		while (inpw(REG_CFCR) & 0x01)
		{
			if (_fmi_bIsCFInsert == FALSE)
				return FMI_NO_CF_CARD;
		}
		while (inpw(REG_FMICR) & 0x08);
#endif

		outpw(REG_CFDCR, 0x07);
		outpw(REG_CFCR, (inpw(REG_CFCR)&0xfffffffb)|0x04);
		val = inpw(REG_CFDR);
		if (val & CF_STATE_ERR)
		{
#ifdef DEBUG
			printf("write data error !!\n");
#endif
			return FMI_CF_STATE_ERROR;
		}

		rSec++;
		wSec++;
		rbuf = ~rbuf & 0x01;
		wbuf = ~wbuf & 0x01;
		count--;
	}

	// the last sector
	if (firstcmd == 1)
	{
		fmiBuffer2CFM(wSec, uBufcnt, wbuf);
		firstcmd = 0;
	}
	else
	{
		if (wbuf == 0)
			outpw(REG_FMICR, (inpw(REG_FMICR) & 0xfffff8ff)|0x100);	// buffer 0
		else
			outpw(REG_FMICR, (inpw(REG_FMICR) & 0xfffff8ff)|0x500);		// buffer 1
	}

	do {
		outpw(REG_CFDCR, 0x07);
		outpw(REG_CFCR, (inpw(REG_CFCR)&0xfffffffb)|0x04);
		val = inpw(REG_CFDR);
		if (_fmi_bIsCFInsert == FALSE)
			return FMI_NO_CF_CARD;
	} while ((!(val & CF_STATE_DRQ)) || (val & CF_STATE_BUSY));

#ifdef _USE_IRQ
	_fmi_uCF_DataReady = 0;
#endif
	outpw(REG_CFDCR, 0x00);
	outpw(REG_CFCR, inpw(REG_CFCR)|0x01);	// buffer->CF
#ifdef _USE_IRQ
	while (_fmi_uCF_DataReady == 0)
	{
		if (_fmi_bIsCFInsert == FALSE)
			return FMI_NO_CF_CARD;
	}
#else
	while (inpw(REG_CFCR) & 0x01)
	{
		if (_fmi_bIsCFInsert == FALSE)
			return FMI_NO_CF_CARD;
	}
#endif

	outpw(REG_CFDCR, 0x07);
	outpw(REG_CFCR, (inpw(REG_CFCR)&0xfffffffb)|0x04);
	val = inpw(REG_CFDR);
	if (val & CF_STATE_ERR)
	{
#ifdef DEBUG
		printf("write data error !!\n");
#endif
		return FMI_CF_STATE_ERROR;
	}

	return FMI_NO_ERR;
}

#endif

INT fmiGet_CF_info(_fmi_CF_INFO_T *_info)
{
	if (fmiCF_Identify(0))
	{
#ifdef DEBUG
		printf("identify fail!!\n");
#endif
		return FMI_CF_INIT_ERROR;
	}
	memcpy((unsigned char *)_info, (unsigned char *)IDBuffer, sizeof(_fmi_CF_INFO_T));
	return FMI_NO_ERR;
}

#endif	//CF_DEVICE

