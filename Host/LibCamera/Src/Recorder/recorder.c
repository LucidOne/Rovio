#ifdef RECORDER
#include "stdio.h"
#include "stdlib.h"
#include "lib_videophone.h"
#include "ftp.h"
#include "mail.h"
#include "asfRecorder.h"
#include "lib3gp_test.h"
#include "recorder.h"
#include "../../WBFAT/ecos/Inc/wbfat.h"
#include "wb_fmi.h"

#define MSG_QUIT 6L
#define MEDIA_BUF_SIZE 0x60000
#define META_BUF_SIZE 0x18000
#define OUT_BUF_SIZE (MEDIA_BUF_SIZE+META_BUF_SIZE)

#define STACK 32*1024
cyg_handle_t ftp_handle,  mail_handle;
cyg_thread ftp_thread, mail_thread;
static cyg_thread_info mail_info;
#ifndef USE_SD
static cyg_thread_info ftp_info;
#endif
unsigned char ftp_stack[STACK];
unsigned char mail_stack[STACK];
Ftp_Info* g_ftpinfo;
Mail_Info* g_mailinfo;

typedef enum 
{
    VIDEO_H263 = 0,
    VIDEO_MP4 = 1
}VIDEO_TYPE;

typedef enum
{
    AUDIO_AMR     = 0,
    AUDIO_PCM     = 1,
    AUDIO_IMA_ADPCM  = 2
} AUDIO_TYPE;
	
//#define EXITED 16         

static VIDEO_TYPE videoformat;
static AUDIO_TYPE audioformat;
static int bRecorder = 0;
static int bEnd = 0;
static int bInit = 0;
static int bStop = 0;
#ifndef USE_SD
static char* recorderbuf = NULL;
static char* mediabuf = NULL;
static char* metabuf = NULL;
#endif
static _ASF_RECORD_T* asf = NULL;
static THREEGP_t* threegp = NULL;

#ifdef USE_SD
#define REC_TIME (100*30)
#endif				
extern void thread_join(cyg_handle_t* handle, cyg_thread* thread, cyg_thread_info* info);
void recorder_releative_thread_create(Ftp_Info* ftpinfo, Mail_Info* mailinfo)
{
    g_ftpinfo = ftpinfo;
    g_mailinfo = mailinfo;
#ifndef USE_SD    
    cyg_thread_create(PTD_PRIORITY, (cyg_thread_entry_t*)&FtpStart, (cyg_addrword_t)g_ftpinfo, "ftp_start", ftp_stack,
                        STACK, &ftp_handle, &ftp_thread);
    if ( ftp_handle == NULL)
    {
        printf("Thread for ftp creation failed!\n");
        exit(-1);
    }
    
    cyg_thread_resume(ftp_handle);
#endif    
	cyg_thread_create(PTD_PRIORITY, &MailStart, (cyg_addrword_t)g_mailinfo, "mail_start", mail_stack,
                        STACK, &mail_handle, &mail_thread);
	if ( mail_handle == NULL)
	{
		printf("Thread for mail creation failed!\n");
		exit(-1);
	}
    
    cyg_thread_resume(mail_handle);
}

void recorder_releative_thread_release()
{
    MSG_T msg;
    cyg_handle_t pMsgFtp;
    cyg_handle_t pMsgMail;

    msg.lMsg = MSG_QUIT;
    if((pMsgFtp = Get_MsgFtp()) != NULL) 
    	cyg_mbox_put(pMsgFtp, &msg);
    if((pMsgMail = Get_MsgMail()) != NULL) 
    	cyg_mbox_put(pMsgMail, &msg);
#ifndef USE_SD        
    if(ftp_handle != NULL)
        thread_join(&ftp_handle, &ftp_thread, &ftp_info);
#endif        
    if(mail_handle != NULL)
        thread_join(&mail_handle, &mail_thread, &mail_info);
    if(g_mailinfo)
  	  free(g_mailinfo);
    if(g_ftpinfo)
  	  free(g_ftpinfo);
}

static int asf_init_buf(void)
{
    if((asf = (_ASF_RECORD_T*)malloc(sizeof(_ASF_RECORD_T))) == NULL)
    {
    	printf("asf struct malloc false\n");
    	return -1;
    }
    memset(asf, 0, sizeof(_ASF_RECORD_T));
#ifndef USE_SD    
    if((recorderbuf = (char *)malloc(OUT_BUF_SIZE)) == NULL)
    {
    	printf("asf recorderbuf malloc false \n");
    	return -1;
    }	
    memset(recorderbuf, 0, OUT_BUF_SIZE);
#endif    
    if((asf->pRec_Buffer_Data = (Rec_Buffer_Stream*)malloc(sizeof(Rec_Buffer_Stream))) == NULL)
    {
    	printf("asf pRec_Buffer_Data malloc false\n");
    	return -1;
    }	
    memset(asf->pRec_Buffer_Data, 0, sizeof(Rec_Buffer_Stream));
    return 0;
}

