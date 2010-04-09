#include "fontbypass.h"

int fontAttrInit(FONT_ATTR *pfar)
{
    FONT_RECT_T *prc;
    
    if(pfar == NULL)
        return FALSE;
    
    pfar->iScreen = SCREEN_WHICH_BIG;
    pfar->iScreenWidth = LCM1_WIDTH;
    pfar->iScreenHeight = LCM1_HEIGHT;
    pfar->bReversal = FALSE;

    prc = &(pfar->rcOrg);
//    prc->nLeft = POSX_DEFAULT;
//    prc->nTop = POSY_DEFAULT;
    prc->nLeft = 10;
    prc->nTop = 40;
    prc->nWidth = font_attr.iScreenWidth - (FRAME_SIZE*2);
    prc->nHeight = font_attr.iScreenHeight - (FRAME_SIZE*2);
    
    prc = &(pfar->rcThis);
    prc->nLeft = POSX_DEFAULT;
    prc->nTop = POSY_DEFAULT;
    prc->nWidth = font_attr.iScreenWidth - (FRAME_SIZE*2);
    prc->nHeight = font_attr.iScreenHeight - (FRAME_SIZE*2);
    
//    pfar->sFontColor = FONT_COLOR_DEFAULT;
//    pfar->sFontColore = FONT_COLOR_DEFAULT;
    pfar->sFontColor = 65535;
    pfar->sFontColore = 63647;
    pfar->iFontLeft = POSX_DEFAULT;
    pfar->fStyleAlign = FONT_STYLE_ALIGN_CENTER;
//    pfar->fStyleBold = FONT_STYLE_BOLD_NONE;
    pfar->fStyleBold = FONT_STYLE_BOLD_ALL;
    
    pfar->fCode = FONT_CODE_ENG;
    
    pfar->bBackground = TRUE;
//    pfar->bVersion = TRUE;
    pfar->bVersion = FALSE;
    pfar->bIpaddress = TRUE;
    pfar->bFontShadow = TRUE;
    
    pfar->pcContent[0] = "W i n b o n d\r\n";
    pfar->pcContent[1] = "\r\n";
    pfar->pcContent[2] = "I P    C A M E R A\r\n";
    pfar->pcContent[3] = "\r\n";
    pfar->pcContent[4] = "V 1 . 0\r\n";
    pfar->pcContent[5] = "\r\n";
    pfar->pcContent[6] = NULL;
    return TRUE;
}

int fontFontSet(FONT_DEVICE_T *pDevice, FONT_ATTR *pfar)
{
    if((pDevice==NULL) || (pfar==NULL))
        return FALSE;
    
    switch (pfar->fCode)
    {
        case 0:
            fontSetFont(pDevice,
                         &g_tFont11x11Eng,
                         fontDrawPixelBypass,
                         NULL);
            break;
#ifdef SUPPORT_FONT_GBK_11X11
        case 1:
            fontSetFont(pDevice,
                         &g_tFont11x11GBK,
                         fontDrawPixelBypass,
                         NULL);
            break;
#endif
#ifdef SUPPORT_FONT_BIG5_11x11
        case 2:
            fontSetFont(pDevice,
                         &g_tFont11x11Big5,
                         fontDrawPixelBypass,
                         NULL);
            break;
#endif
#ifdef SUPPORT_FONT_BIG5_SHARE_GBK_11X11
       case 3:
            fontSetFont(pDevice,
                         &g_tFont11x11Big5ToGBK,
                         fontDrawPixelBypass,
                         NULL);
            break;
#endif
#ifdef SUPPORT_FONT_UNICODE_SHARE_GBK_11X11
        case 4:
            fontSetFont(pDevice,
                         &g_tFont11x11UnicodeToGBK,
                         fontDrawPixelBypass,
                         NULL);
            break;
#endif
        default:
            fontSetFont(pDevice,
                         &g_tFont11x11Eng,
                         fontDrawPixelBypass,
                         NULL);
            break;
    }
    
    return TRUE;
}

