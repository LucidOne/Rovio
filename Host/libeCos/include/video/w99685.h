/*
	Winbond w99683 Webcam CompactFlash driver for uClinux 2.4.x.

	Copyright (C) 2003 PC32 YPXin <ypxin@winbond.com.tw>

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
#ifndef __W99685_H__
#define __W99685_H__
#include "pkgconf/system.h"
#include "pkgconf/io.h"
#include "pkgconf/io_serial.h"
#include "cyg/io/io.h"
#include "cyg/hal/hal_intr.h"
#include "cyg/io/devtab.h"
#include "cyg/io/serial.h"
#include "cyg/infra/diag.h"
#include "cyg/kernel/kapi.h"

#include "linux/types.h"
#include "linux/wait.h"
//#include "linux/delay.h"
//#include "linux/completion.h"
//#include "linux/videodev.h"
#include "linux/list.h"
#include "linux/slab.h"
#include "asm/semaphore.h"
#include "asm/uaccess.h"
#include "sys/ioctl.h"
//#include "asm/io.h"
//#include "asm/hardirq.h"
#include "asm/errno.h"

#include "video/w99685cmds.h"
#include "video/w99685USBBridgecmds.h"

#define HZ	100
#define	printk	diag_printf
/* some type defines */
#define UINT16 	unsigned short
#define INT16 	short
#define UINT8 	unsigned char
#define UINT32 	unsigned int

#define USBBRIDGE
//#define __OUTPUT2TV__
#ifdef USBBRIDGE
//#include "kernelfop.h"
#include "video/W99685disk.h"

#define FIRMWARENUM		4
#define W685_INFO(fmt, arg...)	diag_printf(fmt "\n" , ## arg)
#define W685_DBG(fmt, arg...)	diag_printf("%s: " fmt "\n" , __func__ , ## arg)
#define W685_ERR(fmt, arg...)	diag_printf("%s: " fmt "\n" , __func__ , ## arg)


//#define VPint	*(volatile unsigned int *)
//#define VPshort	*(volatile unsigned short *)
//#define VPchar	*(volatile unsigned char *)

//#define CSR_WRITE(addr,data)	(VPint(addr) = (data))
//#define CSR_READ(addr)	(VPint(addr))


#define BIGBUFFERSIZE		16
#define EXCHANGEBUF_SIZE	1024*1

#define USBVENDORGETINDEX	0x04
#define USBVENDORSETINDEX	0x05

#define GET_DISK_CAPACITY	0xe1
#define GET_DISK_REMAIN		0xe2
#define GET_DISK_FILESNUM	0xe3
#define GET_DISK_FILENAME	0xe4
#define GET_DISK_FILELEN	0xe5
#define GET_DISK_CURRENTFILELEN	0xe6

#define SET_DISK_OPENFILE	0xd1
#define SET_DISK_CLOSEFILE	0xd2
#define SET_DISK_RMFILE		0xd3
#define GET_DISK_VERSION	0x10


#define STATUS_START		0x00
#define STATUS_READY		0x01
#define STATUS_CMDIN		0x02
#define STATUS_DATATRANSFER	0x03

typedef struct USBVendorCmd {
	UINT8 ReqType;
	UINT8 Request;
	UINT16 Value;
	UINT16 Index;
	UINT16 Length;
} USBVendorCmd_t;

#endif


//#define W865THREAD
//#define VERSION825
#define VERSION_MORE_FUNC
#define DEVICECLALSS	"W99685"



#define KDEBUG(format, arg...) //printk("%s->%d:" format "\n", __FUNCTION__, __LINE__, ## arg);
#define TDEBUG(format, arg...) //printk("%s->%d:" format "\n", __FUNCTION__, __LINE__, ## arg);

#define LBYTE(u16)		((UINT8)(u16&0xff))
#define HBYTE(u16)		((UINT8)(u16>>8))
#define LWORD(u32)		((UINT16)(u32&0xffff))
#define HWORD(u32)		((UINT16)(u32>>16))
#define ABYTE(u32)		(LBYTE(LWORD(u32)))
#define BBYTE(u32)		(HBYTE(LWORD(u32)))
#define CBYTE(u32)		(LBYTE(HWORD(u32)))
#define DBYTE(u32)		(HBYTE(HWORD(u32)))

