#ifdef ECOS
#include "string.h"
#else
#include <string.h>
#endif

#include "wbio.h"
#include "fmi.h"

#ifdef CF_8_DEVICE

unsigned int volatile CF_8_DataReady = 0;

#define CF_HW_MODE

unsigned char IDBuffer[512];

// CF functions
void CF_INTHandler(void)
{
	unsigned int isr, val;

	isr = inpw(CFISR);
	if (isr & 0x01)
	{
		CF_8_DataReady = 1;
		outpw(CFISR, isr|0x01);

#ifdef CF_HW_MODE
		do {
			outpw(CFCR, (inpw(CFCR)&0xffffff03)|0x74);
			val = inpw(CFCSR);
		} while (val & CF_STATE_BUSY);
#else
		do {
			outpw(CFDCR, 0x07);
			outpw(CFCR, (inpw(CFCR)&0xfffffffb)|0x04);
			val = inpw(CFDR);
		} while (val & CF_STATE_BUSY);
#endif
	}

	if (isr & 0x02)
	{
		CF_8_DataReady = 1;
		outpw(CFISR, isr|0x02);

#ifdef CF_HW_MODE
		do {
			outpw(CFCR, (inpw(CFCR)&0xffffff03)|0x74);
			val = inpw(CFCSR);
		} while (val & CF_STATE_BUSY);
#else
		do {
			outpw(CFDCR, 0x07);
			outpw(CFCR, (inpw(CFCR)&0xfffffffb)|0x04);
			val = inpw(CFDR);
		} while (val & CF_STATE_BUSY);
#endif
	}
}


void CF_Reset()
{
	outpw(CFCR, inpw(CFCR) & 0xfffeffff);	// reset=0
	outpw(CFCR, inpw(CFCR) | 0x10000);		// reset=1
}


#ifdef CF_HW_MODE
int CF_Initial()
{
#if 0
	unsigned int status;

	status = inpw(CFISR);
	if ((status & 0x10) == 0x10)	// 8-bit
	{
		outpw(CFCR, 0x10000);	// csel=0, reset=1, dma r/w disable, transfer size=512
		printf("8-bit CF Card!!\n");
	}
	else
	{
		printf("Not found 8-bit CF Card!!\n");
	}
#endif

	outpw(CFCR, 0x10000);	// csel=0, reset=1, dma r/w disable, transfer size=512

#ifdef _USE_IRQ
	outpw(CFIER, 0x03);
#endif
	return Success;
}

int CF_Identify(unsigned int bufno)
{
	unsigned int val;

	if (bufno == 0)
		outpw(FMICR, (inpw(FMICR) & 0xffffff8f)|0x10);
	if (bufno == 1)
		outpw(FMICR, (inpw(FMICR) & 0xffffff8f)|0x50);

	do {
		outpw(CFCR, (inpw(CFCR)&0xffffff0b)|0x74);
		val = inpw(CFCSR);
	} while (val & CF_STATE_BUSY);

	outpw(CFSCHR, 0xe0);	// LBA mode, LBA27~24=0

	do {
		outpw(CFCR, (inpw(CFCR)&0xffffff0b)|0x74);
		val = inpw(CFCSR);
	} while ((!(val & CF_STATE_RDY)) || (val & CF_STATE_BUSY));

	outpw(CFCSR, 0xec);		// identify command

	do {
		outpw(CFCR, (inpw(CFCR)&0xffffff0b)|0x74);
		val = inpw(CFCSR);
	} while ((!(val & CF_STATE_DRQ)) || (val & CF_STATE_BUSY));

	outpw(CFDCR, 0x00);
	outpw(CFCR, inpw(CFCR)|0x02);	// CF->buffer
	while (inpw(CFCR) & 0x02);
	Buffer2SDRAM((unsigned int)IDBuffer, bufno);

	outpw(CFCR, (inpw(CFCR)&0xffffff0b)|0x74);
	val = inpw(CFCSR);
	if (val & CF_STATE_ERR)
		return Fail;

	return Success;
}


int CF_Multiple(int Scount)
{
	unsigned int val;

	do {
		outpw(CFCR, (inpw(CFCR)&0xffffff03)|0x74);
		val = inpw(CFCSR);
	} while (val & CF_STATE_BUSY);

	outpw(CFSCR, Scount);	// sector count

	do {
		outpw(CFCR, (inpw(CFCR)&0xffffff03)|0x74);
		val = inpw(CFCSR);
	} while ((!(val & CF_STATE_RDY)) || (val & CF_STATE_BUSY));

	outpw(CFCSR, 0xc6);		// multiple command

	do {
		outpw(CFCR, (inpw(CFCR)&0xffffff03)|0x74);
		val = inpw(CFCSR);
	} while ((!(val & CF_STATE_RDY)) || (val & CF_STATE_BUSY));

	outpw(CFCR, (inpw(CFCR)&0xffffff03)|0x74);
	val = inpw(CFCSR);
	if (val & CF_STATE_ERR)
		return Fail;
	return Success;
}


int Buffer2CF(unsigned int sector, unsigned char bufno)
{
	unsigned int val;

	if (bufno == 0)
		outpw(FMICR, (inpw(FMICR) & 0xfffff8ff)|0x100);
	else if (bufno == 1)
		outpw(FMICR, (inpw(FMICR) & 0xfffff8ff)|0x500);

	do {
		outpw(CFCR, (inpw(CFCR)&0xffffff03)|0x74);
		val = inpw(CFCSR);
	} while (val & CF_STATE_BUSY);

	outpw(CFSCHR, 0xe0);	// LBA mode, LBA27~24=0

	do {
		outpw(CFCR, (inpw(CFCR)&0xffffff03)|0x74);
		val = inpw(CFCSR);
	} while ((!(val & CF_STATE_RDY)) || (val & CF_STATE_BUSY));

	outpw(CFSCR, 1);							// sector count
	outpw(CFSNR, sector & 0xff);				// sector number
	outpw(CFCLR, (sector & 0xff00) >> 8);		// sector number
	outpw(CFCHR, (sector & 0xff0000) >> 16);	// sector number

	outpw(CFCSR, 0x30);		// write sector command

	do {
		outpw(CFCR, (inpw(CFCR)&0xffffff03)|0x74);
		val = inpw(CFCSR);
	} while ((!(val & CF_STATE_DRQ)) || (val & CF_STATE_BUSY));

#ifdef _USE_IRQ
	CF_8_DataReady = 0;
#endif

	outpw(CFDCR, 0x00);
	outpw(CFCR, inpw(CFCR)|0x01);	// buffer->CF
#ifdef _USE_IRQ
	while(CF_8_DataReady == 0);
#else
	while (inpw(CFCR) & 0x01);
#endif

	outpw(CFCR, (inpw(CFCR)&0xffffff0b)|0x74);
	val = inpw(CFCSR);
	if (val & CF_STATE_ERR)
		return Fail;
	return Success;
}


