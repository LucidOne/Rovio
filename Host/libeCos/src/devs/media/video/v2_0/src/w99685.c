/*
	Winbond w99683 Webcam CompactFlash driver for eCos 2.0.

	Copyright (C) 2003 Winbond

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "cyg/hal/plf_io.h"
#include "video/w99685.h"
#include "pkgconf/devs_media_video_w99685.h"

#include "asm/semaphore.h"

#define DRIVER_VERSION "v1.00 for eCos 2.0"
#define EMAIL "ypxin@winbond.com.tw"
#define DRIVER_AUTHOR "ypxin"
#define DRIVER_DESC "W99685 CompactFlash Camera Driver"

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
int W99685_GetData(char *buf, int blocks);
int W99685CMD_GetImageLen(UINT32 *len);
int W99685CMD_SetBridgeMode(UINT8 bToBridge);
int w99685DEBUG_REG(UINT8 flag, UINT8 *Value, UINT16 Address);

static int ResetLCD();
static int InitLCD();
static inline void LCDWriteCMDAddrWORD(UINT16 addr);
static inline void LCDWriteCMDRegWORD(UINT16 data);
static void LCDPowerSettingFunction(void);
static void LCDPowerSettingFunctionTwo(void);
static void LCDInitialFunctionOne(void);
static void LCDInitialFunctionTwo(void);
static void LCDDisplayOnFunction(void);
static void LCM_FillChar(UINT8 *buf, int count);
static void LCM_Zoom2();
static void test_LCM_FillChar(int color, char buf[]);
static int DrawInByPassMode(UINT8 *buf, UINT32 Length);

static w685cf_t		w99685priv;

//YC: modified for ADS
#if 0
DEVTAB_ENTRY(w90n740_99685_video0, 
             CYGDAT_MEDIA_VIDEO_VIDEO0_NAME,
             0,                     // Does not depend on a lower level interface
             &cyg_io_w99685_ops, 
             w90n740_video_init, 
             w90n740_video_lookup,     // Serial driver may need initializing
             &w99685priv
    );


typedef struct cyg_devtab_entry {
    const char        *name;
    const char        *dep_name;
    cyg_devio_table_t *handlers;
    bool             (* init)(struct cyg_devtab_entry *tab);
    Cyg_ErrNo        (*lookup)(struct cyg_devtab_entry **tab, 
                               struct cyg_devtab_entry *sub_tab,
                               const char *name);
    void              *priv;
    unsigned long     status;
} CYG_HAL_TABLE_TYPE cyg_devtab_entry_t;
#endif

#pragma arm section rwdata = "devtab"

struct cyg_devtab_entry  w90n740_99685_video0= 
{
		CYGDAT_MEDIA_VIDEO_VIDEO0_NAME,
		0,                     // Does not depend on a lower level interface
        &cyg_io_w99685_ops, 
        w90n740_video_init, 
	 w90n740_video_lookup,     // Serial driver may need initializing
        &w99685priv,
        CYG_DEVTAB_STATUS_CHAR
};

#pragma arm section rwdata


#define _USE_685_

#define MASSSTORAGEFW

UINT8 W685BRIDGEFW[]={
#ifndef MASSSTORAGEFW
	#include "W99685_USBBridge_1611.h"
#else
	#include "W99685_USBBridge_Mass_1311.h"
#endif
};

//#include "w685bg.h"
static int video_nr = -1; 
int DownLoadFWFlag = 1;
#define LEN_LCMBF		40960
static UINT8 lcm_buf[LEN_LCMBF]={
	0
};

static UINT8 imgbuf[W685BUF_SIZE];

int down_interruptible2(struct semaphore *sem)
{ 
	cyg_drv_mutex_lock((cyg_drv_mutex_t *)sem);
	return 0;
}

void DumpInternalRegs()
{
	UINT16 Addr;
	UINT8 Value;
	
	for(Addr = 0xB080; Addr <= 0xB0FF; Addr++)
	{
		w99685DEBUG_REG(REG_READ, &Value, Addr);
		printk("0x%x: 0x%x   ", Addr, Value);
	}
	printk("\n");
	printk("\n");
}

static void ByPass_Test (void)
{
	int i,j;
	UINT16 color1 = 0xf800; //Red color
	UINT16 color2 = 0x07E0; //Green color
	UINT16 color3 = 0x001f; //Blue color
	
	for (i=0;i<32;i++)
		for (j=0;j<128;j++)
			LCDWriteCMDRegWORD(color1);
	
	for (i=0;i<32;i++)
		for (j=0;j<128;j++)
			LCDWriteCMDRegWORD(color2);

	for (i=0;i<32;i++)
		for (j=0;j<128;j++)
			LCDWriteCMDRegWORD(color3);
	for(i = 0; i < 32; i++)
		for (j = 0; j < 128; j++)
			LCDWriteCMDRegWORD(color1);
			
	for (i=0;i<32;i++)
		for (j=0;j<128;j++)
			LCDWriteCMDRegWORD(color2);
}

int W99685CMD_Suspand(void)
{
	int err = 0;
	CmdParameters_t params;
	UINT16 cmd = GENERAL_CMD;
	UINT16 subcmd = CMD_SUSPENDLCM;

	memset(&params, 0, sizeof(CmdParameters_t));

	INIT_PARAMS(&params, 0x00, 0x00, 0x00, 0x00);
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

#ifdef W865THREAD
static inline int GetImageLenFromContainer(ImageContainer_t *container)
{
	int ret = 0;
	UINT8 choice = container->reading;	// Get Reading Position
	
	if(IMAGELEN(container, choice) == 0) {	//
		ret = 0;
	}
	else if(IMAGELEN(container, choice)>0 && IMAGELEN(container, choice)<W685BUF_SIZE)
	{
		ret = IMAGELEN(container, choice);
	}
	else
		ret = -EIO;
out:
	return ret;
}

static int w99685_thread(void *arg)
{
	int ret = 0;
	UINT32 imglen = 0;;
	UINT16 blocks = 0;
	UINT8 choice = 0;
	w685cf_t *priv = arg;
	ImageContainer_t *imgcontainerp = priv->images;
	/*
	 * This thread doesn't need any user-level access,
	 * so get rid of all our resources..
	 */
	daemonize();
	reparent_to_init();
