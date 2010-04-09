#ifndef __VP_THREAD_CAPTURE_H__
#define __VP_THREAD_CAPTURE_H__

#define MOTION_IGNOREINITCOUNT	5
#define MOTION_DROP				2

void vcptInitThread (void);
void vcptUninitThread (void);
void vcptLock (void);
int vcptCanLock__remove_it (void);
int vcptTryLockInThd (void);
int vcptTryLockInDsr (void);
void vcptUnlock (void);


void vcptStart (void);
void vcptStop (void);
int vcptSetWindow (USHORT usWidth, USHORT usHeight, CMD_ROTATE_E eRotate);
void vcptWaitPrevMsg (void);

void vcptSetContrastBright(int contrast, int bright);
void vcptSetFrequency(int nFrequencyHz);
int vcptGetFrequency(void);


void vcptEnableDrawImageTime(BOOL bEnable);



void vcptJpegTimerInit (void);
BOOL vcptJpegTimerIsEnable (void);
void vcptJpegTimerNotify (void);
void vcptJpegTimerWait (void);
void vcptJpegTimerAck (void);
void vcptJpegTimerUninit (void);


#endif