int CF2Buffer(unsigned int sector, unsigned char bufno)
{
	unsigned int val;

	if (bufno == 0)
		outpw(FMICR, (inpw(FMICR) & 0xffffff8f)|0x10);
	else if (bufno == 1)
		outpw(FMICR, (inpw(FMICR) & 0xffffff8f)|0x50);

	do {
		outpw(CFCR, (inpw(CFCR)&0xffffff03)|0x74);
		val = inpw(CFCSR);
	} while (val & CF_STATE_BUSY);

	outpw(CFSCHR, 0xe0);	// LBA mode, LBA27~24=0

	do {
		outpw(CFCR, (inpw(CFCR)&0xffffff03)|0x74);
		val = inpw(CFCSR);
	} while ((!(val & CF_STATE_RDY)) || (val & CF_STATE_BUSY));

	outpw(CFSCR, 1);							// sector count
	outpw(CFSNR, sector & 0xff);				// sector number
	outpw(CFCLR, (sector & 0xff00) >> 8);		// sector number
	outpw(CFCHR, (sector & 0xff0000) >> 16);	// sector number

	outpw(CFSCHR, 0xe0);	// LBA mode, LBA27~24=0
	outpw(CFCSR, 0x20);		// read sector command

	do {
		outpw(CFCR, (inpw(CFCR)&0xffffff03)|0x74);
		val = inpw(CFCSR);
	} while ((!(val & CF_STATE_DRQ)) || (val & CF_STATE_BUSY));

#ifdef _USE_IRQ
	CF_8_DataReady = 0;
#endif

	outpw(CFDCR, 0x00);
	outpw(CFCR, inpw(CFCR)|0x02);	// CF->buffer
#ifdef _USE_IRQ
	while(CF_8_DataReady == 0);
#else
	while (inpw(CFCR) & 0x02);
#endif
	outpw(CFCR, (inpw(CFCR)&0xffffff0b)|0x74);
	val = inpw(CFCSR);
	if (val & CF_STATE_ERR)
		return Fail;
	return Success;
}


void Buffer2CFM(unsigned int sector, unsigned int bufcnt, unsigned char bufno)
{
	unsigned int val;

	if (bufno == 0)
		outpw(FMICR, (inpw(FMICR) & 0xfffff8ff)|0x100);
	else if (bufno == 1)
		outpw(FMICR, (inpw(FMICR) & 0xfffff8ff)|0x500);

	do {
		outpw(CFCR, (inpw(CFCR)&0xffffff03)|0x74);
		val = inpw(CFCSR);
	} while (val & CF_STATE_BUSY);

	outpw(CFSCHR, 0xe0);	// LBA mode, LBA27~24=0

	do {
		outpw(CFCR, (inpw(CFCR)&0xffffff03)|0x74);
		val = inpw(CFCSR);
	} while ((!(val & CF_STATE_RDY)) || (val & CF_STATE_BUSY));

	outpw(CFSCR, bufcnt);						// sector count
	outpw(CFSNR, sector & 0xff);				// sector number
	outpw(CFCLR, (sector & 0xff00) >> 8);		// sector number
	outpw(CFCHR, (sector & 0xff0000) >> 16);	// sector number

	outpw(CFCSR, 0xc5);		// write multiple command
}


void CF2BufferM(unsigned int sector, unsigned int bufcnt, unsigned char bufno)
{
	unsigned int val;

	if (bufno == 0)
		outpw(FMICR, (inpw(FMICR) & 0xffffff8f)|0x10);
	else if (bufno == 1)
		outpw(FMICR, (inpw(FMICR) & 0xffffff8f)|0x50);

	do {
		outpw(CFCR, (inpw(CFCR)&0xffffff03)|0x74);
		val = inpw(CFCSR);
	} while (val & CF_STATE_BUSY);

	outpw(CFSCHR, 0xe0);	// LBA mode, LBA27~24=0

	do {
		outpw(CFCR, (inpw(CFCR)&0xffffff03)|0x74);
		val = inpw(CFCSR);
	} while ((!(val & CF_STATE_RDY)) || (val & CF_STATE_BUSY));

	outpw(CFSCR, bufcnt);						// sector count
	outpw(CFSNR, sector & 0xff);				// sector number
	outpw(CFCLR, (sector & 0xff00) >> 8);		// sector number
	outpw(CFCHR, (sector & 0xff0000) >> 16);	// sector number

	outpw(CFSCHR, 0xe0);	// LBA mode, LBA27~24=0
	outpw(CFCSR, 0xc4);		// read multiple command
}


