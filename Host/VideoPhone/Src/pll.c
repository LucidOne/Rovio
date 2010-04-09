#include "../Inc/inc.h"

/* 
	evaluate REG_UPLLCON or REG_APLLCON automatically.
	xhchen Jan 20 2006
 */

static UINT32 pllCalcFOUT (UINT32 uFIN, UINT32 uPLLCON)
{
	//UINT32 uSRC	= (uPLLCON & 0x00040000) >> 18;
	//UINT32 uOE	= (uPLLCON & 0x00020000) >> 17;
	//UINT32 uBP	= (uPLLCON & 0x00010000) >> 16;
	//UINT32 uPD	= (uPLLCON & 0x00008000) >> 15;
	UINT32 uOUT_DV;
	UINT32 uIN_DV;
	UINT32 uFB_DV;
	UINT32 uNR;
	UINT32 uNF;
	UINT32 uNO;
	UINT64 u64FOUT;

	uOUT_DV	= (uPLLCON & 0x00006000) >> 13;	/* [0, 3] */
	uIN_DV	= (uPLLCON & 0x00001F00) >> 8;		/* [0, 31] */
	uFB_DV	= (uPLLCON & 0x000000FF) >> 0;		/* [0, 255] */
	
	if (uOUT_DV == 0 || uIN_DV == 0 || uFB_DV == 0)
		return 0;	//PLL value error.
	
	uNR	= uIN_DV;				/* 1, 2, 3, ..., 31 */
	uNF	= 2 * uFB_DV;			/* 2, 4, ..., 510 */
	uNO	= 1 << (uOUT_DV - 1);	/* 1, 2, 4 */

	u64FOUT	= uFIN * (UINT64) uNF / uNR / uNO;
	if ((u64FOUT >> 32) != (UINT64) 0)
		return 0UL;
	else
		return (UINT32) u64FOUT;
}


static __inline UINT32 pllPatchPLLCON (UINT32 uOUT_DV, UINT32 uIN_DV, UINT32 uFB_DV)
{
	UINT32 uPLLCON = ((uOUT_DV << 13UL) & 0x00006000UL) 
		| ((uIN_DV << 8UL) & 0x00001F00UL)
		| ((uFB_DV << 0UL) & 0x000000FFUL);
	return uPLLCON;
}

#define MATH_ABS(a,b)	((a) > (b) ? (a) - (b) : (b) - (a))
static void pllTry (UINT32 uFIN, UINT32 uFOUT, UINT32 uOUT_DV, UINT32 uIN_DV,
		 UINT32 *puPLLCON, UINT32 *puFOUT_new,
		 UINT32 uMinFCO, UINT32 uMaxFCO)
{
	UINT32 uNR;
	UINT32 uNO;
	UINT64 u64NF;
	UINT64 u64FB_DV;
	UINT32 uPLLCON;
	UINT32 uFOUT_new;

	*puPLLCON		= 0UL;
	*puFOUT_new	= 0UL;
	
	uNR = uIN_DV;				/* 1, 2, 3, ..., 31 */
	if (!(2000000UL * uNR < uFIN && uFIN < 8000000UL * uNR))
		return;
	
	uNO = 1UL << (uOUT_DV - 1);	/* 1, 2, 4 */
	//if (!(200000000 < FOUT * NO && FOUT * NO < 400000000))
	if (!(uMinFCO < uFOUT * uNO && uFOUT * uNO < uMaxFCO))
		return;

	u64NF = uFOUT * (UINT64) uNR * uNO / uFIN;		/* 2, 4, ..., 510 */
	u64FB_DV = u64NF / 2UL;

	if ((UINT64) 1 <= u64FB_DV && u64FB_DV <= (UINT64) 255)
	{
		uPLLCON = pllPatchPLLCON (uOUT_DV, uIN_DV, (UINT32) u64FB_DV);
		uFOUT_new = pllCalcFOUT (uFIN, uPLLCON);

		if (MATH_ABS (uFOUT_new, uFOUT) <= MATH_ABS (*puFOUT_new, uFOUT))
		{
			*puPLLCON		= uPLLCON;
			*puFOUT_new	= uFOUT_new;
		}
	}
	
	u64FB_DV++;

	if ((UINT64) 1 <= u64FB_DV && u64FB_DV <= (UINT64) 255)
	{
		uPLLCON = pllPatchPLLCON (uOUT_DV, uIN_DV, (UINT32) u64FB_DV);
		uFOUT_new = pllCalcFOUT (uFIN, uPLLCON);

		if (MATH_ABS (uFOUT_new, uFOUT) <= MATH_ABS (*puFOUT_new, uFOUT))
		{
			*puPLLCON		= uPLLCON;
			*puFOUT_new	= uFOUT_new;
		}	
	}
}


