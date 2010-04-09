#ifdef ECOS
#include "stdlib.h"
#include "string.h"
#else
#include <stdlib.h>
#include <string.h>
#endif

#include "w99702_reg.h"
#include "fmi.h"
#include "wb_fmi.h"
#include "wblib.h"

#ifdef _WB_FAT_
#include "wbfat.h"
extern PDISK_T *pDisk_SM, *pDisk_SM_P;
#endif

#ifdef SM_DEVICE

#ifdef SD_DEVICE

BOOL volatile _fmi_bIsNandAccess = FALSE;

// for T-Flash
extern BOOL volatile _fmi_bIsSDInsert, _fmi_bIsSDTFlash, _fmi_bIsGPIOEnable;
extern INT32 volatile _fmi_nGPIOPower_SD, _fmi_nGPIOInsertState_SD;
extern FMI_CARD_DETECT_T *pCard_SD;
extern void (*fmiSDInsertFun)();
#endif

// for SM
extern UINT32 volatile _fmi_uSM_2NANDZone;
extern UINT32 _fmi_uF4cycle;		// 0:3-cycle address; 1:4-cycle address
extern UINT32 _fmi_u2KPageSize;		// 0:page size 512 byte; 1:page size 2K byte
extern UINT32 _fmi_uBlockPerFlash;
extern UINT32 _fmi_uSM_SectorSize;
extern BOOL volatile _fmi_bIsSamSungNAND, _fmi_bIsSMDefaultCheck;

// for reserved area
extern UINT32 _fmi_uReservedBaseSector;	// logical sector
extern UINT32 _fmi_uSMReservedBlock, _fmi_uSMReservedSector;

UINT32 volatile _fmi_uRAMark;
UINT32 _fmi_uFirst_L2P=0, _fmi_uDelPageOfBlock=0;
INT32 volatile _fmi_nPreDelBlock = -1;
__align(32) UCHAR _fmi_ucSMBuffer[32*512];
UINT8	*_fmi_pSMBuffer;

//#define ZONE_NUM	10
#define ZONE_NUM	1	//xhchen modified to reduce memory usage in IPCam!!!!

struct node {
	UINT32 block;
	UINT32 status;	// 0->erased; 1->not erased
	struct node *next;
};

struct free_block_node {
	struct node *head;
	struct node *tail;
};


struct zone_node {
	UINT32	isScan;				// 1->scan; 0->not scan
	INT		L2P_table[1000];	// logic to physical block number
	struct	free_block_node	free_block_list;
} SM_ZONE[ZONE_NUM];


INT SM_AddFreeBlock(UINT32 zone_no, UINT32 block_no, UINT32 erase_flag)
{
	struct node *p;

	p = (struct node *) malloc(sizeof(struct node));
	if (p == NULL)
	{
#ifdef DEBUG
		printf("malloc fail\n");
#endif
		return FMI_SM_MALLOC_ERROR;
	}
	p->block = block_no;
	p->status = erase_flag;
	p->next = NULL;

	if (SM_ZONE[zone_no].free_block_list.head == NULL)
		SM_ZONE[zone_no].free_block_list.head = p;
		
	if (SM_ZONE[zone_no].free_block_list.tail == NULL)
		SM_ZONE[zone_no].free_block_list.tail = p;
	else
	{
		SM_ZONE[zone_no].free_block_list.tail->next = p;
		SM_ZONE[zone_no].free_block_list.tail = p;
	}
	return FMI_NO_ERR;
}

INT fmiSMClearBlock(UINT32 zone)
{
	struct node *p, *q;

	p = SM_ZONE[zone].free_block_list.head;
	q = SM_ZONE[zone].free_block_list.tail;

	while (p != q)
	{
		if (p->status == 1)
			fmiSM_BlockErase(p->block);
		p = p->next;
	}

	return FMI_NO_ERR;
}

UINT fmiSMGetMarkData(UINT32 uSector)
{
	unsigned char RABuf[16];
	int volatile j=0;

	// use data port to get redundant area
	outpw(REG_SMCMD, 0x50);		// read command
	outpw(REG_SMMADDR, 0x00);	// CA0 - CA7
	outpw(REG_SMMADDR, (uSector & 0xff));	// PA0 - PA7
	if (_fmi_uF4cycle == 0)
		outpw(REG_SMADDR, ((uSector >> 8) & 0xff));		// PA8 - PA15
	else
	{
		outpw(REG_SMMADDR, ((uSector >> 8) & 0xff));		// PA8 - PA15
		outpw(REG_SMADDR, ((uSector >> 16) & 0xff));		// PA16 - PA17
	}

#ifdef _MP_
	while(1)	// rb# interrupt status
	{
		if (inpw(REG_SMISR) & 0x04)
		{
			outpw(REG_SMISR, (inpw(REG_SMISR) & 0xfffffffb)|0x04);
			break;
		}
	}
#endif

	while(!(inpw(REG_SMISR) & 0x10));	// rb# status: wait

	for (j=0; j<4; j++)
		RABuf[j] = inpw(REG_SMDATA) & 0xff;

	fmiSM_Reset();

	return ((RABuf[3] << 24) | (RABuf[2] << 16) | (RABuf[1] << 8));
}