void CF_Read(unsigned int sector, unsigned int bufcnt, unsigned int DAddr)
{
	unsigned int val;
	unsigned int Rcount, Wcount;
	unsigned int Raddr, Waddr=0;
	unsigned int multiCount=0;

	Rcount = Wcount = bufcnt;
	Raddr = sector;
	buffer0 = buffer1 = 0;

	while ((Rcount > 0) || (Wcount > 0))
	{
		if (multiCount > 0xff)
		{
			CF2BufferM(Raddr, bufcnt, 0);
			multiCount = 0;
		}

		if (Rcount > 0)
		{
			if ((buffer0 == 0) && (buffer1 == 0))
			{
#ifdef _USE_IRQ
				CF_8_DataReady = _DataReady = 0;
#endif
				CF2BufferM(Raddr, bufcnt, 0);
				multiCount++;
				do {
					outpw(CFCR, (inpw(CFCR)&0xffffff03)|0x74);
					val = inpw(CFCSR);
				} while ((!(val & CF_STATE_DRQ)) || (val & CF_STATE_BUSY));

				outpw(CFDCR, 0x00);
				outpw(CFCR, inpw(CFCR)|0x02);	// CF->buffer
				Rcount = Rcount - 1;
				Raddr = Raddr + 1;
#ifdef _USE_IRQ
				while(CF_8_DataReady == 0);
#else
				while (inpw(CFCR) & 0x02);
#endif
				buffer0 = 1;

				outpw(CFCR, (inpw(CFCR)&0xffffff0b)|0x74);
				val = inpw(CFCSR);
				if (val & CF_STATE_ERR)
					printf("read data error !!\n");
			}
		}

		if ((Rcount > 0) && (Wcount > 0))
		{
			if ((buffer0 == 1) && (buffer1 == 0))
			{
				outpw(FMICR, (inpw(FMICR) & 0xffffff8f)|0x50);	// buffer 1
				multiCount++;
				Rcount = Rcount - 1;
				Raddr = Raddr + 1;
				SDRAM_Write(DAddr, Waddr, 0);
				Wcount = Wcount - 1;
				Waddr = Waddr + 1;

				do {
					outpw(CFCR, (inpw(CFCR)&0xffffff03)|0x74);
					val = inpw(CFCSR);
				} while ((!(val & CF_STATE_DRQ)) || (val & CF_STATE_BUSY));

#ifdef _USE_IRQ
				CF_8_DataReady = _DataReady = 0;
#endif
				outpw(CFDCR, 0x00);
				outpw(CFCR, inpw(CFCR)|0x02);	// CF->buffer
				outpw(FMICR, inpw(FMICR) | 0x04);	// enable DMA
#ifdef _USE_IRQ
				while ((CF_8_DataReady == 0) || (_DataReady == 0));
#else
				while (inpw(CFCR) & 0x02);
				while (inpw(FMICR) & 0x04);
#endif
				buffer0 = 0;
				buffer1 = 1;

				outpw(CFCR, (inpw(CFCR)&0xffffff0b)|0x74);
				val = inpw(CFCSR);
				if (val & CF_STATE_ERR)
					printf("read data error !!\n");
			}
			else if ((buffer0 == 0) && (buffer1 == 1))
			{
				outpw(FMICR, (inpw(FMICR) & 0xffffff8f)|0x10);	// buffer 0
				multiCount++;
				Rcount = Rcount - 1;
				Raddr = Raddr + 1;
				SDRAM_Write(DAddr, Waddr, 1);
				Wcount = Wcount - 1;
				Waddr = Waddr + 1;

				do {
					outpw(CFCR, (inpw(CFCR)&0xffffff03)|0x74);
					val = inpw(CFCSR);
				} while ((!(val & CF_STATE_DRQ)) || (val & CF_STATE_BUSY));

#ifdef _USE_IRQ
				CF_8_DataReady = _DataReady = 0;
#endif
				outpw(CFDCR, 0x00);
				outpw(CFCR, inpw(CFCR)|0x02);	// CF->buffer
				outpw(FMICR, inpw(FMICR) | 0x04);	// enable DMA
#ifdef _USE_IRQ
				while ((CF_8_DataReady == 0) || (_DataReady == 0));
#else
				while (inpw(CFCR) & 0x02);
				while (inpw(FMICR) & 0x04);
#endif
				buffer0 = 1;
				buffer1 = 0;

				outpw(CFCR, (inpw(CFCR)&0xffffff0b)|0x74);
				val = inpw(CFCSR);
				if (val & CF_STATE_ERR)
					printf("read data error !!\n");
			}
		}

		if ((Rcount == 0) && (Wcount == 1))
		{
			if ((buffer0 == 1) && (buffer1 == 0))
			{
				SDRAM_Write(DAddr, Waddr, 0);
#ifdef _USE_IRQ
				_DataReady = 0;
#endif
				outpw(FMICR, inpw(FMICR) | 0x04);	// enable DMA
				Wcount = Wcount - 1;
				Waddr = Waddr + 1;
#ifdef _USE_IRQ
				while (_DataReady == 0);
#else
				while (inpw(FMICR) & 0x04);
#endif
				buffer0 = 0;
				buffer1 = 0;
			}
			else if ((buffer0 == 0) && (buffer1 == 1))
			{
				SDRAM_Write(DAddr, Waddr, 1);
#ifdef _USE_IRQ
				_DataReady = 0;
#endif
				outpw(FMICR, inpw(FMICR) | 0x04);	// enable DMA
				Wcount = Wcount - 1;
				Waddr = Waddr + 1;
#ifdef _USE_IRQ
				while (_DataReady == 0);
#else
				while (inpw(FMICR) & 0x04);
#endif
				buffer0 = 0;
				buffer1 = 0;
			}
		}
	}
}


