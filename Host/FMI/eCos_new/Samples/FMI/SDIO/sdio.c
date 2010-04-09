#include "drv_api.h"
#include "stdio.h"
#include "stdlib.h"
#include "w99702_reg.h"
#include "wblib.h"
#include "wb_fmi.h"
#include "cistpl.h"

#define printf diag_printf

typedef struct {
	unsigned int clkcon;
	unsigned int apllcon;
	unsigned int upllcon;
	unsigned int clksel;
	unsigned int clkdiv0;
	unsigned int clkdiv1;
	unsigned int sdcon;
	unsigned int sdtime0;
} w99702_clk_t;

static cyg_handle_t		handle_cs;
static cyg_thread		thread_cs;
#pragma arm section zidata = "nozero"
__align(32) static UINT8 main_stack[128*1024];
#pragma arm section zidata


unsigned int volatile FUN_CIS[3];
SDIO_DATA_T sdio;
SDIO_MULTIDATA_T sdio53;
//UINT32 volatile _fmi_SDPreState;
unsigned char CCCR[0x20], CCCR53[0x20];

void GetCCCR()
{
	unsigned int volatile i;

	memset(CCCR, 0, 0x20);
	memset(CCCR53, 0, 0x20);

	// use CMD52
	// IO_RW_DIRECT
	for (i=0; i<0x12; i++)
	{
		sdio.funNo = 0;
		sdio.regAddr = i;
		sdio.IsReadAfterWrite = FALSE;
		sdio.WriteData = 0;
		CCCR[i] = fmiSDIO_Read(&sdio);
	}

	diag_printf("CCCR use CMD52 ==>\n");
	for (i=0; i<0x12; i++)
	{
		printf(" %02x", CCCR[i]);
		if (((i+1) % 8) == 0)
			printf("\n");
	}
	FUN_CIS[0] = (CCCR[0xb]<<16)|(CCCR[0xa]<<8)|CCCR[0x9];

	//====================================================
#if 0
	sdio.funNo = 0;
	sdio.regAddr = 0x02;
	sdio.IsReadAfterWrite = FALSE;
	sdio.WriteData = 0xFE;
	CCCR[i] = fmiSDIO_Write(&sdio);

	sdio.funNo = 0;
	sdio.regAddr = 0x4;
	sdio.IsReadAfterWrite = FALSE;
	sdio.WriteData = 0xFE;
	CCCR[i] = fmiSDIO_Write(&sdio);

	sdio.funNo = 0;
	sdio.regAddr = 0x11;
	sdio.IsReadAfterWrite = FALSE;
	sdio.WriteData = 0x02;
	CCCR[i] = fmiSDIO_Write(&sdio);

	printf("\n========\nfinish write CCCR use CMD52 ..\n");
#endif

	//====================================================
	// use CMD53
	sdio53.regAddr = 0;
	sdio53.bufAddr = (unsigned int)CCCR53;
	sdio53.Count = 0x14;
	sdio53.funNo = 0;
	sdio53.OpCode = FMI_SDIO_INC_ADDRESS;
	sdio53.BlockMode = FMI_SDIO_SINGLE;

	fmiSDIO_BlockRead(&sdio53);

	printf("\n========\nCCCR use CMD53 ==>\n");
	for (i=0; i<0x12; i++)
	{
		printf(" %02x", CCCR53[i]);
		if (((i+1) % 8) == 0)
			printf("\n");
	}
}

void GetFBR(unsigned int funNo)
{
	unsigned int volatile i, j;
	unsigned int funAddr;
	unsigned char FBR[0x12];

	for (j=1; j<=funNo; j++)
	{
		for (i=0; i<0x12; i++)
			FBR[i] = 0;

		funAddr = j << 17;
		// IO_RW_DIRECT / CMD52
		for (i=0; i<0x12; i++)
		{
			sdio.funNo = 0;
			sdio.regAddr = i | (j << 8);
			sdio.IsReadAfterWrite = FALSE;
			sdio.WriteData = 0;
			FBR[i] = fmiSDIO_Read(&sdio);
		}

		printf("\n========\nFBR%d ==>\n", j);
		for (i=0; i<0x12; i++)
		{
			printf(" %02x", FBR[i]);
			if (((i+1) % 8) == 0)
				printf("\n");
		}
		FUN_CIS[j] = (FBR[0xb]<<16)|(FBR[0xa]<<8)|FBR[0x9];

		// use CMD53
		sdio53.regAddr = j << 8;
		sdio53.bufAddr = (unsigned int)FBR;
		sdio53.Count = 0x14;
		sdio53.funNo = 0;
		sdio53.OpCode = FMI_SDIO_INC_ADDRESS;
		sdio53.BlockMode = FMI_SDIO_SINGLE;

		fmiSDIO_BlockRead(&sdio53);
		printf("\n========\nFBR%d use CMD53 ==>\n", j);
		for (i=0; i<0x12; i++)
		{
			printf(" %02x", FBR[i]);
			if (((i+1) % 8) == 0)
				printf("\n");
		}
	}
}

