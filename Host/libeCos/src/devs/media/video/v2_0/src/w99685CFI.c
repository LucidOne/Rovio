#include "video/w99685.h"

int SendCommand2W685(UINT16 cmd, UINT16 subcmd, CmdParameters_t *cmdparams, CmdReturns_t *cmdrets, UINT8 wait, UINT8 *buffer);
int W99685CMD_SetOpMode(UINT8 mode, UINT8 ppc, UINT8 burstn);
int W99685CMD_SetLCMNums(UINT8 lcdn, UINT8 dbus);
int W99685CMD_SelectLCM(UINT8 num);
int W99685CMD_SetLCMRes(UINT16 width, UINT16 height);
int W99685CMD_WriteLCMWndBuff(UINT8 *buff, UINT32 len);
int W99685CMD_SetLCMWndPos(UINT16 winx, UINT16 winy);
int W99685CMD_SndMassDataQuirk(const char *buffer, UINT32 len);
int W99685CMD_SetLCMWndSize(UINT16 width, UINT16 height);
int W99685CMD_PreviewCtrl(UINT8 flag);
int W99685_USBBRIDGE_GetData(char *buf, int blocks);
int W99685_GetData(char *buf, int blocks);
int W99685CMD_GetImageLen(UINT32 *len);
int W99685CMD_SetBridgeMode(UINT8 bToBridge);
int w99685DEBUG_REG(UINT8 flag, UINT8 *Value, UINT16 Address);


void udelay(int us)
{
	int i=0;
	for(i=0; i< 80*us; i++);
}

/* support function */
int SendCommand2W685(UINT16 cmd, UINT16 subcmd, CmdParameters_t *cmdparams, CmdReturns_t *cmdrets, UINT8 wait, UINT8 *buffer)
{
	int blocks = 0, len = 0;
	int i, j, k;
	int err = 0;
	
//	printk("subcmd: %4x, flag: %d\n", subcmd, wait);
//	if(cmdparams) {
//		printk("param1: %2x, param2: %2x, param3: %2x, param4: %2x\n", PARAM1(cmdparams), \
//				PARAM2(cmdparams), PARAM3(cmdparams), PARAM4(cmdparams));	
//	}
	
	k = DATATIMEOUT*60000;
	while(ISBUSY(FREG7)&&k) {
		k--;
		udelay(2);
	};
	
	
	if(!k && ISBUSY(FREG7)) {
		printf("IOERRO: %d\n", __LINE__);
		err = -EIO;
		goto out;
	}
	
	if(cmdparams) {
		FREG2 = PARAM4(cmdparams);
		FREG3 = PARAM3(cmdparams);
		FREG4 = PARAM2(cmdparams);
		FREG5 = PARAM1(cmdparams);
	}
	
	if(subcmd != CMD_DUMMY) {	//==CMD_DUMMY, just read data
		FREG6 = subcmd;
		FREG7 = (cmd&CMDMASK);
	}
	if(wait) {
		k = DATATIMEOUT*3000;
		while(ISBUSY(FREG7)&&k) {
			k--;
			udelay(2);
		};
		if(!k && ISBUSY(FREG7)) {
			printf("IOERRO: %d\n", __LINE__);
			err = -EIO;
			goto out;
		}
		if(cmdrets)
		{
			RETVAL1(cmdrets) = FREG5;
			RETVAL2(cmdrets) = FREG4;
			RETVAL3(cmdrets) = FREG3;
			RETVAL4(cmdrets) = FREG2;
//			printk("ret val1: %2x, val2: %2x, val3: %2x, val4: %2x\n", cmdrets->retval1, \
//					cmdrets->retval2, cmdrets->retval3, cmdrets->retval4);
		}
		
		if(!buffer)
		{
			
		}
		else
		{
			
			len = (PARAM1(cmdparams) << 24) | (PARAM2(cmdparams) << 16) | (PARAM3(cmdparams) << 8) | PARAM4(cmdparams);
			blocks = (len+BLOCKSIZE-1)/BLOCKSIZE;
			KDEBUG("Block size: %d, param1: %2x, param2: %2x, param3: %2x, param4: %2x", blocks, \
				PARAM1(cmdparams), PARAM2(cmdparams), PARAM3(cmdparams), PARAM4(cmdparams));
#if 0
			for(i = 0; i < blocks; i++)
			{
				KDEBUG("Before write mass data buffer, %4x", FREG7);
				while(ISNOTRANSFER(FREG7));
				for(j = 0; j < 512; j++)
				{
					FREG0=*(buffer++);
				}
			}
#endif
			//printf("End write the data buffer in SendCommand2W685\n");
		}
	}
out:	
	
	return err;
}



