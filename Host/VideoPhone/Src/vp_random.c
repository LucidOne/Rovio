#include "../Inc/inc.h"

#ifndef ECOS
static BOOL g_bSetSeed = FALSE;

void vpSrand (UINT32 uValue0,
			  UINT32 uValue1,
			  UINT32 uValue2,
			  UINT32 uValue3)
{
	if (g_bSetSeed == FALSE)
	{
		srand (uValue0 + uValue1 + uValue2 + uValue3);
		g_bSetSeed = TRUE;
	}
}


UINT32 vpRand (void)
{
	return rand ();
}

#endif