static int threegp_init_buf(void)
{

    if((threegp = (THREEGP_t*)malloc(sizeof(THREEGP_t))) == NULL)
    {
    	printf("threegp struct malloc false\n");
    	return -1;
    }	
    memset(threegp, 0, sizeof(THREEGP_t));
#ifndef USE_SD    
    if((recorderbuf = (char *)malloc(OUT_BUF_SIZE)) ==NULL)
    {
    	printf("recorderbuf malloc false\n");
    	return -1;
    }	
    memset(recorderbuf, 0, OUT_BUF_SIZE);
    if((mediabuf = (char *)malloc(MEDIA_BUF_SIZE)) == NULL)
    {
    	printf("mediabuf malloc false\n");
    	return -1;
    }	
    memset(mediabuf, 0, MEDIA_BUF_SIZE);
    if((metabuf = (char *)malloc(META_BUF_SIZE)) == NULL)
    {
    	printf("metabuf malloc false\n");
    	return -1;
    }	
    memset(metabuf, 0, META_BUF_SIZE);
#endif    
    return 0;
}

int recoreder_init_buf(int videotype, int audiotype)
{	
	int rt=-1;
    videoformat = videotype;
    audioformat = audiotype;
	
	if(videoformat == VIDEO_H263)
		rt = threegp_init_buf();
	if(videoformat == VIDEO_MP4)
		rt = asf_init_buf();
	
	return rt;
}
	
void recorder_free(void)
{
    bInit = 0;
    bRecorder = 0;
   	bEnd = 0;
    if(videoformat == VIDEO_H263)
    {
#ifndef USE_SD    	
    	if(recorderbuf != NULL)
        	free(recorderbuf);
    	if(mediabuf != NULL)
        	free(mediabuf);
    	if(metabuf != NULL)
        	free(metabuf);
#endif        	
    	if(threegp != NULL)
   			free(threegp);
   			
    }
    
    if(videoformat == VIDEO_MP4)
    {
#ifndef USE_SD    
    	if(recorderbuf != NULL)
        	free(recorderbuf);
#endif        	
    	if(asf->pRec_Buffer_Data != NULL)
        	free(asf->pRec_Buffer_Data);
    	if(asf != NULL)
    		free(asf);
    }		
   	
}

static VOID  tgpMediaRecordCallback(THREEGP_t *ptgp)
{
	if(bStop == 0)
	{	
#ifndef USE_SD
    	{
    		UINT32 iFreeSpace,uOutMediaSize;

    		tgpGetRecorderSize(ptgp,&uOutMediaSize);
    		iFreeSpace = ptgp->iOutBufSize -uOutMediaSize;
    		if(iFreeSpace  < (100*1024)) 
        		ptgp->data_rec_action = REC_CTRL_STOP;
    	}
#else
		if(cyg_current_time() - ptgp->tgpStartTime > REC_TIME)
			ptgp->data_rec_action = REC_CTRL_STOP;
#endif
	}
	else
		ptgp->data_rec_action = REC_CTRL_STOP;		    	
}

static int init_threegpstruct(THREEGP_t *ptgp, int tgp_width, int tgp_height)
{
    if(!ptgp)
        return FALSE;
    
    ptgp->sVidRecHeight = tgp_height;
	ptgp->sVidRecWidth  = tgp_width;
#ifndef USE_SD    
    ptgp->pucOutBuf = (UCHAR *)recorderbuf;
    ptgp->iOutBufSize = OUT_BUF_SIZE;
    
    ptgp->ucOutMediaBuf =  (UINT32*)mediabuf;
	ptgp->ucOutMetaBuf =   (UINT32*)metabuf;
	ptgp->uOutMediaBufSize = MEDIA_BUF_SIZE;
	ptgp->uOutMetaBufSize = META_BUF_SIZE;
#else
	{
		static int i = 0;
		char metafile[64];
		char mediafile[64];
		
		sprintf(metafile, "D:\\rec_%d.3gp", i);
		sprintf(mediafile, "D:\\rec_%d.mda", i++);
		fsAsciiToUnicode(metafile, ptgp->OutMetaFile, TRUE);
		fsAsciiToUnicode(mediafile, ptgp->OutMediaFile, TRUE);
	}
#endif			
    
    ptgp->tgpGetAudioFrame = NULL;
	ptgp->tgpGetVideoFrame = NULL;
    
    ptgp->ap_time = tgpMediaRecordCallback;
    
    return TRUE;
}

static int threegp_init(THREEGP_t *ptgp, int tgp_width, int tgp_height)
{
    init_threegpstruct(ptgp, tgp_width, tgp_height);
    if(tgpInitMovieConfig(ptgp) < 0)
        return FALSE;
    tgpInitVideoAudioFormat(ptgp);
    
    return TRUE;
}