#define BUF_SIZE 250
int fontGetConf(FONT_ATTR *pfar, char *pConf)
{
    FILE *pf;
    char buf[BUF_SIZE], tmp[BUF_SIZE];
    char *ptmp, *presult;
    int ilen;
    
    FONT_RECT_T *prc, *prcOrg;
    
    if((pfar==NULL) || (pConf==NULL))
        return FALSE;
    
    prc = &(pfar->rcThis);
    prcOrg = &(pfar->rcOrg);

    pf = fopen(pConf, "r");
    if(pf == NULL)
    {
        printf("Font Configure File %s open error!!\n", pConf);
        return FALSE;
    }
    
    while(fgets(buf, BUF_SIZE, pf)!=NULL)
    {
        ilen = strlen(buf)-2;
        buf[ilen] = '\0';
        if((ptmp = strchr(buf, ':'))==NULL)
        {
            continue;
        }
        else
        {
            *ptmp = '\0';
            ptmp++;
        }
        
        ilen = strlen(ptmp);
        if(ilen == 0) continue;
        ptmp[ilen] = '\0';
        
        if((presult = strchr(ptmp, ' '))!=NULL)
        {
            *presult = '\0';
        }
        ilen = strlen(ptmp);
        
        if(strcmp(buf, "CODE")==0)
        {
            int iCode;
            iCode = atoi(ptmp);
            
            if(iCode >=0 && iCode<FONT_CODE_NULL)
                pfar->fCode = iCode;
        }
        else if(strcmp(buf, "SCREENNUM")==0)
        {
            int iScreen;
            iScreen = atoi(ptmp);
            
            if(iScreen >=0 && iScreen<SCREEN_WHICH_NULL)
                pfar->iScreen = iScreen;
            switch (iScreen)
            {
                case SCREEN_WHICH_BIG:
                    pfar->iScreenWidth = LCM1_WIDTH;
                    pfar->iScreenHeight = LCM1_HEIGHT;
                    break;
                case SCREEN_WHICH_SML:
                    pfar->iScreenWidth = LCM2_WIDTH;
                    pfar->iScreenHeight = LCM2_HEIGHT;
                    break;
                default:
                    break;
            }
            prc->nWidth = font_attr.iScreenWidth - (FRAME_SIZE*2);
            prc->nHeight = font_attr.iScreenHeight - (FRAME_SIZE*2);
            prcOrg->nWidth = prc->nWidth;
            prcOrg->nHeight = prc->nHeight;
        }
        else if(strcmp(buf, "REVERSAL")==0)
        {
            if(ptmp[0] == 'Y' || ptmp[0] == 'y')
                pfar->bReversal = TRUE;
            else
                pfar->bReversal = FALSE;
        }
        else if(strcmp(buf, "BACKGROUND")==0)
        {
            if(ptmp[0] == 'Y' || ptmp[0] == 'y')
                pfar->bBackground = TRUE;
            else
                pfar->bBackground = FALSE;
        }
        else if(strcmp(buf, "VERSION")==0)
        {
            if(ptmp[0] == 'Y' || ptmp[0] == 'y')
                pfar->bVersion = TRUE;
            else
                pfar->bVersion = FALSE;
        }
        else if(strcmp(buf, "IPADDRESS")==0)
        {
            if(ptmp[0] == 'Y' || ptmp[0] == 'y')
                pfar->bIpaddress = TRUE;
            else
                pfar->bIpaddress = FALSE;
        }
        else if(strcmp(buf, "STARTPOSX")==0)
        {
            prc->nLeft = atoi(ptmp);
            if(prc->nLeft<POSX_DEFAULT || prc->nLeft>(font_attr.iScreenWidth-FRAME_SIZE))
                prc->nLeft = POSX_DEFAULT;
            prcOrg->nLeft = prc->nLeft;
        }
        else if(strcmp(buf, "STARTPOSY")==0)
        {
            prc->nTop = atoi(ptmp);
            if(prc->nTop<POSY_DEFAULT || prc->nTop>(font_attr.iScreenHeight-FRAME_SIZE))
                prc->nTop = POSY_DEFAULT;
            prcOrg->nTop = prc->nTop;
        }
        else if(strcmp(buf, "ALIGN")==0)
        {
            if(ptmp[0] == 'C' || ptmp[0] == 'c')
                pfar->fStyleAlign = FONT_STYLE_ALIGN_CENTER;
            else if(ptmp[0] == 'L' || ptmp[0] == 'l')
                pfar->fStyleAlign = FONT_STYLE_ALIGN_LEFT;
            else if(ptmp[0] == 'R' || ptmp[0] == 'r')
                pfar->fStyleAlign = FONT_STYLE_ALIGN_RIGHT;
        }
        else if(strcmp(buf, "BOLD")==0)
        {
            if(ptmp[0] == 'Y' || ptmp[0] == 'y')
                pfar->fStyleBold = FONT_STYLE_BOLD_ALL;
            else
                pfar->fStyleBold = FONT_STYLE_BOLD_NONE;
        }
        else if(strcmp(buf, "FONTCOLOR")==0)
        {
            pfar->sFontColor = atoi(ptmp);
            if((pfar->sFontColor<FONT_COLOR_MIX) || (pfar->sFontColor>FONT_COLOR_MAX))
                pfar->sFontColor = FONT_COLOR_DEFAULT;
        }
        else if(strcmp(buf, "FONTCOLOREND")==0)
        {
            pfar->sFontColore = atoi(ptmp);
            if((pfar->sFontColore<FONT_COLOR_MIX) || (pfar->sFontColore>FONT_COLOR_MAX))
                pfar->sFontColore = FONT_COLOR_DEFAULT;
        }
        else if(strcmp(buf, "FONTSHADOW")==0)
        {
            if(ptmp[0] == 'Y' || ptmp[0] == 'y')
                pfar->bFontShadow = TRUE;
            else
                pfar->bFontShadow = FALSE;
        }
    }
    
    fclose(pf);
    return TRUE;
}

