#include "../Inc/inc.h"


typedef struct
{
	//TT_RMUTEX_T	tLock;
	time_t			tOffset;
	//UINT32		uLast
} VP_INFO_T;

typedef struct
{
	char 		*pcYUVBuffer;
	VP_SIZE_T	size;
	int			nStretchFactor;
} VINFO_DRAW_DEVICE;

#define RECT_FRAME_SIZE 2

#define VP_INFO_CONTENT_ARRAY_SIZE 3
typedef struct
{
	char	content[128];
	int		x;
	int		y;
	BOOL	bDraw;
	
	int		finalx;
	int		finaly;
	int		resWidth;
	int		resHeight;
	
	cyg_mutex_t mutex;
} VINFO_DRAW_CONTENT;

static VP_INFO_T g_Info;
static VINFO_DRAW_CONTENT g_InfoContent[VP_INFO_CONTENT_ARRAY_SIZE];

void vinfoSetTime (const struct tm *pTms)
{
	struct tm tms = *pTms;
	time_t set_time = mktime (&tms);
	time_t current_time = (time_t) tt_get_time ();

	g_Info.tOffset = set_time - current_time;
}

void vinfoGetTime (struct tm *pTms)
{
	time_t current_time = (time_t) tt_get_time () + g_Info.tOffset;
	struct tm tms;
	gmtime_r (&current_time, &tms);
	tms.tm_year += 1900;
	tms.tm_mon += 1;
	
	tms.tm_year	= (tms.tm_year <= 9999 && tms.tm_year >= 0 ? tms.tm_year : 0);
	tms.tm_mon	= (tms.tm_mon <= 12 && tms.tm_mon >= 1 ? tms.tm_mon : 0);
	tms.tm_mday	= (tms.tm_mday <= 31 && tms.tm_mday >= 1 ? tms.tm_mday : 0);
	tms.tm_hour	= (tms.tm_hour <= 23 || tms.tm_hour >= 0 ? tms.tm_hour : 0);
	tms.tm_min	= (tms.tm_min <= 59 || tms.tm_min >= 0 ? tms.tm_min : 0);
	tms.tm_sec	= (tms.tm_sec <= 59 || tms.tm_sec >= 0 ? tms.tm_sec : 0);
}

static void vinfoDrawYUV (void *pDevice,
						  EF_BOOL bIsDraw,
						  EF_INT nPosX,
						  EF_INT nPosY)
{
	VINFO_DRAW_DEVICE *pDraw = (VINFO_DRAW_DEVICE *) pDevice;
	if (bIsDraw == TRUE)
	{
		INT x, y;
		INT nPos;
		nPosX *= pDraw->nStretchFactor;
		nPosY *= pDraw->nStretchFactor;
		nPos = nPosY * pDraw->size.usWidth * pDraw->nStretchFactor + nPosX;
		for (y = 0; y < pDraw->nStretchFactor; y++)
		{
			INT nLinePos = nPos;
			for (x = 0; x < pDraw->nStretchFactor; x++)
				pDraw->pcYUVBuffer[nLinePos + x] = (char) 0xFF;
			nPos += pDraw->size.usWidth * pDraw->nStretchFactor;
		}
	}
}



static void vinfoBuildTitle (char *pcTitle, size_t szLen)
{
	time_t current_time = (time_t) tt_get_time () + g_Info.tOffset;
	struct tm tms;
	gmtime_r (&current_time, &tms);
	tms.tm_year += 1900;
	tms.tm_mon += 1;
	
	snprintf (pcTitle, szLen, "%04d-%02d-%02d    %02d:%02d:%02d",
		(tms.tm_year <= 9999 && tms.tm_year >= 0 ? tms.tm_year : 0),
		(tms.tm_mon <= 12 && tms.tm_mon >= 1 ? tms.tm_mon : 0),
		(tms.tm_mday <= 31 && tms.tm_mday >= 1 ? tms.tm_mday : 0),
		(tms.tm_hour <= 23 || tms.tm_hour >= 0 ? tms.tm_hour : 0),
		(tms.tm_min <= 59 || tms.tm_min >= 0 ? tms.tm_min : 0),
		(tms.tm_sec <= 59 || tms.tm_sec >= 0 ? tms.tm_sec : 0)
		);
}

