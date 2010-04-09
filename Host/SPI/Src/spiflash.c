#include "stdio.h"
#include "stdlib.h"
#include "w99702_reg.h"
#include "wblib.h"
#include "wbspi.h"



int usiActive()
{
	int volatile tick;

	outpw(REG_USI_CNTRL, inpw(REG_USI_CNTRL)|0x01);
	tick = sysGetTicks(TIMER0);
	while(inpw(REG_USI_CNTRL) & 0x01)
	{
		if ((sysGetTicks(TIMER0) - tick) > USI_TIMEOUT_TICK)
			return USI_TIMEOUT;
	}

	return USI_NO_ERR;
}

int usiTxLen(int count, int bitLen)
{
	UINT32 reg;

	reg = inpw(REG_USI_CNTRL);

	if ((count < 0) || (count > 3))
		return USI_ERR_COUNT;

	if ((bitLen <= 0) || (bitLen > 32))
		return USI_ERR_BITLEN;

	if (bitLen == 32)
		reg = reg & 0xffffff07;
	else
		reg = (reg & 0xffffff07) | (bitLen << 3);
	reg = (reg & 0xfffffcff) | (count << 8);

	outpw(REG_USI_CNTRL, reg);

	return USI_NO_ERR;
}

int usiCheckBusy()
{
	// check status
	outpw(REG_USI_SSR, inpw(REG_USI_SSR) | 0x01);	// CS0

	// status command
	outpw(REG_USI_Tx0, 0x05);
	usiTxLen(0, 8);
	usiActive();

	// get status
	while(1)
	{
		outpw(REG_USI_Tx0, 0xff);
		usiTxLen(0, 8);
		usiActive();
		if (((inpw(REG_USI_Rx0) & 0xff) & 0x01) != 0x01)
			break;
	}

	outpw(REG_USI_SSR, inpw(REG_USI_SSR) & 0xfe);	// CS0

	return USI_NO_ERR;
}

/*
	addr: memory address 
	len: byte count
	buf: buffer to put the read back data
*/
int usiRead(UINT32 addr, UINT32 len, UINT8 *buf)
{
	int volatile i;

	outpw(REG_USI_SSR, inpw(REG_USI_SSR) | 0x01);	// CS0

	// read command
	outpw(REG_USI_Tx0, 03);
	usiTxLen(0, 8);
	usiActive();

	// address
	outpw(REG_USI_Tx0, addr);
	usiTxLen(0, 24);
	usiActive();

	// data
	for (i=0; i<len; i++)
	{
		outpw(REG_USI_Tx0, 0xff);
		usiTxLen(0, 8);
		usiActive();
		*buf++ = inpw(REG_USI_Rx0) & 0xff;
	}

	outpw(REG_USI_SSR, inpw(REG_USI_SSR) & 0xfe);	// CS0

	return USI_NO_ERR;
}

int usiReadFast(UINT32 addr, UINT32 len, UINT8 *buf)
{
	int volatile i;

	outpw(REG_USI_SSR, inpw(REG_USI_SSR) | 0x01);	// CS0

	// read command
	outpw(REG_USI_Tx0, 0x0b);
	usiTxLen(0, 8);
	usiActive();

	// address
	outpw(REG_USI_Tx0, addr);
	usiTxLen(0, 24);
	usiActive();

	// dummy byte
	outpw(REG_USI_Tx0, 0xff);
	usiTxLen(0, 8);
	usiActive();

	// data
	for (i=0; i<len; i++)
	{
		outpw(REG_USI_Tx0, 0xff);
		usiTxLen(0, 8);
		usiActive();
		*buf++ = inpw(REG_USI_Rx0) & 0xff;
	}

	outpw(REG_USI_SSR, inpw(REG_USI_SSR) & 0xfe);	// CS0

	return USI_NO_ERR;
}

int usiWriteEnable()
{
	outpw(REG_USI_SSR, inpw(REG_USI_SSR) | 0x01);	// CS0

	outpw(REG_USI_Tx0, 0x06);
	usiTxLen(0, 8);
	usiActive();

	outpw(REG_USI_SSR, inpw(REG_USI_SSR) & 0xfe);	// CS0

	return USI_NO_ERR;
}

int usiWriteDisable()
{
	outpw(REG_USI_SSR, inpw(REG_USI_SSR) | 0x01);	// CS0

	outpw(REG_USI_Tx0, 0x04);
	usiTxLen(0, 8);
	usiActive();

	outpw(REG_USI_SSR, inpw(REG_USI_SSR) & 0xfe);	// CS0

	return USI_NO_ERR;
}

