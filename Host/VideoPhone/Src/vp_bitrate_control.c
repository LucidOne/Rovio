#include "../Inc/inc.h"

__STATIC LinkTest *g_LinkTest = NULL;

/*
 * function:
 *		Calculate average net speed per net link
 * return:
 *		none
 */
__STATIC VOID vp_bitrate_control_linkstat(VOID)
{
	LIST_T *phead, *pnode;
	LinkSpeed *pLinkSpeed;
	
	INT counter		= 0;
	INT writebytes	= 0;
	
	phead = pnode = &(g_LinkTest->list);
	pnode = listGetNext(pnode);
	while ((pnode != NULL) && (pnode != phead))
	{
		pLinkSpeed = GetParentAddr(pnode, LinkSpeed, list);
		counter++;
		writebytes += pLinkSpeed->iWriteBytes;
		
		/* Reset write bytes */
		pLinkSpeed->iWriteBytes = 0;
		
		pnode = listGetNext(pnode);
	}
	
	g_LinkTest->linkspeed = ((float)writebytes / counter) / g_LinkTest->refreshInterval;
#ifdef VP_BITRATE_CONTROL_DEBUG
	diag_printf("vp_bitrate_control_linkstat(): speed %db/s(%dk/s)\n", g_LinkTest->linkspeed, g_LinkTest->linkspeed/1024);
#endif
}

/*
 * function:
 *		Change bitrate dynamically according to g_LinkTest->linkspeed
 * return:
 *		none
 */
void vcptGetWindow (USHORT *pusWidth, USHORT *pusHeight, CMD_ROTATE_E *pRotate);
__STATIC VOID vp_bitrate_control_change(VOID)
{
	/* To judge the net speed is slow, middle or fast */
	const INT netspeed[4][2] = 
	{
		{10*1024, 13*1024},		//176 * 144
		{15*1024, 20*1024},		//320 * 240
		{15*1024, 20*1024},		//352 * 288
		{15*1024, 20*1024},		//640 * 480
	};
	/* The bitrates for slow, middle or fast net speed */
	const INT aaBitrateTable[4][3] =
	{
		{64*1024,	128*1024,	192*1024},	//176 * 144
		{256*1024,	512*1024,	768*1024},	//320 * 240
		{256*1024,	512*1024,	768*1024},	//352 * 288
		{512*1024,	1024*1024,	2304*1024},	//640 * 480
	};
	/*
	const INT aaBitrateTable[4][3] =
	{
		{192*1024,	128*1024,	192*1024},	//176 * 144
		{768*1024,	512*1024,	768*1024},	//320 * 240
		{768*1024,	512*1024,	768*1024},	//352 * 288
		{2304*1024,	1024*1024,	2304*1024},	//640 * 480
	};
	*/
	/* The framerate for slow, middle or fast net speed */
	const FLOAT aaFramerateTable[3] = 
	{
		0.5, 1, 1
	};
	
	USHORT width, height;
	INT resolution, bitrate, framerate;
	INT i;
	
	vcptGetWindow(&width, &height, NULL);
	
	if ((width * height) <= (176*144))
	{
		resolution = 0;
	}
	else if ((width * height) <= (320*240))
	{
		resolution = 1;
	}
	else if ((width * height) <= (352*288))
	{
		resolution = 2;
	}
	else
	{
		resolution = 3;
	}
	
	for (i = 0; i < 2; i++)
	{
		if(netspeed[resolution][i] > g_LinkTest->linkspeed)
		{
			break;
		}
	}
	bitrate = i;
	
	if(g_LinkTest->bEnableBitrateChange == TRUE)
	{
		/* Adjust bitrate */
		wb702SetVideoDynamicBitrate(aaBitrateTable[resolution][bitrate]);
		
		/* Adjust framerate */
		vmp4encGetQuality((UINT32*)&framerate, NULL);
		framerate = framerate * aaFramerateTable[i];
		wb702SetVideoDynamicFramerate(framerate);
	}
}

/*
 * function:
 *		Check and remove for broken connections
 * return:
 *		TRUE	success
 *		FALSE	failed
 */
