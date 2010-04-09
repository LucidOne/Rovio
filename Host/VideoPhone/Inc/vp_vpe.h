#ifndef __VP_VPE_H__
#define __VP_VPE_H__



void vvpeInit (void);
void vvpeUninit (void);
void vvpeLock (void);
int vvpeCanLock__remove_it (void);
int vvpeTryLockInThd (void);
int vvpeTryLockInDsr (void);
void vvpeUnlock (void);

int vvpeTrigger (void);
void vvpeWaitProcessOK (void);

#endif
