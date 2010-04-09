#ifndef __THREEGP_H_
#define  __THREEGP_H_

#include "wbtypes.h"
#include "MediaFileLib.h"
#include "cyg/kernel/kapi.h"

typedef struct _ThrGP
{
	UCHAR *pucInBuf;    		//当通过下面的函数指针读取数据的时候，用它指定读取的空间
	int   iInBufSize; 			//给定读取空间的大小(iInBufSize >= MAX video packet size )

	CHAR OutMediaFile[128];
	CHAR OutMetaFile[128];
	
	UCHAR *pucOutBuf;				//指定刻录的输出memory
	int   iOutBufSize;			//输出memory 的大小（iOutBufSize >=uOutMediaBufSize + uOutMetaBufSize）

	UINT32 *ucOutMediaBuf;	//刻录数据(audio and video)输出的空间
	UINT32	uOutMediaBufSize;	//刻录数据(audio and video)输出的空间的大小

	UINT32 *ucOutMetaBuf;			//刻录数据属性输出的空间
	UINT32	uOutMetaBufSize;	//刻录数据属性输出的空间的大小

	//UINT32  uiOut3GPSize;			//输出刻录数据的总大小（uiOut3GPSize <=iOutBufSize）
	unsigned short sVidRecWidth;	//video 的宽度
	unsigned short sVidRecHeight; //video 的高度

//通过提供函数指针读取 video and audio 
	INT  (*tgpGetVideoFrame)(struct _ThrGP *ptgp,UINT8 **pucFrameBuff, UINT32 *puFrameSize);
	INT  (*tgpGetAudioFrame)(struct _ThrGP *ptgp,UINT8 **pucFrameBuff, UINT32 *puFrameSize);
	//提供返回条件
	void (*ap_time)(struct _ThrGP *ptgp);
  	UINT32 data_rec_action;       //刻录标志 
 
	int     device_fd;   //open device ID
	void* pMvCfg;				//保留
	
	cyg_tick_count_t tgpStartTime;
}THREEGP_t;

int tgpRecorderVideoTrack(THREEGP_t *ptgp,UINT8 *pucInBuf,UINT32 uiSize);
int tgpRecorderAudioTrack(THREEGP_t *ptgp,UINT8 *pucInBuf,UINT32 uiSize);
int tgpIsRecorderEnd(THREEGP_t *ptgp);
int tgpInitVideoAudioFormat(THREEGP_t *ptgp);
int  tgpInitMovieConfig(THREEGP_t *ptgp);
unsigned int tgpMergeTGP(THREEGP_t *ptgp);
void tgpGetRecorderSize(THREEGP_t *ptgp,UINT32 *uOutMediaSize);
void tgpRecorderEnd(THREEGP_t *ptgp);

#endif

