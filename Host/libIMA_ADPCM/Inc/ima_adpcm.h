#ifndef _IMA_ADPCM_H
#define _IMA_ADPCM_H
//---------------------------------------------------------------------------------------
#include "wbtypes.h"

#define IMA_CHANNEL_MAX	2		/* The maximun supported channels */	
#define IMA_PACKET_SIZE 256		/* The size of a compressed block */

INT imaadpcmWavBlockAlign(VOID);
INT imaadpcmSamplePerBlock(VOID);
VOID imaadpcmInit(INT nch, UINT sample_rate, INT block_align);
VOID imaadpcmBlockEnc(CHAR *in, INT in_size, CHAR *out, INT *out_size);
VOID imaadpcmBlockDec(CHAR *in, INT packet_size, CHAR *out, INT *blocksize);


//---------------------------------------------------------------------------------------
#endif
