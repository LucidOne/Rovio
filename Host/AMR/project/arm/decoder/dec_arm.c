
/*************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0                
*                                REL-4 Version 4.1.0                
*
********************************************************************************
*
*      File             : decoder.c
*      Purpose          : Speech decoder main program.
*
********************************************************************************
*
*         Usage : decoder  bitstream_file  synth_file
*
*
*         Format for bitstream_file:
*             1 word (2-byte) for the frame type
*               (see frame.h for possible values)
*               Normally, the TX frame type is expected.
*               RX frame type can be forced with "-rxframetype"
*           244 words (2-byte) containing 244 bits.
*               Bit 0 = 0x0000 and Bit 1 = 0x0001
*             1 word (2-byte) for the mode indication
*               (see mode.h for possible values)
*             4 words for future use, currently unused
*
*         Format for synth_file:
*           Synthesis is written to a binary file of 16 bits data.
*
********************************************************************************
*/

/*
********************************************************************************
*                         INCLUDE FILES
********************************************************************************
*/

#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif


#ifdef ECOS
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "errno.h"
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#endif
#include "typedef.h"
#include "n_proc.h"
#include "cnst.h"
#include "mode.h"
#include "frame.h"
#include "strfunc.h"
#include "sp_dec.h"
#include "d_homing.h"

const char decoder_id[] = "@(#)$Id $";
/* frame size in serial bitstream file (frame type + serial stream + flags) */
#define SERIAL_FRAMESIZE (1+MAX_SERIAL_SIZE+5)

#define READ_FILE 1
#define WRITE_FILE 1

/* verify patterns */
#ifndef VAD2
#define VERIFY_DTX		0 // Voice detection
#define VERIFY_MOD		0 // mode switching
#define VERIFY_MR122	0
#define VERIFY_MR102	0
#define VERIFY_MR795	0
#define VERIFY_MR74		0
#define VERIFY_MR67		0
#define VERIFY_MR59		0
#define VERIFY_MR515	0
#define VERIFY_MR475	0
#else
#define VERIFY_DTX		1 // Voice detection
#define VERIFY_MOD		0 // mode switching
#define VERIFY_MR122	0
#define VERIFY_MR102	0
#define VERIFY_MR795	0
#define VERIFY_MR74		0
#define VERIFY_MR67		0
#define VERIFY_MR59		0
#define VERIFY_MR515	0
#define VERIFY_MR475	0
#endif


#ifdef malloc_sum
int malloc_size = 1;
#endif