UINT16 get_block_address(UINT32 block, BOOL flag)
{
	unsigned char RABuf[16];
	UINT16 BA1=0, BA2=0;
	UINT32 offset, j;
	UINT32 volatile mark;

	if (block == 0)
		return 0;

	if (_fmi_u2KPageSize == 0)	// page size = 512bytes
	{
		offset = _fmi_uPagePerBlock * block;

		if (!_fmi_bIsSMDefaultCheck)
		{
			while(!(inpw(REG_SMISR) & 0x10))	// rb# status: wait
			{
				if (_fmi_bIsSMInsert == FALSE)
					return (UINT16)FMI_NO_SM_CARD;
			}
		}

		fmiSM_Reset();
		// use data port to get redundant area
		outpw(REG_SMCMD, 0x50);		// read command
		outpw(REG_SMMADDR, 0x00);	// CA0 - CA7
		outpw(REG_SMMADDR, (offset & 0xff));	// PA0 - PA7
		if (_fmi_uF4cycle == 0)
			outpw(REG_SMADDR, ((offset >> 8) & 0xff));		// PA8 - PA15
		else
		{
			outpw(REG_SMMADDR, ((offset >> 8) & 0xff));		// PA8 - PA15
			outpw(REG_SMADDR, ((offset >> 16) & 0xff));		// PA16 - PA17
		}

#ifdef _MP_
		while(1)	// rb# interrupt status
		{
			mark = inpw(REG_SMISR);
			if (mark & 0x04)
			{
				outpw(REG_SMISR, (mark & 0xfffffffb)|0x04);
				break;
			}
		}
#endif

		while(!(inpw(REG_SMISR) & 0x10))	// rb# status: wait
		{
			if (_fmi_bIsSMInsert == FALSE)
				return (UINT16)FMI_NO_SM_CARD;
		}

		for (j=0; j<16; j++)
			RABuf[j] = inpw(REG_SMDATA) & 0xff;
		mark = (RABuf[3] << 24) | (RABuf[2] << 16) | (RABuf[1] << 8);
		BA1 = (RABuf[6] << 8) | RABuf[7];
		BA2 = (RABuf[11] << 8) | RABuf[12];

		fmiSM_Reset();
	}
	else	// page size = 2kbytes
	{
		outpw(REG_FMIDSA, (UINT32)_fmi_pSMBuffer);	// set DMA transfer starting address
		outpw(REG_FMICR, (inpw(REG_FMICR) & 0xfffff8ff));	// buffer to sdram
		outpw(REG_FMICR, (inpw(REG_FMICR) & 0xffffff8f)|0x20);		// SM to buffer0

		if (!_fmi_bIsSMDefaultCheck)
			while(!(inpw(REG_SMISR) & 0x10));	// rb# status: wait

		offset = _fmi_uPagePerBlock * block;
		outpw(REG_SMCMD, 0x00);		// read command
		outpw(REG_SMMADDR, 0x00);	// CA0 - CA7
		outpw(REG_SMMADDR, 0x00);	// CA8 - CA11
		outpw(REG_SMMADDR, offset & 0xff);	// PA0 - PA7
		if (_fmi_uF4cycle == 0)
			outpw(REG_SMADDR, ((offset >> 8) & 0xff));		// PA8 - PA15
		else
		{
			outpw(REG_SMMADDR, ((offset >> 8) & 0xff));		// PA8 - PA15
			outpw(REG_SMADDR, ((offset >> 16) & 0xff));		// PA16 - PA17
		}
		outpw(REG_SMCMD, 0x30);		// read command

#ifdef _MP_
		while(1)	// rb# interrupt status
		{
			mark = inpw(REG_SMISR);
			if (mark & 0x04)
			{
				outpw(REG_SMISR, (mark & 0xfffffffb)|0x04);
				break;
			}
		}
#endif
		while(!(inpw(REG_SMISR) & 0x10));	// rb# status: wait

#ifdef _USE_IRQ
		_fmi_uSM_DataReady = 0;
#endif
		outpw(REG_SMCR, inpw(REG_SMCR) | 0x02);
#ifdef _USE_IRQ
		while(_fmi_uSM_DataReady == 0);
#else
		while(inpw(REG_SMCR) & 0x02);
#endif
		mark = inpw(REG_SMRA_0);
		BA1 = (inpw(REG_SMRA_2) & 0x0000ff00) |
			  ((inpw(REG_SMRA_2) & 0x00ff0000) >> 16);
	}
	if ((BA1 != 0xffff) && (((mark & 0xffff00) == 0xfcfc00) || ((mark & 0xc0000000) == 0x40000000) || ((mark & 0xc0000000) == 0x80000000)))
	{
		//printf("block[%d] / BA[%x] / mark[%x]\n", block, BA1, mark);
		BA1 = 0xfcfc;
	}

	if (flag == 0)
		return BA1;
	else
		return (mark >> 16);
}

INT fmiCheckBlock(UINT32 oldBlock, UINT32 newBlock)
{
	INT32 oldmark, newmark;
	INT status = 0;

	oldmark = (UINT32)get_block_address(oldBlock, 1);
	oldmark = (oldmark >> 12) & 0x03;
	newmark = (UINT32)get_block_address(newBlock, 1);
	newmark = (newmark >> 12) & 0x03;

	switch (newmark - oldmark)
	{
		case -3:
		case 1:
			if ((status = fmiSM_BlockErase(oldBlock)) < 0)
				return status;
			return newBlock;
		case 3:
		case -1:
			if ((status = fmiSM_BlockErase(newBlock)) < 0)
				return status;
			return oldBlock;
		default:
			return -1;
	}
}

INT fmiSM_L2PTable_Init(UINT32 uZoneNo)
{
	UINT32 volatile i;
	UINT32 sblock, eblock;
	UINT32 zone_num, block_num;
	UINT16 BA;
	int volatile status, block;
	struct node *p;
	
	fmiLock();
	
	_fmi_bIsNandAccess = TRUE;
	if (_fmi_uFirst_L2P == 0)
	{
		_fmi_pSMBuffer = (UINT8 *)((UINT32)_fmi_ucSMBuffer | 0x10000000);

		for (zone_num=0; zone_num<ZONE_NUM; zone_num++)
		{
//printf("[%d]head[%x], tail[%x]\n", zone_num, (int)SM_ZONE[zone_num].free_block_list.head, (int)SM_ZONE[zone_num].free_block_list.tail);
			while(1)
			{
				p = SM_ZONE[zone_num].free_block_list.head;
				if (p != SM_ZONE[zone_num].free_block_list.tail)
				{
					SM_ZONE[zone_num].free_block_list.head = p->next;
					free(p);
				}
				else
				{
					free(p);
					break;
				}
			}

			SM_ZONE[zone_num].isScan = 0;
			SM_ZONE[zone_num].free_block_list.head = NULL;
			SM_ZONE[zone_num].free_block_list.tail = NULL;
			for (i=0; i<1000; i++)
				SM_ZONE[zone_num].L2P_table[i] = -1; //unused
		}
		_fmi_uFirst_L2P = 1;
	}

#ifdef _USE_TWO_NAND_
	if (_fmi_bIs2NAND)
	{
		if (uZoneNo >= _fmi_uSM_2NANDZone)
		{
			fmiSM_Initial(1);
			sblock = (uZoneNo - _fmi_uSM_2NANDZone) * 1024;
			eblock = (uZoneNo - _fmi_uSM_2NANDZone + 1) * 1024;
		}
		else
		{
			fmiSM_Initial(0);
			sblock = uZoneNo * 1024;
			eblock = (uZoneNo + 1) * 1024;
		}
	}
	else
	{
		sblock = uZoneNo * 1024;
		eblock = (uZoneNo + 1) * 1024;
	}
#else
	sblock = uZoneNo * 1024;
	eblock = (uZoneNo + 1) * 1024;
#endif

	// for block per flash less than a zone (1000 block)
	if (_fmi_bIsReserved)
	{
		if (eblock > (_fmi_uBlockPerFlash + _fmi_uSMReservedBlock))
			eblock = _fmi_uBlockPerFlash + _fmi_uSMReservedBlock + 1;
	}
	else
	{
		if (eblock > _fmi_uBlockPerFlash)
			eblock = _fmi_uBlockPerFlash + 1;
	}

	for (i=sblock; i<eblock; i++)
	{
		BA = get_block_address(i, 0);
		switch (BA)
		{
			case 0xffff:	// empty block
				if ((status = SM_AddFreeBlock(uZoneNo, i, 0)) < 0)
				{
					fmiUnLock();
					return status;
				}
				break;

			case 0xffee:	// bad block
			case 0x0000:
				break;

			case 0xfcfc:	// need to erase block
				if ((status = SM_AddFreeBlock(uZoneNo, i, 1)) < 0)
				{
					fmiUnLock();
					return status;
				}
				break;

			default:		// used block
				block_num = (BA & 0xfff) >> 1;		// logic block number
				if (SM_ZONE[uZoneNo].L2P_table[block_num] != -1)
				{
					block = fmiCheckBlock(SM_ZONE[uZoneNo].L2P_table[block_num], i);
					//printf("logic[%d] / block[%d]\n", block_num, block);
					if (block < 0)
					{
						fmiUnLock();
						return block;
					}
					SM_ZONE[uZoneNo].L2P_table[block_num] = block;

					if (block == i)
					{
						if ((status = SM_AddFreeBlock(uZoneNo, SM_ZONE[uZoneNo].L2P_table[block_num], 0)) < 0)
						{
							fmiUnLock();
							return status;
						}
					}
					else
					{
						if ((status = SM_AddFreeBlock(uZoneNo, i, 0)) < 0)
						{
							fmiUnLock();
							return status;
						}
					}
				}
				else
					SM_ZONE[uZoneNo].L2P_table[block_num] = i;
		}
	}
	SM_ZONE[uZoneNo].isScan = 1;

	_fmi_bIsNandAccess = FALSE;
#if 0
	for (i=0; i<1000; i++)
		printf("logic <%04d> -> phy <%04d>\n", i, SM_ZONE[uZoneNo].L2P_table[i]);
#endif
	fmiUnLock();
	return FMI_NO_ERR;
}


