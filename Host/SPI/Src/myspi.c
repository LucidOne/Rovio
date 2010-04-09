#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stddef.h"
#include "wb_syslib_addon.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "errno.h"
#include "wbtypes.h"
#include "sys/types.h"
#include "time.h"
#include "cyg/kernel/kapi.h"
#include "wbio.h"
#include "tt_semaphore.h"

#include "w99702_reg.h"
#include "wblib.h"
#include "wbspi.h"


#define ERASE_SEC 65536 //64*1024
UINT8 g_uUSI_BUF[ERASE_SEC];

void DisableGPIO13(void)
{
    //outpw(REG_GPIO_IE,inpw(REG_GPIO_IE)&(~0x00002000));//enable 10/11/13/15/17/19 DMA interrupt
}
void EnableGPIO13(void)
{
    //outpw(REG_GPIO_OE,inpw(REG_GPIO_OE)|0x00002000);//13  input;

    //outpw(REG_GPIO_IE,inpw(REG_GPIO_IE)|0x00002000);//enable 10/11/13/15/17/19 DMA interrupt
}

#if 1


#if 0	/* For testing */
UINT8 test_buf[70001];
void GetFlashCapability(UINT32 *puTotalSize, UINT32 *puFreeBegin, UINT32 *puFreeEnd);
void test_usi()
{
	int i;
	UINT32 rnd_start;
	UINT32 rnd_length;
	UINT32 total_size, free_begin, free_end;
	
	GetFlashCapability(&total_size, &free_begin, &free_end);

	diag_printf("Flash: %d %d %d\n", total_size, free_begin, free_end);

	rnd_length = free_end - free_begin;
	rnd_length = 512;
	if (rnd_length > sizeof(test_buf))
		rnd_length = sizeof(test_buf);
	rnd_length = rand() % rnd_length;

	rnd_start = free_begin + rand() % (free_end - free_begin - rnd_length);

	diag_printf("Test: %d %d\n", rnd_start, rnd_length);

	for (i = 0; i < rnd_length; ++i)
		test_buf[i] = i * 3 / 7;
	usiMyWrite(rnd_start, rnd_length, test_buf);

	memset(test_buf, 0, rnd_length);
	usiMyRead(rnd_start, rnd_length, test_buf);
	for (i = 0; i < rnd_length; ++i)
	{
		if (test_buf[i] != (UINT8)(i * 3 / 7))
		{
			diag_printf("Test failed at: %d %d\n", i, rnd_start + i);
			break;
		}
	}
	diag_printf("Test successfully\n");
}
#endif

static g_uTotalSize;
static TT_RMUTEX_T g_tUSILock;
static UINT32 g_uUSICacheAddr = (UINT32)-1;


void usiMyLock()
{
	tt_rmutex_lock(&g_tUSILock);
}

void usiMyUnlock()
{
	tt_rmutex_unlock(&g_tUSILock);
}



int usiMyInitDevice(UINT32 clock_by_MHz)
{
	if (g_uTotalSize == 0)
	{
		int rt;
		tt_rmutex_init(&g_tUSILock);
		g_uUSICacheAddr = (UINT32)-1;
		rt = usiInitDevice(clock_by_MHz);
	
		g_uTotalSize = rt * 512;
		diag_printf("Total flash size: %d\n", g_uTotalSize);
		//while(1) test_usi();
	}
	return g_uTotalSize;
}


int usiMyGetTotalSize()
{
	if(g_uTotalSize == 0)
	{
		diag_printf("Please check if usiMyInitDevice() is called before usiMyGetTotalSize()\n");
	}

	return g_uTotalSize;
}


int usiMyRead(UINT32 addr, UINT32 len, UINT8 *buf)
{
	UINT32 i;
	
	tt_rmutex_lock(&g_tUSILock);
	DisableGPIO13();
	for (i = 0; i < len; )
	{
		UINT32 pos = addr + i;
		UINT32 block;
		UINT32 read_len;
		
		if (pos % ERASE_SEC == 0)
			block = pos;
		else
			block = pos - pos % ERASE_SEC;
		
		
		if (pos % 256 == 0)
		{
			read_len = len - i;
			if (read_len > 256)
				read_len = 256;
			
			if (block != g_uUSICacheAddr)
				usiRead(pos, read_len, buf + i);
			else
				memmove(buf + i,
					&g_uUSI_BUF[pos - block],
					read_len);
		}
		else
		{
			UINT8 temp[256];
			read_len = len - i + pos % 256;
			if (read_len > 256)
				read_len = 256;
			
			
			if (block != g_uUSICacheAddr)
				usiRead(pos - pos % 256, read_len, temp);
			else
				memmove(temp,
					&g_uUSI_BUF[pos - pos % 256 - block],
					read_len);
				
			read_len -= pos % 256;
			memmove(buf+i, temp + pos%256, read_len);
		}
		i += read_len;
	}
	
	EnableGPIO13();
	tt_rmutex_unlock(&g_tUSILock);
	return USI_NO_ERR;
}