static void asf_init(_ASF_RECORD_T *asf)
{
#ifndef USE_SD    
    asfBufferStream_Init(&asf->asf_stream, recorderbuf, OUT_BUF_SIZE);
#else
	{
		static int i = 0;
		char asffile[64];
		sprintf(asffile, "D:\\rec_%d.asf", i++);
		asfFileStream_Init(&asf->asf_stream, asffile);
	}
#endif		    
    asfRecorderInit(asf);
}

BOOL recorder_add_video(char* videobuf, int videolen)
{
    if(bRecorder)
    {
        if(videoformat == VIDEO_MP4)
            asfRecorderAddVideo (asf, videobuf, videolen);
        if(videoformat == VIDEO_H263)
            tgpRecorderVideoTrack(threegp, (UCHAR*)videobuf, videolen);
        return TRUE;    
    }
    else
    	return FALSE;
}

void recorder_add_audio(char* audiobuf, int audiolen)
{
    if(bRecorder)
    {
        if(audioformat == AUDIO_IMA_ADPCM)
            asfRecorderAddAudio (asf, audiobuf, audiolen);
        if(audioformat == AUDIO_AMR)
            tgpRecorderAudioTrack(threegp, (UCHAR*)audiobuf, audiolen);
    }
}

void recorder_memset()
{
	bInit = 0;
    bRecorder = 0;
    bEnd = 0; 	
	if(videoformat == VIDEO_MP4)
	{
		Rec_Buffer_Stream* tmp;
		tmp = asf->pRec_Buffer_Data;
		if(asf->pRec_Buffer_Data)
			memset(asf->pRec_Buffer_Data, 0, sizeof(Rec_Buffer_Stream));
#ifndef USE_SD			
		if(recorderbuf)
			memset(recorderbuf, 0, OUT_BUF_SIZE);
#endif			
		if(asf)
			memset(asf, 0, sizeof(_ASF_RECORD_T));
		asf->pRec_Buffer_Data = tmp;
	}
	else
	{	
		if(threegp)
			memset(threegp, 0, sizeof(THREEGP_t));
#ifndef USE_SD			
		if(recorderbuf)
    		memset(recorderbuf, 0, OUT_BUF_SIZE);
    	if(mediabuf)
    		memset(mediabuf, 0, MEDIA_BUF_SIZE);
    	if(metabuf)
    		memset(metabuf, 0, META_BUF_SIZE);
#endif    		
    }		   		
}					

int recorder_run(int* bheader, int video_width, int video_height)
{
    int iFileSize;
	static cyg_tick_count_t tStartTime;

    g_ftpinfo->type = g_mailinfo->type = videoformat;
    if(videoformat == VIDEO_MP4)
    {
        if(!bInit)
        {
            asf_init(asf);
            bRecorder = 1;
            *bheader = 1;
            tStartTime = cyg_current_time();
            bInit = 1;
            wb702OutMP4IFrame(2);
        }
  
        if(bRecorder)
        {
#ifndef USE_SD        
            if(cyg_current_time() - tStartTime >= 200)
            {
                iFileSize = asfRecorderEnd(asf);
                bEnd = 1;
            }
#else
			if(cyg_current_time() - tStartTime > REC_TIME)
			{
                iFileSize = asfRecorderEnd(asf);
                bEnd = 1;
            } 
#endif                       
        }
    }
    else if(videoformat == VIDEO_H263)
    {
        if(!bInit)
        {
            threegp_init(threegp, video_width, video_height);
            bRecorder = 1;
            bInit = 1;
            threegp->tgpStartTime = cyg_current_time();
            wb702OutMP4IFrame(2);
        }
        
        if(bRecorder)
        {
            if(tgpIsRecorderEnd(threegp) == REC_CTRL_STOP)
            {
                iFileSize = tgpMergeTGP(threegp);
                bEnd = 1;
            }
        }
    }
    else 
        return FALSE;
        
    if(bEnd)
    {
#ifndef USE_SD         
        if(*g_mailinfo->MailEnable)
            DO_TestSendMailFile(recorderbuf, iFileSize, g_mailinfo);
        if(*g_ftpinfo->FtpEnable)
            Do_TestFtpUpload(recorderbuf, iFileSize);
#endif            
        recorder_memset();
    }
    
    return TRUE;
}
   
void recorder_dis()
{
	int iFileSize;
	
	if(bInit)
	{
		 if(videoformat == VIDEO_MP4)
		 	iFileSize = asfRecorderEnd(asf);
		 if(videoformat == VIDEO_H263)
		 {
		 	bStop = 1;
		 	if(tgpIsRecorderEnd(threegp) == REC_CTRL_STOP)
                iFileSize = tgpMergeTGP(threegp);
            tgpMergeTGP(threegp);
         }       
#ifndef USE_SD         
        if(*g_mailinfo->MailEnable)
            DO_TestSendMailFile(recorderbuf, iFileSize, g_mailinfo);
        if(*g_ftpinfo->FtpEnable)
            Do_TestFtpUpload(recorderbuf, iFileSize);
#endif
		recorder_memset();  	
	}		
}        
#endif

    
    
        
        