//	exit_files(current);
//	current->files = init_task.files;
//	atomic_inc(&current->files->count);
//	daemonize();
//	reparent_to_init();

	/* avoid getting signals */
//	spin_lock_irq(&current->sigmask_lock);
//	flush_signals(current);
//	sigfillset(&current->blocked);
//	recalc_sigpending(current);
//	spin_unlock_irq(&current->sigmask_lock);

	/* set our name for identification purposes */
	sprintf(current->comm, "W99685CFI_thread");

//Get Image Data, :P	
	for(;;) {
		if(priv->user) { // Someone open the camera, get the data.
			W99685CMD_GetImageLen(&imglen);
			TDEBUG("imagelen: %d", imglen);
			if(imglen > 0&& imglen <= W685BUF_SIZE)
			{
				blocks = (imglen-1+DATABLOCKSIZE)/DATABLOCKSIZE;
				if(blocks < W685BUF_SIZE/DATABLOCKSIZE) {
					choice = imgcontainerp->which;
					TDEBUG("Write choice: %d, and choicelen: %d", choice, IMAGELEN(imgcontainerp, choice));
					down( &((IMAGEHOLD(imgcontainerp, choice)).ilock) );
					if(IMAGELEN(imgcontainerp, choice) > 0) //Have had data, don't overwrite it.
						goto do_next;
					W99685_GetData(IMAGEDATA(imgcontainerp, choice) ,blocks);
					(IMAGEHOLD(imgcontainerp, choice)).len = imglen;
do_next:
					imgcontainerp->which = (++(imgcontainerp->which))%IMGCONTAINERSIZE;
					up( &((IMAGEHOLD(imgcontainerp, choice)).ilock) );
				}
			TDEBUG("Sleep...");
			wait_ms(20);
			}
			else if(imglen < 0)  //can't recover, it crazy :(
			{
				W685_ERR("Get image length error: %d\n", imglen);
				break;
			}
			else {
				wait_ms(500);
				continue;
			}
		}
		else
			sleep_on_timeout(&priv->waitq, 10);
	}
out:
	return ret;
}
#endif

static int w99685_v4l1_open(struct cyg_devtab_entry **tab, 
                  struct cyg_devtab_entry *sub_tab,
                  const char *name)
	
{
	int err = 0;
	w685cf_t *priv = (w685cf_t *)(*tab)->priv;
//	printk("Open W99685\n");
//	down(&priv->lock);
//	err = -EBUSY;
//	if (priv->user)  {
//		printk("erro user count: %d\n", priv->user);
//		goto out;
//	}
//	priv->user++;
//	err = 0;
out:
	//up(&priv->lock);
	return err;
}
#if 0
void w99685_v4l1_close(struct video_device * vdev)
{
	w685cf_t *priv = (w685cf_t *)vdev->priv;
//	printk("Close W99685\n");
	
	down(&priv->lock);
	priv->user--;
	//up(&priv->lock);
	
}
#endif