/*
int fontGetVersion(int iFd, char *pbuf)
{
	WB_VERSION_Ext_t wVersion;
    char *ptmp, *presult;
    int ilen;
    
    if(pbuf == NULL)
        return FALSE;
    
    ilen = wb702GetFirmwareVersionExt(iFd, &wVersion);
    if(ilen == FALSE)
        return FALSE;

#if 0
    printf("version =%s\n", wVersion.version);
    printf("changedate =%s\n", wVersion.changedate);
    printf("builddate =%s\n", wVersion.builddate);
#endif
	
    presult = strchr(wVersion.version, ' ');
    presult++;
    ptmp = strchr(presult, ' ');
    *ptmp = '\0';    
    pbuf = strcpy(pbuf, presult);
    return TRUE;
}
*/

int fontGetRC(FONT_ATTR *pfar, FONT_RECT_T *prc, FONT_SIZE_T *psz)
{
    FONT_RECT_T *prect;
    
    if((pfar==NULL) || (prc==NULL) || (psz==NULL))
        return FALSE;
    prect = &(pfar->rcThis);
    
    
    prc->nLeft = prect->nLeft;
    prc->nTop = prect->nTop;
    prc->nWidth = prect->nWidth;
    prc->nHeight = prect->nHeight;
    
    if(pfar->fStyleAlign == FONT_STYLE_ALIGN_CENTER)
    {
        if(psz->nWidth >= prect->nWidth)
        {
            prc->nLeft = POSX_DEFAULT;
        }
        else
        {
            prc->nLeft = (prect->nWidth - psz->nWidth)/2 + POSX_DEFAULT;
        }
    }
    
    SetFontColorCh(pfar, prc, psz);
    
    return TRUE;
}

