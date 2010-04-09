#ifndef __CMD18_FUNC_H__
#define __CMD18_FUNC_H__

void cmdNull (FTH_THREAD_T *pFTHThread, UCHAR ucCmd, UCHAR ucSubCmd, UCHAR ucD, UCHAR ucC, UCHAR ucB, UCHAR ucA);
void cmd18_00_DoThing (FTH_THREAD_T *pFTHThread, UCHAR ucCmd, UCHAR ucSubCmd, UCHAR ucD, UCHAR ucC, UCHAR ucB, UCHAR ucA);
void cmd18_02_GetInterruptEvent (FTH_THREAD_T *pFTHThread, UCHAR ucCmd, UCHAR ucSubCmd, UCHAR ucD, UCHAR ucC, UCHAR ucB, UCHAR ucA);
void cmd18_03_EnableCommandInterrupt (FTH_THREAD_T *pFTHThread, UCHAR ucCmd, UCHAR ucSubCmd, UCHAR ucD, UCHAR ucC, UCHAR ucB, UCHAR ucA);
void cmd18_04_SendBitstream (FTH_THREAD_T *pFTHThread, UCHAR ucCmd, UCHAR ucSubCmd, UCHAR ucD, UCHAR ucC, UCHAR ucB, UCHAR ucA);
void cmd18_05_RecvBitstream (FTH_THREAD_T *pFTHThread, UCHAR ucCmd, UCHAR ucSubCmd, UCHAR ucD, UCHAR ucC, UCHAR ucB, UCHAR ucA);


#endif