static UINT32 __pllEvalPLLCON (UINT32 uFIN, UINT32 uFOUT, UINT32 uMinFCO, UINT32 uMaxFCO)
{
	UINT32 uIN_DV;
	UINT32 uOUT_DV;
	UINT32 uPLLCON = 0;
	UINT32 uFOUT_new = 0;

	/* Try PLLCON values and select the most exactly matched value. */
	for (uOUT_DV = 3; uOUT_DV >= 1; uOUT_DV--)
	{
		for (uIN_DV = 31; uIN_DV >= 1; uIN_DV--)
		{
			UINT32 uPLLCON1;
			UINT32 uFOUT_new1;
			pllTry (uFIN , uFOUT, uOUT_DV, uIN_DV, &uPLLCON1, &uFOUT_new1,
				uMinFCO, uMaxFCO);
			if (MATH_ABS (uFOUT_new1, uFOUT) <= MATH_ABS (uFOUT_new, uFOUT))
			{
				uPLLCON		= uPLLCON1;
				uFOUT_new	= uFOUT_new1;
			}
		}
	}

	return uPLLCON;
}


#ifdef DEBUG_DUMP_PLL
static int pllDump (UINT32 uFIN, UINT32 uPLLCON)
{
	UINT32 uOUT_DV;
	UINT32 uIN_DV;
	UINT32 uFB_DV;
	UINT32 uNR;
	UINT32 uNF;
	UINT32 uNO;
	UINT64 u64FOUT;

	uOUT_DV	= (uPLLCON & 0x00006000) >> 13;	/* [0, 3] */
	uIN_DV	= (uPLLCON & 0x00001F00) >> 8;		/* [0, 31] */
	uFB_DV	= (uPLLCON & 0x000000FF) >> 0;		/* [0, 255] */
	
	if (uOUT_DV == 0 || uIN_DV == 0 || uFB_DV == 0)
	{
		sysSafePrintf ("Error UPLLCON: %08x\n", uPLLCON);
		return -1;	//PLL value error.
	}
	
	uNR	= uIN_DV;				/* 1, 2, 3, ..., 31 */
	uNF	= 2 * uFB_DV;			/* 2, 4, ..., 510 */
	uNO	= 1 << (uOUT_DV - 1);	/* 1, 2, 4 */

	
	sysSafePrintf ("PLLCON=0x%08x NF=%d NR=%d NO=%d (FOUT=%d)\n", uPLLCON,
		uNF, uNR, uNO,
		pllCalcFOUT (uFIN, uPLLCON));
	u64FOUT	= uFIN * (UINT64) uNF / uNR / uNO;

	sysSafePrintf ("FIN/NR = %dK (2000K, 8000K)\n", uFIN / uNR / 1000);
	sysSafePrintf ("FCO=FIN*NF/NR= %dM (200M, 400M)\n", (UINT32) (uFIN * (UINT64) uNF / uNR / 1000 / 1000));
	
	return 0;
}
#endif



UINT32 pllEvalPLLCON (UINT32 uFIN, UINT32 uFOUT)
{
	UINT32 uPLLCON;
	
	/*
	    As defined in W99702 spec, the range of "FIN * NF / NR" is:
	        200M < FCO < 800M
	    but sometime we can not evaluate PLLCON in this range, so
	    we also try some value out of the range defined in spec.
	 */
	uPLLCON = __pllEvalPLLCON (uFIN, uFOUT, 200000000UL, 400000000UL);

	if (uPLLCON == 0UL)
		uPLLCON = __pllEvalPLLCON (uFIN, uFOUT, 190000000UL, 200000001UL);
	if (uPLLCON == 0UL)
		uPLLCON = __pllEvalPLLCON (uFIN, uFOUT, 400000000UL, 410000001UL);

	if (uPLLCON == 0UL)
		uPLLCON = __pllEvalPLLCON (uFIN, uFOUT, 180000000UL, 190000001UL);
	if (uPLLCON == 0UL)
		uPLLCON = __pllEvalPLLCON (uFIN, uFOUT, 410000000UL, 420000001UL);

	if (uPLLCON == 0UL)
		uPLLCON = __pllEvalPLLCON (uFIN, uFOUT, 170000000UL, 180000001UL);
	if (uPLLCON == 0UL)
		uPLLCON = __pllEvalPLLCON (uFIN, uFOUT, 430000000UL, 440000001UL);


	if (uPLLCON == 0UL)
	{
#if 1
#ifndef ECOS
		dbgSetError (APP_ERR_CANNOT_SET_PLL, "Can not set pll for: FIN=0x%08x, FOUT=%08x\n", uFIN, uFOUT);
#else
		printf ("Can not set pll for: FIN=0x%08x, FOUT=%08x\n", uFIN, uFOUT);
#endif
		sysDisableIRQ();
		while (1);	//System halt
#endif
		uPLLCON = 0x0000E220;	//Default value.
	}
	
#ifdef DEBUG_DUMP_PLL
	pllDump (uFIN, uPLLCON);
#endif
	return uPLLCON;
}