int fontRecoverRC(FONT_ATTR *pfar)
{
    FONT_RECT_T *prc, *prcOrg;
    if(pfar == NULL)
        return FALSE;
    
    prc = &(pfar->rcThis);
    prcOrg = &(pfar->rcOrg);
    
    prc->nLeft = prcOrg->nLeft;
    prc->nTop = prcOrg->nTop;
    prc->nWidth = prcOrg->nWidth;
    prc->nHeight = prcOrg->nHeight;
    
    return TRUE;
}

int fontPrintContent(FONT_DEVICE_T *pDevice, FONT_ATTR *pFontAttr, char *pConf)
{
    FILE *pf;
    char buf[BUF_SIZE];
    //char *ptmp;
    char **ptmp;
    int ilen;
    BOOL bFindContent = FALSE;
    
	FONT_RECT_T rc;
	FONT_SIZE_T sz;

    FONT_RECT_T *prc;

    if((pDevice==NULL) || (pFontAttr==NULL) || (pConf==NULL))
        return FALSE;
    
    prc = &(pFontAttr->rcThis);

    /*
    pf = fopen(pConf, "r");
    if(pf == NULL)
    {
        printf("Font Configure File %s open error!!\n", pConf);
        return FALSE;
    }
    
    while(fgets(buf, BUF_SIZE, pf)!=NULL)
    {
        ilen = strlen(buf)-2;
        buf[ilen] = '\0';
        if((bFindContent==FALSE) && (ptmp = strchr(buf, ':'))==NULL)
        {
            continue;
        }
        else if(bFindContent == FALSE)
        {
            *ptmp = '\0';
        }
        
        if(strcmp(buf, "CONTENT") == 0)
        {
            bFindContent = TRUE;
            continue;
        }
        
        if(bFindContent == FALSE)
            continue;
        
        fontGetSize(pDevice, (unsigned char*)buf, &sz);
        fontGetRC(&font_attr, &rc, &sz);
        fontPrint(pDevice, (unsigned char*)buf, &rc, &sz);
        prc->nTop += (sz.nHeight + LINE_SPACE);
    }
    */
    ptmp = pFontAttr->pcContent;
    for(ilen = 0; ilen < FONT_CONTENT_SIZE; ilen++)
    {
    	if(ptmp[ilen] == NULL) break;
    	
        fontGetSize(pDevice, (unsigned char*)ptmp[ilen], &sz);
        fontGetRC(&font_attr, &rc, &sz);
        fontPrint(pDevice, (unsigned char*)ptmp[ilen], &rc, &sz);
        prc->nTop += (sz.nHeight + LINE_SPACE);
    }
    
    fclose(pf);
    return TRUE;
}

void fontDrawPixelBypass(void *pDevice, EF_BOOL bIsDraw, EF_INT nPosX, EF_INT nPosY)
{
    int iPos;
    int nNewX, nNewY;
    USHORT usColor, usBackColor;
    int ialpha = 1;
    
	if ((nPosX*1+1) > (font_attr.iScreenWidth - FRAME_SIZE)) return;
    if(font_attr.bReversal == TRUE)
    {
        nNewX = font_attr.iScreenWidth - nPosX;
        nNewY = font_attr.iScreenHeight - nPosY;
    }
    else
    {
        nNewX = nPosX;
        nNewY = nPosY;
    }
    
    iPos = ((nNewX*1+1)+(nNewY+1)*font_attr.iScreenWidth)*2;
        
    if(bIsDraw == TRUE)
    {
        usColor = font_attr.sFontColor;

        usBackColor = buf_video[iPos];
        usColor = usColor*ialpha + usBackColor*(1-ialpha);
        
        GetFontNewColor(&font_attr, nPosX, &usColor);
        
        buf_video[iPos++] = (UCHAR) usColor;
        buf_video[iPos++] = (UCHAR) (usColor >> 8);
        if(font_attr.fStyleBold == FONT_STYLE_BOLD_ALL)
        {
            buf_video[iPos++] = (UCHAR) usColor;
            buf_video[iPos++] = (UCHAR) (usColor >> 8);
        }
        if(font_attr.bFontShadow == TRUE)
        {
            usColor = 0x0;
            iPos += font_attr.iScreenWidth*2;
            buf_video[iPos++] = (UCHAR) usColor;
            buf_video[iPos++] = (UCHAR) (usColor >> 8);
        }
    }
}