long w99685_v4l1_read(w685cf_t *priv, char * buf, cyg_uint32 *count)
{
	int k, j;
	long rc = 0;
	long size = 0;
	long blocks = (*count-1+DATABLOCKSIZE)/DATABLOCKSIZE;
	//w685cf_t *priv = (w685cf_t *)vdev->priv;
	
#ifndef W865THREAD	
	char *buffer = priv->rawbuf;
	//KDEBUG("Read data\n");
	if(blocks > W685BUF_SIZE/DATABLOCKSIZE) {
		printf("Too long data: %dbytes\n", *count);
		rc = -EINVAL;
		goto out;
	}
	if (down_interruptible2(&(priv->lock))) {
		rc = -EINTR;
		goto out;
	}

	if (!buf ) {
		printf("W99865:vdev || !buf \n");
		rc = -EFAULT;
		goto out_up;
	}
	
	rc = W99685_GetData(buffer, blocks);
	if(!rc)
		rc = ENOERR;//clyu
	//if(rc > 0) //yc
	{
		if (copy_to_user(buf, buffer, *count)<0) {
			printf("copy_to_user\n");
			rc = -EFAULT;
			goto out_up;
		}
	}
	#if 0
	int i;
	for(i = 0; i < 20; i++)
		diag_printf("%x ", *(buffer+i));
	diag_printf("\n");
	
	for(i = 0; i < 20; i++)
		diag_printf("%x ", *(buf+i));
	diag_printf("\n");
	#endif
#endif

#ifdef	W865THREAD
	ImageContainer_t *container = priv->images;
	UINT8 choice = container->reading;	//Get Write Position
	TDEBUG("Begin reading...");
	
	if(blocks > W685BUF_SIZE/DATABLOCKSIZE) {
		printk("Too long data: %dbytes\n", *count);
		rc = -EINVAL;
		goto out;
	}
	if (down_interruptible2(&priv->lock)) {
		rc = -EINTR;
		goto out;
	}

	down( &(IMAGEHOLD(container, choice).ilock) );
	TDEBUG("read choice: %d, imglen: %d", choice, IMAGELEN(container, choice));
	if(IMAGELEN(container, choice) == 0) {
		// == 0, shouldn't happen, if so, just out.
		rc = 0;
		goto flag_up;
	}
	else if(IMAGELEN(container, choice) != *count)
	{	// != count, :(, wrong data ,remove it
		IMAGELEN(container, choice) = 0;
		rc = 0;
		goto flag_up;
	}
	if (copy_to_user(buf, IMAGEDATA(container, choice), *count)) {
		printk("copy_to_user\n");
		rc = -EFAULT;
		goto flag_up;
	}
	IMAGELEN(container, choice) = 0;	// clean the container's data
	container->reading = (++(container->reading))%IMGCONTAINERSIZE; // Goto Next Reading Position
	
	rc = ENOERR;//clyu
	//rc = *count;//clyu
flag_up:
	up( &(IMAGEHOLD(container, choice).ilock) );
#endif

out_up:
	up(&priv->lock);
out:	
	//TDEBUG("End reading...");	
	//diag_printf("read rc: %d \n",rc);
	return rc;
}

long w99685_v4l1_write(w685cf_t *priv, const char * buf, cyg_uint32 *count)
{
	
	long size = 0;
	//w685cf_t *priv = (w685cf_t *)vdev->priv;
	
	//diag_printf("w99685_v4l1_write :Write Length: ------------> %x, %d\n", count,*count);
	//diag_printf("Enter w99685_v4l1_write  %d\n", *count);
	if(*count > W685BUF_SIZE)
	{
		size = -EINTR;
		goto out;
	}
	
//	UINT8 imagebuf[LEN_LCMBF];
	if (down_interruptible2(&priv->lock)) {
		size = -EINTR;
		goto out;
	}
//	W99685CMD_WriteLCMWndBuff(buf, *count);
	size = W99685CMD_SndMassDataQuirk(buf, *count);
	if(size >= 0)
		size = ENOERR;//clyu
		//size = *count;//clyu
	up(&priv->lock);
out:
//	printk("end of w99685_v4l1_write\n");
	return size;
}

void CommandDump(CmdReq_t *reqp, CmdParameters_t * paramp)
{
#ifdef COMMAND_DUMP	
	CmdParameters_t *cmdparamp = paramp;
	
	printk("subcmd: %4x, flag: %d\n", reqp->subcmd, reqp->flag);
	if(cmdparamp) {
		printk("param1: %2x, param2: %2x, param3: %2x, param4: %2x\n", PARAM1(cmdparamp), \
				PARAM2(cmdparamp), PARAM3(cmdparamp), PARAM4(cmdparamp));	
	}
#endif
}


