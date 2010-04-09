#include "../Inc/inc.h"


typedef struct
{
	int					nRefCount;
	TT_RMUTEX_T			mtLock;
} VP_MPGFX_T;

#pragma arm section zidata = "non_init"
static VP_MPGFX_T g_vpMyGFX;
#pragma arm section zidata



void vgfxInit (void)
{
	tt_rmutex_init (&g_vpMyGFX.mtLock);
	g_vpMyGFX.nRefCount = 0;
	//vgfxAddRef ();
}


static void vgfxAddRef (void)
{
	if (g_vpMyGFX.nRefCount == 0)
	{
		//TURN_ON_GFX_CLOCK;
		
		{
			GFX_INFO_T gfxinfo;
			gfxinfo.nDestWidth		= 0;
			gfxinfo.nDestHeight		= 0;
			gfxinfo.nDestPitch		= 0;
			gfxinfo.nSrcWidth		= 0;
			gfxinfo.nSrcHeight		= 0;
			gfxinfo.nSrcPitch		= 0;
#if MAX_LCM_BYTES_PER_PIXEL == 2
			gfxinfo.nColorFormat	= GFX_BPP_565;
#elif MAX_LCM_BYTES_PER_PIXEL == 4
			gfxinfo.nColorFormat	= GFX_BPP_888;
#endif
			gfxinfo.uDestStartAddr	= 0;
			gfxinfo.uColorPatternStartAddr	= 0;
			gfxinfo.uSrcStartAddr	= 0;
			gfxOpenEnv (&gfxinfo);
		}
	}
	g_vpMyGFX.nRefCount++;
	//sysSafePrintf ("After add: %d\n", g_vpMyGFX.nRefCount);
}


static void vgfxDecRef (void)
{
	//sysSafePrintf ("Before dec: %d\n", g_vpMyGFX.nRefCount);
	g_vpMyGFX.nRefCount--;
	if (g_vpMyGFX.nRefCount == 0)
	{
		gfxCloseEnv ();
		//TURN_OFF_GFX_CLOCK;
	}
}



void vgfxWaitEngineReady (void)
{
    if ((inpw(REG_GE_TRIGGER) & 0x01) != 0) // engine busy?
    {
        while ((inpw(REG_GE_INTS) & 0x01) == 0)
        {
#ifndef ECOS
        	tt_yield_thread (); // wait for command complete
#else
			//cyg_thread_yield();
			cyg_thread_delay(1);
#endif
        }
    }         
    outpw(REG_GE_INTS, 1); // clear interrupt status
}


//Add by xhchen: trigger, but not wait the finish status.
static     UINT32 g_gfx_temp_misc, g_gfx_temp_mask;
INT vgfxMemcpyTrigger(PVOID dest, PVOID src, INT count)
{
//    UINT32 new_dest, new_src;
//    INT M, N;
    
    gfxWaitEngineReady();

    // backup BPP information
    g_gfx_temp_misc = inpw(REG_GE_MISC);
    
    // must use 8-BPP  
    outpw(REG_GE_MISC, g_gfx_temp_misc & 0xffffffcf); // filter always uses 8-BPP

    // backup write mask
    g_gfx_temp_mask = inpw(REG_GE_WRITE_MASK);
    
    outpw(REG_GE_WRITE_MASK, 0x0000ff);
  
    /*
    ** This function uses linear addressing mode. The REG_GE_DEST_START00_ADDR
    ** and REG_GE_SRC_START00_ADDR are not used. The starting address is 
    ** always mapped to 0 of physical memory.
    */
  
    outpw(REG_GE_CONTROL, 0xcc420004); // linear addressing
    
    outpw(REG_GE_DEST_START_ADDR, (UINT32)dest+count-1);
    
    outpw(REG_GE_SRC_START_ADDR, (UINT32)src+count-1);
    
    outpw(REG_GE_DIMENSION, (UINT32)count);
       
    outpw(REG_GE_TRIGGER, 1); 

	return 0;    
}

void vgfxMemcpyWait (void)
{
	vgfxWaitEngineReady ();
    
    outpw(REG_GE_MISC, g_gfx_temp_misc); // restore BPP
    outpw(REG_GE_WRITE_MASK, g_gfx_temp_mask); // restore mask
}




void vgfxLock (void)
{
	tt_rmutex_lock (&g_vpMyGFX.mtLock);
	vgfxAddRef ();
}

int vgfxTryLockInThd (void)
{
	int rt = tt_rmutex_try_lock_in_thd (&g_vpMyGFX.mtLock);
	if (rt == 0)
		vgfxAddRef ();
	return rt;
}

int vgfxTryLockInDsr (void)
{
	int rt = tt_rmutex_try_lock_in_dsr (&g_vpMyGFX.mtLock);
	if (rt == 0)
		vgfxAddRef ();
	return rt;
}


void vgfxUnlock (void)
{
	vgfxDecRef ();
	tt_rmutex_unlock (&g_vpMyGFX.mtLock);
}