#define MKHWORD(u16,a,b)		u16=((UINT16)a|((UINT16)b<<8))
#define MKWORDH(u32,a,b)		u32=((UINT32)a|((UINT32)b<<16))
#define MKWORDB(u32,a,b,c,d)	u32=((UINT32)a|((UINT32)b<<8)|((UINT32)c<<16)|((UINT32)d<<24))


typedef struct cmdparameters CmdParameters_t;
typedef struct cmdreturns CmdReturns_t;
typedef struct cmdreq CmdReq_t;
typedef struct W99685CF w685cf_t;


/* Functions phototypes */
static int w99685_v4l1_open(struct cyg_devtab_entry **tab, 
                  struct cyg_devtab_entry *sub_tab,
                  const char *name);
//void w99685_v4l1_close(struct video_device *);
long w99685_v4l1_read(w685cf_t *priv, char * buf, cyg_uint32 *count);
long w99685_v4l1_write(w685cf_t *priv, const char * buf, cyg_uint32 *count);
int w99685_v4l1_ioctl(w685cf_t *priv, unsigned int cmd, void * buf);
//int w99685_v411_init(struct video_device *); 
static bool w90n740_video_init(struct cyg_devtab_entry *tab);
static int w90n740_video_lookup(struct cyg_devtab_entry **tab, 
                  struct cyg_devtab_entry *sub_tab,
                  const char *name);



#define WAIT	1
#define ASYN	0
#define GETDATE	0x0000
#define PUTDATE 0x0100
#define CMDMASK 0x00ff

#define ISPUTDATE(x) ((x&0xff00) & 0x0100)	

/* System base address */
#define BASE_ADDR		0xFFF00000
#define EXT0CON			(BASE_ADDR+0x01018)
/* CompactFlash Device related defines */
#define EBIO0_BA		0xD0000000
#define BN				8	//????
/* now support data bus is 16 bits */
/* Base address */
#define XBYTE			((UINT8 volatile *) EBIO0_BA)
#define XHWORD			((UINT16 volatile *) EBIO0_BA)

#define FREG0_W			XHWORD[0]
#define FREG1_W			XHWORD[1]

#if 1
#define FREG0			XBYTE[BN+0]
#define FREG1			XBYTE[BN+1]
#define FREG2			XBYTE[BN+2]
#define FREG3			XBYTE[BN+3]
#define FREG4			XBYTE[BN+4]
#define FREG5			XBYTE[BN+5]
#define FREG6			XBYTE[BN+6]
#define FREG7			XBYTE[BN+7]

#else
#define FREG0			XHWORD[BN+0]
#define FREG1			XHWORD[BN+1]
#define FREG2			XHWORD[BN+2]
#define FREG3			XHWORD[BN+3]
#define FREG4			XHWORD[BN+4]
#define FREG5			XHWORD[BN+5]
#define FREG6			XHWORD[BN+6]
#define FREG7			XHWORD[BN+7]
#endif

#define DATATIMEOUT		300
#define DATABLOCKSIZE		512

#define W685BUF_SIZE 		1024*150

struct cmdparameters {
	UINT8 param1;
	UINT8 param2;
	UINT8 param3;
	UINT8 param4;
};

struct cmdreturns {
	UINT8 retval1;
	UINT8 retval2;
	UINT8 retval3;
	UINT8 retval4;
};

struct cmdreq {
	UINT16 subcmd;
	CmdParameters_t *cmd_params;
	CmdReturns_t *cmd_rets;
	UINT32 datalen;
	UINT8 *data;
	UINT8 flag;
};

typedef struct firmware {
	UINT32 Length;
	char *Data;
}Firmware_t;

#ifdef W865THREAD
#define IMGCONTAINERSIZE 2
typedef struct ImageData {
	char *rawbuf;
	int len;
	struct semaphore ilock;
} ImageData_t;

typedef struct ImageContainer {
	UINT8 which;	//write data pos
	UINT8 reading;	//reading pos, don't touch the data
	ImageData_t hold[IMGCONTAINERSIZE];
}ImageContainer_t;

#define PRIV_HOLDER(priv)	((priv)->images)
#define IMAGEHOLD(container, which) ((container)->hold[which])
#define IMAGELEN(container, which) 	(IMAGEHOLD(container, which).len)
#define IMAGEDATA(container, which) (IMAGEHOLD(container, which).rawbuf)
#endif

typedef long (*w685write_t)(w685cf_t *priv, const char * buf, cyg_uint32  * count);
typedef w685write_t w685writefunc;
    // Fetch one character from the device
