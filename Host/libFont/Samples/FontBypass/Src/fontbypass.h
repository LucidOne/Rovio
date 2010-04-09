#ifndef __FONT_BYPASS_H
#define __FONT_BYPASS_H

#include "stdio.h"
#include "stdlib.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#ifndef SOCKET_BRIDGE
#include "unistd.h"
#include "sys/ioctl.h"
#include "sys/socket.h"
#include "netinet/in.h"
#include "netdb.h"
#include "unistd.h"
#include "net/if.h"
#endif

#include "wb_syslib_addon.h"
//#include "wbtypes.h"
//#include "vp_command.h"
//#include "wb702if.h"
#include "font.h"
#include "bmp_io.h"
#include "lib_videophone.h"

#ifdef SOCKET_BRIDGE
#include "RemoteFunc.h"
#include "RemoteNet.h"
#endif

#define FONT_DEBUG
//#define FONT_STANDALONE

#define FONTFUNCBUFFER_SIZE 1024

#define LCM1_WIDTH	320
#define LCM1_HEIGHT	240
#define LCM2_WIDTH	96
#define LCM2_HEIGHT	96

#define SIGNATURE_WORD  0xA0FFFF9F
#define FLASH_BASE      (0x7F000000)
#define BOOTER_BLOCK_LENGTH		(0x00010000)
#define MAX_FOOTER_NUM  8


#define VENDOR_DEFAULT "WINBOND"
#define PRODUCT_DEFAULT "IP  CAMERA"
#define VERSION_DEFAULT "V1.0"
#define IPADDRESS_DEFAULT "127.0.0.1"

#define FRAME_SIZE 3
#define LINE_SPACE 2

#define POSX_DEFAULT FRAME_SIZE
#define POSY_DEFAULT 25

#define FONT_COLOR_DEFAULT 0xF800
#define FONT_COLOR_MAX 0xFFFF
#define FONT_COLOR_MIX 0x0000

#define FONT_CONF_PATH "/etc/font.conf"
#define BMP_BACKGROUND_PATH "/etc/bg.bmp"

typedef struct t_footer
{
        UINT32 num;
        UINT32 base;
        UINT32 length;
        UINT32 load_address;
        UINT32 exec_address;
        char name[16];
        UINT32 image_checksum;
        UINT32 signature;
        UINT32 type;
        UINT32 checksum;
} tfooter;

typedef struct _t_bootloaer_info
{
    UINT32 length;
    UINT32 type;
    char mac0[6];
    char ip0[6];
    char mac1[6];
    char ip1[6];
    UINT32 cache;
    UINT32 dhcp;
    UINT32 net_mac;
    UINT32 phy;
    UINT32 buf_base;
    UINT32 buf_size;
} tbl_info;

typedef enum FONT_STYLE_ALIGN
{
    FONT_STYLE_ALIGN_LEFT=0,
    FONT_STYLE_ALIGN_CENTER,
    FONT_STYLE_ALIGN_RIGHT
}FONT_STYLE_ALIGN;

typedef enum FONT_STYLE_BOLD
{
    FONT_STYLE_BOLD_NONE=0,
    FONT_STYLE_BOLD_ALL
}FONT_STYLE_BOLD;

typedef enum FONT_CODE
{
    FONT_CODE_ENG = 0,
    FONT_CODE_GBK,
    FONT_CODE_BIG5,
    FONT_CODE_BIG5GBK,
    FONT_CODE_UNICODEGBK,
    FONT_CODE_NULL
}FONT_CODE;

typedef enum SCREEN_WHICH
{
    SCREEN_WHICH_BIG = 0,
    SCREEN_WHICH_SML,
    SCREEN_WHICH_NULL
}SCREEN_WHICH;

typedef struct FONT_COLOR_CHANGE
{
    int iFound;
    int iLen1, iLen2;
    int iStep1, iStep2;
    unsigned short usCstep1, usCstep2;
}FONT_COLOR_CHANGE;

#define FONT_CONTENT_SIZE 10
typedef struct FONT_ATTR
{
    FONT_RECT_T         rcOrg;
    FONT_RECT_T         rcThis;
    FONT_STYLE_ALIGN    fStyleAlign;
    FONT_STYLE_BOLD     fStyleBold;
    
    FONT_CODE           fCode;
    
    unsigned short   sFontColor;
    unsigned short   sFontColore;
    int               iFontLeft;
    FONT_COLOR_CHANGE  fcChange[3]; //R, G, B
    
    BOOL    bBackground;
    BOOL    bVersion;
    BOOL    bIpaddress;
    BOOL    bFontShadow;
    
    int		iScreen;
    int		iScreenWidth;
    int		iScreenHeight;
    BOOL	bReversal;
    
    char 	*pcContent[FONT_CONTENT_SIZE];
}FONT_ATTR;

typedef struct
{
	BMP2RAW_SRC_T	src;
	char			*pcSrcBuffer;
	int				nSrcLen;
	int				nSrcLenOK;
} MyBMP2RAW_SRC_T;

typedef struct
{
	BMP2RAW_DES_T	des;
	char			*pcDesBuffer;
	int				nDesLen;
	int				nDesLenOK;
} MyBMP2RAW_DES_T;


extern FONT_ATTR font_attr;
extern UCHAR buf_video[320*240*2];


void drawLCDRGB(UCHAR *buf, int size);
void drawLCDYP(UCHAR *buf, int size);
int wb702Init(int fd);

int fontAttrInit(FONT_ATTR *pfar);
int fontFontSet(FONT_DEVICE_T *pDevice, FONT_ATTR *pfar);
int fontGetConf(FONT_ATTR *pfar, char *pConf);
int fontGetVersion(int iFd, char *pbuf);
int fontGetRC(FONT_ATTR *pfar, FONT_RECT_T *prc, FONT_SIZE_T *psz);
int fontRecoverRC(FONT_ATTR *pfar);
int fontPrintContent(FONT_DEVICE_T *pDevice, FONT_ATTR *pFontAttr, char *pConf);
void fontDrawPixelBypass(void *pDevice, EF_BOOL bIsDraw, EF_INT nPosX, EF_INT nPosY);

unsigned long GetGeneralIP(const char *pcInterface, int iRequest);
unsigned long GetIPAddress(const char *pcInterface);

int SetFontColorCh(FONT_ATTR *pFontAttr, FONT_RECT_T *prc, FONT_SIZE_T *psz);
int SetFontColorChange(FONT_ATTR *pFontAttr, unsigned short sFontColor, unsigned short sFontColore, int nWidth);
int GetFontNewColor(FONT_ATTR *pFontAttr, int nPosX, USHORT *pusColor);

int MyRead (BMP2RAW_SRC_T *pSrc, void *pBuf, int nLen);
int MyWrite (BMP2RAW_DES_T *pDes, unsigned char ucR, unsigned char ucG, unsigned char ucB);
int bmpconv(char *pf, unsigned char *pBuf, int iWidth, int iHeight);

#endif