int w99685_v4l1_ioctl(w685cf_t *priv, unsigned int cmd, void * buf)
{
	
	int err = 0;
	CmdParameters_t *paramsp = NULL;
	CmdReturns_t *retsp = NULL;
	Firmware_t ifirmware;
	UINT8 *datap;
	int i;
#ifdef 	W865THREAD
	int whichselect;
	ImageContainer_t *containerp = priv->images;
#endif

	if (down_interruptible2(&priv->lock)) {
		printk("Device has been locked\n");
		err = -EINTR;
		goto out;
	} 
	
	
	
	//printk("Cmd: %x, getclass: %x, IOCTLSETFW_CAMERA = %x\n", cmd, IOCTLGETCLASS,IOCTLSETFW_CAMERA);
	if(cmd == IOCTLGETCLASS)
	{
		if(!buf) {
			printk("Illegal buffer address\n");
			err = -EINVAL;
			goto outup;
		}	
		
		if(copy_to_user(buf, DEVICECLALSS, (strlen(DEVICECLALSS)+1)))
		{
			printk("Cannot put data to user\n");
			err = -EFAULT;
		}
		err = (strlen(DEVICECLALSS)+1);
		printk("Get Class name: %s\n", DEVICECLALSS);
		goto outup;
	}
	else if(cmd == IOCTLGETSTATUS)
	{
		if(!buf) {
			printk("Illegal buffer address\n");
			err = -EINVAL;
			goto outup;
		}	
		
		if(copy_to_user(buf, (char *)&(FREG7), sizeof(FREG7)))
		{
			printk("Cannot put data to user\n");
			err = -EFAULT;
		}
		goto outup;
	}
	else if(cmd == IOCTLSETFW_CAMERA)
	{
		diag_printf("IOCTLSETFW_CAMERA = %x\n",IOCTLSETFW_CAMERA);
		if(copy_from_user(&ifirmware, (void *)buf, sizeof(Firmware_t))) {
			printk("Cannot copy data from user\n");
			err = -EFAULT;
 			goto outup;
		}
		printk("Firmware size: %d, %x\n", ifirmware.Length, ifirmware.Data);
		
		datap = (UINT8 *)malloc(ifirmware.Length*sizeof(UINT8));
		diag_printf("datap %p\n",datap);
		if(!datap)
		{
			printk("No enough memory\n");
			err = -ENOMEM;
 			goto outup;
		}
		if(copy_from_user(datap, (void *)ifirmware.Data, ifirmware.Length*sizeof(UINT8))) {
			printk("Cannot copy data from user\n");
			free(datap);
			err = -EFAULT;
 			goto outup;
		}
		InitCFI(0);
		DownLoadFirmware2Camera(datap, ifirmware.Length);
		free(datap);
		for(i=0;i<100;i++)
			wait_ms(20);
		
		printk("DownLoad Firmware ok\n");
		InitCFI(0);
		wait_ms(40);
		//CardDection();
		//W99685CMD_Suspand();
		//wait_ms(40);	
#if 1//ndef __OUTPUT2TV__
		W99685CMD_SetOpMode(OP_SINGLE, PPC_YUV422, 0);

		W99685CMD_SetLCMNums(LCD1_ON_685,LCD_BUS_16);
		//KDEBUG("Have set LCMNUMS");
		W99685CMD_SelectLCM(LCM_1ST);
		//KDEBUG("Have selected LCM\n");
		W99685CMD_SetLCMRes(128, 161);
		//KDEBUG("Have set LCM resolution\n");

		LCM_FillChar(lcm_buf, LEN_LCMBF);
		//wait_ms(100000);
		//memset(lcm_buf, 0xff, LEN_LCMBF);
		//LCM_FillChar(lcm_buf, LEN_LCMBF);
		//KDEBUG("Have Filled LCM with char\n");

		W99685CMD_SetLCMWndPos(0, 32);

		W99685CMD_SetLCMWndSize(128, 96);
#endif
		goto outup;
	}
	else if(cmd == IOCTLSETFW_USBBRIDGE)
	{
		DownLoadFirmware2Camera(W685BRIDGEFW, sizeof(W685BRIDGEFW));
		printk("Have DownLoad Bridge Firmware to W99685\n");
		goto outup;
	}
	else if(cmd == IOCTLDRAWINBYPASS) 
	{
#if 1
		if(copy_from_user(&ifirmware, (void *)buf, sizeof(Firmware_t))) {
			diag_printf("Cannot copy data from user\n");
			err = -EFAULT;
 			goto outup;
		}
		printk("Image size: %d\n", ifirmware.Length);
		
		datap = (UINT8 *)malloc(ifirmware.Length*sizeof(UINT8));
		if(!datap)
		{
			printk("No enough memory\n");
			err = -ENOMEM;
 			goto outup;
		}
		if(copy_from_user(datap, (void *)ifirmware.Data, ifirmware.Length*sizeof(UINT8))) {
			printk("Cannot copy data from user\n");
			free(datap);
			err = -EFAULT;
 			goto outup;
		}
		InitCFI(1);
		wait_ms(20);
		DrawInByPassMode(datap, ifirmware.Length);
		InitCFI(0);
		wait_ms(20);
		free(datap);
#else
		InitCFI(1);
		wait_ms(20);
		ByPass_Test();
		InitCFI(0);
		wait_ms(20);
#endif	
		goto outup;
	}
	
	
	
	
	if (copy_from_user(&priv->m_req, (void *)buf, sizeof(CmdReq_t))) {
		diag_printf("Cannot copy data from user\n");
		err = -EFAULT;
 		goto outup;
 	}

	
	
	if(priv->m_req.cmd_params)
	{
		if (copy_from_user(&priv->m_cmdparams, (void *)priv->m_req.cmd_params, sizeof(CmdParameters_t))) {
			diag_printf("Cannot copy data from user\n");
			err = -EFAULT;
 			goto outup;
 		}
 		paramsp = &priv->m_cmdparams;
	}
	
// allow no parameters
//	else {
//		printk("Command has not parameters\n");
//		err = -EINVAL;
//		goto outup;
//	}
	
	//printk("priv->m_req.cmd_rets: %x, %d\n", priv->m_req.cmd_rets, __LINE__);	
	if(priv->m_req.cmd_rets)
	{
		retsp = &priv->m_cmdrets;
		memset(retsp, 0, sizeof(CmdReturns_t));
	}
//	printk("priv->m_req.cmd_rets: %x, %d\n", priv->m_req.cmd_rets, __LINE__);
//	CommandDump(&priv->m_req, &priv->m_cmdparams);
	
#ifdef 	W865THREAD
	if(priv->m_req.subcmd == CMD_GETJPGBITS)
	{
		err = GetImageLenFromContainer(containerp);	
		TDEBUG("Get Image len: %d, which: %d", err, containerp->reading);
		if(err >= 0) {
			RETVAL4(retsp) = ABYTE(err);
			RETVAL3(retsp) = BBYTE(err);
			RETVAL2(retsp) = CBYTE(err);
			RETVAL1(retsp) = DBYTE(err);
		}
		
		if(priv->m_req.cmd_rets)
		{
			if(copy_to_user(priv->m_req.cmd_rets, retsp, sizeof(CmdReturns_t)))
			{
				err = -EFAULT;
				goto outup;
			}
		}
		up(&priv->lock);
		
		return err;
	}
#endif
	
//	printk("subcmd: 0x%x, priv->m_req.cmd_rets: %x, %d\n", priv->m_req.subcmd, priv->m_req.cmd_rets, __LINE__);
	err = SendCommand2W685(cmd, priv->m_req.subcmd, paramsp, retsp, priv->m_req.flag, NULL);
	
	if(err < 0)
	{
		printf("W99685: send command erro\n");
		goto outup;
	}

	if(priv->m_req.cmd_rets)
	{
//		printk("Copy to user retvals: %x, val: %x, %x, %x, %x\n", priv->m_req.cmd_rets,\
//				RETVAL4(retsp), RETVAL3(retsp), RETVAL2(retsp), RETVAL1(retsp));
		if(copy_to_user(priv->m_req.cmd_rets, retsp, sizeof(CmdReturns_t)))
		{
			diag_printf("Cannot copy data to user\n");
			err = -EFAULT;
			goto outup;
		}
	}
outup:
	
	
	up(&priv->lock);
	
	
//	printk("return value: %d\n", err);
out:
	return err;
}