struct file_list_t
{
  char *path;
  char *name;
  char *ext;
  char *mode;	// AMR mode
}file_list[]={


#if !READ_FILE && WRITE_FILE
	{"../../../../../pattern/dec/input/", "T21", ".cod", "MR122"},	  
#endif	

	{"../../../../../pattern/dec/input/", "T10", ".cod", "MR475"},	  


#if VERIFY_DTX
# ifndef VAD2
	{"../../../../../pattern/dec/DTX/", "DTX1", ".cod", "MR122"},	  
	{"../../../../../pattern/dec/DTX/", "DTX2", ".cod", "MR122"},	  
	{"../../../../../pattern/dec/DTX/", "DTX3", ".cod", "MR122"},	  
	{"../../../../../pattern/dec/DTX/", "DTX3", ".cod", "MR102"},	  
	{"../../../../../pattern/dec/DTX/", "DTX3", ".cod", "MR795"},	  
	{"../../../../../pattern/dec/DTX/", "DTX3", ".cod", "MR74"},	  
	{"../../../../../pattern/dec/DTX/", "DTX3", ".cod", "MR67"},	  
	{"../../../../../pattern/dec/DTX/", "DTX3", ".cod", "MR59"},	  
	{"../../../../../pattern/dec/DTX/", "DTX3", ".cod", "MR515"},	  
	{"../../../../../pattern/dec/DTX/", "DTX3", ".cod", "MR475"},	  
	{"../../../../../pattern/dec/DTX/", "DTX4", ".cod", "MR122"},	  
# else
	{"../../../../../pattern/dec/DTX2/", "DT21", ".cod", "MR122"},	  
	{"../../../../../pattern/dec/DTX2/", "DT22", ".cod", "MR122"},	  
	{"../../../../../pattern/dec/DTX2/", "DT22", ".cod", "MR102"},	  
	{"../../../../../pattern/dec/DTX2/", "DT22", ".cod", "MR795"},	  
	{"../../../../../pattern/dec/DTX2/", "DT22", ".cod", "MR74"},	  
	{"../../../../../pattern/dec/DTX2/", "DT22", ".cod", "MR67"},	  
	{"../../../../../pattern/dec/DTX2/", "DT22", ".cod", "MR59"},	  
	{"../../../../../pattern/dec/DTX2/", "DT22", ".cod", "MR515"},	  
	{"../../../../../pattern/dec/DTX2/", "DT22", ".cod", "MR475"},	  
	{"../../../../../pattern/dec/DTX2/", "DT23", ".cod", "MR122"},	  
	{"../../../../../pattern/dec/DTX2/", "DT24", ".cod", "MR122"},	  
# endif // VAD2
#endif

#if VERIFY_MOD
	{"../../../../../pattern/dec/input/", "T21", ".cod", "XXXXX"},	  
#endif

#if VERIFY_MR122
	{"../../../../../pattern/dec/input/", "T00", ".cod", "MR122"},	  
	{"../../../../../pattern/dec/input/", "T01", ".cod", "MR122"},	  
	{"../../../../../pattern/dec/input/", "T02", ".cod", "MR122"},	  
	{"../../../../../pattern/dec/input/", "T03", ".cod", "MR122"},	  
	{"../../../../../pattern/dec/input/", "T04", ".cod", "MR122"},	  
	{"../../../../../pattern/dec/input/", "T05", ".cod", "MR122"},	  
	{"../../../../../pattern/dec/input/", "T06", ".cod", "MR122"},	  
	{"../../../../../pattern/dec/input/", "T07", ".cod", "MR122"},	  
	{"../../../../../pattern/dec/input/", "T08", ".cod", "MR122"},	  
	{"../../../../../pattern/dec/input/", "T09", ".cod", "MR122"},	  
	{"../../../../../pattern/dec/input/", "T10", ".cod", "MR122"},	  
	{"../../../../../pattern/dec/input/", "T11", ".cod", "MR122"},	  
	{"../../../../../pattern/dec/input/", "T12", ".cod", "MR122"},	  
	{"../../../../../pattern/dec/input/", "T13", ".cod", "MR122"},	  
	{"../../../../../pattern/dec/input/", "T14", ".cod", "MR122"},	  
	{"../../../../../pattern/dec/input/", "T15", ".cod", "MR122"},	  
	{"../../../../../pattern/dec/input/", "T16", ".cod", "MR122"},	  
	{"../../../../../pattern/dec/input/", "T17", ".cod", "MR122"},	  
	{"../../../../../pattern/dec/input/", "T18", ".cod", "MR122"},	  
	{"../../../../../pattern/dec/input/", "T19", ".cod", "MR122"},	  
	{"../../../../../pattern/dec/input/", "T20", ".cod", "MR122"},	  
#endif

#if VERIFY_MR102
	{"../../../../../pattern/dec/input/", "T00", ".cod", "MR102"},	  
	{"../../../../../pattern/dec/input/", "T01", ".cod", "MR102"},	  
	{"../../../../../pattern/dec/input/", "T02", ".cod", "MR102"},	  
	{"../../../../../pattern/dec/input/", "T03", ".cod", "MR102"},	  
	{"../../../../../pattern/dec/input/", "T04", ".cod", "MR102"},	  
	{"../../../../../pattern/dec/input/", "T05", ".cod", "MR102"},	  
	{"../../../../../pattern/dec/input/", "T06", ".cod", "MR102"},	  
	{"../../../../../pattern/dec/input/", "T07", ".cod", "MR102"},	  
	{"../../../../../pattern/dec/input/", "T08", ".cod", "MR102"},	  
	{"../../../../../pattern/dec/input/", "T09", ".cod", "MR102"},	  
	{"../../../../../pattern/dec/input/", "T10", ".cod", "MR102"},	  
	{"../../../../../pattern/dec/input/", "T11", ".cod", "MR102"},	  
	{"../../../../../pattern/dec/input/", "T12", ".cod", "MR102"},	  
	{"../../../../../pattern/dec/input/", "T13", ".cod", "MR102"},	  
	{"../../../../../pattern/dec/input/", "T14", ".cod", "MR102"},	  
	{"../../../../../pattern/dec/input/", "T15", ".cod", "MR102"},	  
	{"../../../../../pattern/dec/input/", "T16", ".cod", "MR102"},	  
	{"../../../../../pattern/dec/input/", "T17", ".cod", "MR102"},	  
	{"../../../../../pattern/dec/input/", "T18", ".cod", "MR102"},	  
	{"../../../../../pattern/dec/input/", "T19", ".cod", "MR102"},	  
	{"../../../../../pattern/dec/input/", "T20", ".cod", "MR102"},	  
#endif

#if VERIFY_MR795
	{"../../../../../pattern/dec/input/", "T00", ".cod", "MR795"},	  
	{"../../../../../pattern/dec/input/", "T01", ".cod", "MR795"},	  
	{"../../../../../pattern/dec/input/", "T02", ".cod", "MR795"},	  
	{"../../../../../pattern/dec/input/", "T03", ".cod", "MR795"},	  
	{"../../../../../pattern/dec/input/", "T04", ".cod", "MR795"},	  
	{"../../../../../pattern/dec/input/", "T05", ".cod", "MR795"},	  
	{"../../../../../pattern/dec/input/", "T06", ".cod", "MR795"},	  
	{"../../../../../pattern/dec/input/", "T07", ".cod", "MR795"},	  
	{"../../../../../pattern/dec/input/", "T08", ".cod", "MR795"},	  
	{"../../../../../pattern/dec/input/", "T09", ".cod", "MR795"},	  
	{"../../../../../pattern/dec/input/", "T10", ".cod", "MR795"},	  
	{"../../../../../pattern/dec/input/", "T11", ".cod", "MR795"},	  
	{"../../../../../pattern/dec/input/", "T12", ".cod", "MR795"},	  
	{"../../../../../pattern/dec/input/", "T13", ".cod", "MR795"},	  
	{"../../../../../pattern/dec/input/", "T14", ".cod", "MR795"},	  
	{"../../../../../pattern/dec/input/", "T15", ".cod", "MR795"},	  
	{"../../../../../pattern/dec/input/", "T16", ".cod", "MR795"},	  
	{"../../../../../pattern/dec/input/", "T17", ".cod", "MR795"},	  
	{"../../../../../pattern/dec/input/", "T18", ".cod", "MR795"},	  
	{"../../../../../pattern/dec/input/", "T19", ".cod", "MR795"},	  
	{"../../../../../pattern/dec/input/", "T20", ".cod", "MR795"},	  
#endif

#if VERIFY_MR74
	{"../../../../../pattern/dec/input/", "T00", ".cod", "MR74"},	  
	{"../../../../../pattern/dec/input/", "T01", ".cod", "MR74"},	  
	{"../../../../../pattern/dec/input/", "T02", ".cod", "MR74"},	  
	{"../../../../../pattern/dec/input/", "T03", ".cod", "MR74"},	  
	{"../../../../../pattern/dec/input/", "T04", ".cod", "MR74"},	  
	{"../../../../../pattern/dec/input/", "T05", ".cod", "MR74"},	  
	{"../../../../../pattern/dec/input/", "T06", ".cod", "MR74"},	  
	{"../../../../../pattern/dec/input/", "T07", ".cod", "MR74"},	  
	{"../../../../../pattern/dec/input/", "T08", ".cod", "MR74"},	  
	{"../../../../../pattern/dec/input/", "T09", ".cod", "MR74"},	  
	{"../../../../../pattern/dec/input/", "T10", ".cod", "MR74"},	  
	{"../../../../../pattern/dec/input/", "T11", ".cod", "MR74"},	  
	{"../../../../../pattern/dec/input/", "T12", ".cod", "MR74"},	  
	{"../../../../../pattern/dec/input/", "T13", ".cod", "MR74"},	  
	{"../../../../../pattern/dec/input/", "T14", ".cod", "MR74"},	  
	{"../../../../../pattern/dec/input/", "T15", ".cod", "MR74"},	  
	{"../../../../../pattern/dec/input/", "T16", ".cod", "MR74"},	  
	{"../../../../../pattern/dec/input/", "T17", ".cod", "MR74"},	  
	{"../../../../../pattern/dec/input/", "T18", ".cod", "MR74"},	  
	{"../../../../../pattern/dec/input/", "T19", ".cod", "MR74"},	  
	{"../../../../../pattern/dec/input/", "T20", ".cod", "MR74"},	  
#endif

#if VERIFY_MR67
	{"../../../../../pattern/dec/input/", "T00", ".cod", "MR67"},	  
	{"../../../../../pattern/dec/input/", "T01", ".cod", "MR67"},	  
	{"../../../../../pattern/dec/input/", "T02", ".cod", "MR67"},	  
	{"../../../../../pattern/dec/input/", "T03", ".cod", "MR67"},	  
	{"../../../../../pattern/dec/input/", "T04", ".cod", "MR67"},	  
	{"../../../../../pattern/dec/input/", "T05", ".cod", "MR67"},	  
	{"../../../../../pattern/dec/input/", "T06", ".cod", "MR67"},	  
	{"../../../../../pattern/dec/input/", "T07", ".cod", "MR67"},	  
	{"../../../../../pattern/dec/input/", "T08", ".cod", "MR67"},	  
	{"../../../../../pattern/dec/input/", "T09", ".cod", "MR67"},	  
	{"../../../../../pattern/dec/input/", "T10", ".cod", "MR67"},	  
	{"../../../../../pattern/dec/input/", "T11", ".cod", "MR67"},	  
	{"../../../../../pattern/dec/input/", "T12", ".cod", "MR67"},	  
	{"../../../../../pattern/dec/input/", "T13", ".cod", "MR67"},	  
	{"../../../../../pattern/dec/input/", "T14", ".cod", "MR67"},	  
	{"../../../../../pattern/dec/input/", "T15", ".cod", "MR67"},	  
	{"../../../../../pattern/dec/input/", "T16", ".cod", "MR67"},	  
	{"../../../../../pattern/dec/input/", "T17", ".cod", "MR67"},	  
	{"../../../../../pattern/dec/input/", "T18", ".cod", "MR67"},	  
	{"../../../../../pattern/dec/input/", "T19", ".cod", "MR67"},	  
	{"../../../../../pattern/dec/input/", "T20", ".cod", "MR67"},	  
#endif

#if VERIFY_MR59
	{"../../../../../pattern/dec/input/", "T00", ".cod", "MR59"},	  
	{"../../../../../pattern/dec/input/", "T01", ".cod", "MR59"},	  
	{"../../../../../pattern/dec/input/", "T02", ".cod", "MR59"},	  
	{"../../../../../pattern/dec/input/", "T03", ".cod", "MR59"},	  
	{"../../../../../pattern/dec/input/", "T04", ".cod", "MR59"},	  
	{"../../../../../pattern/dec/input/", "T05", ".cod", "MR59"},	  
	{"../../../../../pattern/dec/input/", "T06", ".cod", "MR59"},	  
	{"../../../../../pattern/dec/input/", "T07", ".cod", "MR59"},	  
	{"../../../../../pattern/dec/input/", "T08", ".cod", "MR59"},	  
	{"../../../../../pattern/dec/input/", "T09", ".cod", "MR59"},	  
	{"../../../../../pattern/dec/input/", "T10", ".cod", "MR59"},	  
	{"../../../../../pattern/dec/input/", "T11", ".cod", "MR59"},	  
	{"../../../../../pattern/dec/input/", "T12", ".cod", "MR59"},	  
	{"../../../../../pattern/dec/input/", "T13", ".cod", "MR59"},	  
	{"../../../../../pattern/dec/input/", "T14", ".cod", "MR59"},	  
	{"../../../../../pattern/dec/input/", "T15", ".cod", "MR59"},	  
	{"../../../../../pattern/dec/input/", "T16", ".cod", "MR59"},	  
	{"../../../../../pattern/dec/input/", "T17", ".cod", "MR59"},	  
	{"../../../../../pattern/dec/input/", "T18", ".cod", "MR59"},	  
	{"../../../../../pattern/dec/input/", "T19", ".cod", "MR59"},	  
	{"../../../../../pattern/dec/input/", "T20", ".cod", "MR59"},	  
#endif

#if VERIFY_MR515
	{"../../../../../pattern/dec/input/", "T00", ".cod", "MR515"},	  
	{"../../../../../pattern/dec/input/", "T01", ".cod", "MR515"},	  
	{"../../../../../pattern/dec/input/", "T02", ".cod", "MR515"},	  
	{"../../../../../pattern/dec/input/", "T03", ".cod", "MR515"},	  
	{"../../../../../pattern/dec/input/", "T04", ".cod", "MR515"},	  
	{"../../../../../pattern/dec/input/", "T05", ".cod", "MR515"},	  
	{"../../../../../pattern/dec/input/", "T06", ".cod", "MR515"},	  
	{"../../../../../pattern/dec/input/", "T07", ".cod", "MR515"},	  
	{"../../../../../pattern/dec/input/", "T08", ".cod", "MR515"},	  
	{"../../../../../pattern/dec/input/", "T09", ".cod", "MR515"},	  
	{"../../../../../pattern/dec/input/", "T10", ".cod", "MR515"},	  
	{"../../../../../pattern/dec/input/", "T11", ".cod", "MR515"},	  
	{"../../../../../pattern/dec/input/", "T12", ".cod", "MR515"},	  
	{"../../../../../pattern/dec/input/", "T13", ".cod", "MR515"},	  
	{"../../../../../pattern/dec/input/", "T14", ".cod", "MR515"},	  
	{"../../../../../pattern/dec/input/", "T15", ".cod", "MR515"},	  
	{"../../../../../pattern/dec/input/", "T16", ".cod", "MR515"},	  
	{"../../../../../pattern/dec/input/", "T17", ".cod", "MR515"},	  
	{"../../../../../pattern/dec/input/", "T18", ".cod", "MR515"},	  
	{"../../../../../pattern/dec/input/", "T19", ".cod", "MR515"},	  
	{"../../../../../pattern/dec/input/", "T20", ".cod", "MR515"},	  
#endif

#if VERIFY_MR475
	{"../../../../../pattern/dec/input/", "T00", ".cod", "MR475"},	  
	{"../../../../../pattern/dec/input/", "T01", ".cod", "MR475"},	  
	{"../../../../../pattern/dec/input/", "T02", ".cod", "MR475"},	  
	{"../../../../../pattern/dec/input/", "T03", ".cod", "MR475"},	  
	{"../../../../../pattern/dec/input/", "T04", ".cod", "MR475"},	  
	{"../../../../../pattern/dec/input/", "T05", ".cod", "MR475"},	  
	{"../../../../../pattern/dec/input/", "T06", ".cod", "MR475"},	  
	{"../../../../../pattern/dec/input/", "T07", ".cod", "MR475"},	  
	{"../../../../../pattern/dec/input/", "T08", ".cod", "MR475"},	  
	{"../../../../../pattern/dec/input/", "T09", ".cod", "MR475"},	  
	{"../../../../../pattern/dec/input/", "T10", ".cod", "MR475"},	  
	{"../../../../../pattern/dec/input/", "T11", ".cod", "MR475"},	  
	{"../../../../../pattern/dec/input/", "T12", ".cod", "MR475"},	  
	{"../../../../../pattern/dec/input/", "T13", ".cod", "MR475"},	  
	{"../../../../../pattern/dec/input/", "T14", ".cod", "MR475"},	  
	{"../../../../../pattern/dec/input/", "T15", ".cod", "MR475"},	  
	{"../../../../../pattern/dec/input/", "T16", ".cod", "MR475"},	  
	{"../../../../../pattern/dec/input/", "T17", ".cod", "MR475"},	  
	{"../../../../../pattern/dec/input/", "T18", ".cod", "MR475"},	  
	{"../../../../../pattern/dec/input/", "T19", ".cod", "MR475"},	  
	{"../../../../../pattern/dec/input/", "T20", ".cod", "MR475"},	  
#endif

//	{"../../../../../pattern/dec/input/", "T07", ".cod", "MR122"},	  
//	{"../../../../../pattern/dec/input/", "T06", ".cod", "MR122"},	  

};