static void vinfoBuildDate(char *pbuf, int buflen)
{
	time_t current_time = (time_t) tt_get_time () + g_Info.tOffset;
	struct tm tms;
	gmtime_r (&current_time, &tms);
	tms.tm_year += 1900;
	tms.tm_mon += 1;
	
	snprintf (pbuf, buflen, "%04d-%02d-%02d",
		(tms.tm_year <= 9999 && tms.tm_year >= 0 ? tms.tm_year : 0),
		(tms.tm_mon <= 12 && tms.tm_mon >= 1 ? tms.tm_mon : 0),
		(tms.tm_mday <= 31 && tms.tm_mday >= 1 ? tms.tm_mday : 0)
		);
}

static void vinfoBuildTime(char *pbuf, int buflen)
{
	time_t current_time = (time_t) tt_get_time () + g_Info.tOffset;
	struct tm tms;
	gmtime_r (&current_time, &tms);
	tms.tm_year += 1900;
	tms.tm_mon += 1;
	
	snprintf (pbuf, buflen, "%02d:%02d:%02d",
		(tms.tm_hour <= 23 || tms.tm_hour >= 0 ? tms.tm_hour : 0),
		(tms.tm_min <= 59 || tms.tm_min >= 0 ? tms.tm_min : 0),
		(tms.tm_sec <= 59 || tms.tm_sec >= 0 ? tms.tm_sec : 0)
		);
}


extern const char *g_apcRevision[];
static void vinfoBuildVer(char *pbuf, int buflen)
{
	snprintf (pbuf, buflen, "%s", g_apcRevision[0]);
}

void vinfoDrawTitle (void *pYUVBuffer, VP_SIZE_T size)
{
	char acTitle[128];
	
	FONT_DEVICE_T fd;
	FONT_RECT_T rc;
	FONT_SIZE_T sz;
	VINFO_DRAW_DEVICE device;
	
	vinfoBuildTitle (acTitle, sizeof (acTitle));
	
	if (size.usWidth < 320)
		device.nStretchFactor = 1;
	else
		device.nStretchFactor = 2;
	
	device.pcYUVBuffer	= (char *) pYUVBuffer;
	device.size.usWidth	= size.usWidth / device.nStretchFactor;
	device.size.usHeight= size.usHeight / device.nStretchFactor;

	fontSetFont(&fd,
				 &g_tFont11x11Eng,
				 vinfoDrawYUV,
				 &device);

	rc.nLeft = 3;
	rc.nTop = 2;
	rc.nWidth = (INT) device.size.usWidth - rc.nLeft;
	rc.nHeight = (INT) device.size.usHeight - rc.nTop;

	//fontGetSize(&fd, "§â°s°Ý«C¤Ñ", &sz);
	//sysSafePrintf("Size: %d %d\n", sz.nWidth, sz.nHeight);

	fontPrint(&fd,
			  (unsigned char *) acTitle,
              &rc,
              &sz);
}

static void vinfoRegetXY(VINFO_DRAW_CONTENT *pcontent, VP_SIZE_T size, FONT_SIZE_T sz)
{
	int nStretchFactor;
	int width;
	int height;
	int x;
	int y;
	
	if (size.usWidth < 320)
		nStretchFactor = 1;
	else
		nStretchFactor = 2;
	
	width	= size.usWidth / nStretchFactor;
	height	= size.usHeight / nStretchFactor;
	x		= pcontent->x;
	y		= pcontent->y;
	
	/******************************************************************/
	/* When x<0 or y<0, the coordinate is not the coordinate of       */
	/* Top Left hand corner. So we have to transform it to the        */
	/* coordinate of Top Left hand corner                             */
	/******************************************************************/
	
	/* When x < 0, x is the coordinate of Top/Bottom Right hand corner */
	if (x < 0)
	{
		x = width + x - 1 - sz.nWidth;
		if (x < RECT_FRAME_SIZE)
			x = RECT_FRAME_SIZE;
	}
	/* When y < 0, y is the coordinate of Bottom Left/Right hand corner */
	if (y < 0)
	{
		y = height + y - 1 - sz.nHeight;
		if (y < RECT_FRAME_SIZE)
			y = RECT_FRAME_SIZE;
	}
	
	/* Judge and adjust coordinate to make all content can be displayed on video */
	if (x + sz.nWidth >= width - RECT_FRAME_SIZE)
	{
		/* Adjust the x coordinate */
		if (sz.nWidth > width - RECT_FRAME_SIZE * 2)
		{	/* Content length larger than image width */
			x = RECT_FRAME_SIZE;
		}
		else
		{	/* Image width is enough, but x is too large */
			x = width - sz.nWidth - RECT_FRAME_SIZE;
		}
	}
	if (y + sz.nHeight >= height - RECT_FRAME_SIZE)
	{
		/* Adjust the y coordinate */
		if (sz.nHeight > height - RECT_FRAME_SIZE * 2)
		{	/* Content height larger than image height */
			y = RECT_FRAME_SIZE;
		}
		else
		{	/* Image height is enough, but y is too large */
			y = height - sz.nHeight - RECT_FRAME_SIZE;
		}
	}
	
	pcontent->finalx = x;
	pcontent->finaly = y;
	pcontent->resWidth = size.usWidth;
	pcontent->resHeight = size.usHeight;
}