#if 0
void w99685_v4l1_release(struct video_device *vdev)
{
	w685cf_t *priv = (w685cf_t *)vdev->priv;
	
	if(priv->rawbuf)
	{
		free(priv->rawbuf);
		priv->rawbuf = NULL;
	}
	
	free(priv);
}
#endif

// This routine is called when the device is "looked" up (i.e. attached)

static Cyg_ErrNo w90n740_video_lookup(struct cyg_devtab_entry **tab, 
                  struct cyg_devtab_entry *sub_tab,
                  const char *name)
{
	w685cf_t *priv = (w685cf_t *)(*tab)->priv;
	int err = 0;
	int i;
#ifdef	W865THREAD
	int pid;
#endif
	
	init_waitqueue_head(&priv->wq);
	init_MUTEX(&priv->lock);

#ifndef W865THREAD	
	priv->rawbuf = (char *)malloc(W685BUF_SIZE*sizeof(char));
	if(!priv->rawbuf) 
	{
		err = -ENOMEM;
		goto out_nomem1;
	}
#endif
#ifdef W865THREAD
	init_waitqueue_head(&(priv->waitq));
	
	priv->images = (ImageContainer_t *)malloc(sizeof(ImageContainer_t));
	if(!priv->images)
	{	
		err = -ENOMEM;
		goto out_nomem1;
	}
	
	memset(priv->images, 0, sizeof(ImageContainer_t));
	priv->images->reading = IMGCONTAINERSIZE-1;	//init, put it to IMGCONTAINERSIZE-1
	IMAGEDATA(priv->images, 0) = (char *)kmalloc(W685BUF_SIZE*sizeof(char), GFP_KERNEL);
	if(!(IMAGEDATA(priv->images, 0))) 
	{
		err = -ENOMEM;
	}
	
	IMAGEDATA(priv->images, 1) = (char *)kmalloc(W685BUF_SIZE*sizeof(char), GFP_KERNEL);
	if(!(IMAGEDATA(priv->images, 1))) 
	{
		err = -ENOMEM;
	}
	for(i = 0; i < IMGCONTAINERSIZE; i++)
		init_MUTEX( &(IMAGEHOLD(priv->images, i).ilock) );
	
#endif
	//vdev->priv = priv;

#ifndef VERSION825
	InitCFI(0);
	wait_ms(20);
	if(DownLoadFWFlag == 1) {
		ResetLCD();
		wait_ms(20);
	
		InitCFI(1);
		wait_ms(20);
		InitLCD();
		wait_ms(20);
	}
	else if(DownLoadFWFlag == 0)
	{
		
		W99685CMD_SetOpMode(OP_STREAMING, PPC_YUV422, 1);

		W99685CMD_SetLCMNums(LCD1_ON_685,LCD_BUS_16);
		//KDEBUG("Have set LCMNUMS");
		W99685CMD_SelectLCM(LCM_1ST);
		printk("Have selected LCM\n");
		W99685CMD_SetLCMRes(128, 161);
		printk("Have set LCM resolution\n");
	
	//	W99685CMD_SetBridgeMode(0);
		LCM_FillChar(lcm_buf, LEN_LCMBF);
		printk("Have Filled LCM with char\n");

		W99685CMD_SetLCMWndPos(0, 32);
	//	KDEBUG("Have set LCM Wdn Pos");
	//	
		W99685CMD_SetLCMWndSize(128, 96);
	}
//		W99685CMD_SetOpMode(OP_SINGLE, PPC_RGB565, 0);
	
#else
		W99685CMD_SetOpMode(OP_STREAMING, PPC_YUV422, 1);
#endif

	
#ifdef	W865THREAD 
	pid = kernel_thread(w99685_thread, priv,
		CLONE_FS | CLONE_FILES | CLONE_SIGNAL);
	if(pid < 0) {
		printk("Init W99685 Thread failed\n");
		goto out;
	}
#endif
	w99685_v4l1_open(tab, sub_tab, name);
	return err;
out:
#ifndef W865THREAD	
	free(priv->rawbuf);
#endif
out_nomem1:
	free(priv);
out_nomem0:
	return err;
}     