#define SUB_DIR "decode/"
#define WFILE_EXT ".pcm"
#define FILE_SUFFIX ""

#if !READ_FILE
extern void * stream_buffer_start;
extern void * stream_buffer_end;
#endif



/*
********************************************************************************
*                         LOCAL PROGRAM CODE
********************************************************************************
*/
static enum RXFrameType tx_to_rx (enum TXFrameType tx_type)
{
    switch (tx_type) {
      case TX_SPEECH_GOOD:      return RX_SPEECH_GOOD;
      case TX_SPEECH_DEGRADED:  return RX_SPEECH_DEGRADED;
      case TX_SPEECH_BAD:       return RX_SPEECH_BAD;
      case TX_SID_FIRST:        return RX_SID_FIRST;
      case TX_SID_UPDATE:       return RX_SID_UPDATE;
      case TX_SID_BAD:          return RX_SID_BAD;
      case TX_ONSET:            return RX_ONSET;
      case TX_NO_DATA:          return RX_NO_DATA;
      default:
        printf("tx_to_rx: unknown(RX_SPEECH_GOOD) TX frame type %d\n", tx_type);
        return RX_SPEECH_GOOD;
    }
}

/*
********************************************************************************
*                             MAIN PROGRAM 
********************************************************************************
*/

int main (int argc, char *argv[])
{
  Speech_Decode_FrameState *speech_decoder_state = NULL;
#if READ_FILE
  ALIGN(4) Word16 serial[SERIAL_FRAMESIZE];   /* coded bits                    */
#else
	Word16 * serial = (Word16 *)&stream_buffer_start;		
#endif  

  ALIGN(4) Word16 synth[L_FRAME];             /* Synthesis                     */
  Word32 frame;

  //Word16 *ptr_in,*ptr_out;
  FILE *file_speech = NULL;           /* File of speech data               */
  FILE *file_serial = NULL;           /* File of coded bits                */

  int rxframetypeMode = 0;           /* use RX frame type codes       */
  enum Mode mode = (enum Mode)0;
  enum RXFrameType rx_type = (enum RXFrameType)0;
  enum TXFrameType tx_type = (enum TXFrameType)0;
     
  Word16 reset_flag = 0;
  Word16 reset_flag_old = 1;
  Word16 i;

	int nfile,k=0;
	char in_file[128]="";
	char out_file[128]="";
	unsigned int clk;

#if defined(OPT_AMR_FORMAT)
	char amr_file_id[6];
	int packet_size;
	char amr_serial[32]; /* amr packet buffer */
#endif

#if 0
	unsigned int aa[2];
	unsigned int a1, a2;
	aa[0] = 240;
	aa[1] = 13;
	
	printf("--------\n");
	init_UDIV_32d16_16r16( &aa[0], &aa[1] ) ;
	printf("Q: %d, R:%d\n",aa[0], aa[1]);
	return 0;
#endif


  proc_head ("Decoder");


  /*----------------------------------------------------------------------*
   * Process command line options                                         *
   *----------------------------------------------------------------------*/
	printf("pragram start\n");
  /*----------------------------------------------------------------------*
   * Check number of arguments                                            *
   *----------------------------------------------------------------------*/
	if ( argc != 3 && 0 )
  {
    printf (
      " Usage:\n\n"
      "   %s speech_file  bitstream_file\n\n",
             argv[0]);
      exit (1);
  }

	nfile=sizeof(file_list)/sizeof(file_list[0]);
	printf("There are %d input files\n",nfile);
	
	clk=clock();

#if READ_FILE
	for(k=0;k<nfile;k++)	
	{
		/* input file name and path */
		strcpy(in_file, file_list[k].path);
		strcat(in_file, file_list[k].name);
		if( file_list[k].mode[0] == 'M' )
		{
			strcat(in_file, "_");
			strcat(in_file, &file_list[k].mode[2]);
		}
#endif // READ_FILE

#if defined(OPT_AMR_FORMAT)
		strcat(in_file, ".amr");
#else
		strcat(in_file, file_list[k].ext);
#endif

		/* output file name and path */
		strcpy(out_file, file_list[k].path);
		strcat(out_file, SUB_DIR);
		strcat(out_file, file_list[k].name);
		strcat(out_file, "_");
		strcat(out_file, &file_list[k].mode[2]);
		strcat(out_file, WFILE_EXT);

#if READ_FILE
		if( (file_serial=fopen(in_file,"rb"))==NULL)
		{
			printf("Can't open serial input file: %s\n",in_file);
			exit(1);
		}
#endif // READ_FILE		

#if WRITE_FILE
		if( (file_speech=fopen(out_file,"wb"))==NULL)
		{
			printf("Can't open speech output file: %s\n",out_file);
			exit(1);
		}
#endif		

#if defined(OPT_AMR_FORMAT)
		/* check AMR file ID */
		fread(amr_file_id, 6, 1, file_serial);
		if( strncmp(amr_file_id, "#!AMR\n",6) )
		{
			printf("ERROR: AMR file ID miss!\n");
			exit(1);
		}
#endif

#if READ_FILE
		printf("input : %s\n",in_file);
#endif // READ_FILE

		printf("output: %s\n",out_file);

		/*-----------------------------------------------------------------------*
		 * Initialization of decoder                                             *
		 *-----------------------------------------------------------------------*/
		reset_flag = 0;
		reset_flag_old = 1;

		if (Speech_Decode_Frame_init(&speech_decoder_state, "Decoder"))
			exit(-1);
		/*-----------------------------------------------------------------------*
		 * process serial bitstream frame by frame                               *
		 *-----------------------------------------------------------------------*/
		frame = 0;
		while (1)
		{
#if defined(OPT_AMR_FORMAT)
			int amr_mode;
			/* check AMR mode first to find the packet size */
			fread(&amr_mode, 1, 1, file_serial);
			fseek(file_serial,-1,SEEK_CUR); /* return to the packet start */
			amr_mode = (amr_mode >> 3) & 0xF; 
			packet_size = amrPacketSize(amr_mode);
			if( fread(amr_serial, 1, packet_size,file_serial)!=packet_size )
				break;
			/* conver amr file format into cod file format */
			amr2cod(amr_serial, serial);

#else
#	if READ_FILE
			if( fread(serial, 1, SERIAL_FRAMESIZE*sizeof(Word16),file_serial)!=SERIAL_FRAMESIZE*sizeof(Word16) )
				break;
#	else
			if(serial >= (Word16 *)&stream_buffer_end) break;
#	endif				
#endif
			frame++;
			if ( (frame&0x0F) == 0)
				printf ("\n\rframe=%d  ", frame);

			/* get frame type and mode information from frame */
			if (rxframetypeMode)
			{
				rx_type = (enum RXFrameType)serial[0];
			}
			else
			{
				tx_type = (enum TXFrameType)serial[0];
				rx_type = tx_to_rx (tx_type);
			}
			mode = (enum Mode) serial[1+MAX_SERIAL_SIZE];
			if (rx_type == RX_NO_DATA)
				mode = speech_decoder_state->prev_mode;
			else
				speech_decoder_state->prev_mode = mode;

			/* if homed: check if this frame is another homing frame */
			if (reset_flag_old == 1)
			{
				/* only check until end of first subframe */
				reset_flag = decoder_homing_frame_test_first(&serial[1], mode);
			}
			/* produce encoder homing frame if homed & input=decoder homing frame */
			if ((reset_flag != 0) && (reset_flag_old != 0))
			{
				for (i = 0; i < L_FRAME; i++)
				{
					synth[i] = EHF_MASK;
				}
			}
			else
			{     
				/* decode frame */
				Speech_Decode_Frame(speech_decoder_state, mode, &serial[1], rx_type, synth);
			}

#if WRITE_FILE     
			/* write synthesized speech to file */
			fwrite(synth, 1, L_FRAME*sizeof(Word16),file_speech);
#endif			
			
#if !READ_FILE
			serial += SERIAL_FRAMESIZE;
#endif			

			/* if not homed: check whether current frame is a homing frame */
			if (reset_flag_old == 0)
				reset_flag = decoder_homing_frame_test(&serial[1], mode); /* check whole frame */
			/* reset decoder if current frame is a homing frame */
			if (reset_flag != 0)
				Speech_Decode_Frame_reset(speech_decoder_state);
			reset_flag_old = reset_flag;
		} // while(1)
		printf ("\n%d frame(s) processed\n", frame);
  
		/*-----------------------------------------------------------------------*
		 * Close down speech decoder                                             *
		 *-----------------------------------------------------------------------*/
		Speech_Decode_Frame_exit(&speech_decoder_state);

#if WRITE_FILE
		fclose(file_speech);
#endif		
#if READ_FILE
		fclose(file_serial);
	} // for each file
#endif		

	printf ("\nTotal %dms used\n", (clock()-clk));
	//printf("press any key to continue...\n");
	//getchar();
	return 0;
}