void CF_Write(unsigned int sector, unsigned int bufcnt, unsigned int SAddr)
{
	unsigned int val;
	unsigned int Rcount, Wcount;
	unsigned int Raddr=0, Waddr;
	unsigned int firstcmd=1, multiCount=0;

	Rcount = Wcount = bufcnt;
	Waddr = sector;
	buffer0 = buffer1 = 0;

	while ((Rcount > 0) || (Wcount > 0))
	{
		if (multiCount > 0xff)
		{
			multiCount = 0;
			firstcmd = 1;
		}

		if (Rcount > 0)
		{
			if ((buffer0 == 0) && (buffer1 == 0))
			{
#ifdef _USE_IRQ
				CF_8_DataReady = _DataReady = 0;
#endif
				SDRAM_Read(SAddr, Raddr, 0);
				outpw(FMICR, inpw(FMICR) | 0x08);	// enable DMA
				Rcount = Rcount - 1;
				Raddr = Raddr + 1;
#ifdef _USE_IRQ
				while (_DataReady == 0);
#else
				while (inpw(FMICR) & 0x08);
#endif
				buffer0 = 1;
			}
		}

		if ((Rcount > 0) && (Wcount > 0))
		{
			if ((buffer0 == 1) && (buffer1 == 0))
			{
				if (firstcmd == 1)
				{
					Buffer2CFM(Waddr, bufcnt, 0);
					multiCount++;
					firstcmd = 0;
				}
				else
				{
					outpw(FMICR, (inpw(FMICR) & 0xfffff8ff)|0x100);	// buffer 0
					multiCount++;
				}
				Wcount = Wcount - 1;
				Waddr = Waddr + 1;

				SDRAM_Read(SAddr, Raddr, 1);
				Rcount = Rcount - 1;
				Raddr = Raddr + 1;

				do {
					outpw(CFCR, (inpw(CFCR)&0xffffff03)|0x74);
					val = inpw(CFCSR);
				} while ((!(val & CF_STATE_DRQ)) || (val & CF_STATE_BUSY));

#ifdef _USE_IRQ
				CF_8_DataReady = _DataReady = 0;
#endif
				outpw(CFDCR, 0x00);
				outpw(CFCR, inpw(CFCR)|0x01);	// buffer->CF
				outpw(FMICR, inpw(FMICR) | 0x08);	// enable DMA
#ifdef _USE_IRQ
				while ((_DataReady == 0) || (CF_8_DataReady == 0));
#else
				while (inpw(CFCR) & 0x01);
				while (inpw(FMICR) & 0x08);
#endif
				buffer0 = 0;
				buffer1 = 1;

				outpw(CFCR, (inpw(CFCR)&0xffffff0b)|0x74);
				val = inpw(CFCSR);
				if (val & CF_STATE_ERR)
					printf("write data error !!\n");
			}
			else if ((buffer0 == 0) && (buffer1 == 1))
			{
				outpw(FMICR, (inpw(FMICR) & 0xfffff8ff)|0x500);		// buffer 1
				multiCount++;
				Wcount = Wcount - 1;
				Waddr = Waddr + 1;

				SDRAM_Read(SAddr, Raddr, 0);
				Rcount = Rcount - 1;
				Raddr = Raddr + 1;

				do {
					outpw(CFCR, (inpw(CFCR)&0xffffff03)|0x74);
					val = inpw(CFCSR);
				} while ((!(val & CF_STATE_DRQ)) || (val & CF_STATE_BUSY));

#ifdef _USE_IRQ
				CF_8_DataReady = _DataReady = 0;
#endif
				outpw(CFDCR, 0x00);
				outpw(CFCR, inpw(CFCR)|0x01);	// buffer->CF
				outpw(FMICR, inpw(FMICR) | 0x08);	// enable DMA
#ifdef _USE_IRQ
				while ((_DataReady == 0) || (CF_8_DataReady == 0));
#else
				while (inpw(CFCR) & 0x01);
				while (inpw(FMICR) & 0x08);
#endif
				buffer0 = 1;
				buffer1 = 0;

				outpw(CFCR, (inpw(CFCR)&0xffffff0b)|0x74);
				val = inpw(CFCSR);
				if (val & CF_STATE_ERR)
					printf("write data error !!\n");
			}
		}

		if ((Rcount == 0) && (Wcount == 1))
		{
			if ((buffer0 == 1) && (buffer1 == 0))
			{
				if (firstcmd == 1)
				{
					Buffer2CFM(Waddr, bufcnt, 0);
					multiCount++;
					firstcmd = 0;
				}
				else
				{
					outpw(FMICR, (inpw(FMICR) & 0xfffff8ff)|0x100);	// buffer 0
					multiCount++;
				}
				do {
					outpw(CFCR, (inpw(CFCR)&0xffffff03)|0x74);
					val = inpw(CFCSR);
				} while ((!(val & CF_STATE_DRQ)) || (val & CF_STATE_BUSY));

#ifdef _USE_IRQ
				CF_8_DataReady = 0;
#endif
				outpw(CFDCR, 0x00);
				outpw(CFCR, inpw(CFCR)|0x01);	// buffer->CF
				Wcount = Wcount - 1;
				Waddr = Waddr + 1;
#ifdef _USE_IRQ
				while (CF_8_DataReady == 0);
#else
				while (inpw(CFCR) & 0x01);
#endif
				buffer0 = 0;
				buffer1 = 0;
			}
			else if ((buffer0 == 0) && (buffer1 == 1))
			{
				if (firstcmd == 1)
				{
					Buffer2CFM(Waddr, bufcnt, 0);
					multiCount++;
					firstcmd = 0;
				}
				else
				{
					outpw(FMICR, (inpw(FMICR) & 0xfffff8ff)|0x500);		// buffer 1
					multiCount++;
				}
				do {
					outpw(CFCR, (inpw(CFCR)&0xffffff03)|0x74);
					val = inpw(CFCSR);
				} while ((!(val & CF_STATE_DRQ)) || (val & CF_STATE_BUSY));

#ifdef _USE_IRQ
				CF_8_DataReady = 0;
#endif
				outpw(CFDCR, 0x00);
				outpw(CFCR, inpw(CFCR)|0x01);	// buffer->CF
				Wcount = Wcount - 1;
				Waddr = Waddr + 1;
#ifdef _USE_IRQ
				while (CF_8_DataReady == 0);
#else
				while (inpw(CFCR) & 0x01);
#endif
				buffer0 = 0;
				buffer1 = 0;
			}
		}
	}

	outpw(CFCR, (inpw(CFCR)&0xffffff0b)|0x74);
	val = inpw(CFCSR);
	if (val & CF_STATE_ERR)
		printf("write data error !!\n");
}