int get_free_block(UINT32 zone)
{
	struct node *p;
	int free_block;

Get_Block:
	p = SM_ZONE[zone].free_block_list.head;
	free_block = p->block;
	SM_ZONE[zone].free_block_list.head = p->next;

	if (p->status == 1)
	{
		if (fmiSM_BlockErase(free_block) == FMI_SM_STATE_ERROR)
		{
			//printf("erase %d block fail!!\n", free_block);
			goto Get_Block;
		}
	}

	if (SM_ZONE[zone].free_block_list.head == NULL)
		 SM_ZONE[zone].free_block_list.tail = NULL;
	free(p);

	return free_block;
}


VOID get_page_data(UINT32 page, UINT32 sector, UINT32 sec_count, UINT32 buff)
{
#ifdef DEBUG
	//printf("get_page:page[%d], sector[%d], count[%d]\n", page, sector, sec_count);
#endif
	if (sec_count == 4)
		fmiSM_Read_2K(page, sector, buff);
	else
	{
		outpw(REG_SMCR, inpw(REG_SMCR) & 0xffffff7f);	// set page size = 512 bytes
		fmiSM_Read_1(page, sector, sec_count, buff);
		outpw(REG_SMCR, (inpw(REG_SMCR) & 0xffffff7f) | 0x80);	// set page size = 2048 bytes
	}
}


int get_block_data(UINT32 zone, UINT32 block, UINT32 sector, UINT32 sec_count, UINT32 buff)
{
	int count, status;
	UINT32 psector;
	UINT32 rpage, rsector, sector_count, pBuf;

	if (SM_ZONE[zone].isScan == 0)
		fmiSM_L2PTable_Init(zone);

	if (SM_ZONE[zone].L2P_table[block] == -1)
		return FMI_SM_EMPTY_BLOCK;

	if (_fmi_u2KPageSize == 0)	// page size 512 bytes
	{
		psector = SM_ZONE[zone].L2P_table[block] * _fmi_uPagePerBlock + sector;
		status = fmiSM_Read_512(psector, sec_count, buff);
		if (status != FMI_NO_ERR)
			return status;
	}
	else	// page size 2K bytes
	{
		rpage = sector / 4;		// relative page number
		rsector = sector % 4;	// relative sector number of page
		count = sec_count;
		psector = SM_ZONE[zone].L2P_table[block] * _fmi_uPagePerBlock + rpage;

		pBuf = buff;
		if (count > (4 - rsector))
			sector_count = 4 - rsector;
		else
			sector_count = count;

		while(1)
		{
			get_page_data(psector, rsector, sector_count, pBuf);
			count = count - sector_count;
			if (count <= 0)
				break;
			rsector = 0;	// relative sector of page
			psector++;		// physical page address
			pBuf = pBuf + sector_count*512;

			if (count < 4)
				sector_count = count;
			else
				sector_count = 4;
		}
	}
	return FMI_NO_ERR;
}

INT update_page_data(int Spage, int Dpage, UINT32 column, UINT32 sec_count, UINT32 buff)
{
	int status;
	int volatile i, mark=0, reg;

#ifdef DEBUG
	//printf("update_page:Spage[%d], Dpage[%d], column[%d], count[%d]\n", Spage, Dpage, column, sec_count);
#endif
	if (sec_count == 4)
		status = fmiSM_Write_2K(Dpage, 0, buff, _fmi_uRAMark|0xc0fefeff);		
	else
	{
		for (i=0; i<sec_count; i++)
			mark = mark | (1 << (column + i));

		// read the page data back and update
		if (Spage == -1)
		{
			memset(_fmi_pSMBuffer, 0xff, 2048);
			reg = 0xfffefeff;
		}
		else
		{
			fmiSM_Read_2K(Spage, 0, (UINT32)_fmi_pSMBuffer);
			reg = inpw(REG_SMRA_0);
			if ((reg & 0xffff00) == 0xfcfc00)
				reg = (reg & 0xff0000ff) | 0xfefe00;
		}
		memcpy(_fmi_pSMBuffer + column*_fmi_uSM_SectorSize, (unsigned char *)buff, sec_count*_fmi_uSM_SectorSize);
		mark = (reg & (~mark << 24)) | (reg & 0xffffff);
		mark = (mark & 0xcfffffff) | _fmi_uRAMark;
//printf("update_page_data: mark[%x]\n", mark);
		status = fmiSM_Write_2K(Dpage, 0, (UINT32)_fmi_pSMBuffer, mark);
	}
	return status;
}


