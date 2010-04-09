#ifndef _AMR_DEC_H
#define _AMR_DEC_H
//-----------------------------------------------------------------------------------------------------

#define AMR_FRAME_SIZE 160
#define AMR_MAX_PACKET_SIZE	32

#define AMR_MODE(x)		(((x)>>3)&0xF)

short amrInitDecode(void);
short amrDecode(char *packet,short *synth);
short amrFinishDecode(void);
int amrPacketSize(int mode);

//-----------------------------------------------------------------------------------------------------
#endif