#else	// software mode
int CF_Initial()
{
#if 0
	unsigned int status;

	status = inpw(CFISR);
	if ((status & 0x10) == 0x10)	// 8-bit
	{
		outpw(CFCR, 0x10100);	// csel=0, reset=1, dma r/w disable, transfer size=512
		outpw(CFDAR, 0x05);		// ce2_=1, ce1_=0, reg_=1
		printf("8-bit CF-card!\n");
	}
	else	// 16-bit
	{
		printf("Not found 8-bit CF-card!\n");
	}
#endif

	outpw(CFCR, 0x10100);	// csel=0, reset=1, dma r/w disable, transfer size=512
	outpw(CFDAR, 0x05);		// ce2_=1, ce1_=0, reg_=1

#ifdef _USE_IRQ
	outpw(CFIER, 0x03);
#endif
	return Success;
}


int CF_Identify(unsigned int bufno)
{
	unsigned int val;

	if (bufno == 0)
		outpw(FMICR, (inpw(FMICR) & 0xffffff8f)|0x10);
	else if (bufno == 1)
		outpw(FMICR, (inpw(FMICR) & 0xffffff8f)|0x50);

	do {
		outpw(CFDCR, 0x07);
		outpw(CFCR, (inpw(CFCR)&0xfffffffb)|0x04);
		val = inpw(CFDR);
	} while (val & CF_STATE_BUSY);

	outpw(CFDCR, 0x06);
	outpw(CFDR, 0xe0);	// LBA mode, LBA27~24=0

	do {
		outpw(CFDCR, 0x07);
		outpw(CFCR, (inpw(CFCR)&0xfffffffb)|0x04);
		val = inpw(CFDR);
	} while ((!(val & CF_STATE_RDY)) || (val & CF_STATE_BUSY));

	outpw(CFDCR, 0x07);
	outpw(CFDR, 0xec);	// identify command

	do {
		outpw(CFDCR, 0x07);
		outpw(CFCR, (inpw(CFCR)&0xfffffffb)|0x04);
		val = inpw(CFDR);
	} while ((!(val & CF_STATE_DRQ)) || (val & CF_STATE_BUSY));

	outpw(CFDCR, 0x00);
	outpw(CFCR, inpw(CFCR)|0x02);	// CF->buffer
	while (inpw(CFCR) & 0x02);
	Buffer2SDRAM((unsigned int)IDBuffer, bufno);

	outpw(CFDCR, 0x07);
	outpw(CFCR, (inpw(CFCR)&0xfffffffb)|0x04);
	val = inpw(CFDR);
	if (val & CF_STATE_ERR)
		return Fail;

	return Success;
}


int CF_Multiple(int Scount)
{
	unsigned int val;

	do {
		outpw(CFDCR, 0x07);
		outpw(CFCR, (inpw(CFCR)&0xfffffffb)|0x04);
		val = inpw(CFDR);
	} while (val & CF_STATE_BUSY);

	outpw(CFDCR, 0x02);
	outpw(CFDR, Scount);	// sector count

	do {
		outpw(CFDCR, 0x07);
		outpw(CFCR, (inpw(CFCR)&0xfffffffb)|0x04);
		val = inpw(CFDR);
	} while ((!(val & CF_STATE_RDY)) || (val & CF_STATE_BUSY));

	outpw(CFDCR, 0x07);
	outpw(CFDR, 0xc6);	// multiple command

	do {
		outpw(CFDCR, 0x07);
		outpw(CFCR, (inpw(CFCR)&0xfffffffb)|0x04);
		val = inpw(CFDR);
	} while ((!(val & CF_STATE_RDY)) || (val & CF_STATE_BUSY));

	outpw(CFDCR, 0x07);
	outpw(CFCR, (inpw(CFCR)&0xfffffffb)|0x04);
	val = inpw(CFDR);
	if (val & CF_STATE_ERR)
		return Fail;

	return Success;
}


int Buffer2CF(unsigned int sector, unsigned char bufno)
{
	unsigned int val;

	if (bufno == 0)
		outpw(FMICR, (inpw(FMICR) & 0xfffff8ff)|0x100);
	else if (bufno == 1)
		outpw(FMICR, (inpw(FMICR) & 0xfffff8ff)|0x500);

	do {
		outpw(CFDCR, 0x07);
		outpw(CFCR, (inpw(CFCR)&0xfffffffb)|0x04);
		val = inpw(CFDR);
	} while (val & CF_STATE_BUSY);

	outpw(CFDCR, 0x06);
	outpw(CFDR, 0xe0);	// LBA mode, LBA27~24=0

	do {
		outpw(CFDCR, 0x07);
		outpw(CFCR, (inpw(CFCR)&0xfffffffb)|0x04);
		val = inpw(CFDR);
	} while ((!(val & CF_STATE_RDY)) || (val & CF_STATE_BUSY));

	outpw(CFDCR, 0x02);
	outpw(CFDR, 0x01);				// sector count
	outpw(CFDCR, 0x03);
	outpw(CFDR, sector & 0xff);		// sector NO.
	outpw(CFDCR, 0x04);
	outpw(CFDR, (sector & 0xff00) >> 8);	// cylinder low
	outpw(CFDCR, 0x05);
	outpw(CFDR, (sector & 0xff0000) >> 16);	// cylinder high

	outpw(CFDCR, 0x07);
	outpw(CFDR, 0x30);	// write sector command

	do {
		outpw(CFDCR, 0x07);
		outpw(CFCR, (inpw(CFCR)&0xfffffffb)|0x04);
		val = inpw(CFDR);
	} while ((!(val & CF_STATE_DRQ)) || (val & CF_STATE_BUSY));

#ifdef _USE_IRQ
	CF_8_DataReady = 0;
#endif

	outpw(CFDCR, 0x0);
	outpw(CFCR, inpw(CFCR)|0x01);	// buffer->CF

#ifdef _USE_IRQ
	while(CF_8_DataReady == 0);
#else
	while (inpw(CFCR) & 0x01);
#endif

	outpw(CFDCR, 0x07);
	outpw(CFCR, (inpw(CFCR)&0xfffffffb)|0x04);
	val = inpw(CFDR);
	if (val & CF_STATE_ERR)
		return Fail;

	return Success;
}