/* check whether or not page has been used */
INT check_page_mark(INT32 page, UINT32 count, UINT32 sector)
{
	int volatile i, Cnt;
	UINT32 volatile reg, sector_count, mark, rsec;

//printf("check_page: page[%d], count[%d], sector[%d]\n", page, count, sector);

	if (_fmi_u2KPageSize)
	{
		rsec = sector;
		Cnt = count;
		if (Cnt > (4 - rsec))
			sector_count = 4 - rsec;
		else
			sector_count = Cnt;

		while(1)
		{
			fmiSM_Read_2K(page, 0, (UINT32)_fmi_pSMBuffer);
			reg = inpw(REG_SMRA_0);
			mark = (reg & 0xff000000) >> 24;
			if ((reg & 0xffff00) == 0xfefe00)
			{ 
				for (i=0; i<sector_count; i++)
				{
					if ((mark & (1 << (rsec + i))) == 0)
					{
						//printf("1.reg[%x], mark[%x], bit[%x]\n", reg, mark, (1<<(rsec+i)));
						return 1;
					}
					//else
						//printf("2.reg[%x], mark[%x], bit[%x]\n", reg, mark, (1<<(rsec+i)));
				}
			}
			else if ((reg & 0xffff00) == 0xfcfc00)
				return 1;
			//else
				//printf("reg[%x]\n", reg);

			page++;
			Cnt = Cnt - sector_count;
			if (Cnt <= 0)
				break;
			rsec = 0;
			if (Cnt < 4)
				sector_count = Cnt;
			else
				sector_count = 4;
		}
	}
	else	// page size = 512 bytes
	{
		for (i=0; i<count; i++)
		{
			fmiSM2Buffer(page+i, 0, 0);
			reg = inpw(REG_SMRA_0);
			if ((reg & 0xffff00) != 0xffff00)
				return 1;	// has been used
		}
	}
	return 0;	// havn't been used
}


int update_block_data(UINT32 zone, UINT32 block, UINT32 sector, UINT32 sec_count, UINT32 buff)
{
	int pblock, Ssector, Dsector, free_block, status=0;

	if (SM_ZONE[zone].isScan == 0)
		fmiSM_L2PTable_Init(zone);

	pblock = SM_ZONE[zone].L2P_table[block];
	if ((free_block = get_free_block(zone)) < 0)
	{
#ifdef DEBUG
		printf("1.get_free_block fail\n");
#endif
		return free_block;
	}

	fmiSM_GetBlockAddr(block);

    /* Needn't back up old block */
	if (sec_count == _fmi_uSectorPerBlock)
	{
		if (pblock != -1)
		{
			_fmi_uRAMark = fmiSMGetMarkData(pblock*_fmi_uPagePerBlock);
			_fmi_uRAMark = (_fmi_uRAMark & 0x30000000) >> 28;
			_fmi_uRAMark = ((_fmi_uRAMark + 1) << 28) & 0x30000000;
		}
		else
			_fmi_uRAMark = 0x0;

		Dsector = free_block * _fmi_uPagePerBlock;

		fmiSML2PWriteProtect(Dsector, 0x7f);
		status = fmiSM_Write_512(Dsector, sec_count, buff, _fmi_uRAMark|0xc0fefeff);
		if (status != FMI_NO_ERR)
		{
#ifdef DEBUG
			if (fmiSM_BlockErase(free_block) != FMI_NO_ERR)
				printf("1.erase %d block fail!!\n", free_block);
#else
			fmiSM_BlockErase(free_block);
#endif
			return status;
		}

		fmiSML2PWriteProtect(Dsector, 0x3f);
		SM_ZONE[zone].L2P_table[block] = free_block;

		if (pblock != -1)
		{
			status = fmiSM_BlockErase(pblock);
#ifdef DEBUG
			if (status  != FMI_NO_ERR)
				printf("4.erase %d block fail!!\n", pblock);
			else
#endif
				if ((status = SM_AddFreeBlock(zone, pblock, 0)) < 0)
					return status;
		}
	}
	/* Should back up old block */
	else
	{
		if (pblock != -1)
			Ssector = pblock * _fmi_uPagePerBlock;		// source page
		Dsector = free_block * _fmi_uPagePerBlock;	// destination page

		if (pblock != -1)
		{
			_fmi_uRAMark = fmiSMGetMarkData(pblock*_fmi_uPagePerBlock) & 0x30000000;

			if (check_page_mark(Ssector+sector, sec_count, sector) == 1)
			{
				_fmi_uRAMark = (((_fmi_uRAMark >> 28) + 1) << 28) & 0x30000000;

				fmiSML2PWriteProtect(Dsector, 0x7f);

				// read the block data back and update
				status = fmiSM_Read_512(Ssector, _fmi_uPagePerBlock, (UINT32)_fmi_pSMBuffer);
				memcpy(_fmi_pSMBuffer+sector*_fmi_uSM_SectorSize, (unsigned char *)buff, sec_count*_fmi_uSM_SectorSize);

				status = fmiSM_Write_512(Dsector, _fmi_uPagePerBlock, (UINT32)_fmi_pSMBuffer, _fmi_uRAMark|0xc0fefeff);
				if (status != FMI_NO_ERR)
				{
					status = fmiSM_BlockErase(free_block);
					if (status != FMI_NO_ERR)
					{
#ifdef DEBUG
						printf("erase %d block fail!!\n", free_block);
#endif
						return status;
					}
					else
					{
						if ((status = SM_AddFreeBlock(zone, free_block, 0)) < 0)
							return status;
					}
				}

				if (sector != 0)
					fmiSM_WritePage0(Dsector, _fmi_uRAMark|0xcfffffff);

				fmiSML2PWriteProtect(Dsector, 0x3f);
				SM_ZONE[zone].L2P_table[block] = free_block;

				status = fmiSM_BlockErase(pblock);
				if (status != FMI_NO_ERR)
				{
#ifdef DEBUG
					printf("erase %d block fail!!\n", pblock);
#endif
					return status;
				}
				else
				{
					if ((status = SM_AddFreeBlock(zone, pblock, 0)) < 0)
						return status;
				}
			}
			else
			{
				Dsector = pblock * _fmi_uPagePerBlock + sector;			
				status = fmiSM_Write_512(Dsector, sec_count, (UINT32)buff, _fmi_uRAMark|0xc0fefeff);
				if (status != FMI_NO_ERR)
					return status;

				if ((status = SM_AddFreeBlock(zone, free_block, 0)) < 0)
					return status;
			}
		}
		else
		{
			Dsector = free_block * _fmi_uPagePerBlock + sector;
			fmiSML2PWriteProtect(Dsector-sector, 0x7f);
			status = fmiSM_Write_512(Dsector, sec_count, (UINT32)buff, 0xc0fefeff);
			if (status != FMI_NO_ERR)
				return status;
			SM_ZONE[zone].L2P_table[block] = free_block;
			if (sector != 0)
				fmiSM_WritePage0(Dsector-sector, 0xcfffffff);
			fmiSML2PWriteProtect(Dsector-sector, 0x3f);
		}
	}

	return FMI_NO_ERR;
}