unsigned char ReadData(unsigned int addr)
{
	unsigned char rsp;

	sdio.funNo = 0;
	sdio.regAddr = addr;
	sdio.IsReadAfterWrite = FALSE;
	sdio.WriteData = 0;
	rsp = fmiSDIO_Read(&sdio);

	return rsp; 
}


int GetFirstTuple(unsigned int volatile *base_addr, unsigned char volatile *cistpl_id, unsigned char volatile *cistpl_link)
{
	unsigned char volatile ch;

	ch = ReadData(*base_addr) & 0xFF;

	if (ch == CISTPL_NULL || ch == CISTPL_END)  
	{
		*base_addr += 1;
		ch = ReadData(*base_addr) & 0xFF;
	}
 
	*cistpl_id   = ch;
	*cistpl_link = ReadData(*base_addr + 1) & 0xFF;

	return Successful;
}


int GetNextTuple(unsigned int volatile base_addr, unsigned char volatile *cistpl_id, unsigned char volatile *cistpl_link)
{
	*cistpl_id = ReadData(base_addr) & 0xFF;
	*cistpl_link = ReadData(base_addr + 1) & 0xFF;

	return Successful;
}


void DisplayTuple(int index, unsigned int volatile base_addr, unsigned char volatile cistpl_id, unsigned char volatile cistpl_link)
{
	int i;
	unsigned int body_addr;

	body_addr = base_addr + 2;

	printf("base addr [0x%x]\n", base_addr);
	printf("[%2d]  CISTPL_CODE : %x\n", index, cistpl_id);
	printf("      CISTPL_LINK : %x\n", cistpl_link);
	printf("      CISTPL_BODY : ");

	for (i=0; i<cistpl_link; i++)
	{
		printf("%2x ", ReadData(body_addr) & 0xFF);
		body_addr += 1;
		if ((i+1) % 8 == 0)  printf("\n                    ");
	}

	printf("\n\n");
}


void GetCIS(unsigned int funCount)
{
	unsigned int volatile i;
	int index;
	unsigned int volatile base_addr;
	unsigned char volatile cistpl_id, cistpl_link;

	for (i=0; i<=funCount; i++)
	{
		base_addr = FUN_CIS[i];
		printf("\n\nFunction%d CIS Information\n", i);
		printf("-----------------------------------------------------------\n");
		index = 1;
		if (GetFirstTuple(&base_addr, &cistpl_id, &cistpl_link) == -1)
		{
			printf("INVALID CISTPL_ID at Attribute memory 0 => 0x%4x\n", cistpl_id);
			exit(0);
		}

		DisplayTuple(index++, base_addr, cistpl_id, cistpl_link);  
		base_addr += (cistpl_link + 2);

		while(1)
		{
			if (GetNextTuple(base_addr, &cistpl_id, &cistpl_link) == -1)
			{
				printf("Error in read Attribute Memory !\n");
				exit(0);
			}

			if (cistpl_id == CISTPL_END)
				break;

			DisplayTuple(index++, base_addr, cistpl_id, cistpl_link);  
			base_addr += (cistpl_link + 2);  
		}
		printf("\n");
	}
}


#define DELAY_LOOPS	0x400
#define DELAY(x)	{volatile int delay;delay = (x); while(delay--);}


w99702_clk_t clk_profile[] = {

	/* Clock setting profile of menu operation mode */
	{0x0F003090, 0x0000E220, 0x00004212, 0x0000030E, 0x00400303, 0x0010A9AA, 0x130080FF, 0xF8005748},
};

