#include "fontbypass.h"

#define TESTOSDWIDTH

FONT_ATTR font_attr;
//UCHAR buf_video[128*160*2];
UCHAR buf_video[320*240*2];

#ifdef SOCKET_BRIDGE
__align(32)
char g_FontFuncBuffer[FONTFUNCBUFFER_SIZE];
#endif

/* Draw red, green, blue and white in 320*240 */
void drawTestDisWidth(UCHAR *pBuf)
{
	int iHeight, iRow;
	for(iHeight = 0; iHeight < font_attr.iScreenHeight; iHeight++)
	{
		for(iRow = 0; iRow < 100; iRow++)
		{
			*(short*)(&pBuf[iHeight*320*2 + iRow*2]) = 0xf800; //red
		}
		/*
		for (; iRow < 101; iRow++)
			*(short*)(&pBuf[iHeight*320*2 + iRow*2]) = 0x841f; //blue
		for (; iRow < 103; iRow++)
			*(short*)(&pBuf[iHeight*320*2 + iRow*2]) = 0x07e0; //green
		for (; iRow < 107; iRow++)
			*(short*)(&pBuf[iHeight*320*2 + iRow*2]) = 0xf800; //read
		for (; iRow < 115; iRow++)
			*(short*)(&pBuf[iHeight*320*2 + iRow*2]) = 0x841f; //blue
		*/
		
		for(iRow = 0; iRow < 100; iRow++)
		{
			*(short*)(&pBuf[iHeight*320*2 + 100*2 + iRow*2]) = 0x07e0; //green
		}
		for(iRow = 0; iRow < 100; iRow++)
		{
			//*(short*)(&pBuf[iHeight*320*2 + 200*2 + iRow*2]) = 0x841f; //blue
			*(short*)(&pBuf[iHeight*320*2 + 200*2 + iRow*2]) = 0x001f; //blue
		}
		for(iRow = 0; iRow < 20; iRow++)
		{
			*(short*)(&pBuf[iHeight*320*2 + 300*2 + iRow*2]) = 0xffff; //white
		}
	}
}

int main(int argc, char *argv[])
{
	int count = 0 ;
    int i, j;
    
    //for font-->
	FONT_DEVICE_T fd;
	FONT_RECT_T rc;
	FONT_SIZE_T sz;
    
	cyg_interrupt_disable();
	cyg_interrupt_disable();
	cyg_interrupt_disable();
	
	cyg_interrupt_enable();
	VideoPhoneInit();
	//SuspendTvp5150();
	
    fontAttrInit(&font_attr);
    //fontGetConf(&font_attr, FONT_CONF_PATH);
    fontFontSet(&fd, &font_attr);
    //for font end<--

	ControlBackLight(6, 0);
	wb702ConfigureLCM(320, 240, 2, 1, 3, 0, TRUE, 1);
#if defined(TESTOSD) || defined(TESTOSDWIDTH)
	wb702SetOSDColor(5, 0x00, 0x00, 0x00, 0xF8, 0xFC, 0xF8);
#endif
	ControlLcmTvPower(2);
	ControlBackLight(6, 1);	
	
#if defined(TESTOSD) || defined(TESTOSDWIDTH)
	wb702EnableDisplay(CMD_DISPLAY_OSD, TRUE);
	wb702FillOSD(TRUE, 0x00, 0x00, 0x00);
#endif
		
    drawLCDRGB(buf_video,font_attr.iScreenWidth*font_attr.iScreenHeight*2);
    //wb702BypassBitmap(font_attr.iScreen, buf_video, font_attr.iScreenWidth*font_attr.iScreenHeight*2);
    
	count = 0;
    while(1)
    {
	    switch(count%4)
	    {
	    	diag_printf("select %d\n", count%4);
	    	case 0:
	    		diag_printf("enter 0\n");
#if defined(TESTVIDEO) || defined(TESTVIDEOWIDTH)
	        	memset(vlcmGetLCMBuffer ()->aucData, 0, font_attr.iScreenWidth*font_attr.iScreenHeight*2);
#endif
#if defined(TESTOSD) || defined(TESTOSDWIDTH)
	        	memset(buf_video, 0, font_attr.iScreenWidth*font_attr.iScreenHeight*2);
#endif
	        	break;
	    	case 1:
	    		diag_printf("enter 1\n");
#ifdef TESTVIDEO
	        	memset(vlcmGetLCMBuffer ()->aucData, 0x66, font_attr.iScreenWidth*font_attr.iScreenHeight*2);
#endif
#ifdef TESTOSD
	        	memset(buf_video, 0x66, font_attr.iScreenWidth*font_attr.iScreenHeight*2);
#endif
#ifdef TESTVIDEOWIDTH
	        	drawTestDisWidth(vlcmGetLCMBuffer ()->aucData);
#endif
#ifdef TESTOSDWIDTH
	        	drawTestDisWidth(buf_video);
#endif
	        	break;
	    	case 2:
	    		diag_printf("enter 2\n");
#ifdef TESTVIDEO
	        	memset(vlcmGetLCMBuffer ()->aucData, 0xbb, font_attr.iScreenWidth*font_attr.iScreenHeight*2);
#endif
#ifdef TESTOSD
	        	memset(buf_video, 0xbb, font_attr.iScreenWidth*font_attr.iScreenHeight*2);
#endif
#ifdef TESTVIDEOWIDTH
	        	drawTestDisWidth(vlcmGetLCMBuffer ()->aucData);
#endif
#ifdef TESTOSDWIDTH
	        	drawTestDisWidth(buf_video);
#endif
	        	break;
	    	case 3:
	    		diag_printf("enter 3\n");
#ifdef TESTVIDEO
	        	memset(vlcmGetLCMBuffer ()->aucData, 0xFF, font_attr.iScreenWidth*font_attr.iScreenHeight*2);
#endif
#ifdef TESTOSD
	        	memset(buf_video, 0xFF, font_attr.iScreenWidth*font_attr.iScreenHeight*2);
#endif
#ifdef TESTVIDEOWIDTH
	        	drawTestDisWidth(vlcmGetLCMBuffer ()->aucData);
#endif
#ifdef TESTOSDWIDTH
	        	drawTestDisWidth(buf_video);
#endif
	        	break;
	    }
#if defined(TESTOSD) || defined(TESTOSDWIDTH)
	    wb702BypassBitmap(font_attr.iScreen, buf_video, font_attr.iScreenWidth*font_attr.iScreenHeight*2);
#endif
		count++;
        
        tt_msleep(1000);
    }
        

    return 0;
}