INT update_block_data_2K(UINT32 zone, UINT32 block, UINT32 sector, UINT32 sec_count, UINT32 buff)
{
	int volatile count, status=0, sector_count;
	UINT32 volatile i, mark;
	int pblock, Ssector, Dsector, free_block;
	UINT32 rpage, rsector, pBuf;

	if (SM_ZONE[zone].isScan == 0)
		fmiSM_L2PTable_Init(zone);

	pblock = SM_ZONE[zone].L2P_table[block];
	if ((free_block = get_free_block(zone)) < 0)
	{
#ifdef DEBUG
		printf("2.get_free_block fail\n");
#endif
		return free_block;
	}

	fmiSM_GetBlockAddr(block);

//printf("update_2K: logic[%d/%d/%d], count[%d], pblock[%d], free[%d], buff[0x%x] \n", zone, block, sector, sec_count, pblock, free_block, buff);

    /* Needn't back up old block */
	if (sec_count == _fmi_uSectorPerBlock)
	{
		if (pblock != -1)
		{
			fmiSM_Read_2K(pblock*_fmi_uPagePerBlock, 0, (UINT32)_fmi_pSMBuffer);
			mark = (inpw(REG_SMRA_0) & 0x30000000) >> 28;
			mark = ((mark + 1) << 28) & 0x30000000;
		}
		else
			mark = 0x0;

		Dsector = free_block * _fmi_uPagePerBlock;
		fmiSML2PWriteProtect(Dsector, 0x7f);
		pBuf = buff;
		for (i=0; i<_fmi_uPagePerBlock; i++)
		{
			status = fmiSM_Write_2K(Dsector+i, 0, pBuf, mark|0xc0fefeff);
			if (status != FMI_NO_ERR)
				break;
			pBuf += 2048;
		}

		if (status != FMI_NO_ERR)
		{
#ifdef DEBUG
			if (fmiSM_BlockErase(free_block) != FMI_NO_ERR)
				printf("2K:1.erase %d block fail!!\n", free_block);
#else
			fmiSM_BlockErase(free_block);
#endif
			return status;
		}

		fmiSML2PWriteProtect(Dsector, 0x3f);
		SM_ZONE[zone].L2P_table[block] = free_block;

		if (pblock != -1)
		{
			status = fmiSM_BlockErase(pblock);
			if (status != FMI_NO_ERR)
			{
#ifdef DEBUG
				printf("erase %d block fail!!\n", pblock);
#endif
				return status;
			}
			else
			{
				if ((status = SM_AddFreeBlock(zone, pblock, 0)) < 0)
					return status;
			}
		}
	}
	/* Should back up old block */
	else
	{
		if (pblock != -1)
			Ssector = pblock * _fmi_uPagePerBlock;		// source page
		Dsector = free_block * _fmi_uPagePerBlock;	// destination page

		rpage = sector / 4;		// relative page number
		rsector = sector % 4;	// relative sector number of page

		if (pblock != -1)
		{
			fmiSM_Read_2K(Ssector, 0, (UINT32)_fmi_pSMBuffer);
			_fmi_uRAMark = inpw(REG_SMRA_0) & 0x30000000;

			/* need update to new block */
			if (check_page_mark(Ssector+rpage, sec_count, rsector) == 1)
			{
				_fmi_uRAMark = (((_fmi_uRAMark >> 28) + 1) << 28) & 0x30000000;

				fmiSML2PWriteProtect(Dsector, 0x7f);
				// move oringial page
				for (i=0; i<rpage; i++)
				{
					fmiSM_Read_2K(Ssector+i, 0, (UINT32)_fmi_pSMBuffer);
					mark = inpw(REG_SMRA_0) | 0xc0000000;
					if (((mark & 0xffff00) == 0xfcfc00) || ((mark & 0xffff00) == 0xffff00))
						continue;

					if ((mark & 0xffff00) == 0xfefe00)
					{
						if (_fmi_bIsSamSungNAND)
						{
							status = fmiSM_Write_2K_CB(Ssector+i, Dsector+i);
							if (status != FMI_NO_ERR)
								break;
						}
						else
						{
							status = fmiSM_Write_2K(Dsector+i, 0, (UINT32)_fmi_pSMBuffer, _fmi_uRAMark|(mark&0xcfffffff));
							if (status != FMI_NO_ERR)
								break;
						}
					}
				}

				if (rpage != 0)
					fmiSM_WritePage0(Dsector, _fmi_uRAMark|0xcfffffff);

				if (status != FMI_NO_ERR)
				{
					if (fmiSM_BlockErase(free_block) != FMI_NO_ERR)
					{
#ifdef DEBUG
						printf("erase %d block fail!!\n", free_block);
#endif
						return status;
					}
					else
					{
						//if ((SM_AddFreeBlock(zone, free_block, 0)) < 0)
							return status;
					}
				}

				// need to update page
				count = sec_count;
				pBuf = buff;
				if (count > (4 - rsector))
					sector_count = 4 - rsector;
				else
					sector_count = count;

				Ssector = Ssector + rpage;
				Dsector = Dsector + rpage;
				while(1)
				{
					status = update_page_data(Ssector, Dsector, rsector, sector_count, pBuf);				
					if (status != FMI_NO_ERR)
						break;
					Ssector++;		// physical page address
					Dsector++;
					count = count - sector_count;
					if (count <= 0)
						break;
					rsector = 0;	// relative sector of page
					pBuf = pBuf + sector_count*512;

					if (count < 4)
						sector_count = count;
					else
						sector_count = 4;
				}
				if (status != FMI_NO_ERR)
				{
					if (fmiSM_BlockErase(free_block) != FMI_NO_ERR)
					{
#ifdef DEBUG
						printf("erase %d block fail!!\n", free_block);
#endif
						return status;
					}
					else
					{
						//if ((SM_AddFreeBlock(zone, free_block, 0)) < 0)
							return status;
					}
				}

				// move original page
				if (((sector + sec_count) % 4) != 0)
					rpage = ((sector + sec_count) / 4) + 1;
				else
					rpage = (sector + sec_count) / 4;

				for (i=0; i<_fmi_uPagePerBlock-rpage; i++)
				{
					fmiSM_Read_2K(Ssector+i, 0, (UINT32)_fmi_pSMBuffer);
					mark = inpw(REG_SMRA_0);
					if (((mark & 0xffff00) == 0xfcfc00) || ((mark & 0xffff00) == 0xffff00))
						continue;

					if ((mark & 0xffff00) == 0xfefe00)
					{
						if (_fmi_bIsSamSungNAND)
						{
							status = fmiSM_Write_2K_CB(Ssector+i, Dsector+i);
							if (status != FMI_NO_ERR)
								break;
						}
						else
						{
							status = fmiSM_Write_2K(Dsector+i, 0, (UINT32)_fmi_pSMBuffer, _fmi_uRAMark|(mark&0xcfffffff));
							if (status != FMI_NO_ERR)
								break;
						}
					}
				}
				if (status != FMI_NO_ERR)
				{
					if (fmiSM_BlockErase(free_block) != FMI_NO_ERR)
					{
#ifdef DEBUG
						printf("erase %d block fail!!\n", free_block);
#endif
						return status;
					}
					else
					{
						//if ((SM_AddFreeBlock(zone, free_block, 0)) < 0)
							return status;
					}
				}

				fmiSML2PWriteProtect(free_block * _fmi_uPagePerBlock, 0x3f);
				SM_ZONE[zone].L2P_table[block] = free_block;

				status = fmiSM_BlockErase(pblock);
				if (status != FMI_NO_ERR)
				{
#ifdef DEBUG
					printf("erase %d block fail!!\n", pblock);
#endif
					return status;
				}
				else
				{
					if ((status = SM_AddFreeBlock(zone, pblock, 0)) < 0)
						return status;
				}
			}
			else	// needn't update to new block
			{
				count = sec_count;
				pBuf = buff;
				if (count > (4 - rsector))
					sector_count = 4 - rsector;
				else
					sector_count = count;

				Dsector = pblock * _fmi_uPagePerBlock + rpage;
			
				while(1)
				{
					status = update_page_data(-1, Dsector, rsector, sector_count, pBuf);				
					if (status != FMI_NO_ERR)
						return status;
					Dsector++;		// physical page address
					count = count - sector_count;
					if (count <= 0)
						break;
					rsector = 0;	// relative sector of page
					pBuf = pBuf + sector_count*512;

					if (count < 4)
						sector_count = count;
					else
						sector_count = 4;
				}

				if ((status = SM_AddFreeBlock(zone, free_block, 0)) < 0)
					return status;
			}
		}
		else
		{
			_fmi_uRAMark = 0x0;
			count = sec_count;
			pBuf = buff;
			if (count > (4 - rsector))
				sector_count = 4 - rsector;
			else
				sector_count = count;

			Dsector = free_block * _fmi_uPagePerBlock + rpage;

			fmiSML2PWriteProtect(Dsector-rpage, 0x7f);
			if (rpage != 0)
				fmiSM_WritePage0(Dsector-rpage, _fmi_uRAMark|0xcfffffff);

			while(1)
			{
				status = update_page_data(-1, Dsector, rsector, sector_count, pBuf);				
				if (status != FMI_NO_ERR)
					return status;
				Dsector++;		// physical page address
				count = count - sector_count;
				if (count <= 0)
					break;
				rsector = 0;	// relative sector of page
				pBuf = pBuf + sector_count*512;

				if (count < 4)
					sector_count = count;
				else
					sector_count = 4;
			}

			fmiSML2PWriteProtect(free_block * _fmi_uPagePerBlock, 0x3f);
			SM_ZONE[zone].L2P_table[block] = free_block;
		}
	}

	return FMI_NO_ERR;
}