int usiMyFlash()
{
	int j;
	
	tt_rmutex_lock(&g_tUSILock);
	DisableGPIO13();
	
	if (g_uUSICacheAddr != (UINT32)-1)
	{
		diag_printf("Erase&Write: %x\n", g_uUSICacheAddr); 
		usiEraseSector(g_uUSICacheAddr, 1);
		for (j = 0; j < ERASE_SEC; j += 256)
			usiWrite(g_uUSICacheAddr + j, 256, g_uUSI_BUF + j);
	}
	g_uUSICacheAddr = (UINT32)-1;
	
	EnableGPIO13();
	tt_rmutex_unlock(&g_tUSILock);
	
	return USI_NO_ERR;
}


int usiMyWrite(UINT32 addr,UINT32 len,UINT8 *buf)
{
	UINT32 i;

	tt_rmutex_lock(&g_tUSILock);
	DisableGPIO13();
	for (i = 0; i < len; )
	{
		UINT32 pos = addr + i;
		UINT32 block;
		int copy_len;
		
		if (pos % ERASE_SEC == 0)
			block = pos;
		else
			block = pos - pos % ERASE_SEC;

		
		if (block != g_uUSICacheAddr)
		{
			UINT32 j;
			
			EnableGPIO13();
			usiMyFlash();
			DisableGPIO13();
			
			diag_printf("Read: %x\n", block); 
			for (j = 0; j < ERASE_SEC; j += 256)
				usiRead(block + j, 256, g_uUSI_BUF + j);
			g_uUSICacheAddr = block;
		}
		
		
		copy_len = len - i;
		if (copy_len > ERASE_SEC - pos % ERASE_SEC)
			copy_len = ERASE_SEC - pos % ERASE_SEC;
			
		diag_printf("Write: %x %x %x, Copy: %x, %x, %x\n", 
			addr, len, buf,
			pos % ERASE_SEC, buf+i, copy_len);
		memmove(g_uUSI_BUF + pos % ERASE_SEC, buf + i, copy_len);
		i += copy_len;
	}
	EnableGPIO13();
	tt_rmutex_unlock(&g_tUSILock);
	
	return USI_NO_ERR;
}


#else
int usiMyRead(UINT32 addr,UINT32 len,UINT8 *buf)
{
	int rt = -1;
	DisableGPIO13();
	if(addr % 256 == 0)
	{
		rt = usiRead(addr,len,buf);
		EnableGPIO13();
		return rt;
	}
	else
	{
		int i = addr / 256;
		int j = addr % 256;
		UINT8 temp[256];
		if(len <= 256-j)
		{
			usiRead(i*256,256,temp);
			memcpy(buf,temp+j,len);
		}
		else
		{
			usiRead(i*256,256,temp);
			memcpy(buf,temp+j,256-j);
			usiRead((i+1)*256,len - (256 - j),buf+256-j);
		}
		EnableGPIO13();		
		return USI_NO_ERR;		
	}	
}

int usiMyWrite(UINT32 addr,UINT32 len,UINT8 *buf)
{
	UINT8 *tempbuf = buf;
	int i = (addr / ERASE_SEC);
	int j = (addr % ERASE_SEC);
	 // if address is not begin with 64*1024*i
	DisableGPIO13();
	if ( j != 0)
	{

		usiRead(i * ERASE_SEC,ERASE_SEC,g_uUSI_BUF);
		if (len > ERASE_SEC - j)
		{
			memcpy((char *)g_uUSI_BUF + j,tempbuf,ERASE_SEC - j);
			tempbuf = tempbuf + (ERASE_SEC - j);
			len = len - (ERASE_SEC - j);		
		}
		else
		{
			memcpy((char *)g_uUSI_BUF + j,tempbuf,len);
			tempbuf = tempbuf + len;
			len = 0;
		
		}
			
		usiEraseSector(i * ERASE_SEC , 1);
		usiWrite(i * ERASE_SEC,ERASE_SEC,g_uUSI_BUF);
		addr  = (i+1) * ERASE_SEC;
			
	}
	if(len > 0)
	{
		/*erase and wirte full section */			
		usiEraseSector(addr,len / ERASE_SEC);
		usiWrite(addr,(len / ERASE_SEC) * ERASE_SEC, tempbuf);
		
		addr  = addr +(len / ERASE_SEC) * ERASE_SEC;
		tempbuf  = tempbuf +(len / ERASE_SEC) * ERASE_SEC;
		len = len % ERASE_SEC;
			
		usiRead(addr ,ERASE_SEC,g_uUSI_BUF);
		usiEraseSector(addr , 1);
		memcpy((char *)g_uUSI_BUF,tempbuf,len );
		usiWrite(addr,ERASE_SEC,g_uUSI_BUF);		
	}
	EnableGPIO13();	
	return USI_NO_ERR;	
			
}

#endif