__STATIC BOOL vp_bitrate_control_linkrefresh(VOID)
{
	LIST_T *phead, *pnext, *pnode;
	LinkSpeed *pLinkSpeed;
	cyg_tick_count_t currentTime = cyg_current_time();
	
	phead = pnode = &(g_LinkTest->list);
	pnode = listGetNext(pnode);
	while ((pnode != NULL) && (pnode != phead))
	{
		pnext = listGetNext(pnode);
		
		pLinkSpeed = GetParentAddr(pnode, LinkSpeed, list);
		if (tt_ticks_to_msec(currentTime - pLinkSpeed->iLastWriteTime) / 1000 
			> g_LinkTest->refreshInterval)
		{
			listDetach(pnode);
			free(pLinkSpeed);
		}
		pnode = pnext;
	}
	return TRUE;
}

/* Use alarm to update net speed and change bitrate periodically */
#if 0
__STATIC VOID vp_bitrate_control_event(cyg_handle_t alarm, cyg_addrword_t data)
{
	cyg_mutex_lock(&(g_LinkTest->mutexHandle));
	
	vp_bitrate_control_linkstat();
	vp_bitrate_control_change();
	vp_bitrate_control_linkrefresh();
	
	cyg_mutex_unlock(&(g_LinkTest->mutexHandle));
}

__STATIC VOID vp_bitrate_control_createalarm(VOID)
{
	cyg_handle_t clock;
	cyg_handle_t counter;
	
	clock = cyg_real_time_clock();
	cyg_clock_to_counter(clock, &counter);
	
	cyg_alarm_create(counter, vp_bitrate_control_event, NULL, 
					&(g_LinkTest->alarmHandle), &(g_LinkTest->alarm));
	cyg_alarm_initialize(g_LinkTest->alarmHandle, 
						tt_msec_to_ticks(g_LinkTest->refreshInterval * 1000),
						tt_msec_to_ticks(g_LinkTest->refreshInterval * 1000));
}

__STATIC VOID vp_bitrate_control_deletealarm(VOID)
{
	cyg_alarm_delete(g_LinkTest->alarmHandle);
}
#endif

/* Use thread and sleep to update net speed and change bitrate periodically */
__STATIC VOID vp_bitrate_control_entry(cyg_addrword_t data)
{
	while(1)
	{
		tt_msleep(g_LinkTest->refreshInterval*1000);
		
		cyg_mutex_lock(&(g_LinkTest->mutexHandle));
		
		vp_bitrate_control_linkstat();
		vp_bitrate_control_change();
		vp_bitrate_control_linkrefresh();
		
		cyg_mutex_unlock(&(g_LinkTest->mutexHandle));
	}
}

__STATIC VOID vp_bitrate_control_createthread(VOID)
{
	cyg_thread_create(10, vp_bitrate_control_entry, NULL, "vp_bitrate_control thread", 
						g_LinkTest->threadBuffer, VP_BITRATE_CONTROL_BUFFERSIZE, 
						&g_LinkTest->threadHandle, &g_LinkTest->thread);
	cyg_thread_resume(g_LinkTest->threadHandle);
}

__STATIC VOID vp_bitrate_control_deletethread(VOID)
{
	cyg_thread_kill(g_LinkTest->threadHandle);
	cyg_thread_delete(g_LinkTest->threadHandle);
}

/*
 * function:
 *		Search the list node which is corresponding to the given connection
 * return:
 *		none null		success
 *		null			failed
 */
__STATIC LinkSpeed *vp_bitrate_control_linksearch(INT fd)
{
	LIST_T *phead, *pnode;
	LinkSpeed *pLinkSpeed;
	
	phead = pnode = &(g_LinkTest->list);
	pnode = listGetNext(pnode);
	while ((pnode != NULL) && (pnode!= phead))
	{
		pLinkSpeed = GetParentAddr(pnode, LinkSpeed, list);
		if (fd == pLinkSpeed->fd)
		{
			return pLinkSpeed;
		}
		pnode = listGetNext(pnode);
	}
	return NULL;
}

/*
 * function:
 *		Add a new connection
 * return:
 *		TRUE	success
 *		FALSE	failed
 */
__STATIC BOOL vp_bitrate_control_linkadd(INT fd, INT writebytes)
{
	LinkSpeed *pLinkSpeed;
	
	pLinkSpeed = malloc(sizeof(LinkSpeed));
	if (pLinkSpeed == NULL)
	{
		diag_printf("vp_bitrate_control_init(): malloc failed!!!\n");
		return FALSE;
	}
	
	pLinkSpeed->fd = fd;
	pLinkSpeed->iWriteBytes = writebytes;
	pLinkSpeed->iLastWriteTime = cyg_current_time();
	listInit(&(pLinkSpeed->list));
	
	listAttach(&(g_LinkTest->list), &(pLinkSpeed->list));
	
	return TRUE;
}