INT fmiSM_Read(UINT32 uSector, UINT32 uBufcnt, UINT32 uDAddr)
{
	unsigned char *pBuf;
	int volatile count, status, retryCount=0;
	UINT32 rpage, sblock, rblock, zone_no, page_count;
	
	fmiLock();
	
#ifdef SD_DEVICE
	_fmi_bIsNandAccess = TRUE;
	// disable GPIO interrupt when t-flash enable
	if (_fmi_bIsGPIOEnable)
	{
		if (pCard_SD->uGPIO != FMI_NO_CARD_DETECT)
			outpw(REG_GPIO_IE, inpw(REG_GPIO_IE) & ~(1 << pCard_SD->uGPIO));
	}
#endif

	// [2004/10/27] multi PAD control to SM host
	outpw(0x7ff00020, (inpw(0x7ff00020)&0x9fffffff)|0x80000000);

#ifdef DEBUG
	//printf("read: sector_no[%d], sector_cnt[%d]\n", uSector, uBufcnt);
#endif

	count = uBufcnt;
	sblock = uSector / _fmi_uSectorPerBlock;	// logic start block no.
	rpage = uSector % _fmi_uSectorPerBlock;	// relative sector offset
	zone_no = sblock / 1000;
	rblock = sblock % 1000;		// relative logic block

	pBuf = (unsigned char *)uDAddr;
	if (count > (_fmi_uSectorPerBlock - rpage))
		page_count = _fmi_uSectorPerBlock - rpage;
	else
		page_count = count;

#ifdef _USE_TWO_NAND_
	/* select device 0 or 1 */
	if (_fmi_bIs2NAND)
	{
		if (zone_no >= _fmi_uSM_2NANDZone)
			fmiSM_Initial(1);
		else
			fmiSM_Initial(0);
	}
#endif

	while(1)
	{
_fmi_retry_r_:
		status = get_block_data(zone_no, rblock, rpage, page_count, (UINT32)pBuf);
		//if (status != FMI_NO_ERR)
		if (status < 0)
		{
#ifdef DEBUG
			printf("retry_r [%x]\n", status);
#endif
			diag_printf("[%d]retry_r [%x]\n", retryCount, status);
			retryCount++;
			if (retryCount < 3)
				goto _fmi_retry_r_;
			else
			{
				fmiUnLock();
				return status;
			}
		}

		count = count - page_count;
		if (count <= 0)
			break;
		rpage = 0;
		pBuf = pBuf + page_count*_fmi_uSM_SectorSize;

		if (_fmi_uSectorPerBlock > count)
			page_count = count;
		else
			page_count = _fmi_uSectorPerBlock;

		rblock++;
		if (rblock >= 1000)
		{
			zone_no++;
			rblock = 0;

#ifdef _USE_TWO_NAND_
			if (_fmi_bIs2NAND)
				if (zone_no >= _fmi_uSM_2NANDZone)
					fmiSM_Initial(1);
#endif
		}
	}

#ifdef _USE_TWO_NAND_
	if (_fmi_bIs2NAND)
		fmiSM_Initial(7);
#endif

#ifdef SD_DEVICE
	_fmi_bIsNandAccess = FALSE;

	// enable GPIO interrupt after sm_write when t-flash enable
	if (_fmi_bIsGPIOEnable)
	{
		//for (count=0; count<0x600; count++);
		outpw(REG_SMCMD, 0xf0);
		if (pCard_SD->uGPIO != FMI_NO_CARD_DETECT)
		{
			status = inpw(REG_GPIO_STS);		// status register
			if ((status & (1 << pCard_SD->uGPIO)) == _fmi_nGPIOInsertState_SD)
			{
				_fmi_bIsSDInsert = TRUE;	// card insert
				_fmi_bIsGPIOEnable = FALSE;
				outpw(REG_GPIO_IE, inpw(REG_GPIO_IE) & ~(1 << pCard_SD->uGPIO));	// disable interrupt
				outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) & ~_fmi_nGPIOPower_SD);	// set GPIO12 output low
				if (fmiSDInsertFun != NULL)
					(*fmiSDInsertFun)();
			}
			else
			{
				_fmi_bIsSDInsert = FALSE;	// card remove
				_fmi_bIsGPIOEnable = TRUE;
				outpw(REG_GPIO_IS, inpw(REG_GPIO_IS) | (1 << pCard_SD->uGPIO));
				outpw(REG_GPIO_IE, inpw(REG_GPIO_IE) | (1 << pCard_SD->uGPIO));
			}
		}
	}
