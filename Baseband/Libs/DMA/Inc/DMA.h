#ifndef __DMA_H__
#define __DMA_H__

typedef void (*FUN_ON_DMA) (void);

void dmaInitialze (void);
void dmaMemcpy (UINT32 uSrc, UINT32 uDes,
	UINT32 uBlockSize, UINT32 uBlockNum,
	int nSrcStep, int nDesStep,
	FUN_ON_DMA fnOnOK,
	FUN_ON_DMA fnOnError);

#endif