void pwr_set_clk(w99702_clk_t *cfg)
{
	unsigned int reg,bak,bak_audio, bak_mp4dec;
	unsigned int clkcon;
	
	/* if UPLL is enabled, switch the PLL to APLL first */
	bak = 0; bak_audio = 0; bak_mp4dec = 0;
	reg = inpw(REG_CLKSEL);
	
	/* Reserve the audio clock */	
	outpw(REG_APLLCON, 0x6210);

	if( 	inpw(REG_UPLLCON) == cfg->upllcon && 
			(inpw(REG_CLKDIV0)&0xF0000000UL) == (cfg->clkdiv0&0xF0000000UL) &&
			(inpw(REG_CLKDIV1)&0xF0000000UL) == (cfg->clkdiv1&0xF0000000UL) )
	{
		/* Just change dividor when UPLL & HCLK is not changed */
			
		/* select the clock source */
		if(inpw(REG_CLKSEL) != cfg->clksel)
		{
			outpw(REG_CLKSEL, cfg->clksel);
			DELAY(DELAY_LOOPS);
		}

		clkcon = inpw(REG_CLKCON);	
		if(clkcon != cfg->clkcon)
		{
			/* Enable IP's step by step */		
			clkcon |= (cfg->clkcon&0xF0000000);
			outpw(REG_CLKCON, clkcon);
			DELAY(DELAY_LOOPS);
			clkcon |= (cfg->clkcon&0x00F00000);
			outpw(REG_CLKCON, clkcon);
			DELAY(DELAY_LOOPS);
			clkcon |= (cfg->clkcon&0x000F0000);
			outpw(REG_CLKCON, clkcon);
			DELAY(DELAY_LOOPS);
			clkcon |= (cfg->clkcon&0x00000F00);
			outpw(REG_CLKCON, clkcon);
			DELAY(DELAY_LOOPS);
		}

		/* setting the dividor */		
		if(inpw(REG_CLKDIV0) != cfg->clkdiv0)
		{
			outpw(REG_CLKDIV0, cfg->clkdiv0);
			DELAY(DELAY_LOOPS);
		}

		if(inpw(REG_CLKDIV1) != cfg->clkdiv1)
		{
			outpw(REG_CLKDIV1, cfg->clkdiv1);
			DELAY(DELAY_LOOPS);
		}

		/* setting SDRAM control */
		if(inpw(REG_SDICON) != cfg->sdcon)
		{
			outpw(REG_SDICON, cfg->sdcon);
			DELAY(DELAY_LOOPS);
		}

		/* setting SDRAM timing */
		if(inpw(REG_SDTIME0) != cfg->sdtime0)
		{
			outpw(REG_SDTIME0, cfg->sdtime0);
			DELAY(DELAY_LOOPS);
		}
	}
	else if( inpw(REG_UPLLCON) == cfg->upllcon )
	{
		/* change dividor and SDRAM timing when only UPLL fixed */
		/* Force the SDRAM timing to slowest */
		outpw(REG_SDTIME0, 0xF8006948);
		DELAY(DELAY_LOOPS);

		/* select the clock source */
		if(inpw(REG_CLKSEL) != cfg->clksel)
		{
			outpw(REG_CLKSEL, cfg->clksel);
			DELAY(DELAY_LOOPS);
		}
		
		clkcon = inpw(REG_CLKCON);
		if( clkcon != cfg->clkcon)
		{
			/* Enable IP's step by step */		
			clkcon |= (cfg->clkcon&0xF0000000);
			outpw(REG_CLKCON, clkcon);
			DELAY(DELAY_LOOPS);
			clkcon |= (cfg->clkcon&0x00F00000);
			outpw(REG_CLKCON, clkcon);
			DELAY(DELAY_LOOPS);
			clkcon |= (cfg->clkcon&0x000F0000);
			outpw(REG_CLKCON, clkcon);
			DELAY(DELAY_LOOPS);
			clkcon |= (cfg->clkcon&0x00000F00);
			outpw(REG_CLKCON, clkcon);
			DELAY(DELAY_LOOPS);
		}
		
		/* setting the dividor */		
		if(inpw(REG_CLKDIV0) != cfg->clkdiv0)
		{
			outpw(REG_CLKDIV0, cfg->clkdiv0);
			DELAY(DELAY_LOOPS);
		}
		
		if(inpw(REG_CLKDIV1) != cfg->clkdiv1)
		{
			outpw(REG_CLKDIV1, cfg->clkdiv1);
			DELAY(DELAY_LOOPS);
		}
		
		/* setting SDRAM control */
		if(inpw(REG_SDICON) != cfg->sdcon)
		{
			outpw(REG_SDICON, cfg->sdcon);
			DELAY(DELAY_LOOPS);
		}
		
		/* setting SDRAM timing */
		if(inpw(REG_SDTIME0) != cfg->sdtime0)
		{
			outpw(REG_SDTIME0, cfg->sdtime0);
			DELAY(DELAY_LOOPS);
		}
	}
	else
	{
		/* Force the SDRAM timing to slowest */
		outpw(REG_SDTIME0, 0xF8006948);
		DELAY(DELAY_LOOPS);

		/* close some IP to make system stable */
		clkcon = cfg->clkcon & 0x1F00F0F0;
		outpw(REG_CLKCON , clkcon);
		
		/* force the clock source to crystal for futur clock change */
		outpw(REG_CLKSEL , 0x00000114);
		/* change the UPLL first, and wait for stable */
		outpw(REG_UPLLCON, cfg->upllcon);
		DELAY(DELAY_LOOPS);
		
		/* select the clock source to UPLL */
		outpw(REG_CLKSEL, cfg->clksel);
		DELAY(DELAY_LOOPS);
		
		/* Enable IP's step by step */		
		clkcon |= (cfg->clkcon&0xE0000000);
		outpw(REG_CLKCON, clkcon);
		DELAY(DELAY_LOOPS);
		
		clkcon |= (cfg->clkcon&0x00F00000);
		outpw(REG_CLKCON, clkcon);
		DELAY(DELAY_LOOPS);
		
		clkcon |= (cfg->clkcon&0x000F0000);
		outpw(REG_CLKCON, clkcon);
		DELAY(DELAY_LOOPS);
		
		clkcon |= (cfg->clkcon&0x00000F00);
		outpw(REG_CLKCON, clkcon);
		DELAY(DELAY_LOOPS);
		
		/* setting the dividor */		
		outpw(REG_CLKDIV0, cfg->clkdiv0);
		outpw(REG_CLKDIV1, cfg->clkdiv1);
		DELAY(DELAY_LOOPS);
		
		/* setting SDRAM control */
		outpw(REG_SDICON, cfg->sdcon);
		DELAY(DELAY_LOOPS);
		
		/* setting SDRAM timing */
		outpw(REG_SDTIME0, cfg->sdtime0);
		DELAY(DELAY_LOOPS);
	}	
}


