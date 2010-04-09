#ifndef __SYSLIB_PORTABLE_H__
#define __SYSLIB_PORTABLE_H__

#include "wb_syslib_addon.h"
#include "../../SysLib/Inc/wblib.h"
#include "../../SysLib/Inc/w99702_reg.h"

#define GE_ERR_ID		0xFFFF0800	/* 2D graphics library ID */
#define APP_ERR_ID		0xFFFFB000	/* Application defined ID */


#ifndef NON_CACHE
#define NON_CACHE(addr)		((int) (addr) | 0x10000000UL)
#endif
#ifndef SET_CACHE
#define SET_CACHE(addr)		((int) (addr) & ~0x10000000UL)
#endif


#define PSR_MODE_MASK	0x1F
#define PSR_MODE_FIQ	0x11
#define PSR_MODE_IRQ	0x12
#define PSR_IRQ_ENABLE	0x80
#define PSR_FIQ_ENABLE	0x40


extern int	g_iIRQ_disable_count;
extern BOOL	g_bIRQ_real_disable;


__inline BOOL sysIsInIRQ (void)
{
	int tmp;
	__asm
	{
		MRS tmp, CPSR;
	}
	tmp &= PSR_MODE_MASK;
	
	if (tmp == PSR_MODE_IRQ)
		return TRUE;
	else
		return FALSE;
}


__inline BOOL sysIsIRQDisabled (void)
{
	int tmp;
	__asm
	{
		MRS tmp, CPSR
	}
	if ((tmp & PSR_IRQ_ENABLE) == 0)
		return FALSE;
	else
		return TRUE;
}


__inline void sysEnableIRQ (void)
{
	cyg_interrupt_enable();
}



__inline void sysDisableIRQ (void)
{
	cyg_interrupt_disable();
}


__inline void sysRegSetBit (UINT32 uReg, UINT32 uBitVal)
{
	UINT32 uValue;
	cyg_interrupt_disable();
	uValue = inpw(uReg);
	uValue |= uBitVal;
	outpw(uReg, uValue);
	cyg_interrupt_enable();
}


__inline void sysRegClrBit (UINT32 uReg, UINT32 uBitVal)
{
	UINT32 uValue;
	cyg_interrupt_disable();
	uValue = inpw(uReg);
	uValue &= ~uBitVal;
	outpw(uReg, uValue);
	cyg_interrupt_enable();
}


#ifdef DEBUG_DUMP_IRQ
__inline void sysDumpIRQ (void)
{
	sysprintf ("IRQ level: %d\n", g_iIRQ_disable_count);
}
#endif


#if !defined NO_PRINTF
#define sysSafePrintf(...) \
do \
{ \
	sysDisableIRQ (); \
	sysprintf(__VA_ARGS__); \
	sysEnableIRQ (); \
} while (0)
#else
__inline void __NoPrintf(char *fmt, ...)
{
}
#define sysSafePrintf(...) do {__NoPrintf(__VA_ARGS__);} while (0)
//#define sysSafePrintf(...) do {; } while (0)
#endif



#define ASSERT(expr) \
if (!(expr)) \
{ \
	sysSafePrintf ("Assert failed in line %d (%s):\n    %s\n", __LINE__, __FILE__, #expr); \
	sysDisableIRQ (); \
	while (1); \
}

#define PTL \
do \
{ \
	sysSafePrintf ("In %s (%d)\n", __func__, __LINE__); \
} \
while (0)

#define ptl \
do \
{ \
	sysDisableIRQ (); \
	sysprintf ("In %s (%d)\n", __func__, __LINE__); \
	sysEnableIRQ (); \
} \
while (0)


#endif


