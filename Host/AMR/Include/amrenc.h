#ifndef _AMR_ENC_H
#define _AMR_ENC_H
//--------------------------------------------------------------------------------------------------

#define AMR_ENC_LENGTH 160
#define AMR_ENC_MAX_PACKET_SIZE 32

/* This id must be in the head of amr file */
#define AMR_ENC_AMR_FILE_ID "#!AMR\n"

short amrInitEncode(void);
short amrFinishEncode(void);

/*******************************************************
	Arguments:
  		mode 		- 0=4.75kb, 1=5.15kb, 2=5.9kb, 3=6.7kb, 4=7.4kb, 5=7.95kb, 6=10.2kb, 7=12.2kb
  		reset_flag	- homing reset
  		in_buff		- input data buffer
  		out_buff	- output data buffer
  		size		- output data size
********************************************************/  		
short amrEncode(int mode, int reset_flag, short *in_buff, short *out_buff, int *size);



//--------------------------------------------------------------------------------------------------
#endif