/*
 * function:
 *		Update the connection status for given fd
 * return:
 *		TRUE	success
 *		FALSE	failed
 */
__STATIC BOOL vp_bitrate_control_linkupdate(INT fd, INT writebytes)
{
	LinkSpeed *pLinkSpeed;
	
	pLinkSpeed = vp_bitrate_control_linksearch(fd);
	if (pLinkSpeed != NULL)
	{
		ASSERT(pLinkSpeed->fd == fd);
		pLinkSpeed->iWriteBytes += writebytes;
		pLinkSpeed->iLastWriteTime = cyg_current_time();
	}
	else
	{
		return vp_bitrate_control_linkadd(fd, writebytes);
	}
	
	return TRUE;
}

BOOL vp_bitrate_control_init(INT refreshInterval)
{
	if(g_LinkTest != NULL)
	{
		diag_printf("vp_bitrate_control_init(): vp_bitrate_control has not been uninited!!!\n");
		return FALSE;
	}
	
	g_LinkTest = malloc(sizeof(LinkTest));
	if (g_LinkTest == NULL)
	{
		diag_printf("vp_bitrate_control_init(): malloc failed!!!\n");
		return FALSE;
	}
	
	listInit(&(g_LinkTest->list));
	g_LinkTest->linkspeed = 0;
	g_LinkTest->bEnableBitrateChange = FALSE;
	g_LinkTest->refreshInterval = refreshInterval;
	if(g_LinkTest->refreshInterval <= 0)
	{
		g_LinkTest->refreshInterval = VP_BITRATE_CONTROL_TIMEOUT;
	}
	
	cyg_mutex_init(&(g_LinkTest->mutexHandle));
	
	//vp_bitrate_control_createalarm();
	vp_bitrate_control_createthread();
	
	return TRUE;
}

BOOL vp_bitrate_control_uninit(VOID)
{
	LIST_T *phead, *pnext, *pnode;
	LinkSpeed *pLinkSpeed;
	
	if(g_LinkTest == NULL)
	{
		diag_printf("vp_bitrate_control_uninit(): vp_bitrate_control has not been inited!!!\n");
		return FALSE;
	}
	
	/* Wait for no operations */
	cyg_mutex_lock(&(g_LinkTest->mutexHandle));
	
	//vp_bitrate_control_deletealarm();
	vp_bitrate_control_deletethread();
	
	phead = pnode = &(g_LinkTest->list);
	pnode = listGetNext(pnode);
	while ((pnode != NULL) && (pnode != phead))
	{
		pnext = listGetNext(pnode);
		
		listDetach(pnode);
		pLinkSpeed = GetParentAddr(pnode, LinkSpeed, list);
		free(pLinkSpeed);
		
		pnode = pnext;
	}
	
	/* Destroy mutex, so don't need to unlock */
	cyg_mutex_destroy(&(g_LinkTest->mutexHandle));
	
	free(g_LinkTest);
	g_LinkTest = NULL;
	
	return TRUE;
}

VOID vp_bitrate_control_write(INT fd, INT writebytes)
{
	if(g_LinkTest == NULL)
	{
		diag_printf("vp_bitrate_control_write(): vp_bitrate_control has not been inited!!!\n");
		return;
	}
	
	cyg_mutex_lock(&(g_LinkTest->mutexHandle));
	vp_bitrate_control_linkupdate(fd, writebytes);
	cyg_mutex_unlock(&(g_LinkTest->mutexHandle));
}

INT vp_bitrate_control_getspeed(VOID)
{
	INT speed;
	
	if(g_LinkTest == NULL)
	{
		diag_printf("vp_bitrate_control_getspeed(): vp_bitrate_control has not been inited!!!\n");
		return 0;
	}
	
	cyg_mutex_lock(&(g_LinkTest->mutexHandle));
	speed = g_LinkTest->linkspeed;
	cyg_mutex_unlock(&(g_LinkTest->mutexHandle));
	
	return speed;
}

VOID vp_bitrate_control_change_enable(BOOL bEnable)
{
	cyg_mutex_lock(&(g_LinkTest->mutexHandle));
	g_LinkTest->bEnableBitrateChange = TRUE;
	cyg_mutex_unlock(&(g_LinkTest->mutexHandle));
}