/* Check whether SD Card is inserted or not in System */
int CardDection()
{
	int had = 0;
#ifdef SDSUPPORT
	CmdParameters_t params;
	CmdReturns_t retvals;
	UINT16 cmd = SD_CMD;
	UINT16 subcmd = CMD_CARDINSERTED;
	
	memset(&params, 0, sizeof(CmdParameters_t));
	memset(&retvals, 0, sizeof(CmdReturns_t));
	
	INIT_PARAMS(&params, DUMMY, DUMMY, DUMMY, DUMMY);
	
	if((had = SendCommand2W685(cmd, subcmd, &params, &retvals, WAIT, NULL)) < 0) {
		goto out;
	}
	if(!RETVAL4(&retvals)) {//had it 
		printk("SD inserted\n");
		had = 1;
	}
	else if(RETVAL4(&retvals) == 1) {
		printk("SD not inserted\n");
		had = 0;
	}
	else
		had = -EIO;
#endif
out:	
	return had;
}

/* Map the CompactFlash interface to our system */
static void InitCFI(UINT8 dbus)
{
	UINT32	val;
	// Use EXTIO0 as CFI
	if(!dbus)//8-bit
		val=(EBIO0_BA<<1)|0x00007FFD; 
	else//16-bit
		val=(EBIO0_BA<<1)|0x00007FFE; 
	CSR_WRITE(EXT0CON,val);
	//KDEBUG("Map the CompactFlash interface to our system");
}

/*---------------------------------------- Start of LCD Functions -------------------------------*/
/* Reset LCD */
static int ResetLCD()
{
	int err = 0;
	UINT8 r2, r3;
	CmdParameters_t params;
	CmdReturns_t retvals;
	UINT16 cmd = GENERAL_CMD;
	UINT16 subcmd = 0x4E;
	
	memset(&params, 0, sizeof(CmdParameters_t));
	memset(&retvals, 0, sizeof(CmdReturns_t));
	
	INIT_PARAMS(&params, 0x01, 0x00, 0x00, 0x00);
	if( (err = SendCommand2W685(cmd, subcmd, &params, &retvals, WAIT, NULL)) < 0)
	{
		printk("Reset LCD erro: %d, line:%d\n", err, __LINE__);
		goto out;
	}
	r3 = RETVAL3(&retvals);
	r2 = RETVAL4(&retvals);
	r3 = r3&0xfd;
	
	cmd = GENERAL_CMD;
	subcmd = 0x4E;
	INIT_PARAMS(&params, 0x00, 0x00, r3, r2);
	if( (err = SendCommand2W685(cmd, subcmd, &params, NULL, ASYN, NULL)) < 0)
	{
		printk("Reset LCD erro: %d, line: %d\n", err, __LINE__);
		goto out;
	}
	
	r3 |= r3|0x02;
	cmd = GENERAL_CMD;
	subcmd = 0x4E;
	INIT_PARAMS(&params, 0x00, 0x00, r3, r2);
	if( (err = SendCommand2W685(cmd, subcmd, &params, NULL, ASYN, NULL)) < 0)
	{
		printk("Reset LCD erro: %d, line: %d\n", err, __LINE__);
		goto out;
	}
	wait_ms(20);
	
out:
	return err;
}

/* init LCD */
static int InitLCD()
{
	int err = 0;
	
	//Samsung LCM Powe on procedure Start
    	LCDPowerSettingFunction();	
	wait_ms(40);
    	LCDInitialFunctionOne();
    	LCDPowerSettingFunctionTwo();    
    	wait_ms(40);
    	LCDInitialFunctionTwo();   
    	wait_ms(20);
    	LCDDisplayOnFunction();  
	//Samsung LCM Powe on procedure End 
	wait_ms(20);
	return err;
}

