#ifndef __ANY_PLATFORM_H__
#define __ANY_PLATFORM_H__


#if defined PLATFORM_ADS_W90N740
#	include "../ads_W90N740/Inc/Platform.h"
#elif defined PLATFORM_GCC_W90N740
#	include "../gcc_W90N740/Inc/Platform.h"
#else
#	error "No platform defined."
#endif


/* To make a porting for other platform,
   replace the following marios with your own.
   The check the functions like "sysXXX", if it is called by the demo
   or by other libs, it's porting version must also be given.
 */



/* Memory mapping address for windbond chip. */
#define REG_INDEX8(reg)		PLATFROM_DEFINE_REG_INDEX8(reg)
#define REG_INDEX16(reg)	PLATFROM_DEFINE_REG_INDEX16(reg)
#define REG_INDEX32(reg)	PLATFROM_DEFINE_REG_INDEX32(reg)

#define REG_A8(reg)		PLATFROM_DEFINE_REG_A8(reg)
#define REG_A16(reg)	PLATFROM_DEFINE_REG_A16(reg)
#define REG_A32(reg)	PLATFROM_DEFINE_REG_A32(reg)



//Initialize platform.
BOOL sysInitialize (VOID);
//Printf
INT sysPrintf (CONST CHAR *cpcFormat, ...);
//Get character
INT sysGetChar (VOID);
//Ticks since booting, 1 millisecond per tick.
UINT32 sysGetTickCount (VOID);
//Sleep some millisecond.
VOID sysMSleep(UINT32 uMilliSecond);

//Memory allocation
VOID *sysMalloc(size_t szSize);
VOID *sysRealloc(VOID *pMem, size_t szSize);
VOID sysFree(VOID *pMem);

//Hardware reset
VOID sysResetLcd(VOID);	//Reset LCD
VOID sysResetWbc(VOID);	//Reset winbond chip

//Interrupt from winbond clip
VOID sysDisableWbcIsr (VOID);
VOID sysEnableWbcIsr (VOID);
PVOID sysInstallWbcIsr (PVOID fnNewISR);

//Lock
VOID sysLockWbc (VOID);
VOID sysUnlockWbc (VOID);

//Waiting object
typedef struct
{
	PLATFORM_WAIT_OBJ_T sWaitObj;
} SYS_WAIT_OBJ_T;
//Initialize the "wait object".
VOID sysInitWaitObj (SYS_WAIT_OBJ_T *pWaitObj);
//Wait on the "wait object",
//return 0 for waiting ok,
//return -1 for timeout.
INT sysWait (SYS_WAIT_OBJ_T *pWaitObj, UINT32 uTimeout_MSec);
//Wake up the "wait object".
VOID sysWakeup (SYS_WAIT_OBJ_T *pWaitObj);




//Debug macros, you can define them to nothing.
#define D_TRACE(code,str)
#define D_WARN(str) do {sysPrintf("%s,%d,(W) %s", __FILE__, __LINE__, (str));} while (0)
#define D_ERROR(str) do {sysPrintf("%s,%d,(E) %s", __FILE__, __LINE__, (str));} while (0)
#define ASSERT(exp)	assert(exp)

#endif