void SDIOTest(cyg_addrword_t data)
{
	unsigned int volatile IO_FUN = 0, i;
	FMI_CARD_DETECT_T card;

//	writew(0x7ff00218, (readw(0x7ff00218)&0xfff0ffff)|0x00010000);	//CPU:HCLK2=2:1
//	writew(0x7ff0020c, 0x6550);
//	writew(0x7ff00210, 0x324);

	pwr_set_clk(&clk_profile[0]);

//	sysSetTimerReferenceClock(TIMER0, 12000000);
//	sysStartTimer(TIMER0, 100, PERIODIC_MODE);

	// set FMI reference clock to 48MHz
	fmiSetFMIReferenceClock(108000);
	fmiSetSDOutputClockbykHz(12000);

#if 0	// DARTS
	card.uCard = FMI_SD_CARD;					// card type
	card.uGPIO = 4;					// card detect GPIO pin
	card.uWriteProtect = -1;					// card detect GPIO pin
	card.uInsert = 1;				// 0/1 which one is insert
	card.nPowerPin = -1;				// card power pin, -1 means no power pin
#endif
#if 0	// PMP
	card.uCard = FMI_SD_CARD;					// card type
	card.uGPIO = 17;					// card detect GPIO pin
	card.uWriteProtect = 6;					// card detect GPIO pin
	card.uInsert = 0;				// 0/1 which one is insert
	card.nPowerPin = 7;				// card power pin, -1 means no power pin
#endif
#if 1	// demo
	card.uCard = FMI_SD_CARD;					// card type
	card.uGPIO = 4;					// card detect GPIO pin
	card.uWriteProtect = 16;					// card detect GPIO pin
	card.uInsert = 0;				// 0/1 which one is insert
	card.nPowerPin = 12;				// card power pin, -1 means no power pin
#endif
#if 0	// Module
	card.uCard = FMI_SD_CARD;		// card type
	card.uGPIO = 17;				// card detect GPIO pin
	card.uWriteProtect = 6;			// card detect GPIO pin
	card.uInsert = 0;				// 0/1 which one is insert
	card.nPowerPin = -1;			// card power pin, -1 means no power pin
#endif
	card.bIsTFlashCard = FALSE;
	fmiSetCardDetection(&card);

	// initial FMI
	fmiInitDevice();

	for (i=0; i<0x5000; i++);
	IO_FUN = fmiSDDeviceInit();

	GetCCCR();
//	GetFBR(IO_FUN);
//	GetCIS(IO_FUN);

	printf("\n\n");
	return;
}

int main()
{
	diag_printf("\n\nenter main\n\n");

	cyg_thread_create(10, SDIOTest, 0, "SDIO Test",
       				(void *)main_stack, sizeof(main_stack), &handle_cs, &thread_cs);
	cyg_thread_resume(handle_cs);
					
	return 0;
}