/* Download firmware t2o Camera */
static int DownLoadFirmware2Camera(UINT8 *firmware, int fwlength)
{
	int err = 0;
#if 1
	int	i,len;
    	UINT8	*p = firmware;
	//KDEBUG("Now Download sensor data to camera");
//	printk("data: %x, %x, %x, %x\n", *p, *(p+1), *(p+2), *(p+3));
	// just delay for a while.
    	wait_ms(20);
 
    	len = fwlength/4 + ((fwlength%4)!=0);
//        printk("len: %d\n", len);
    	FREG7 = 0xaa;
    	FREG5 = 0x55;
    	FREG4 = 0x80;
    	FREG6 = 0xaa;
    	FREG7 = 0x55;
    	FREG6 = 0x00;
    	FREG5 = 0x00;
    	FREG4 = 0x00;
   
    	for(i=0; i<len; i++)
    	{
		FREG3 = *p++;
		FREG2 = *p++;
		FREG1 = *p++;
		FREG0 = *p++;
		FREG7 = 0x00;
    	}
 
	FREG6 = 0xff;
	FREG5 = 0xff;
	FREG4 = 0xff;
	FREG7 = 0x00;  // reboot
	
	//KDEBUG("Downloaded OK.\nNow rebooting...");
	wait_ms(20);
	wait_ms(20);
#endif
	return err;
}

/* Send Command to LCD */
static inline void LCDWriteCMDAddrWORD(UINT16 addr)
{
	FREG0_W = addr;
}

/* Write register in LCD */
static inline void LCDWriteCMDRegWORD(UINT16 data)
{
	FREG1_W = data;
}

/* LCD Power Setting */
static void LCDPowerSettingFunction(void)
{
	LCDWriteCMDAddrWORD(0x07);	
   	LCDWriteCMDRegWORD(0x0104);	    

	LCDWriteCMDAddrWORD(0x0d);
#ifdef SUMSUNG_TFTLCD_V33	
    	LCDWriteCMDRegWORD(0x0100);
#endif
#ifdef SUMSUNG_TFTLCD_V28	
    	LCDWriteCMDRegWORD(0x0202);
#endif

	LCDWriteCMDAddrWORD(0x0e);
#ifdef SUMSUNG_TFTLCD_V33	
    	LCDWriteCMDRegWORD(0x0d18);
#endif
#ifdef SUMSUNG_TFTLCD_V28	
    	LCDWriteCMDRegWORD(0x0e19);
#endif
}

static void LCDInitialFunctionOne(void)
{    
    	LCDWriteCMDAddrWORD(0x01);    
    	LCDWriteCMDRegWORD(0x0113);
	
	LCDWriteCMDAddrWORD(0x02);	
    	LCDWriteCMDRegWORD(0x0700);	        
	
	LCDWriteCMDAddrWORD(0x05);	
    	LCDWriteCMDRegWORD(0x1230);	        
	
	LCDWriteCMDAddrWORD(0x06);	
    	LCDWriteCMDRegWORD(0x0000);   
	
    	LCDWriteCMDAddrWORD(0x0b);	
    	LCDWriteCMDRegWORD(0x4008);
}

static void LCDPowerSettingFunctionTwo(void)
{
	LCDWriteCMDAddrWORD(0x0c);	
    	LCDWriteCMDRegWORD(0x0000);	

	LCDWriteCMDAddrWORD(0x03);
#ifdef SUMSUNG_TFTLCD_V33	
    	LCDWriteCMDRegWORD(0x0008);
#endif
#ifdef SUMSUNG_TFTLCD_V28	
    	LCDWriteCMDRegWORD(0x0008);
#endif   

	LCDWriteCMDAddrWORD(0x04);	
    	LCDWriteCMDRegWORD(0x0000);	
	wait_ms(80);
	
	LCDWriteCMDAddrWORD(0x0e);	
#ifdef SUMSUNG_TFTLCD_V33	
    	LCDWriteCMDRegWORD(0x2d18);
#endif
#ifdef SUMSUNG_TFTLCD_V28	
    	LCDWriteCMDRegWORD(0x2e19);
#endif
	wait_ms(20);
	LCDWriteCMDAddrWORD(0x0d);	
#ifdef SUMSUNG_TFTLCD_V33	
    	LCDWriteCMDRegWORD(0x0110);
#endif
#ifdef SUMSUNG_TFTLCD_V28	
    	LCDWriteCMDRegWORD(0x0212);
#endif  
}

