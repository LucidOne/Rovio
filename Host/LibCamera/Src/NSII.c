#include "../../Inc/CommonDef.h"

#if defined IPCAM_CONFIG_IP_CAM_VER_1 || defined IPCAM_CONFIG_IP_CAM_VER_2 || defined IPCAM_CONFIG_IP_CAM_VER_3
void ERSP_rovio_libns_thread_resume();
void ERSP_rovio_libns_thread_suspend();

#define NS_GPIO	0x40000000UL

void nsSuspend()
{
	ERSP_rovio_libns_thread_suspend();

	outpw(REG_GPIOB_OE,inpw(REG_GPIOB_OE) & ~NS_GPIO);		//GPIOB 30 output;
    outpw(REG_GPIOB_DAT,inpw(REG_GPIOB_DAT) & ~NS_GPIO);	//GBIOB 30 low	
}

void nsWakeup()
{
	outpw(REG_GPIOB_OE,inpw(REG_GPIOB_OE) & ~NS_GPIO);		//GPIOB 30 output;
    outpw(REG_GPIOB_DAT,inpw(REG_GPIOB_DAT) | NS_GPIO);	//GBIOB 30 high	

	ERSP_rovio_libns_thread_resume();
}
#elif defined IPCAM_CONFIG_IP_CAM_VER_0 || defined IPCAM_CONFIG_MP4_EVB_VER_0 || defined IPCAM_CONFIG_MP4_EVB_VER_1
void nsSuspend()
{
}

void nsWakeup()
{
}
#else
#	error "No hardware config defined!"
#endif