int CF2Buffer(unsigned int sector, unsigned char bufno)
{
	unsigned int val;

	if (bufno == 0)
		outpw(FMICR, (inpw(FMICR) & 0xffffff8f)|0x10);
	else if (bufno == 1)
		outpw(FMICR, (inpw(FMICR) & 0xffffff8f)|0x50);

	do {
		outpw(CFDCR, 0x07);
		outpw(CFCR, (inpw(CFCR)&0xfffffffb)|0x04);
		val = inpw(CFDR);
	} while (val & CF_STATE_BUSY);

	outpw(CFDCR, 0x06);
	outpw(CFDR, 0xe0);	// LBA mode, LBA27~24=0

	do {
		outpw(CFDCR, 0x07);
		outpw(CFCR, (inpw(CFCR)&0xfffffffb)|0x04);
		val = inpw(CFDR);
	} while ((!(val & CF_STATE_RDY)) || (val & CF_STATE_BUSY));

	outpw(CFDCR, 0x02);
	outpw(CFDR, 0x01);				// sector count
	outpw(CFDCR, 0x03);
	outpw(CFDR, sector & 0xff);		// sector NO.
	outpw(CFDCR, 0x04);
	outpw(CFDR, (sector & 0xff00) >> 8);	// cylinder low
	outpw(CFDCR, 0x05);
	outpw(CFDR, (sector & 0xff0000) >> 16);	// cylinder high
	outpw(CFDCR, 0x06);
	outpw(CFDR, 0xe0);	// LBA mode, LBA27~24=0

	outpw(CFDCR, 0x07);
	outpw(CFDR, 0x20);	// read sector command

	do {
		outpw(CFDCR, 0x07);
		outpw(CFCR, (inpw(CFCR)&0xfffffffb)|0x04);
		val = inpw(CFDR);
	} while ((!(val & CF_STATE_DRQ)) || (val & CF_STATE_BUSY));

#ifdef _USE_IRQ
	CF_8_DataReady = 0;
#endif

	outpw(CFDCR, 0x0);
	outpw(CFCR, inpw(CFCR)|0x02);	// CF->buffer

#ifdef _USE_IRQ
	while(CF_8_DataReady == 0);
#else
	while (inpw(CFCR) & 0x02);
#endif

	outpw(CFDCR, 0x07);
	outpw(CFCR, (inpw(CFCR)&0xfffffffb)|0x04);
	val = inpw(CFDR);
	if (val & CF_STATE_ERR)
		return Fail;

	return Success;
}


void Buffer2CFM(unsigned int sector, unsigned int bufcnt, unsigned char bufno)
{
	unsigned int val;

	if (bufno == 0)
		outpw(FMICR, (inpw(FMICR) & 0xfffff8ff)|0x100);
	else if (bufno == 1)
		outpw(FMICR, (inpw(FMICR) & 0xfffff8ff)|0x500);

	do {
		outpw(CFDCR, 0x07);
		outpw(CFCR, (inpw(CFCR)&0xfffffffb)|0x04);
		val = inpw(CFDR);
	} while (val & CF_STATE_BUSY);

	outpw(CFDCR, 0x06);
	outpw(CFDR, 0xe0);	// LBA mode, LBA27~24=0

	do {
		outpw(CFDCR, 0x07);
		outpw(CFCR, (inpw(CFCR)&0xfffffffb)|0x04);
		val = inpw(CFDR);
	} while ((!(val & CF_STATE_RDY)) || (val & CF_STATE_BUSY));

	outpw(CFDCR, 0x02);
	outpw(CFDR, bufcnt);			// sector count
	outpw(CFDCR, 0x03);
	outpw(CFDR, sector & 0xff);		// sector NO.
	outpw(CFDCR, 0x04);
	outpw(CFDR, (sector & 0xff00) >> 8);	// cylinder low
	outpw(CFDCR, 0x05);
	outpw(CFDR, (sector & 0xff0000) >> 16);	// cylinder high

	outpw(CFDCR, 0x07);
	outpw(CFDR, 0xc5);	// write multiple command
}


void CF2BufferM(unsigned int sector, unsigned int bufcnt, unsigned char bufno)
{
	unsigned int val;

	if (bufno == 0)
		outpw(FMICR, (inpw(FMICR) & 0xffffff8f)|0x10);
	else if (bufno == 1)
		outpw(FMICR, (inpw(FMICR) & 0xffffff8f)|0x50);

	do {
		outpw(CFDCR, 0x07);
		outpw(CFCR, (inpw(CFCR)&0xfffffffb)|0x04);
		val = inpw(CFDR);
	} while (val & CF_STATE_BUSY);

	outpw(CFDCR, 0x06);
	outpw(CFDR, 0xe0);	// LBA mode, LBA27~24=0

	do {
		outpw(CFDCR, 0x07);
		outpw(CFCR, (inpw(CFCR)&0xfffffffb)|0x04);
		val = inpw(CFDR);
	} while ((!(val & CF_STATE_RDY)) || (val & CF_STATE_BUSY));

	outpw(CFDCR, 0x02);
	outpw(CFDR, bufcnt);			// sector count
	outpw(CFDCR, 0x03);
	outpw(CFDR, sector & 0xff);		// sector NO.
	outpw(CFDCR, 0x04);
	outpw(CFDR, (sector & 0xff00) >> 8);	// cylinder low
	outpw(CFDCR, 0x05);
	outpw(CFDR, (sector & 0xff0000) >> 16);	// cylinder high
	outpw(CFDCR, 0x06);
	outpw(CFDR, 0xe0);	// LBA mode, LBA27~24=0

	outpw(CFDCR, 0x07);
	outpw(CFDR, 0xc4);	// read multiple command
}