typedef long (*w685read_t)(w685cf_t *priv, char * buf, cyg_uint32  * count);
typedef w685read_t w685readfunc;
    // Change hardware configuration (baud rate, etc)
typedef int (*w685ioctl_t)(w685cf_t *priv, unsigned int cmd, void * buf);
typedef w685ioctl_t w685ioctlfunc;


/* Use as videodevice's private data */
struct W99685CF {
	int user;
	
	CmdReq_t m_req;
	CmdParameters_t m_cmdparams;
	CmdReturns_t m_cmdrets;
#ifdef W865THREAD
	ImageContainer_t *images;
	wait_queue_head_t waitq;
#else
	char *rawbuf;	//??
#endif

#ifdef USBBRIDGE
	struct list_head event_list;	//USB Bridge event list
	struct semaphore kusbbridged_sem;
	int status;
	char *exchangebuf;
	struct semaphore exchangebuf_sem;
	int exchangeblocks;
	USBVendorCmd_t current_vendorcmd;
	//int exchangelen;
#endif
	
	wait_queue_head_t wq;	/* Processes waiting */
	struct semaphore  lock;
	   // Send one character to the output device, return true if consumed
    long (*write)(w685cf_t *priv, const char * buf, cyg_uint32 * count);
    // Fetch one character from the device
    long (*read)(w685cf_t *priv, char * buf, cyg_uint32 * count);
    // Change hardware configuration (baud rate, etc)
    int (*ioctl)(w685cf_t *priv, unsigned int cmd, void * buf);
};

#define ISBUSY(reg) ((reg)&((UINT16)0x80))
#define ISNOTRANSFER(reg) (((reg)&((UINT16)0x88)) != 0x08)
#define ISNOTRANSFER1THBLCK(reg)	(((reg)&((UINT16)0x88)) != 0x00)
#define INIT_PARAMS(cmdparamp, p1, p2, p3, p4) \
do {\
	(cmdparamp)->param1=p1;\
	(cmdparamp)->param2=p2;\
	(cmdparamp)->param3=p3;\
	(cmdparamp)->param4=p4;\
    } while (0)

#define PARAM1(cmdparamp)	((cmdparamp)->param1) 
#define PARAM2(cmdparamp)	((cmdparamp)->param2)
#define PARAM3(cmdparamp)	((cmdparamp)->param3)
#define PARAM4(cmdparamp)	((cmdparamp)->param4)  
 
#define RETVAL1(retvalsp)	((retvalsp)->retval1)
#define RETVAL2(retvalsp)	((retvalsp)->retval2)
#define RETVAL3(retvalsp)	((retvalsp)->retval3)
#define RETVAL4(retvalsp)	((retvalsp)->retval4)

#define IOCTLGETCLASS		_IOR('v', 3, UINT32)			/* Get Device Class */
#define IOCTLSETFW_CAMERA	_IOR('v', 4, UINT32)			/* Set Device Camera Firmware */
#define IOCTLSETFW_USBBRIDGE	_IOR('v', 5, UINT32)			/* Set Device USBBridge Firmware */
#define IOCTLDRAWINBYPASS	_IOR('v', 6, UINT32)			/* Draw image in bypass mode */
#define IOCTLGETSTATUS		_IOR('v', 7, UINT32)			/* Get W99685 status */

#define BLOCKSIZE	512   
 
void CommandDump(CmdReq_t *reqp, CmdParameters_t * paramp);
/* Support functions */
int SendCommand2W685(UINT16 cmd, UINT16 subcmd, CmdParameters_t *cmdparams, CmdReturns_t *cmdrets, UINT8 wait, UINT8 *buffer);
int CardDection();
static void InitCFI(UINT8 dbus);
static int DownLoadFirmware2Camera();

/* About LCD */
#define SUMSUNG_TFTLCD_V28



/* CFI Control Command functions */
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

/* waits for the specified miliseconds */
static inline void wait_ms(unsigned int ms) {
#if 0
	if (!in_interrupt()) {
		set_current_state(TASK_UNINTERRUPTIBLE);
		schedule_timeout(1 + ms * HZ / 1000);
	}
	else
		mdelay(ms);
#else

	cyg_thread_delay(ms * HZ / 1000);

#endif
}

extern cyg_devio_table_t cyg_io_w99685_ops;

#endif