#endif
	fmiUnLock();
	return FMI_NO_ERR;
}


INT fmiSM_Write(UINT32 uSector, UINT32 uBufcnt, UINT32 uSAddr)
{
	unsigned char *pBuf;
	int count, status, retryCount=0;
	UINT32 rpage, sblock, rblock, zone_no, page_count;
	
	fmiLock();
#ifdef DEBUG
	//printf("write: sector_no[%d], sector_cnt[%d]\n", uSector, uBufcnt);
#endif

#ifdef SD_DEVICE
	_fmi_bIsNandAccess = TRUE;

	// disable GPIO interrupt when t-flash enable
	if (_fmi_bIsGPIOEnable)
	{
		if (pCard_SD->uGPIO != FMI_NO_CARD_DETECT)
			outpw(REG_GPIO_IE, inpw(REG_GPIO_IE) & ~(1 << pCard_SD->uGPIO));
	}
#endif

	// [2004/10/27] multi PAD control to SM host
	outpw(0x7ff00020, (inpw(0x7ff00020)&0x9fffffff)|0x80000000);

	count = uBufcnt;
	sblock = uSector / _fmi_uSectorPerBlock;	// logic start block no.
	rpage = uSector % _fmi_uSectorPerBlock;	// relative page offset
	zone_no = sblock / 1000;
	rblock = sblock % 1000;		// relative logic block

	pBuf = (unsigned char *)uSAddr;
	if (count > (_fmi_uSectorPerBlock - rpage))
		page_count = _fmi_uSectorPerBlock - rpage;
	else
		page_count = count;

#ifdef _USE_TWO_NAND_
	/* select device 0 or 1 */
	if (_fmi_bIs2NAND)
	{
		if (zone_no >= _fmi_uSM_2NANDZone)
			fmiSM_Initial(1);
		else
			fmiSM_Initial(0);
	}
#endif

	while(1)
	{
_fmi_retry_w_:
		if (_fmi_u2KPageSize)
			status = update_block_data_2K(zone_no, rblock, rpage, page_count, (UINT32)pBuf);
		else
			status = update_block_data(zone_no, rblock, rpage, page_count, (UINT32)pBuf);
		if (status != FMI_NO_ERR)
		{
#ifdef DEBUG
			printf("retry_w [%x]\n", status);
#endif
			retryCount++;
			if (retryCount < 3)
				goto _fmi_retry_w_;
			else
			{
				fmiUnLock();
				return status;
			}
		}
		count = count - page_count;
		if (count <= 0)
			break;
		rpage = 0;
		pBuf = pBuf + page_count*_fmi_uSM_SectorSize;

		if (_fmi_uSectorPerBlock > count)
			page_count = count;
		else
			page_count = _fmi_uSectorPerBlock;

		rblock++;
		if (rblock >= 1000)
		{
			zone_no++;
			rblock = 0;

#ifdef _USE_TWO_NAND_
			if (_fmi_bIs2NAND)
				if (zone_no >= _fmi_uSM_2NANDZone)
					fmiSM_Initial(1);
#endif
		}
	}

#ifdef _USE_TWO_NAND_
	if (_fmi_bIs2NAND)
		fmiSM_Initial(7);
#endif

#ifdef SD_DEVICE
	_fmi_bIsNandAccess = FALSE;

	// enable GPIO interrupt after sm_write when t-flash enable
	if (_fmi_bIsGPIOEnable)
	{
		outpw(REG_SMCMD, 0xf0);
		if (pCard_SD->uGPIO != FMI_NO_CARD_DETECT)
		{
			status = inpw(REG_GPIO_STS);		// status register
			if ((status & (1 << pCard_SD->uGPIO)) == _fmi_nGPIOInsertState_SD)
			{
				_fmi_bIsSDInsert = TRUE;	// card insert
				_fmi_bIsGPIOEnable = FALSE;
				outpw(REG_GPIO_IE, inpw(REG_GPIO_IE) & ~(1 << pCard_SD->uGPIO));	// disable interrupt
				outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) & ~_fmi_nGPIOPower_SD);	// set GPIO12 output low
				if (fmiSDInsertFun != NULL)
					(*fmiSDInsertFun)();
			}
			else
			{
				_fmi_bIsSDInsert = FALSE;	// card remove
				_fmi_bIsGPIOEnable = TRUE;
				outpw(REG_GPIO_IS, inpw(REG_GPIO_IS) | (1 << pCard_SD->uGPIO));
				outpw(REG_GPIO_IE, inpw(REG_GPIO_IE) | (1 << pCard_SD->uGPIO));
			}
		}
	}
#endif
	fmiUnLock();
	return FMI_NO_ERR;
}