void CF_Read(unsigned int sector, unsigned int bufcnt, unsigned int DAddr)
{
	unsigned int val;
	unsigned int Rcount, Wcount;
	unsigned int Raddr, Waddr=0;

	Rcount = Wcount = bufcnt;
	Raddr = sector;
	buffer0 = buffer1 = 0;

	while ((Rcount > 0) || (Wcount > 0))
	{
		if (Rcount > 0)
		{
			if ((buffer0 == 0) && (buffer1 == 0))
			{
#ifdef _USE_IRQ
				CF_8_DataReady = _DataReady = 0;
#endif
				CF2BufferM(Raddr, bufcnt, 0);
				do {
					outpw(CFDCR, 0x07);
					outpw(CFCR, (inpw(CFCR)&0xfffffffb)|0x04);
					val = inpw(CFDR);
				} while ((!(val & CF_STATE_DRQ)) || (val & CF_STATE_BUSY));

				outpw(CFDCR, 0x0);
				outpw(CFCR, inpw(CFCR)|0x02);	// CF->buffer
				Rcount = Rcount - 1;
				Raddr = Raddr + 1;
#ifdef _USE_IRQ
				while(CF_8_DataReady == 0);
#else
				while (inpw(CFCR) & 0x02);
#endif
				buffer0 = 1;
				outpw(CFDCR, 0x07);
				outpw(CFCR, (inpw(CFCR)&0xfffffffb)|0x04);
				val = inpw(CFDR);
				if (val & CF_STATE_ERR)
					printf("read data error !!\n");
			}
		}

		if ((Rcount > 0) && (Wcount > 0))
		{
			if ((buffer0 == 1) && (buffer1 == 0))
			{
				outpw(FMICR, (inpw(FMICR) & 0xffffff8f)|0x50);	// buffer 1
				Rcount = Rcount - 1;
				Raddr = Raddr + 1;
				SDRAM_Write(DAddr, Waddr, 0);
				Wcount = Wcount - 1;
				Waddr = Waddr + 1;

				do {
					outpw(CFDCR, 0x07);
					outpw(CFCR, (inpw(CFCR)&0xfffffffb)|0x04);
					val = inpw(CFDR);
				} while ((!(val & CF_STATE_DRQ)) || (val & CF_STATE_BUSY));

#ifdef _USE_IRQ
				CF_8_DataReady = _DataReady = 0;
#endif
				outpw(CFDCR, 0x0);
				outpw(CFCR, inpw(CFCR)|0x02);	// CF->buffer
				outpw(FMICR, inpw(FMICR) | 0x04);	// enable DMA
#ifdef _USE_IRQ
				while ((CF_8_DataReady == 0) || (_DataReady == 0));
#else
				while (inpw(CFCR) & 0x02);
				while (inpw(FMICR) & 0x04);
#endif
				buffer0 = 0;
				buffer1 = 1;
				outpw(CFDCR, 0x07);
				outpw(CFCR, (inpw(CFCR)&0xfffffffb)|0x04);
				val = inpw(CFDR);
				if (val & CF_STATE_ERR)
					printf("read data error !!\n");
			}
			else if ((buffer0 == 0) && (buffer1 == 1))
			{
				outpw(FMICR, (inpw(FMICR) & 0xffffff8f)|0x10);	// buffer 0
				Rcount = Rcount - 1;
				Raddr = Raddr + 1;
				SDRAM_Write(DAddr, Waddr, 1);
				Wcount = Wcount - 1;
				Waddr = Waddr + 1;

				do {
					outpw(CFDCR, 0x07);
					outpw(CFCR, (inpw(CFCR)&0xfffffffb)|0x04);
					val = inpw(CFDR);
				} while ((!(val & CF_STATE_DRQ)) || (val & CF_STATE_BUSY));

#ifdef _USE_IRQ
				CF_8_DataReady = _DataReady = 0;
#endif
				outpw(CFDCR, 0x0);
				outpw(CFCR, inpw(CFCR)|0x02);	// CF->buffer
				outpw(FMICR, inpw(FMICR) | 0x04);	// enable DMA
#ifdef _USE_IRQ
				while ((CF_8_DataReady == 0) || (_DataReady == 0));
#else
				while (inpw(CFCR) & 0x02);
				while (inpw(FMICR) & 0x04);
#endif
				buffer0 = 1;
				buffer1 = 0;
				outpw(CFDCR, 0x07);
				outpw(CFCR, (inpw(CFCR)&0xfffffffb)|0x04);
				val = inpw(CFDR);
				if (val & CF_STATE_ERR)
					printf("read data error !!\n");
			}
		}

		if ((Rcount == 0) && (Wcount == 1))
		{
			if ((buffer0 == 1) && (buffer1 == 0))
			{
				SDRAM_Write(DAddr, Waddr, 0);
#ifdef _USE_IRQ
				_DataReady = 0;
#endif
				outpw(FMICR, inpw(FMICR) | 0x04);	// enable DMA
				Wcount = Wcount - 1;
				Waddr = Waddr + 1;
#ifdef _USE_IRQ
				while (_DataReady == 0);
#else
				while (inpw(FMICR) & 0x04);
#endif
				buffer0 = 0;
				buffer1 = 0;
			}
			else if ((buffer0 == 0) && (buffer1 == 1))
			{
				SDRAM_Write(DAddr, Waddr, 1);
#ifdef _USE_IRQ
				_DataReady = 0;
#endif
				outpw(FMICR, inpw(FMICR) | 0x04);	// enable DMA
				Wcount = Wcount - 1;
				Waddr = Waddr + 1;
#ifdef _USE_IRQ
				while (_DataReady == 0);
#else
				while (inpw(FMICR) & 0x04);
#endif
				buffer0 = 0;
				buffer1 = 0;
			}
		}
	}
}