typedef struct
{
	char *key;
	void (* func) (char *pbuf, int buflen);
}VINFO_KEYWORD;

VINFO_KEYWORD g_vInfoKeyWord[] =
{
	{"time", vinfoBuildTime},
	{"date", vinfoBuildDate},
	{"ver", vinfoBuildVer}
};

static void vinfoReplaceKeyWord(char *pbuf, int buflen)
{
	char *pkeyb, *pkeye;
	char *posBuf, *posContent;
	char finalContent[128];
	char tmp[128];
	int copylen;
	int i;
	
	posContent = finalContent;
	posBuf = pbuf;
	
	while (1)
	{
		pkeyb = strchr(posBuf, '{');
		if (pkeyb == NULL)
		{
			break;
		}
		pkeye = strchr(pkeyb, '}');
		if (pkeye == NULL)
		{
			break;
		}
		*pkeye = '\0';
		
		copylen = sizeof(finalContent) - (posContent - finalContent) - 1;
		copylen = copylen > (pkeyb - posBuf) ? (pkeyb - posBuf) : copylen;
		strncpy(posContent, posBuf, copylen);
		posContent += copylen;
		
		for (i = 0; i < sizeof(g_vInfoKeyWord) / sizeof(VINFO_KEYWORD); i++)
		{
			if (strcmp(pkeyb + 1, g_vInfoKeyWord[i].key) == 0)
			{
				(*g_vInfoKeyWord[i].func)(tmp, sizeof(tmp));
				
				copylen = sizeof(finalContent) - (posContent - finalContent) - 1;
				copylen = copylen > strlen(tmp) ? strlen(tmp) : copylen;
				strncpy(posContent, tmp, copylen);
				posContent += copylen;
				
				break;
			}
		}
		*pkeye = '}';
		
		if (i == sizeof(g_vInfoKeyWord) / sizeof(VINFO_KEYWORD))
		{
			copylen = sizeof(finalContent) - (posContent - finalContent) - 1;
			copylen = copylen > (pkeye - pkeyb + 1) ? (pkeye - pkeyb + 1) : copylen;
			strncpy(posContent, pkeyb, copylen);
			posContent += copylen;
		}
		
		posBuf = pkeye + 1;
	}
	copylen = sizeof(finalContent) - (posContent - finalContent) - 1;
	copylen = copylen > (strlen(pbuf) - (posBuf - pbuf)) ? 
				(strlen(pbuf) - (posBuf - pbuf)) : copylen;
	if (copylen != 0)
	{
		strncpy(posContent, posBuf, copylen);
		posContent += copylen;
	}
	*posContent = '\0';
	
	copylen = strlen(finalContent) > (buflen - 1) ? (buflen - 1) : strlen(finalContent);
	strncpy(pbuf, finalContent, copylen);
	pbuf[copylen] = '\0';
}