#ifdef _WB_FAT_
INT mark_block_data(UINT32 zone, UINT32 block, UINT32 sector, INT32 sec_count)
{
	int count, status=0, sector_count;
	int pblock, Ssector, rsector;
	UINT32 rpage;

	if (SM_ZONE[zone].isScan == 0)
		fmiSM_L2PTable_Init(zone);

	pblock = SM_ZONE[zone].L2P_table[block];

	if (pblock == -1)
		return FMI_NO_ERR;

	/* delete this block */
	if (sec_count == _fmi_uSectorPerBlock)
	{
		if ((status = fmiSM_BlockErase(pblock)) != FMI_NO_ERR)
		{
#ifdef DEBUG
			printf("1. mark_block_data_2K:erase %d block fail [%x]!!\n", pblock, status);
#endif
			return status;
		}

		SM_ZONE[zone].L2P_table[block] = -1;

		if ((status = SM_AddFreeBlock(zone, pblock, 0)) < 0)
			return status;
	}
	/* mark these sectors */
	else
	{
		if (_fmi_nPreDelBlock != pblock)
		{
			_fmi_nPreDelBlock = pblock;
			_fmi_uDelPageOfBlock = 0;
		}

		if (_fmi_u2KPageSize)
		{
			count = sec_count;
			rpage = sector / 4;
			rsector = sector % 4;

			if (count > (4 - rsector))
				sector_count = 4 - rsector;
			else
				sector_count = count;

			Ssector = pblock * _fmi_uPagePerBlock + rpage;

			while(1)
			{
				if (sector_count == 4)
				{
					fmiSM_SetMarkFlag(Ssector, 0xfc);
					_fmi_uDelPageOfBlock++;
				}

				Ssector++;
				count = count - sector_count;
				if (count <= 0)
					break;

				if (count < 4)
					sector_count = count;
				else
					sector_count = 4;
			}
		}
		else
		{
			count = sec_count;
			Ssector = pblock * _fmi_uPagePerBlock + sector;
			while(1)
			{			
				fmiSM_SetMarkFlag(Ssector, 0xfc);
				_fmi_uDelPageOfBlock++;
				Ssector++;
				if ((--count) <= 0)
					break;
			}
		}
	}

	if (_fmi_uDelPageOfBlock == _fmi_uPagePerBlock)
	{
		//printf("marge block OK L<%d>, P<%d>\n", block, _fmi_nPreDelBlock);
		if ((status = fmiSM_BlockErase(_fmi_nPreDelBlock)) != FMI_NO_ERR)
		{
#ifdef DEBUG
			printf("2. mark_block_data_2K:erase %d block fail [%x]!!\n", _fmi_nPreDelBlock, status);
#endif
			return status;
		}

		SM_ZONE[zone].L2P_table[block] = -1;

		if ((status = SM_AddFreeBlock(zone, _fmi_nPreDelBlock, 0)) < 0)
			return status;
	}

	return FMI_NO_ERR;
}


VOID fmiSMDelete(PDISK_T *ptPDisk, UINT32 uSector, UINT32 uSecCount)
{
	int count, status;
	UINT32 rpage, sblock, rblock, zone_no, page_count;

	if ((ptPDisk != pDisk_SM) || (ptPDisk != pDisk_SM_P))
	{
#ifdef DEBUG
		//printf("Not SM device\n");
#endif
		return; 
	}

	if (ptPDisk == pDisk_SM_P)
		uSector = uSector + _fmi_uReservedBaseSector;

#ifdef DEBUG
	//printf("fmiSMDelete: sector_no[%d], sector_cnt[%d]\n", uSector, uSecCount);
#endif

	count = uSecCount;
	sblock = uSector / _fmi_uSectorPerBlock;	// logic start block no.
	rpage = uSector % _fmi_uSectorPerBlock;	// relative page offset
	zone_no = sblock / 1000;
	rblock = sblock % 1000;		// relative logic block

	if (count > (_fmi_uSectorPerBlock - rpage))
		page_count = _fmi_uSectorPerBlock - rpage;
	else
		page_count = count;

#ifdef _USE_TWO_NAND_
	/* select device 0 or 1 */
	if (_fmi_bIs2NAND)
	{
		if (zone_no >= _fmi_uSM_2NANDZone)
			fmiSM_Initial(1);
		else
			fmiSM_Initial(0);
	}
#endif

	while(1)
	{
		status = mark_block_data(zone_no, rblock, rpage, page_count);
		count = count - page_count;
		if (count <= 0)
			break;
		rpage = 0;

		if (_fmi_uSectorPerBlock > count)
			page_count = count;
		else
			page_count = _fmi_uSectorPerBlock;

		rblock++;
		if (rblock >= 1000)
		{
			zone_no++;
			rblock = 0;

#ifdef _USE_TWO_NAND_
			if (_fmi_bIs2NAND)
				if (zone_no >= _fmi_uSM_2NANDZone)
					fmiSM_Initial(1);
#endif
		}
	}

#ifdef _USE_TWO_NAND_
	if (_fmi_bIs2NAND)
		fmiSM_Initial(7);
#endif
}
#endif

INT fmiReservedAreaRead(UINT32 uNandAddr, UINT32 uLen, UINT32 uDstAddr)
{
	int volatile status=0;
	UINT32 sector, count;

	if ((_fmi_bIsReserved) && (_fmi_bIs2Disk == FALSE))
	{
		sector = uNandAddr >> 9;
		count = uLen >> 9;
		if ((uLen % 512) != 0)
			count++;

		status = fmiSM_Read(_fmi_uReservedBaseSector+sector, count, uDstAddr);
		if (status != FMI_NO_ERR)
			return status;

		return FMI_NO_ERR;
	}
	else
		return FMI_NO_SM_RESERVED;
}

INT fmiReservedAreaWrite(UINT32 uNandAddr, UINT32 uLen, UINT32 uSrcAddr)
{
	int volatile status=0;
	UINT32 sector, count;

	if ((_fmi_bIsReserved) && (_fmi_bIs2Disk == FALSE))
	{
		sector = uNandAddr >> 9;
		count = uLen >> 9;
		if ((uLen % 512) != 0)
			count++;

		status = fmiSM_Write(_fmi_uReservedBaseSector+sector, count, uSrcAddr);
		if (status != FMI_NO_ERR)
			return status;

		return FMI_NO_ERR;
	}
	else
		return FMI_NO_SM_RESERVED;
}

INT fmiReservedAreaErase(VOID)
{
	int volatile i, pblock, status;
	UINT32 sblock, rblock, zone_no;

	if ((_fmi_bIsReserved) && (_fmi_bIs2Disk == FALSE))
	{
		sblock = _fmi_uReservedBaseSector / _fmi_uSectorPerBlock;	// logic start block no.
		zone_no = sblock / 1000;
		rblock = sblock % 1000;		// relative logic block

		for (i=0; i<_fmi_uSMReservedBlock; i++)
		{
			pblock = SM_ZONE[zone_no].L2P_table[rblock+i];

			if (pblock == -1)
				break;
//printf("erase: logic[%d/%d], pblock[%d] \n", zone_no, rblock+i, pblock);

			status = fmiSM_BlockErase(pblock);
			if (status != FMI_NO_ERR)
			{
#ifdef DEBUG
				printf("Reserved Area block erase fail <%d>!!\n", rblock+i);
#endif
				return status;
			}
			SM_ZONE[zone_no].L2P_table[rblock+i] = -1;
		}
		return FMI_NO_ERR;
	}
	else
		return FMI_NO_SM_RESERVED;
}

#endif	// SM_DEVICE

