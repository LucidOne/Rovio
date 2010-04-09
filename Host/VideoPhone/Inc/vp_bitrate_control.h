#ifndef __VP_BITRATE_CONTROL_H__
#define __VP_BITRATE_CONTROL_H__

//#define VP_BITRATE_CONTROL_DEBUG

#ifdef VP_BITRATE_CONTROL_DEBUG
#define __STATIC
#else
#define __STATIC static
#endif

#define VP_BITRATE_CONTROL_TIMEOUT 5

#define VP_BITRATE_CONTROL_BUFFERSIZE (32*1024)

typedef struct LinkTest
{
	cyg_handle_t	threadHandle;
	cyg_thread		thread;
	CHAR			threadBuffer[VP_BITRATE_CONTROL_BUFFERSIZE];
	
	cyg_mutex_t		mutexHandle;
	
	cyg_handle_t	alarmHandle;
	cyg_alarm		alarm;
	
	BOOL			bEnableBitrateChange;
	INT				refreshInterval;	/* seconds to update link stat */
	
	INT				linkspeed;			/* bytes per second */
	
	LIST_T			list;
}LinkTest;

typedef struct LinkSpeed
{
	INT fd;
	INT iWriteBytes;
	INT iLastWriteTime;	/* This is for link valid judgement */
	
	LIST_T list;
}LinkSpeed;

BOOL vp_bitrate_control_init(INT refreshInterval);
BOOL vp_bitrate_control_uninit(VOID);
VOID vp_bitrate_control_write(INT fd, INT writebytes);
INT vp_bitrate_control_getspeed(VOID);
VOID vp_bitrate_control_change_enable(BOOL bEnable);

#ifdef VP_BITRATE_CONTROL_DEBUG
VOID vp_bitrate_control_linkstat(VOID);
VOID vp_bitrate_control_change(VOID);
BOOL vp_bitrate_control_linkrefresh(VOID);

VOID vp_bitrate_control_event(cyg_handle_t alarm, cyg_addrword_t data);
VOID vp_bitrate_control_createalarm(VOID);
VOID vp_bitrate_control_deletealarm(VOID);

VOID vp_bitrate_control_entry(cyg_addrword_t data);
VOID vp_bitrate_control_createthread(VOID);
VOID vp_bitrate_control_deletethread(VOID);

LinkSpeed *vp_bitrate_control_linksearch(INT fd);
BOOL vp_bitrate_control_linkadd(INT fd, INT writebytes);
BOOL vp_bitrate_control_linkupdate(INT fd, INT writebytes);
#endif


#endif