void vinfoDrawContent(void *pYUVBuffer, VP_SIZE_T size)
{
	char content[128];
	FONT_DEVICE_T fd;
	FONT_RECT_T rc;
	FONT_SIZE_T sz;
	VINFO_DRAW_DEVICE device;
	int index;
	int i;
	
	for (i = 0; i < VP_INFO_CONTENT_ARRAY_SIZE; i++)
	{
		index = i;
		
		cyg_mutex_lock(&(g_InfoContent[index].mutex));
		if(g_InfoContent[index].bDraw == FALSE)
		{
			cyg_mutex_unlock(&(g_InfoContent[index].mutex));
			continue;
		}
		
		if (size.usWidth < 320)
			device.nStretchFactor = 1;
		else
			device.nStretchFactor = 2;
		
		device.pcYUVBuffer	= (char *) pYUVBuffer;
		device.size.usWidth	= size.usWidth / device.nStretchFactor;
		device.size.usHeight= size.usHeight / device.nStretchFactor;

		fontSetFont(&fd,
					 &g_tFont11x11Eng,
					 vinfoDrawYUV,
					 &device);
		
		strcpy(content, g_InfoContent[index].content);
		vinfoReplaceKeyWord(content, sizeof(content));
		
		fontGetSize(&fd, (UCHAR*)content, &sz);
		
		vinfoRegetXY(&g_InfoContent[index], size, sz);
		
		rc.nLeft = g_InfoContent[index].finalx;
		rc.nTop = g_InfoContent[index].finaly;
		
		rc.nWidth = (INT) device.size.usWidth - rc.nLeft - 1 - RECT_FRAME_SIZE;
		rc.nHeight = (INT) device.size.usHeight - rc.nTop - 1 - RECT_FRAME_SIZE;

		fontPrint(&fd,
				  (unsigned char *) content,
	              &rc,
	              &sz);
		
		cyg_mutex_unlock(&(g_InfoContent[index].mutex));
	}
}

void vinfoSetContent(int index, char *pcContent, int x, int y)
{
	int copylen;
	
	if (index < 0 || index >= VP_INFO_CONTENT_ARRAY_SIZE)
	{
		diag_printf("vinfoSetContent(): invalid index, it should be 0~%d\n", VP_INFO_CONTENT_ARRAY_SIZE);
		return;
	}
	
	cyg_mutex_lock(&(g_InfoContent[index].mutex));
	if (pcContent == NULL || strlen(pcContent) == 0)
	{
		g_InfoContent[index].bDraw = FALSE;
		cyg_mutex_unlock(&(g_InfoContent[index].mutex));
		return;
	}
	
	copylen = sizeof(g_InfoContent[index].content) - 1;
	copylen = copylen > strlen(pcContent) ? strlen(pcContent) : copylen;
	strncpy(g_InfoContent[index].content, pcContent, copylen);
	g_InfoContent[index].content[copylen] = '\0';
	
	/* Keep some space from the video border */
	if (x >= 0 && x < RECT_FRAME_SIZE)
	{
		x = RECT_FRAME_SIZE;
	}
	else if (x < 0 && x > -RECT_FRAME_SIZE)
	{
		x = -RECT_FRAME_SIZE;
	}
	
	if (y >= 0 && y < RECT_FRAME_SIZE)
	{
		y = RECT_FRAME_SIZE;
	}
	else if (y < 0 && y > -RECT_FRAME_SIZE)
	{
		y = -RECT_FRAME_SIZE;
	}
	g_InfoContent[index].x = x;
	g_InfoContent[index].y = y;
	g_InfoContent[index].bDraw = TRUE;
	
	cyg_mutex_unlock(&(g_InfoContent[index].mutex));
}

void vinfoClearContent(int index)
{
	if (index < 0 || index >= VP_INFO_CONTENT_ARRAY_SIZE)
	{
		diag_printf("vinfoSetContent(): invalid index, it should be 0~%d\n", VP_INFO_CONTENT_ARRAY_SIZE);
		return;
	}
	
	cyg_mutex_lock(&(g_InfoContent[index].mutex));
	g_InfoContent[index].bDraw = FALSE;
	cyg_mutex_unlock(&(g_InfoContent[index].mutex));
}

void vinfoInit(void)
{
	int i;
	
	for (i = 0; i < VP_INFO_CONTENT_ARRAY_SIZE; i++)
	{
		memset(&g_InfoContent[i], 0, sizeof(VINFO_DRAW_CONTENT));
		g_InfoContent[i].bDraw = FALSE;
		
		cyg_mutex_init(&(g_InfoContent[i].mutex));
	}
}