/*---------------------------------- Start of CFI Control Command Functions ---------------------*/
int W99685CMD_SetOpMode(UINT8 mode, UINT8 ppc, UINT8 burstn)
{
	int err = 0;
	CmdParameters_t params;
	UINT16 cmd = GENERAL_CMD;
	UINT16 subcmd = CMD_SETOPMODE;
	
	memset(&params, 0, sizeof(CmdParameters_t));
	INIT_PARAMS(&params, mode, ppc, burstn, 0x00);
	if( (err = SendCommand2W685(cmd, subcmd, &params, NULL, ASYN, NULL)) < 0)
	{
		printk("Reset LCD erro: %d, line:%d\n", err, __LINE__);
		goto out;
	}
out:
	return err;
} 

int W99685CMD_SetLCMNums(UINT8 lcdn, UINT8 dbus)
{
	int err = 0;
	CmdParameters_t params;
	UINT16 cmd = GENERAL_CMD;
	UINT16 subcmd = CMD_SETLCMNUM;
	
	memset(&params, 0, sizeof(CmdParameters_t));

	INIT_PARAMS(&params, lcdn, dbus, 0x00, 0x00);
	if( (err = SendCommand2W685(cmd, subcmd, &params, NULL, ASYN, NULL)) < 0)
	{
		printk("Reset LCD erro: %d, line:%d\n", err, __LINE__);
		goto out;
	}
out:
	return err;
}

int W99685CMD_SelectLCM(UINT8 num)
{
	int err = 0;
	CmdParameters_t params;
	UINT16 cmd = GENERAL_CMD;
	UINT16 subcmd = CMD_SELECTLCM;
	
	memset(&params, 0, sizeof(CmdParameters_t));

	INIT_PARAMS(&params, num, 0x00, 0x00, 0x00);
	if( (err = SendCommand2W685(cmd, subcmd, &params, NULL, ASYN, NULL)) < 0)
	{
		printk("Reset LCD erro: %d, line:%d\n", err, __LINE__);
		goto out;
	}
out:
	return err;
}

int W99685CMD_SetLCMRes(UINT16 width, UINT16 height)
{
	int err = 0;
	UINT8 r2, r3, r4, r5;
	CmdParameters_t params;
	UINT16 cmd = GENERAL_CMD;
	UINT16 subcmd = CMD_SETLCMRES;
	
	r2=LBYTE(height);
	r3=HBYTE(height);
	r4=LBYTE(width);
	r5=HBYTE(width);
	memset(&params, 0, sizeof(CmdParameters_t));

	INIT_PARAMS(&params, r5, r4, r3, r2);
	if( (err = SendCommand2W685(cmd, subcmd, &params, NULL, ASYN, NULL)) < 0)
	{
		printk("Reset LCD erro: %d, line:%d\n", err, __LINE__);
		goto out;
	}
out:
	return err;
}

int W99685CMD_WriteLCMWndBuff(UINT8 *buff, UINT32 len)
{
	// We support mode 1 only here.
	int err = 0;
	UINT8 r2, r3, r4;
	const char * pbuff =(const char * )buff;
	CmdParameters_t params;
	UINT16 cmd = GENERAL_CMD;
	UINT16 subcmd = CMD_WRITELCMWBUF;
	
	memset(&params, 0, sizeof(CmdParameters_t));
	if( (err = SendCommand2W685(cmd, subcmd, &params, NULL, ASYN, NULL)) < 0)
	{
		printk("Reset LCD erro: %d, line:%d\n", err, __LINE__);
		goto out;
	}

	r2=ABYTE(len);
	r3=BBYTE(len);
	r4=CBYTE(len);
	INIT_PARAMS(&params, 1, r4, r3, r2);
	//KDEBUG("len: %x, %4x, low16: %4x, hig8: %2x", len, len&0xffff, LWORD(len), BBYTE(len));
	//KDEBUG("param1: %2x, param2: %2x, param3: %2x, param4: %2x\n", 1, r4, r3, r2);
	cmd = GENERAL_CMD;
	subcmd = CMD_WRITELCMWBUF;
	if( (err = SendCommand2W685(cmd, subcmd, &params, NULL, WAIT, NULL)) < 0)
	{
		printk("Reset LCD erro: %d, line:%d\n", err, __LINE__);
		goto out;
	}
	
	W99685CMD_SndMassDataQuirk(pbuff, len);
	
	INIT_PARAMS(&params, 0x04, 0x00, 0x00, 0x00);
	cmd = GENERAL_CMD;
	subcmd = CMD_WRITELCMWBUF;
	if( (err = SendCommand2W685(cmd, subcmd, &params, NULL, ASYN, NULL)) < 0)
	{
		printk("Reset LCD erro: %d, line:%d\n", err, __LINE__);
		goto out;
	}
out:
	return err;	
}