static void LCDInitialFunctionTwo(void)
{
	LCDWriteCMDAddrWORD(0x21);	
    	LCDWriteCMDRegWORD(0x0000);	    

	LCDWriteCMDAddrWORD(0x30);	
    	LCDWriteCMDRegWORD(0x0700);	    

	LCDWriteCMDAddrWORD(0x31);	
    	LCDWriteCMDRegWORD(0x0007);		

	LCDWriteCMDAddrWORD(0x32);	
    	LCDWriteCMDRegWORD(0x0000);		

	LCDWriteCMDAddrWORD(0x33);	
    	LCDWriteCMDRegWORD(0x0100);	    	

	LCDWriteCMDAddrWORD(0x34);	
    	LCDWriteCMDRegWORD(0x0707);	    

	LCDWriteCMDAddrWORD(0x35);	
    	LCDWriteCMDRegWORD(0x0007);		

	LCDWriteCMDAddrWORD(0x36);	
    	LCDWriteCMDRegWORD(0x0700);		

	LCDWriteCMDAddrWORD(0x37);	
    	LCDWriteCMDRegWORD(0x0000);
    	//LCDWriteCMDRegWORD(0x0001);//dh

	LCDWriteCMDAddrWORD(0x0f);	
    	LCDWriteCMDRegWORD(0x0000);	    	

	LCDWriteCMDAddrWORD(0x11);	
    	LCDWriteCMDRegWORD(0x0000);	    	

	LCDWriteCMDAddrWORD(0x14);	
    	LCDWriteCMDRegWORD(0x5c00);	    

	LCDWriteCMDAddrWORD(0x15);	
    	LCDWriteCMDRegWORD(0xa05d);
    	//LCDWriteCMDRegWORD(0x9f5d);//dh

	LCDWriteCMDAddrWORD(0x16);	
    	LCDWriteCMDRegWORD(0x7f00);		

	LCDWriteCMDAddrWORD(0x17);	
    	LCDWriteCMDRegWORD(0xa000);
    	//LCDWriteCMDRegWORD(0x9f00);//dh

	LCDWriteCMDAddrWORD(0x3a);	
    	LCDWriteCMDRegWORD(0x1800);

	LCDWriteCMDAddrWORD(0x3b);	
    	LCDWriteCMDRegWORD(0x0007);    
}

static void LCDDisplayOnFunction(void)
{
	LCDWriteCMDAddrWORD(0x07);	
    	LCDWriteCMDRegWORD(0x0105);	
	wait_ms(80);	
	LCDWriteCMDAddrWORD(0x07);	
    	LCDWriteCMDRegWORD(0x0125);	    
	LCDWriteCMDAddrWORD(0x07);	
    	LCDWriteCMDRegWORD(0x0127);	
	wait_ms(80);	
	LCDWriteCMDAddrWORD(0x07);	
    	LCDWriteCMDRegWORD(0x0137);	    

    	LCDWriteCMDAddrWORD(0x21);    
    	LCDWriteCMDRegWORD(0x0000);		
    	LCDWriteCMDAddrWORD(0x22);    
}

static void LCM_FillChar(UINT8 *buf, int count)
{
	int i, j;
#if 0
	int divid = 4;
	int mul = 0;

	UINT16 color = 0xffff;
	for(i = 0; i < 160; i++) {
		for(j = 0; j < 128; j++) {
			if(j < (128/divid))
				mul = 0;
			else if( j < (124*2/divid))
				mul = 1;
			else if( j < (124*3/divid))
				mul = 2;
			else 
				mul = 3;
			lcm_buf[(i*128 + j)*2] = (UINT8)((0xffff*mul/divid)>>8);
			lcm_buf[(i*128 + j)*2 + 1] = (UINT8)((0xffff*mul/divid)&0xff);
		}
	}
#endif
	W99685CMD_WriteLCMWndBuff(buf, count);
}

static void test_LCM_FillChar(int color, char buf[])
{
	//W99685CMD_WriteLCMWndBuff(buf,LEN_LCMBF);
}

static int DrawInByPassMode(UINT8 *buf, UINT32 Length)
{
	int ret = 0;
	int i = 0;
	for(i = 0; i < Length; i+=2)
	{
		LCDWriteCMDRegWORD(buf[i]|((buf[i+1]<<8)&0xff00)); 
	}
	for(i = 0; i < 128; i++)
		LCDWriteCMDRegWORD(0x001f);//Append line
	return ret;
}

static void LCM_Zoom2()
{
	CmdParameters_t params;
	UINT16 cmd = GENERAL_CMD;
	UINT16 subcmd = CMD_SETZOOM;
	
	memset(&params, 0, sizeof(CmdParameters_t));
	
	SendCommand2W685(cmd, subcmd, &params, NULL, ASYN, NULL);
}

/*--------------------------------------- End of LCD Functions --------------------------------*/

#ifndef TRUE
#define TRUE                1
#endif
// Module entry point

static bool w90n740_video_init(struct cyg_devtab_entry *tab)
{		
	
	memset(&w99685priv, 0, sizeof(w685cf_t));
	w99685priv.write = (w685writefunc)&w99685_v4l1_write;
	w99685priv.read = (w685readfunc)&w99685_v4l1_read;
	w99685priv.ioctl = (w685ioctlfunc)&w99685_v4l1_ioctl;
	return TRUE;
}

// Module cleanup
static void w99685_mod_term(void)
{
#if 0
	w99685_v4l1_release(&w685dev);
	video_unregister_device(&w685dev);
#endif
}