void CF_Write(unsigned int sector, unsigned int bufcnt, unsigned int SAddr)
{
	unsigned int val;
	unsigned int Rcount, Wcount;
	unsigned int Raddr=0, Waddr;
	unsigned int firstcmd=1;

	Rcount = Wcount = bufcnt;
	Waddr = sector;
	buffer0 = buffer1 = 0;

	while ((Rcount > 0) || (Wcount > 0))
	{
		if (Rcount > 0)
		{
			if ((buffer0 == 0) && (buffer1 == 0))
			{
#ifdef _USE_IRQ
				CF_8_DataReady = _DataReady = 0;
#endif
				SDRAM_Read(SAddr, Raddr, 0);
				outpw(FMICR, inpw(FMICR) | 0x08);	// enable DMA
				Rcount = Rcount - 1;
				Raddr = Raddr + 1;
#ifdef _USE_IRQ
				while (_DataReady == 0);
#else
				while (inpw(FMICR) & 0x08);
#endif
				buffer0 = 1;
			}
		}

		if ((Rcount > 0) && (Wcount > 0))
		{
			if ((buffer0 == 1) && (buffer1 == 0))
			{
				if (firstcmd == 1)
				{
					Buffer2CFM(Waddr, bufcnt, 0);
					firstcmd = 0;
				}
				else
				{
					outpw(FMICR, (inpw(FMICR) & 0xfffff8ff)|0x100);	// buffer 0
				}
				Wcount = Wcount - 1;
				Waddr = Waddr + 1;

				SDRAM_Read(SAddr, Raddr, 1);
				Rcount = Rcount - 1;
				Raddr = Raddr + 1;

				do {
					outpw(CFDCR, 0x07);
					outpw(CFCR, (inpw(CFCR)&0xfffffffb)|0x04);
					val = inpw(CFDR);
				} while ((!(val & CF_STATE_DRQ)) || (val & CF_STATE_BUSY));

#ifdef _USE_IRQ
				CF_8_DataReady = _DataReady = 0;
#endif
				outpw(CFDCR, 0x0);
				outpw(CFCR, inpw(CFCR)|0x01);	// buffer->CF
				outpw(FMICR, inpw(FMICR) | 0x08);	// enable DMA
#ifdef _USE_IRQ
				while ((_DataReady == 0) || (CF_8_DataReady == 0));
#else
				while (inpw(CFCR) & 0x01);
				while (inpw(FMICR) & 0x08);
#endif
				buffer0 = 1;
				buffer1 = 0;
				outpw(CFDCR, 0x07);
				outpw(CFCR, (inpw(CFCR)&0xfffffffb)|0x04);
				val = inpw(CFDR);
				if (val & CF_STATE_ERR)
					printf("write data error !!\n");
			}
			else if ((buffer0 == 0) && (buffer1 == 1))
			{
				outpw(FMICR, (inpw(FMICR) & 0xfffff8ff)|0x500);		// buffer 1
				Wcount = Wcount - 1;
				Waddr = Waddr + 1;

				SDRAM_Read(SAddr, Raddr, 0);
				Rcount = Rcount - 1;
				Raddr = Raddr + 1;

				do {
					outpw(CFDCR, 0x07);
					outpw(CFCR, (inpw(CFCR)&0xfffffffb)|0x04);
					val = inpw(CFDR);
				} while ((!(val & CF_STATE_DRQ)) || (val & CF_STATE_BUSY));

#ifdef _USE_IRQ
				CF_8_DataReady = _DataReady = 0;
#endif
				outpw(CFDCR, 0x0);
				outpw(CFCR, inpw(CFCR)|0x01);	// buffer->CF
				outpw(FMICR, inpw(FMICR) | 0x08);	// enable DMA
#ifdef _USE_IRQ
				while ((_DataReady == 0) || (CF_8_DataReady == 0));
#else
				while (inpw(CFCR) & 0x01);
				while (inpw(FMICR) & 0x08);
#endif
				buffer0 = 0;
				buffer1 = 1;
				outpw(CFDCR, 0x07);
				outpw(CFCR, (inpw(CFCR)&0xfffffffb)|0x04);
				val = inpw(CFDR);
				if (val & CF_STATE_ERR)
					printf("write data error !!\n");
			}
		}

		if ((Rcount == 0) && (Wcount == 1))
		{
			if ((buffer0 == 1) && (buffer1 == 0))
			{
				if (firstcmd == 1)
				{
					Buffer2CFM(Waddr, bufcnt, 0);
					firstcmd = 0;
				}
				else
				{
					outpw(FMICR, (inpw(FMICR) & 0xfffff8ff)|0x100);	// buffer 0
				}
				do {
					outpw(CFDCR, 0x07);
					outpw(CFCR, (inpw(CFCR)&0xfffffffb)|0x04);
					val = inpw(CFDR);
				} while ((!(val & CF_STATE_DRQ)) || (val & CF_STATE_BUSY));

#ifdef _USE_IRQ
				CF_8_DataReady = 0;
#endif
				outpw(CFDCR, 0x0);
				outpw(CFCR, inpw(CFCR)|0x01);	// buffer->CF
				Wcount = Wcount - 1;
				Waddr = Waddr + 1;
#ifdef _USE_IRQ
				while (CF_8_DataReady == 0);
#else
				while (inpw(CFCR) & 0x01);
#endif
				buffer0 = 0;
				buffer1 = 0;
			}
			else if ((buffer0 == 0) && (buffer1 == 1))
			{
				if (firstcmd == 1)
				{
					Buffer2CFM(Waddr, bufcnt, 0);
					firstcmd = 0;
				}
				else
				{
					outpw(FMICR, (inpw(FMICR) & 0xfffff8ff)|0x500);		// buffer 1
				}
				do {
					outpw(CFDCR, 0x07);
					outpw(CFCR, (inpw(CFCR)&0xfffffffb)|0x04);
					val = inpw(CFDR);
				} while ((!(val & CF_STATE_DRQ)) || (val & CF_STATE_BUSY));

#ifdef _USE_IRQ
				CF_8_DataReady = 0;
#endif
				outpw(CFDCR, 0x0);
				outpw(CFCR, inpw(CFCR)|0x01);	// buffer->CF
				Wcount = Wcount - 1;
				Waddr = Waddr + 1;
#ifdef _USE_IRQ
				while (CF_8_DataReady == 0);
#else
				while (inpw(CFCR) & 0x01);
#endif
				buffer0 = 0;
				buffer1 = 0;
			}
		}
	}
	outpw(CFDCR, 0x07);
	outpw(CFCR, (inpw(CFCR)&0xfffffffb)|0x04);
	val = inpw(CFDR);
	if (val & CF_STATE_ERR)
		printf("write data error !!\n");
}


#endif

void Get_CF_info(_cf_information *_info)
{
	if (CF_Identify(0))
		printf("identify fail!!\n");
	memcpy((unsigned char *)_info, (unsigned char *)IDBuffer, sizeof(_cf_information));
}

#endif	//CF_8_DEVICE