int W99685CMD_SetLCMWndPos(UINT16 winx, UINT16 winy)
{
	int err = 0;
	UINT8 r2,r3,r4,r5;
	CmdParameters_t params;
	UINT16 cmd = GENERAL_CMD;
	UINT16 subcmd = CMD_SETLCMWNDPOS;
	
	memset(&params, 0, sizeof(CmdParameters_t));
	
	r2=LBYTE(winy);
	r3=HBYTE(winy);
	r4=LBYTE(winx);
	r5=HBYTE(winx);
	
	INIT_PARAMS(&params, r5, r4, r3, r2);
#if 1
	if( (err = SendCommand2W685(cmd, subcmd, &params, NULL, ASYN, NULL)) < 0)
	{
		printk("Reset LCD erro: %d, line:%d\n", err, __LINE__);
		goto out;
	}
out:
#endif
	return err;
}

int W99685CMD_SndMassDataQuirk(const char *buffer, UINT32 len)
{
	int err = 0;
	int blocks;
	int i, j, k;
	UINT32 uLenSent;
	uLenSent = 0;
	blocks = (len+BLOCKSIZE-1)/BLOCKSIZE;
	
	
	for(i = 0; i < blocks; i++)
	{
		//KDEBUG("Before write mass data buffer, %4x, %2x", FREG7, ISNOTRANSFER(FREG7));
		k = DATATIMEOUT;
		if(i == 0){
			while(ISNOTRANSFER1THBLCK(FREG7)&&k) {
				k--;
				udelay(2);
			}
		}
		else
			while(ISNOTRANSFER(FREG7)&&k) {
				k--;
				udelay(2);
			}
		if(k == 0 && ISNOTRANSFER(FREG7))
			diag_printf("Write 685 timeout\n");
		for(j = 0; j < 512; j++)
		{
			if(uLenSent < len) {
				FREG0=*buffer;
				buffer++;
				//FREG0 = 0xff;
				uLenSent++;
			}
			else
				FREG0=(UINT8)0;
		}
	}
	//KDEBUG(".");
	return err;
}

int W99685CMD_SetLCMWndSize(UINT16 width, UINT16 height)
{
	int err = 0;
	UINT8 r2,r3,r4,r5;
	CmdParameters_t params;
	UINT16 cmd = GENERAL_CMD;
	UINT16 subcmd = CMD_SETLCMWNDSIZ;
	
	memset(&params, 0, sizeof(CmdParameters_t));
	
	r2=LBYTE(height);
	r3=HBYTE(height);
	r4=LBYTE(width);
	r5=HBYTE(width);
	
	INIT_PARAMS(&params, r5, r4, r3, r2);
	if( (err = SendCommand2W685(cmd, subcmd, &params, NULL, ASYN, NULL)) < 0)
	{
		printk("Reset LCD erro: %d, line:%d\n", err, __LINE__);
		goto out;
	}
out:
	return err;
}

int W99685CMD_PreviewCtrl(UINT8 flag)
{
	int err = 0;
	CmdParameters_t params;
	UINT16 cmd = GENERAL_CMD;
	UINT16 subcmd = CMD_PREVIEW_CTRL;
	
	memset(&params, 0, sizeof(CmdParameters_t));
	
	INIT_PARAMS(&params, flag, 0x00, 0x00, 0x00);
#if 1
	if( (err = SendCommand2W685(cmd, subcmd, &params, NULL, ASYN, NULL)) < 0)
	{
		printk("Reset LCD erro: %d, line:%d\n", err, __LINE__);
		goto out;
	}
out:
#endif
	return err;
}

int W99685_USBBRIDGE_GetData(char *buf, int blocks)
{
	int err = 0;
	int i, j, k;
	char aa;

	for(i = 0; i < blocks; i++)
	{
		k = DATATIMEOUT*10;
		if(i == 0) {
			while(ISNOTRANSFER1THBLCK(FREG7) && k) {
				//printk("Freg7: %x\n", FREG7);
    				k--;
				udelay(2);
			}
		}
		else {
			while(ISNOTRANSFER(FREG7) && k) {
				k--;
				udelay(2);
			}
		}

		if(!k && ISNOTRANSFER(FREG7)) {
			printk("Read Data timeout\n");
			err = -EIO;
			goto out;
		}
		
		for(j = 0; j < 512; j++)
		{
			*(buf++) = FREG0;
		}

	}
out:
	return err;
}

