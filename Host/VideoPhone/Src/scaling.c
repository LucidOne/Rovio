#include "../Inc/inc.h"


static UINT32 __gcd (UINT32 a, UINT32 b)
{
    while (1)
    {
		if (a == 0)
			return b;
        if (b == 0)
        	return a;
        if (a > b)
            a = a % b;
        else
            b = b % a;
    }
}


static BOOL
__IsScalingOK (UINT32 uOldSize, UINT32 uNewSize, UINT32 uMaxScalingFactor)
{
	USHORT usGCD = __gcd (uOldSize, uNewSize);
	if (usGCD == 0)
		return FALSE;
	
	return (uOldSize / usGCD <= uMaxScalingFactor
		&& uNewSize / usGCD <= uMaxScalingFactor) ? TRUE : FALSE;		
}


/*
	Evaluate the possible new scaling size.
	Parameters:
		uOldSize: old size
		uNewSize: new size wanted
		uMaxScalingFactor: max scaling factor
		nTolerance: max tolerance for new size
	Return:
		The new size that can be scaled.
		0 indicates evaluation failed.
 */
UINT32
EvaluateScalingSize (UINT32 uOldSize, UINT32 uNewSize, UINT32 uMaxScalingFactor, INT nTolerance)
{
	INT i;
	if (nTolerance >= 0)
	{
		for (i = nTolerance; i >= 0; i--)
		{
			if (__IsScalingOK (uOldSize, uNewSize, uMaxScalingFactor))
				return uNewSize;
			uNewSize++;
		}
	}
	else
	{
		for (i = nTolerance; i <= 0; i++)
		{
			if (__IsScalingOK (uOldSize, uNewSize, uMaxScalingFactor))	
				return uNewSize;
			uNewSize--;
		}
	}
	return 0;	//Failed
}

