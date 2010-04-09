#include "../../Platform/Inc/Platform.h"
#include "../../SoftPipe/include/softpipe.h"
#include "../Inc/HIC.h"


#define POWER_PWRSAV	0x01
#define POWER_REG_XHG	0x02
#define POWER_S2PWREN	0x04
#define POWER_LCM1		0x08
#define POWER_CLKEN		0x10
#define POWER_RSTEN		0x20
#define POWER_DF_TRANS	0xC0


BOOL wbhicIsPowerSavingMode (VOID)
{
	UCHAR ucTmp;
	ucTmp = CF_POWERCON;
	return (ucTmp & POWER_PWRSAV) == 0 ? FALSE : TRUE;
}


BOOL wbhicEnterPowerSavingMode (VOID)
{
	UINT32 auGPIO_REGS[6];
	UCHAR ucTmp;

	D_TRACE(0x0000, "wbhicEnterPowerSavingMode\n");

	if (wbhicIsPowerSavingMode ())
		return TRUE;	/* Already in power saving mode. */

	/* backup the GPIO registers */
	/* wbhicBurstRead (REG_GPIO_OE, sizeof (auGPIO_REGS), (VOID *) auGPIO_REGS);
	   Burst reading does not apply endian swapping in big-endian system!
	*/
	if (!wbhicSingleRead (REG_GPIO_OE, &auGPIO_REGS[0]))
		return FALSE;
	if (!wbhicSingleRead (REG_GPIO_DAT, &auGPIO_REGS[1]))
		return FALSE;
	if (!wbhicSingleRead (REG_GPIO_IE, &auGPIO_REGS[3]))
		return FALSE;
	if (!wbhicSingleRead (REG_GPIO_PE, &auGPIO_REGS[5]))
		return FALSE;

	/* read power control port */
	ucTmp = CF_POWERCON;
	sysMSleep (10);

	/* re-define Host interface ports */
	ucTmp |= POWER_PWRSAV;
	CF_POWERCON = ucTmp;
	sysMSleep (10);
	ucTmp = CF_POWERCON;

#ifdef _LCM_18BIT_HW16
	/* Set LCD DFTrans callback function. */
	wbhicLCDDFTrans_UsePowerSaving (TRUE);
#endif

	/* control the GPIO register from section 1 */
	CF_GPIOOE	= auGPIO_REGS[0];
	CF_GPIODATA	= auGPIO_REGS[1];
	CF_GPIOIE	= auGPIO_REGS[3];
	CF_GPIOPE	= auGPIO_REGS[5];

	/* Change the GPIO control to section 1 */
	ucTmp |= POWER_REG_XHG;
	CF_POWERCON = ucTmp;
	sysMSleep (10);
	ucTmp = CF_POWERCON;

	/* close section 2 power & hold CPU is reset state */
	ucTmp &= ~ (UCHAR) POWER_S2PWREN;
	ucTmp |= POWER_RSTEN;
	CF_POWERCON = ucTmp;
	sysMSleep (10);
	ucTmp = CF_POWERCON;

	return TRUE;
}

#define P do {sysPrintf ("File: %s %d\n", __FILE__, __LINE__); } while (0)
BOOL wbhicEnterPowerNormalMode (VOID)
{
	UINT32 auGPIO_REGS[6];
	UCHAR ucTmp;

	D_TRACE(0x0000, "WBHIC_EnterNormalMode\n");

	if (! wbhicIsPowerSavingMode ())
		return TRUE;	/* Already in power normal mode. */
P;sysPrintf ("CON: %x\n", (int) CF_POWERCON);
	/* read HIC GPIO port */
	auGPIO_REGS[0] = CF_GPIOOE;
	auGPIO_REGS[1] = CF_GPIODATA;
	auGPIO_REGS[3] = CF_GPIOIE;
	auGPIO_REGS[5] = CF_GPIOPE;
P;sysPrintf ("CON: %x\n", (int) CF_POWERCON);
	/* read HIC power control port */
	ucTmp = CF_POWERCON;
	sysMSleep(10);

	/* Turn on section 2 power but host the CPU is reset state */
	ucTmp |= POWER_S2PWREN;
	ucTmp |= POWER_RSTEN;
	CF_POWERCON = ucTmp;
	sysMSleep (10);
	ucTmp = CF_POWERCON;
P;sysPrintf ("CON: %x\n", (int) CF_POWERCON);
	/* re-define the host interface port */
	ucTmp &= ~ (UCHAR) POWER_RSTEN;
	ucTmp &= ~ (UCHAR) POWER_PWRSAV;
	CF_POWERCON = ucTmp;
	sysMSleep (10);
	ucTmp = CF_POWERCON;

#ifdef _LCM_18BIT_HW16
	/* Set LCD DFTrans callback function. */
	wbhicLCDDFTrans_UsePowerSaving (FALSE);
#endif
P;sysPrintf ("CON: %x\n", (int) CF_POWERCON);
	/* Change the GPIO control back to section 2 */
	ucTmp &= ~ (UCHAR) POWER_REG_XHG;
P;sysPrintf ("CON: %x %x\n", (int) CF_POWERCON, (int) ucTmp);
	CF_POWERCON = ucTmp;
P;sysPrintf ("CON: %x\n", (int) CF_I_REG7);
P;sysPrintf ("CON: %x\n", (int) CF_I_REG7);
	sysMSleep (10);
P;sysPrintf ("CON: %x\n", (int) CF_I_REG7);
P;sysPrintf ("CON: %x\n", (int) CF_I_REG7);
	ucTmp = CF_I_REG7;
P;sysPrintf ("CON: %x\n", (int) CF_I_REG7);
P;sysPrintf ("CON: %x\n", (int) CF_I_REG7);
	sysMSleep (10);
P;sysPrintf ("CON: %x\n", (int) CF_I_REG7);
P;sysPrintf ("CON: %x\n", (int) CF_I_REG7);

	/* HIC is i8 after change GPIO back to section 2 */
	if (!wbhicInit ())
		return FALSE;
P;
	/* restore the GPIO content */
	if (!wbhicSingleWrite (REG_GPIO_OE, auGPIO_REGS[0]))
		return FALSE;
	if (!wbhicSingleWrite (REG_GPIO_DAT, auGPIO_REGS[1]))
		return FALSE;
	if (!wbhicSingleWrite (REG_GPIO_IE, auGPIO_REGS[3]))
		return FALSE;
	if (!wbhicSingleWrite (REG_GPIO_PE, auGPIO_REGS[5]))
		return FALSE;
P;
	return TRUE;
}
