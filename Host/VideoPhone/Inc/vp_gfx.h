#ifndef __VP_GFX_H__
#define __VP_GFX_H__



void vgfxInit (void);
void vgfxWaitEngineReady (void);
INT vgfxMemcpyTrigger(PVOID dest, PVOID src, INT count);
void vgfxMemcpyWait(void);
void vgfxLock (void);
int vgfxTryLockInThd (void);
int vgfxTryLockInDsr (void);
void vgfxUnlock (void);

#endif