/*
	addr: memory address
	len: byte count
	buf: buffer with write data
*/
int usiWrite(UINT32 addr, UINT32 len, UINT8 *buf)
{
	int volatile count=0, page, i;

	count = len / 256;
	if ((len % 256) != 0)
		count++;

	for (i=0; i<count; i++)
	{
		// check data len
		if (len >= 256)
		{
			page = 256;
			len = len - 256;
		}
		else
			page = len;

		usiWriteEnable();

		outpw(REG_USI_SSR, inpw(REG_USI_SSR) | 0x01);	// CS0

		// write command
		outpw(REG_USI_Tx0, 0x02);
		usiTxLen(0, 8);
		usiActive();

		// address
		outpw(REG_USI_Tx0, addr+i*256);
		usiTxLen(0, 24);
		usiActive();

		// write data
		while (page-- > 0)
		{
			outpw(REG_USI_Tx0, *buf++);
			usiTxLen(0, 8);
			usiActive();
		}

		outpw(REG_USI_SSR, inpw(REG_USI_SSR) & 0xfe);	// CS0

		// check status
		usiCheckBusy();
	}

	return USI_NO_ERR;
}


int usiEraseSector(UINT32 addr, UINT32 secCount)
{
	int volatile i;

	for (i=0; i<secCount; i++)
	{
		usiWriteEnable();

		outpw(REG_USI_SSR, inpw(REG_USI_SSR) | 0x01);	// CS0

		// erase command
		outpw(REG_USI_Tx0, 0xd8);
		usiTxLen(0, 8);
		usiActive();

		// address
		outpw(REG_USI_Tx0, addr+i*0x10000);	// 1 sector = 64KB
		usiTxLen(0, 24);
		usiActive();

		outpw(REG_USI_SSR, inpw(REG_USI_SSR) & 0xfe);	// CS0

		// check status
		usiCheckBusy();
	}
	return USI_NO_ERR;
}

int usiEraseAll()
{
	usiWriteEnable();

	outpw(REG_USI_SSR, inpw(REG_USI_SSR) | 0x01);	// CS0

	outpw(REG_USI_Tx0, 0xc7);
	usiTxLen(0, 8);
	usiActive();

	outpw(REG_USI_SSR, inpw(REG_USI_SSR) & 0xfe);	// CS0

	// check status
	usiCheckBusy();

	return USI_NO_ERR;
}


UINT16 usiReadID()
{
	UINT16 volatile id;

	outpw(REG_USI_SSR, inpw(REG_USI_SSR) | 0x01);	// CS0

	// command 8 bit
	outpw(REG_USI_Tx0, 0x90);
	usiTxLen(0, 8);
	usiActive();

	// address 24 bit
	outpw(REG_USI_Tx0, 0x000000);
	usiTxLen(0, 24);
	usiActive();

	// data 16 bit
	outpw(REG_USI_Tx0, 0xffff);
	usiTxLen(0, 16);
	usiActive();
	id = inpw(REG_USI_Rx0) & 0xffff;

	outpw(REG_USI_SSR, inpw(REG_USI_SSR) & 0xfe);	// CS0

	return id;
}

UINT8 usiStatusRead()
{
	UINT32 status;

	outpw(REG_USI_SSR, inpw(REG_USI_SSR) | 0x01);	// CS0

	// status command
	outpw(REG_USI_Tx0, 0x05);
	usiTxLen(0, 8);
	usiActive();

	// get status
	outpw(REG_USI_Tx0, 0xff);
	usiTxLen(0, 8);
	usiActive();
	status = inpw(REG_USI_Rx0) & 0xff;

	outpw(REG_USI_SSR, inpw(REG_USI_SSR) & 0xfe);	// CS0

	return status;
}

int usiStatusWrite(UINT8 data)
{
	usiWriteEnable();

	outpw(REG_USI_SSR, inpw(REG_USI_SSR) | 0x01);	// CS0

	// status command
	outpw(REG_USI_Tx0, 0x01);
	usiTxLen(0, 8);
	usiActive();

	// write status
	outpw(REG_USI_Tx0, data);
	usiTxLen(0, 8);
	usiActive();

	outpw(REG_USI_SSR, inpw(REG_USI_SSR) & 0xfe);	// CS0

	// check status
	usiCheckBusy();

	return USI_NO_ERR;
}

int usiInitDevice(UINT32 clock_by_MHz)
{
	int volatile rate, id;

	// multi PAD control
	outpw(REG_PADC0, inpw(REG_PADC0)|0x40);
	/* init SPI interface */
	rate = clock_by_MHz / 40; // access SPIFlash 20MHz
	if (((clock_by_MHz % 40) == 0) && (rate != 1))
		rate = rate - 1;

	outpw(REG_USI_DIVIDER, rate);
	id = usiReadID();
//	sysprintf("id [0x%x]\n", id);

	if (id == 0xef12)
		return 1024; // 512KB, 1024 sectors(for file system, 1 sector=512 byte)
	else if (id == 0xef13)
		return 2048; // 1MB, 2048 sectors(for file system, 1 sector=512 byte)
	else if (id == 0xef14)
		return 4096; // 2MB, 4096 sectors
	else if (id == 0xef15)
		return 8192; // 4MB, 8192 sectors
	else
		return USI_ERR_DEVICE;

	return USI_NO_ERR;
}


