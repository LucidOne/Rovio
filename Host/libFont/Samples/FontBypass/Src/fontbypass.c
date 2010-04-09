#include "fontbypass.h"


FONT_ATTR font_attr;
UCHAR buf_video[320*240*2];

#ifdef SOCKET_BRIDGE
__align(32)
char g_FontFuncBuffer[FONTFUNCBUFFER_SIZE];
#endif


int main(int argc, char *argv[])
{
	int fd_video,buf_len_video;
	int event = 0;
	int count = 0 ;
	int cnt = 0,flag=0;
    char *pcontent;
    unsigned long ulIPOldeth1 = -1, ulIPOldppp0 = -1;
    BOOL bIPEth1Change = FALSE, bIPPPP0Change = FALSE;
    
    //for font-->
	FONT_DEVICE_T fd;
	FONT_RECT_T rc;
	FONT_SIZE_T sz;
    
	cyg_interrupt_disable();
	cyg_interrupt_disable();
	cyg_interrupt_disable();
	
	cyg_interrupt_enable();
	VideoPhoneInit();
	
    fontAttrInit(&font_attr);
    //fontGetConf(&font_attr, FONT_CONF_PATH);
    fontFontSet(&fd, &font_attr);
    //for font end<--

	ControlBackLight(6, 0);
	wb702ConfigureLCM(320, 240, 2, 1, 3, 0, TRUE, 1);
	//wb702SetOSDColor(5, 0x00, 0x00, 0x00, 0xF8, 0xFC, 0xF8);
	ControlLcmTvPower(2);
	ControlBackLight(6, 1);	
#ifdef FONT_STANDALONE
	//wb702SetIoctlRW(fd_video, TRUE);//select ioctl method
	wb702EnableMotionDetect(TRUE, CMD_MOTION_DETECT_LOW);
	wb702Init(fd_video);
#endif
	
    drawLCDRGB(buf_video,font_attr.iScreenWidth*font_attr.iScreenHeight*2);
    //wb702BypassBitmap(font_attr.iScreen, buf_video, font_attr.iScreenWidth*font_attr.iScreenHeight*2);
    
    while(1)
    {
        drawLCDRGB(buf_video,font_attr.iScreenWidth*font_attr.iScreenHeight*2);

        //if(font_attr.bBackground == TRUE)
        {
            bmpconv(BMP_BACKGROUND_PATH, buf_video, font_attr.iScreenWidth, font_attr.iScreenHeight);
        }

        fontPrintContent(&fd, &font_attr, FONT_CONF_PATH);
		
		/*
        if(font_attr.bVersion == TRUE)
        {
            char versionbuf[32];
            char cmd[32];
            
            if(fontGetVersion(fd_video, versionbuf) == TRUE)
            {
                sprintf(cmd, "V%s", versionbuf);
                pcontent = cmd;
            }
            else
                pcontent = VERSION_DEFAULT;
            fontGetSize(&fd, (unsigned char*)pcontent, &sz);
            fontGetRC(&font_attr, &rc, &sz);
#ifdef FONT_DEBUG
            printf("Size: %d %d\n", sz.nWidth, sz.nHeight);
#endif
            fontPrint(&fd, (unsigned char*)pcontent, &rc, &sz);
            font_attr.rcThis.nTop += (sz.nHeight + LINE_SPACE);
        }
        */
        
        /*
        if(font_attr.bIpaddress == TRUE)
        {
            unsigned long ulIPeth1, ulIPppp0;
            BOOL ipresult;
            
            ulIPeth1 = GetIPAddress("eth1");
            if(ulIPeth1 != ulIPOldeth1)
            {
                bIPEth1Change = TRUE;
                ulIPOldeth1 = ulIPeth1;
                
                if(ulIPeth1 != 0)
                {
                    pcontent = malloc(20);
#ifdef SOCKET_BRIDGE
                    sprintf(pcontent, "ETH1: %s", inet_ntoa(*(struct in_addr*)&ulIPeth1, g_FontFuncBuffer, FONTFUNCBUFFER_SIZE));
#else
                    sprintf(pcontent, "ETH1: %s", inet_ntoa(*(struct in_addr*)&ulIPeth1));
#endif
                    fontGetSize(&fd, (unsigned char*)pcontent, &sz);
                    fontGetRC(&font_attr, &rc, &sz);
    #ifdef FONT_DEBUG
                    printf("Size: %d %d\n", sz.nWidth, sz.nHeight);
    #endif
                    fontPrint(&fd, (unsigned char*)pcontent, &rc, &sz);
                    font_attr.rcThis.nTop += (sz.nHeight + LINE_SPACE);
                    
                    free(pcontent);
                }
            }
            

            ulIPppp0 = GetIPAddress("ppp0");
            if(ulIPppp0 != ulIPOldppp0)
            {
                bIPPPP0Change = TRUE;
                ulIPOldppp0 = ulIPppp0;
                
                if(ulIPppp0 != 0)
                {
                    pcontent = malloc(20);
#ifdef SOCKET_BRIDGE
                    sprintf(pcontent, "PPP: %s", inet_ntoa(*(struct in_addr*)ulIPppp0, g_FontFuncBuffer, FONTFUNCBUFFER_SIZE));
#else
                    sprintf(pcontent, "PPP: %s", inet_ntoa(*(struct in_addr*)ulIPppp0));
#endif
                    fontGetSize(&fd, (unsigned char*)pcontent, &sz);
                    fontGetRC(&font_attr, &rc, &sz);
#ifdef FONT_DEBUG
                    printf("Size: %d %d\n", sz.nWidth, sz.nHeight);
#endif
                    fontPrint(&fd, (unsigned char*)pcontent, &rc, &sz);
                    font_attr.rcThis.nTop += (sz.nHeight + LINE_SPACE);
                
                    free(pcontent);
                }
            }
        }
        */
                {
                	bIPEth1Change = TRUE;
                    pcontent = malloc(20);
                    sprintf(pcontent, "ETH1: %s", "10.132.11.11");
                    fontGetSize(&fd, (unsigned char*)pcontent, &sz);
                    fontGetRC(&font_attr, &rc, &sz);
                    fontPrint(&fd, (unsigned char*)pcontent, &rc, &sz);
                    font_attr.rcThis.nTop += (sz.nHeight + LINE_SPACE);
                    
                    free(pcontent);
                }
        
        //if((bIPEth1Change==TRUE) || (bIPPPP0Change==TRUE))
        {
#ifdef FONT_DEBUG
            printf("wb702BypassBitmap!!!\n");
#endif
	        memcpy(vlcmGetLCMBuffer ()->aucData, buf_video, font_attr.iScreenWidth*font_attr.iScreenHeight*2);
           // wb702BypassBitmap(font_attr.iScreen, buf_video, font_attr.iScreenWidth*font_attr.iScreenHeight*2);
            bIPEth1Change = FALSE;
            bIPPPP0Change = FALSE;
        }
        
        fontRecoverRC(&font_attr);
        tt_msleep(5000);
    }
        

    return 0;
}