#ifdef SOCKET_BRIDGE
extern char g_FontFuncBuffer[FONTFUNCBUFFER_SIZE];
#endif

unsigned long GetGeneralIP(const char *pcInterface, int iRequest)
{
	struct ifreq ifr;
	int fd;
	unsigned long ulRt;

	if (pcInterface == NULL)
	{
		fprintf(stderr, "illegal call function GetGeneralIP!\n");
		return 0L;
	}

#ifdef SOCKET_BRIDGE
	if ((fd = socket(AF_INET,SOCK_DGRAM,0, g_FontFuncBuffer, FONTFUNCBUFFER_SIZE)) < 0) return 0L;
#else
	if ((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0) return 0L;
#endif
	strcpy(ifr.ifr_name, pcInterface);
#ifdef SOCKET_BRIDGE
	if (netioctl(fd, iRequest, &ifr, sizeof(ifr), g_FontFuncBuffer, FONTFUNCBUFFER_SIZE) < 0)
#else
	if (ioctl(fd, iRequest, &ifr, sizeof(ifr)) < 0)
#endif
		ulRt = 0;
	else
		ulRt = (*(struct sockaddr_in *)&(ifr.ifr_addr)).sin_addr.s_addr;
#ifdef SOCKET_BRIDGE
	netclose(fd, g_FontFuncBuffer, FONTFUNCBUFFER_SIZE);
#else
	close(fd);
#endif

	return ulRt;
}

unsigned long GetIPAddress(const char *pcInterface)
{
	return GetGeneralIP(pcInterface, SIOCGIFADDR);
}

int SetFontColorCh(FONT_ATTR *pFontAttr, FONT_RECT_T *prc, FONT_SIZE_T *psz)
{
    unsigned short usR, usG, usB;
    unsigned short usRe, usGe, usBe;
    
    pFontAttr->iFontLeft = prc->nLeft;
    
    SetFontColorChange(pFontAttr, pFontAttr->sFontColor, pFontAttr->sFontColore, psz->nWidth+2);
}

int SetFontColorChange(FONT_ATTR *pFontAttr, unsigned short sFontColor, unsigned short sFontColore, int nWidth)
{
    unsigned short usColor[3], usColore[3], usG, usGe;
    
    int len1, len2, step1, step2, cstep1, cstep2;
    unsigned short cstepr1, cstepr2;
    int ifound = FALSE;
    int i;
    
    usColor[0] = (sFontColor >> 11) & 0x001f;
    usColor[1] = (sFontColor >> 5) & 0x003f;
    usColor[2] = (sFontColor) & 0x001f;
    usColore[0] = (sFontColore >> 11) & 0x001f;
    usColore[1] = (sFontColore >> 5) & 0x003f;
    usColore[2] = (sFontColore) & 0x001f;
    
    for(i=0; i<3; i++)
    {
        len1 = len2 = step1 = step2 = cstep1 = cstep2 = 0;
        cstepr1 = cstepr2 = 0;
        ifound = FALSE;
        usG = usColor[i];
        usGe = usColore[i];

#ifdef FONT_DEBUG
        printf("nWidth=%d abs((short)(usGe - usG))=%d\n", nWidth, abs((short)(usGe - usG)));
#endif
        if(nWidth >= (abs((short)(usGe - usG))+1))
        {
            int i, j;
            
            step1 = nWidth/(abs((short)(usGe - usG))+1);
            cstep1 = 1;
            for(i=1; i*step1<=nWidth; i++)
            {
                for(j=0; (i+j)*step1<=nWidth; j++)
                {
                    if((i-1)*cstep1==abs((short)(usGe - usG)))
                    {
                        if(i*step1 == nWidth)
                        {
                            cstep2 = 0;
                            step2 = 0;
                            ifound  = TRUE;
                            break;
                        }
                    }
                    if(j-1>=0 && (i-1)*cstep1+(j-1)==abs((short)(usGe - usG)))
                    {
                        int m = (nWidth-i*step1)/j;
                        if(i*step1+j*m == nWidth)
                        {
                            cstep2 = 1;
                            step2 = m;
                            ifound  = TRUE;
                            break;
                        }
                    }
                }
                if(ifound == TRUE)
                    break;
            }
            len1 = i;
            len2 = j;
        }
        if(nWidth < (abs((short)(usGe - usG))+1))
        {
            int i, j;
            
            step1 = 1;
            cstep1 = (abs((short)(usGe - usG))+1)/nWidth;
            for(i=1; i*step1<=nWidth; i++)
            {
                for(j=0; (i+j)*step1<=nWidth; j++)
                {
                    if(i*step1==nWidth)
                    {
                        if(i*step1*cstep1 == abs((short)(usGe - usG))+1)
                        {
                            cstep2 = 0;
                            step2 = 0;
                            ifound  = TRUE;
                            break;
                        }
                    }
                    if(i*step1+j==nWidth)
                    {
                        int m = ((abs((short)(usGe - usG))+1)-i*cstep1)/j;
                        if(i*cstep1+j*m == abs((short)(usGe - usG))+1)
                        {
                            cstep2 = m;
                            step2 = 1;
                            ifound  = TRUE;
                            break;
                        }
                    }
                }
                if(ifound == TRUE)
                    break;
            }
            len1 = i;
            len2 = j;
        }
        if((short)(usGe - usG)<0)
        {
            cstepr1 = ~cstep1+1;
            cstepr2 = ~cstep2+1;
        }
        else
        {
            cstepr1 = cstep1;
            cstepr2 = cstep2;
        }
        
        pFontAttr->fcChange[i].iFound = ifound;
        //if(ifound == TRUE)
        {
            pFontAttr->fcChange[i].iLen1 = len1;
            pFontAttr->fcChange[i].iLen2 = len2;
            pFontAttr->fcChange[i].iStep1 = step1;
            pFontAttr->fcChange[i].iStep2 = step2;
            if(i == 1)
            {
                pFontAttr->fcChange[i].usCstep1 = cstepr1&0x003f;
                pFontAttr->fcChange[i].usCstep2 = cstepr2&0x003f;
            }
            else
            {
                pFontAttr->fcChange[i].usCstep1 = cstepr1&0x001f;
                pFontAttr->fcChange[i].usCstep2 = cstepr2&0x001f;
            }
        }
        
#ifdef FONT_DEBUG
        printf("len1=%d step1=%d cstep1=0x%x(%d), len2=%d step2=%d cstep2=0x%x(%d)\n", len1, step1, cstep1, cstep1, len2, step2, cstep2, cstep2);
        printf("cstepr1=0x%x(%d) cstepr2=0x%x(%d)\n", cstepr1, (short)cstepr1, cstepr2, (short)cstepr2);
#endif
    }
    
    return 0;
}

int GetFontNewColor(FONT_ATTR *pFontAttr, int nPosX, USHORT *pusColor)
{
    int iPosCh;
    unsigned short usR, usG, usB, usRe, usGe, usBe;
    int iStep1Len, iStep2Len;
    
    usR = (pFontAttr->sFontColor >> 11) & 0x001f;
    usG = (pFontAttr->sFontColor >> 5) & 0x003f;
    usB = (pFontAttr->sFontColor) & 0x001f;
    
    iPosCh = nPosX - pFontAttr->iFontLeft + 1;
    
    iStep1Len = pFontAttr->fcChange[0].iLen1 * pFontAttr->fcChange[0].iStep1;
    iStep2Len = pFontAttr->fcChange[0].iLen2 * pFontAttr->fcChange[0].iStep2;
    //if(pFontAttr->fcChange[0].iFound == TRUE)
    {
        if(iPosCh <= iStep1Len)
        {
            usRe = (usR + iPosCh / pFontAttr->fcChange[0].iStep1 * pFontAttr->fcChange[0].usCstep1) & 0x001f;
        }
        else if(iPosCh > iStep1Len && iPosCh <= (iStep1Len + iStep2Len))
        {
            usRe = (usR + pFontAttr->fcChange[0].iLen1 * pFontAttr->fcChange[0].usCstep1 + (iPosCh - iStep1Len) / pFontAttr->fcChange[0].iStep2 * pFontAttr->fcChange[0].usCstep2) & 0x001f;
        }
        else
        {
            usRe = (usR + pFontAttr->fcChange[0].iLen1 * pFontAttr->fcChange[0].usCstep1 + pFontAttr->fcChange[0].iLen2 * pFontAttr->fcChange[0].usCstep2) & 0x001f;
        }
    }
    //else
    {
        //usRe = usR;
    }
    
    iStep1Len = pFontAttr->fcChange[1].iLen1 * pFontAttr->fcChange[1].iStep1;
    iStep2Len = pFontAttr->fcChange[1].iLen2 * pFontAttr->fcChange[1].iStep2;
    //if(pFontAttr->fcChange[1].iFound == TRUE)
    {
        if(iPosCh <= iStep1Len)
        {
            usGe = (usG + iPosCh / pFontAttr->fcChange[1].iStep1 * pFontAttr->fcChange[1].usCstep1) & 0x003f;
        }
        else if(iPosCh > iStep1Len && iPosCh <= (iStep1Len + iStep2Len))
        {
            usGe = (usG + pFontAttr->fcChange[1].iLen1 * pFontAttr->fcChange[1].usCstep1 + (iPosCh - iStep1Len) / pFontAttr->fcChange[1].iStep2 * pFontAttr->fcChange[1].usCstep2) & 0x003f;
        }
        else
        {
            usGe = (usG + pFontAttr->fcChange[1].iLen1 * pFontAttr->fcChange[1].usCstep1 + pFontAttr->fcChange[1].iLen2 * pFontAttr->fcChange[1].usCstep2) & 0x003f;
        }
    }
    //else
    {
        //usGe = usG;
    }
#ifdef FONT_DEBUG
    //printf("nPosX=%d usGe=0x%x(%d) iPosCh=%d iStep1Len=%d iStep2Len=%d\n", nPosX, (unsigned short)usGe, (short)usGe, iPosCh, iStep1Len, iStep2Len);
#endif
    
    iStep1Len = pFontAttr->fcChange[2].iLen1 * pFontAttr->fcChange[2].iStep1;
    iStep2Len = pFontAttr->fcChange[2].iLen2 * pFontAttr->fcChange[2].iStep2;
    //if(pFontAttr->fcChange[2].iFound == TRUE)
    {
        if(iPosCh <= iStep1Len)
        {
            usBe = (usB + iPosCh / pFontAttr->fcChange[2].iStep1 * pFontAttr->fcChange[2].usCstep1) & 0x001f;
        }
        else if(iPosCh > iStep1Len && iPosCh <= (iStep1Len + iStep2Len))
        {
            usBe = (usB + pFontAttr->fcChange[2].iLen1 * pFontAttr->fcChange[2].usCstep1 + (iPosCh - iStep1Len) / pFontAttr->fcChange[2].iStep2 * pFontAttr->fcChange[2].usCstep2) & 0x001f;
        }
        else
        {
            usBe = (usB + pFontAttr->fcChange[2].iLen1 * pFontAttr->fcChange[2].usCstep1 + pFontAttr->fcChange[2].iLen2 * pFontAttr->fcChange[2].usCstep2) & 0x001f;
        }
    }
    //else
    {
        //usBe = usB;
    }
    
    *pusColor = ((usRe<<11)&0xf800) | ((usGe<<5)&0x07e0) | (usBe&0x001f);
#ifdef FONT_DEBUG
    printf("nPosX=%d iPosCh=%d usColor=0x%x(%d)\n", nPosX, iPosCh, *pusColor, *pusColor);
#endif
    
    return 0;
}