int W99685_GetData(char *buf, int blocks)
{
	//diag_printf("W99685_GetData!\n");
	int err = 0;
	int i, j, k;
	char aa;

	for(i = 0; i < blocks; i++)
	{
		k = DATATIMEOUT*100;
		while(ISNOTRANSFER(FREG7) && k) {
			k--;
			udelay(2);
		}

		if(!k && ISNOTRANSFER(FREG7)) {
			diag_printf("Read Data timeout, i: %d,FREG7 %x \n",i,FREG7);
			err = -EIO;
			goto out;
		}
		
		for(j = 0; j < 512; j++)
		{
			*(buf++) = FREG0;
		}

	}
out:
	return err;
}


int W99685CMD_GetImageLen(UINT32 *len)
{
	int err = 0;
	CmdParameters_t params;
	CmdReturns_t retvals;
	UINT16 cmd = GENERAL_CMD;
	UINT16 subcmd = CMD_GETJPGBITS;
	
	memset(&params, 0, sizeof(CmdParameters_t));
	memset(&retvals, 0, sizeof(CmdReturns_t));
	
	if( (err = SendCommand2W685(cmd, subcmd, &params, &retvals, WAIT, NULL)) < 0)
	{
		printk("Reset LCD erro: %d, line:%d\n", err, __LINE__);
		goto out;
	}
	MKWORDB(*len, (UINT8)RETVAL4(&retvals), (UINT8)RETVAL3(&retvals), (UINT8)RETVAL2(&retvals), (UINT8)RETVAL1(&retvals));
out:
	return err;
} 

int W99685CMD_SetBridgeMode(UINT8 bToBridge)
{
	int ret = 0;
	CmdParameters_t params;
	CmdReturns_t rets;
	UINT16 cmd = GENERAL_CMD;
	UINT16 subcmd = CMD_SELECTMODE;
	
	memset(&params, 0, sizeof(CmdParameters_t));
	
	INIT_PARAMS(&params, bToBridge, 0x00, 0x00, 0x00);

	if( (ret = SendCommand2W685(cmd, subcmd, &params, NULL, ASYN, NULL)) < 0)
	{
		printk("Reset LCD erro: %d, line:%d\n", ret, __LINE__);
		goto out;
	}
out:
	return ret;
}

int w99685DEBUG_REG(UINT8 flag, UINT8 *Value, UINT16 Address)
{
	int err = 0;
	CmdParameters_t params;
	CmdReturns_t retvals;
	UINT16 cmd = DEBUG_CMD;
	UINT16 subcmd = REG_DEBUG;
	UINT8 wflag;
	
	if(flag == REG_READ)
		wflag = WAIT;
	else
		wflag = ASYN;
	
	memset(&params, 0, sizeof(CmdParameters_t));
	memset(&retvals, 0, sizeof(CmdReturns_t));
	
	INIT_PARAMS(&params, flag, *Value, HBYTE(Address), LBYTE(Address));
	if( (err = SendCommand2W685(cmd, subcmd, &params, &retvals, wflag, NULL)) < 0)
	{
		printk("Reset LCD erro: %d, line:%d\n", err, __LINE__);
		goto out;
	}
	
	if(flag == REG_READ)
		*Value = RETVAL2(&retvals);
out:
	return err;
}

#if 0
EXPORT_SYMBOL(SendCommand2W685);
EXPORT_SYMBOL(W99685CMD_SetOpMode);
EXPORT_SYMBOL(W99685CMD_SetLCMNums);
EXPORT_SYMBOL(W99685CMD_SelectLCM);
EXPORT_SYMBOL(W99685CMD_SetLCMRes);
EXPORT_SYMBOL(W99685CMD_WriteLCMWndBuff);
EXPORT_SYMBOL(W99685CMD_SetLCMWndPos);
EXPORT_SYMBOL(W99685CMD_SndMassDataQuirk);
EXPORT_SYMBOL(W99685CMD_SetLCMWndSize);
EXPORT_SYMBOL(W99685CMD_PreviewCtrl);
EXPORT_SYMBOL(W99685_USBBRIDGE_GetData);
EXPORT_SYMBOL(W99685_GetData);
EXPORT_SYMBOL(W99685CMD_GetImageLen);
EXPORT_SYMBOL(W99685CMD_SetBridgeMode);
EXPORT_SYMBOL(w99685DEBUG_REG);
#endif
