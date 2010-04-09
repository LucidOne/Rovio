#ifndef __WAIT_IRQ_H__
#define __WAIT_IRQ_H__


#ifndef GetParentAddr
#define GetParentAddr(pMe,tParent,eMyName) \
	((tParent *)((char *)(pMe) - (int)&((tParent *)0)->eMyName))
#endif


#define OS_SUPPORT
//#undef OS_SUPPORT

#ifdef OS_SUPPORT
typedef struct
{
	HIC_WAIT_IRQ_OBJ_T waitObj;
	cyg_flag_t flagWakeup;
} MY_WAIT_OBJ_T;

#else
typedef struct
{
	HIC_WAIT_IRQ_OBJ_T waitObj;
	BOOL volatile bWakeup;
} MY_WAIT_OBJ_T;

#endif


void myWait (HIC_WAIT_IRQ_OBJ_T *pWaitIrqObj);
void myWakeup (HIC_WAIT_IRQ_OBJ_T *pWaitIrqObj);
void myMSleep (HIC_WAIT_IRQ_OBJ_T *pWaitIrqObj, UINT32 uMilliSecond);
void myInitWaitObj (MY_WAIT_OBJ_T *pMyWaitObj);


#endif
