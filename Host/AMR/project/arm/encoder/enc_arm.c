/*
*****************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0                
*                                REL-4 Version 4.1.0                
*
*****************************************************************************
*
*      File             : coder.c
*      Purpose          : Speech encoder main program.
*
*****************************************************************************
*
*    Usage : coder speech_file  bitstream_file
*
*    Format for speech_file:
*      Speech is read from a binary file of 16 bits data.
*
*    Format for bitstream_file:
*        1 word (2-byte) for the TX frame type
*          (see frame.h for possible values)
*      244 words (2-byte) containing 244 bits.
*          Bit 0 = 0x0000 and Bit 1 = 0x0001
*        1 word (2-byte) for the mode indication
*          (see mode.h for possible values)
*        4 words for future use, currently written as zero
*
*****************************************************************************
*/
/*-------------wtcheng test----------------*/




/*------------------------------------------*/
 
/*
*****************************************************************************
*                         INCLUDE FILES
*****************************************************************************
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
#include "cnst.h"
#include "n_proc.h"
#include "mode.h"
#include "frame.h"
#include "strfunc.h"
#include "sp_enc.h"
#include "pre_proc.h"
#include "sid_sync.h"
#include "vadname.h"
#include "e_homing.h"

//const char coder_id[] = "@(#)$Id $";

/* frame size in serial bitstream file (frame type + serial stream + flags) */
#define SERIAL_FRAMESIZE (1+MAX_SERIAL_SIZE+5)

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
unsigned int malloc_size=0;
#endif

struct file_list_t
{
  char *path;
  char *name;
  char *ext;
  char *mode;	// AMR mode
  int position; // SYNC test
  int dtx;		// DTX test
}file_list[]={


	//../../../../../pattern/enc/DTX/", "DTX1", ".inp", "MR122",0,1},	  
	//../../../../../pattern/enc/DTX/", "DTX3", ".inp", "MR795",0,1},	  
	//{"../../../../../pattern/enc/input/", "T16",".inp", "MR122",0,1},
	{"../../../../../pattern/enc/input/", "T10", ".inp", "MR67",0,0},	  
	//{"../../../../../pattern/enc/input/", "T10", ".inp", "MR122",0,0},	  


#if VERIFY_DTX
# ifndef VAD2
	{"../../../../../pattern/enc/DTX/", "DTX1", ".inp", "MR122",0,1},	  
	{"../../../../../pattern/enc/DTX/", "DTX2", ".inp", "MR122",0,1},	  
	{"../../../../../pattern/enc/DTX/", "DTX3", ".inp", "MR122",0,1},	  
	{"../../../../../pattern/enc/DTX/", "DTX3", ".inp", "MR102",0,1},	  
	{"../../../../../pattern/enc/DTX/", "DTX3", ".inp", "MR795",0,1},	  
	{"../../../../../pattern/enc/DTX/", "DTX3", ".inp", "MR74",0,1},	  
	{"../../../../../pattern/enc/DTX/", "DTX3", ".inp", "MR67",0,1},	  
	{"../../../../../pattern/enc/DTX/", "DTX3", ".inp", "MR59",0,1},	  
	{"../../../../../pattern/enc/DTX/", "DTX3", ".inp", "MR515",0,1},	  
	{"../../../../../pattern/enc/DTX/", "DTX3", ".inp", "MR475",0,1},	  
	{"../../../../../pattern/enc/DTX/", "DTX4", ".inp", "MR122",0,1},	  
# else
	{"../../../../../pattern/enc/DTX2/", "DT21", ".inp", "MR122",0,1},	  
	{"../../../../../pattern/enc/DTX2/", "DT22", ".inp", "MR122",0,1},	  
	{"../../../../../pattern/enc/DTX2/", "DT22", ".inp", "MR102",0,1},	  
	{"../../../../../pattern/enc/DTX2/", "DT22", ".inp", "MR795",0,1},	  
	{"../../../../../pattern/enc/DTX2/", "DT22", ".inp", "MR74",0,1},	  
	{"../../../../../pattern/enc/DTX2/", "DT22", ".inp", "MR67",0,1},	  
	{"../../../../../pattern/enc/DTX2/", "DT22", ".inp", "MR59",0,1},	  
	{"../../../../../pattern/enc/DTX2/", "DT22", ".inp", "MR515",0,1},	  
	{"../../../../../pattern/enc/DTX2/", "DT22", ".inp", "MR475",0,1},	  
	{"../../../../../pattern/enc/DTX2/", "DT23", ".inp", "MR122",0,1},	  
	{"../../../../../pattern/enc/DTX2/", "DT24", ".inp", "MR122",0,1},	  
# endif // VAD2
#endif


#if VERIFY_MR122
	{"../../../../../pattern/enc/input/", "T00", ".inp", "MR122",0,0},	  
	{"../../../../../pattern/enc/input/", "T01", ".inp", "MR122",0,0},	  
	{"../../../../../pattern/enc/input/", "T02", ".inp", "MR122",0,0},	  
	{"../../../../../pattern/enc/input/", "T03", ".inp", "MR122",0,0},	  
	{"../../../../../pattern/enc/input/", "T04", ".inp", "MR122",0,0},	  
	{"../../../../../pattern/enc/input/", "T05", ".inp", "MR122",0,0},	  
	{"../../../../../pattern/enc/input/", "T06", ".inp", "MR122",0,0},	  
	{"../../../../../pattern/enc/input/", "T07", ".inp", "MR122",0,0},	  
	{"../../../../../pattern/enc/input/", "T08", ".inp", "MR122",0,0},	  
	{"../../../../../pattern/enc/input/", "T09", ".inp", "MR122",0,0},	  
	{"../../../../../pattern/enc/input/", "T10", ".inp", "MR122",0,0},	  
	{"../../../../../pattern/enc/input/", "T11", ".inp", "MR122",0,0},	  
	{"../../../../../pattern/enc/input/", "T12", ".inp", "MR122",0,0},	  
	{"../../../../../pattern/enc/input/", "T13", ".inp", "MR122",0,0},	  
	{"../../../../../pattern/enc/input/", "T14", ".inp", "MR122",0,0},	  
	{"../../../../../pattern/enc/input/", "T15", ".inp", "MR122",0,0},	  
	{"../../../../../pattern/enc/input/", "T16", ".inp", "MR122",0,0},	  
	{"../../../../../pattern/enc/input/", "T17", ".inp", "MR122",0,0},	  
	{"../../../../../pattern/enc/input/", "T18", ".inp", "MR122",0,0},	  
	{"../../../../../pattern/enc/input/", "T19", ".inp", "MR122",0,0},	  
	{"../../../../../pattern/enc/input/", "T20", ".inp", "MR122",0,0},	  
	{"../../../../../pattern/enc/input/", "T22", ".inp", "MR122",0,0},	  
#endif

#if VERIFY_MR102
	{"../../../../../pattern/enc/input/", "T00", ".inp", "MR102",0,0},	  
	{"../../../../../pattern/enc/input/", "T01", ".inp", "MR102",0,0},	  
	{"../../../../../pattern/enc/input/", "T02", ".inp", "MR102",0,0},	  
	{"../../../../../pattern/enc/input/", "T03", ".inp", "MR102",0,0},	  
	{"../../../../../pattern/enc/input/", "T04", ".inp", "MR102",0,0},	  
	{"../../../../../pattern/enc/input/", "T05", ".inp", "MR102",0,0},	  
	{"../../../../../pattern/enc/input/", "T06", ".inp", "MR102",0,0},	  
	{"../../../../../pattern/enc/input/", "T07", ".inp", "MR102",0,0},	  
	{"../../../../../pattern/enc/input/", "T08", ".inp", "MR102",0,0},	  
	{"../../../../../pattern/enc/input/", "T09", ".inp", "MR102",0,0},	  
	{"../../../../../pattern/enc/input/", "T10", ".inp", "MR102",0,0},	  
	{"../../../../../pattern/enc/input/", "T11", ".inp", "MR102",0,0},	  
	{"../../../../../pattern/enc/input/", "T12", ".inp", "MR102",0,0},	  
	{"../../../../../pattern/enc/input/", "T13", ".inp", "MR102",0,0},	  
	{"../../../../../pattern/enc/input/", "T14", ".inp", "MR102",0,0},	  
	{"../../../../../pattern/enc/input/", "T15", ".inp", "MR102",0,0},	  
	{"../../../../../pattern/enc/input/", "T16", ".inp", "MR102",0,0},	  
	{"../../../../../pattern/enc/input/", "T17", ".inp", "MR102",0,0},	  
	{"../../../../../pattern/enc/input/", "T18", ".inp", "MR102",0,0},	  
	{"../../../../../pattern/enc/input/", "T19", ".inp", "MR102",0,0},	  
	{"../../../../../pattern/enc/input/", "T20", ".inp", "MR102",0,0},	  
	{"../../../../../pattern/enc/input/", "T22", ".inp", "MR102",0,0},	  
#endif
#if VERIFY_MR795
	{"../../../../../pattern/enc/input/", "T00", ".inp", "MR795",0,0},	  
	{"../../../../../pattern/enc/input/", "T01", ".inp", "MR795",0,0},	  
	{"../../../../../pattern/enc/input/", "T02", ".inp", "MR795",0,0},	  
	{"../../../../../pattern/enc/input/", "T03", ".inp", "MR795",0,0},	  
	{"../../../../../pattern/enc/input/", "T04", ".inp", "MR795",0,0},	  
	{"../../../../../pattern/enc/input/", "T05", ".inp", "MR795",0,0},	  
	{"../../../../../pattern/enc/input/", "T06", ".inp", "MR795",0,0},	  
	{"../../../../../pattern/enc/input/", "T07", ".inp", "MR795",0,0},	  
	{"../../../../../pattern/enc/input/", "T08", ".inp", "MR795",0,0},	  
	{"../../../../../pattern/enc/input/", "T09", ".inp", "MR795",0,0},	  
	{"../../../../../pattern/enc/input/", "T10", ".inp", "MR795",0,0},	  
	{"../../../../../pattern/enc/input/", "T11", ".inp", "MR795",0,0},	  
	{"../../../../../pattern/enc/input/", "T12", ".inp", "MR795",0,0},	  
	{"../../../../../pattern/enc/input/", "T13", ".inp", "MR795",0,0},	  
	{"../../../../../pattern/enc/input/", "T14", ".inp", "MR795",0,0},	  
	{"../../../../../pattern/enc/input/", "T15", ".inp", "MR795",0,0},	  
	{"../../../../../pattern/enc/input/", "T16", ".inp", "MR795",0,0},	  
	{"../../../../../pattern/enc/input/", "T17", ".inp", "MR795",0,0},	  
	{"../../../../../pattern/enc/input/", "T18", ".inp", "MR795",0,0},	  
	{"../../../../../pattern/enc/input/", "T19", ".inp", "MR795",0,0},	  
	{"../../../../../pattern/enc/input/", "T20", ".inp", "MR795",0,0},	  
	{"../../../../../pattern/enc/input/", "T22", ".inp", "MR795",0,0},	  
#endif

#if VERIFY_MR74
	{"../../../../../pattern/enc/input/", "T00", ".inp", "MR74",0,0},	  
	{"../../../../../pattern/enc/input/", "T01", ".inp", "MR74",0,0},	  
	{"../../../../../pattern/enc/input/", "T02", ".inp", "MR74",0,0},	  
	{"../../../../../pattern/enc/input/", "T03", ".inp", "MR74",0,0},	  
	{"../../../../../pattern/enc/input/", "T04", ".inp", "MR74",0,0},	  
	{"../../../../../pattern/enc/input/", "T05", ".inp", "MR74",0,0},	  
	{"../../../../../pattern/enc/input/", "T06", ".inp", "MR74",0,0},	  
	{"../../../../../pattern/enc/input/", "T07", ".inp", "MR74",0,0},	  
	{"../../../../../pattern/enc/input/", "T08", ".inp", "MR74",0,0},	  
	{"../../../../../pattern/enc/input/", "T09", ".inp", "MR74",0,0},	  
	{"../../../../../pattern/enc/input/", "T10", ".inp", "MR74",0,0},	  
	{"../../../../../pattern/enc/input/", "T11", ".inp", "MR74",0,0},	  
	{"../../../../../pattern/enc/input/", "T12", ".inp", "MR74",0,0},	  
	{"../../../../../pattern/enc/input/", "T13", ".inp", "MR74",0,0},	  
	{"../../../../../pattern/enc/input/", "T14", ".inp", "MR74",0,0},	  
	{"../../../../../pattern/enc/input/", "T15", ".inp", "MR74",0,0},	  
	{"../../../../../pattern/enc/input/", "T16", ".inp", "MR74",0,0},	  
	{"../../../../../pattern/enc/input/", "T17", ".inp", "MR74",0,0},	  
	{"../../../../../pattern/enc/input/", "T18", ".inp", "MR74",0,0},	  
	{"../../../../../pattern/enc/input/", "T19", ".inp", "MR74",0,0},	  
	{"../../../../../pattern/enc/input/", "T20", ".inp", "MR74",0,0},	  
	{"../../../../../pattern/enc/input/", "T22", ".inp", "MR74",0,0},	  
#endif

#if VERIFY_MR67
	{"../../../../../pattern/enc/input/", "T00", ".inp", "MR67",0,0},	  
	{"../../../../../pattern/enc/input/", "T01", ".inp", "MR67",0,0},	  
	{"../../../../../pattern/enc/input/", "T02", ".inp", "MR67",0,0},	  
	{"../../../../../pattern/enc/input/", "T03", ".inp", "MR67",0,0},	  
	{"../../../../../pattern/enc/input/", "T04", ".inp", "MR67",0,0},	  
	{"../../../../../pattern/enc/input/", "T05", ".inp", "MR67",0,0},	  
	{"../../../../../pattern/enc/input/", "T06", ".inp", "MR67",0,0},	  
	{"../../../../../pattern/enc/input/", "T07", ".inp", "MR67",0,0},	  
	{"../../../../../pattern/enc/input/", "T08", ".inp", "MR67",0,0},	  
	{"../../../../../pattern/enc/input/", "T09", ".inp", "MR67",0,0},	  
	{"../../../../../pattern/enc/input/", "T10", ".inp", "MR67",0,0},	  
	{"../../../../../pattern/enc/input/", "T11", ".inp", "MR67",0,0},	  
	{"../../../../../pattern/enc/input/", "T12", ".inp", "MR67",0,0},	  
	{"../../../../../pattern/enc/input/", "T13", ".inp", "MR67",0,0},	  
	{"../../../../../pattern/enc/input/", "T14", ".inp", "MR67",0,0},	  
	{"../../../../../pattern/enc/input/", "T15", ".inp", "MR67",0,0},	  
	{"../../../../../pattern/enc/input/", "T16", ".inp", "MR67",0,0},	  
	{"../../../../../pattern/enc/input/", "T17", ".inp", "MR67",0,0},	  
	{"../../../../../pattern/enc/input/", "T18", ".inp", "MR67",0,0},	  
	{"../../../../../pattern/enc/input/", "T19", ".inp", "MR67",0,0},	  
	{"../../../../../pattern/enc/input/", "T20", ".inp", "MR67",0,0},	  
	{"../../../../../pattern/enc/input/", "T22", ".inp", "MR67",0,0},	  
#endif

#if VERIFY_MR59
	{"../../../../../pattern/enc/input/", "T00", ".inp", "MR59",0,0},	  
	{"../../../../../pattern/enc/input/", "T01", ".inp", "MR59",0,0},	  
	{"../../../../../pattern/enc/input/", "T02", ".inp", "MR59",0,0},	  
	{"../../../../../pattern/enc/input/", "T03", ".inp", "MR59",0,0},	  
	{"../../../../../pattern/enc/input/", "T04", ".inp", "MR59",0,0},	  
	{"../../../../../pattern/enc/input/", "T05", ".inp", "MR59",0,0},	  
	{"../../../../../pattern/enc/input/", "T06", ".inp", "MR59",0,0},	  
	{"../../../../../pattern/enc/input/", "T07", ".inp", "MR59",0,0},	  
	{"../../../../../pattern/enc/input/", "T08", ".inp", "MR59",0,0},	  
	{"../../../../../pattern/enc/input/", "T09", ".inp", "MR59",0,0},	  
	{"../../../../../pattern/enc/input/", "T10", ".inp", "MR59",0,0},	  
	{"../../../../../pattern/enc/input/", "T11", ".inp", "MR59",0,0},	  
	{"../../../../../pattern/enc/input/", "T12", ".inp", "MR59",0,0},	  
	{"../../../../../pattern/enc/input/", "T13", ".inp", "MR59",0,0},	  
	{"../../../../../pattern/enc/input/", "T14", ".inp", "MR59",0,0},	  
	{"../../../../../pattern/enc/input/", "T15", ".inp", "MR59",0,0},	  
	{"../../../../../pattern/enc/input/", "T16", ".inp", "MR59",0,0},	  
	{"../../../../../pattern/enc/input/", "T17", ".inp", "MR59",0,0},	  
	{"../../../../../pattern/enc/input/", "T18", ".inp", "MR59",0,0},	  
	{"../../../../../pattern/enc/input/", "T19", ".inp", "MR59",0,0},	  
	{"../../../../../pattern/enc/input/", "T20", ".inp", "MR59",0,0},	  
	{"../../../../../pattern/enc/input/", "T22", ".inp", "MR59",0,0},	  
#endif

#if VERIFY_MR515
	{"../../../../../pattern/enc/input/", "T00", ".inp", "MR515",0,0},	  
	{"../../../../../pattern/enc/input/", "T01", ".inp", "MR515",0,0},	  
	{"../../../../../pattern/enc/input/", "T02", ".inp", "MR515",0,0},	  
	{"../../../../../pattern/enc/input/", "T03", ".inp", "MR515",0,0},	  
	{"../../../../../pattern/enc/input/", "T04", ".inp", "MR515",0,0},	  
	{"../../../../../pattern/enc/input/", "T05", ".inp", "MR515",0,0},	  
	{"../../../../../pattern/enc/input/", "T06", ".inp", "MR515",0,0},	  
	{"../../../../../pattern/enc/input/", "T07", ".inp", "MR515",0,0},	  
	{"../../../../../pattern/enc/input/", "T08", ".inp", "MR515",0,0},	  
	{"../../../../../pattern/enc/input/", "T09", ".inp", "MR515",0,0},	  
	{"../../../../../pattern/enc/input/", "T10", ".inp", "MR515",0,0},	  
	{"../../../../../pattern/enc/input/", "T11", ".inp", "MR515",0,0},	  
	{"../../../../../pattern/enc/input/", "T12", ".inp", "MR515",0,0},	  
	{"../../../../../pattern/enc/input/", "T13", ".inp", "MR515",0,0},	  
	{"../../../../../pattern/enc/input/", "T14", ".inp", "MR515",0,0},	  
	{"../../../../../pattern/enc/input/", "T15", ".inp", "MR515",0,0},	  
	{"../../../../../pattern/enc/input/", "T16", ".inp", "MR515",0,0},	  
	{"../../../../../pattern/enc/input/", "T17", ".inp", "MR515",0,0},	  
	{"../../../../../pattern/enc/input/", "T18", ".inp", "MR515",0,0},	  
	{"../../../../../pattern/enc/input/", "T19", ".inp", "MR515",0,0},	  
	{"../../../../../pattern/enc/input/", "T20", ".inp", "MR515",0,0},	  
	{"../../../../../pattern/enc/input/", "T22", ".inp", "MR515",0,0},	  
#endif

#if VERIFY_MR475
	{"../../../../../pattern/enc/input/", "T00", ".inp", "MR475",0,0},	  
	{"../../../../../pattern/enc/input/", "T01", ".inp", "MR475",0,0},	  
	{"../../../../../pattern/enc/input/", "T02", ".inp", "MR475",0,0},	  
	{"../../../../../pattern/enc/input/", "T03", ".inp", "MR475",0,0},	  
	{"../../../../../pattern/enc/input/", "T04", ".inp", "MR475",0,0},	  
	{"../../../../../pattern/enc/input/", "T05", ".inp", "MR475",0,0},	  
	{"../../../../../pattern/enc/input/", "T06", ".inp", "MR475",0,0},	  
	{"../../../../../pattern/enc/input/", "T07", ".inp", "MR475",0,0},	  
	{"../../../../../pattern/enc/input/", "T08", ".inp", "MR475",0,0},	  
	{"../../../../../pattern/enc/input/", "T09", ".inp", "MR475",0,0},	  
	{"../../../../../pattern/enc/input/", "T10", ".inp", "MR475",0,0},	  
	{"../../../../../pattern/enc/input/", "T11", ".inp", "MR475",0,0},	  
	{"../../../../../pattern/enc/input/", "T12", ".inp", "MR475",0,0},	  
	{"../../../../../pattern/enc/input/", "T13", ".inp", "MR475",0,0},	  
	{"../../../../../pattern/enc/input/", "T14", ".inp", "MR475",0,0},	  
	{"../../../../../pattern/enc/input/", "T15", ".inp", "MR475",0,0},	  
	{"../../../../../pattern/enc/input/", "T16", ".inp", "MR475",0,0},	  
	{"../../../../../pattern/enc/input/", "T17", ".inp", "MR475",0,0},	  
	{"../../../../../pattern/enc/input/", "T18", ".inp", "MR475",0,0},	  
	{"../../../../../pattern/enc/input/", "T19", ".inp", "MR475",0,0},	  
	{"../../../../../pattern/enc/input/", "T20", ".inp", "MR475",0,0},	  
	{"../../../../../pattern/enc/input/", "T22", ".inp", "MR475",0,0},	  
#endif

#if VERIFY_MOD
	/* "__MOD" means useing mod_data[] to test mode switching pattern T21 */
	{"../../../../../pattern/enc/input/", "T21", ".inp", "__MOD",0,0}, 
#endif

#if VERIFY_HOME
	/* homing sequences test */
	{"../../../../../pattern/enc/input/", "T22", ".inp", "MR122",0,0}, 
	{"../../../../../pattern/enc/input/", "T22", ".inp", "MR102",0,0}, 
	{"../../../../../pattern/enc/input/", "T22", ".inp", "MR795",0,0}, 
	{"../../../../../pattern/enc/input/", "T22", ".inp", "MR74",0,0}, 
	{"../../../../../pattern/enc/input/", "T22", ".inp", "MR67",0,0}, 
	{"../../../../../pattern/enc/input/", "T22", ".inp", "MR59",0,0}, 
	{"../../../../../pattern/enc/input/", "T22", ".inp", "MR515",0,0}, 
	{"../../../../../pattern/enc/input/", "T22", ".inp", "MR475",0,0}, 
#endif

#if VERIFY_SEQ
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",0,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",0,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",0,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",0,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",0,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",0,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",0,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",0,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",1,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",1,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",1,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",1,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",1,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",1,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",1,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",1,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",2,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",2,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",2,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",2,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",2,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",2,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",2,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",2,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",3,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",3,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",3,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",3,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",3,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",3,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",3,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",3,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",4,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",4,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",4,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",4,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",4,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",4,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",4,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",4,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",5,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",5,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",5,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",5,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",5,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",5,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",5,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",5,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",6,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",6,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",6,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",6,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",6,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",6,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",6,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",6,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",7,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",7,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",7,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",7,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",7,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",7,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",7,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",7,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",8,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",8,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",8,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",8,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",8,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",8,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",8,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",8,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",9,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",9,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",9,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",9,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",9,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",9,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",9,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",9,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",10,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",10,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",10,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",10,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",10,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",10,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",10,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",10,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",11,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",11,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",11,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",11,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",11,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",11,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",11,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",11,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",12,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",12,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",12,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",12,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",12,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",12,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",12,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",12,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",13,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",13,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",13,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",13,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",13,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",13,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",13,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",13,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",14,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",14,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",14,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",14,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",14,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",14,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",14,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",14,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",15,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",15,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",15,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",15,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",15,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",15,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",15,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",15,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",16,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",16,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",16,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",16,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",16,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",16,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",16,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",16,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",17,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",17,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",17,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",17,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",17,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",17,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",17,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",17,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",18,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",18,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",18,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",18,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",18,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",18,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",18,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",18,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",19,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",19,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",19,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",19,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",19,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",19,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",19,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",19,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",20,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",20,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",20,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",20,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",20,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",20,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",20,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",20,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",21,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",21,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",21,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",21,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",21,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",21,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",21,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",21,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",22,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",22,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",22,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",22,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",22,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",22,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",22,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",22,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",23,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",23,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",23,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",23,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",23,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",23,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",23,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",23,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",24,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",24,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",24,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",24,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",24,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",24,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",24,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",24,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",25,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",25,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",25,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",25,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",25,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",25,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",25,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",25,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",26,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",26,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",26,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",26,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",26,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",26,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",26,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",26,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",27,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",27,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",27,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",27,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",27,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",27,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",27,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",27,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",28,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",28,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",28,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",28,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",28,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",28,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",28,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",28,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",29,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",29,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",29,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",29,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",29,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",29,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",29,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",29,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",30,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",30,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",30,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",30,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",30,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",30,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",30,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",30,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",31,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",31,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",31,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",31,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",31,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",31,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",31,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",31,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",32,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",32,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",32,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",32,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",32,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",32,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",32,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",32,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",33,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",33,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",33,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",33,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",33,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",33,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",33,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",33,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",34,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",34,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",34,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",34,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",34,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",34,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",34,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",34,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",35,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",35,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",35,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",35,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",35,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",35,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",35,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",35,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",36,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",36,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",36,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",36,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",36,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",36,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",36,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",36,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",37,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",37,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",37,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",37,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",37,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",37,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",37,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",37,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",38,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",38,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",38,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",38,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",38,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",38,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",38,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",38,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",39,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",39,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",39,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",39,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",39,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",39,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",39,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",39,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",40,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",40,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",40,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",40,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",40,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",40,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",40,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",40,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",41,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",41,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",41,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",41,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",41,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",41,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",41,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",41,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",42,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",42,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",42,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",42,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",42,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",42,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",42,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",42,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",43,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",43,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",43,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",43,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",43,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",43,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",43,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",43,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",44,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",44,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",44,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",44,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",44,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",44,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",44,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",44,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",45,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",45,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",45,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",45,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",45,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",45,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",45,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",45,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",46,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",46,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",46,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",46,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",46,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",46,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",46,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",46,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",47,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",47,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",47,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",47,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",47,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",47,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",47,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",47,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",48,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",48,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",48,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",48,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",48,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",48,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",48,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",48,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",49,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",49,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",49,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",49,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",49,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",49,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",49,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",49,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",50,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",50,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",50,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",50,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",50,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",50,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",50,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",50,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",51,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",51,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",51,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",51,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",51,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",51,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",51,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",51,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",52,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",52,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",52,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",52,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",52,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",52,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",52,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",52,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",53,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",53,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",53,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",53,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",53,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",53,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",53,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",53,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",54,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",54,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",54,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",54,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",54,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",54,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",54,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",54,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",55,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",55,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",55,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",55,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",55,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",55,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",55,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",55,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",56,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",56,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",56,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",56,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",56,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",56,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",56,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",56,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",57,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",57,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",57,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",57,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",57,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",57,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",57,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",57,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",58,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",58,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",58,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",58,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",58,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",58,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",58,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",58,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",59,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",59,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",59,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",59,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",59,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",59,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",59,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",59,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",60,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",60,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",60,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",60,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",60,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",60,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",60,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",60,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",61,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",61,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",61,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",61,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",61,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",61,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",61,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",61,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",62,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",62,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",62,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",62,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",62,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",62,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",62,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",62,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",63,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",63,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",63,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",63,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",63,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",63,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",63,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",63,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",64,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",64,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",64,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",64,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",64,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",64,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",64,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",64,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",65,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",65,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",65,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",65,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",65,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",65,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",65,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",65,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",66,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",66,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",66,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",66,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",66,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",66,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",66,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",66,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",67,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",67,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",67,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",67,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",67,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",67,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",67,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",67,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",68,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",68,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",68,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",68,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",68,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",68,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",68,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",68,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",69,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",69,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",69,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",69,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",69,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",69,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",69,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",69,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",70,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",70,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",70,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",70,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",70,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",70,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",70,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",70,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",71,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",71,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",71,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",71,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",71,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",71,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",71,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",71,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",72,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",72,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",72,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",72,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",72,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",72,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",72,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",72,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",73,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",73,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",73,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",73,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",73,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",73,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",73,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",73,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",74,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",74,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",74,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",74,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",74,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",74,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",74,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",74,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",75,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",75,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",75,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",75,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",75,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",75,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",75,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",75,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",76,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",76,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",76,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",76,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",76,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",76,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",76,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",76,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",77,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",77,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",77,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",77,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",77,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",77,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",77,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",77,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",78,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",78,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",78,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",78,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",78,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",78,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",78,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",78,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",79,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",79,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",79,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",79,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",79,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",79,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",79,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",79,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",80,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",80,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",80,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",80,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",80,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",80,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",80,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",80,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",81,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",81,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",81,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",81,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",81,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",81,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",81,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",81,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",82,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",82,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",82,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",82,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",82,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",82,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",82,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",82,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",83,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",83,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",83,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",83,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",83,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",83,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",83,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",83,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",84,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",84,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",84,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",84,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",84,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",84,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",84,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",84,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",85,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",85,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",85,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",85,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",85,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",85,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",85,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",85,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",86,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",86,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",86,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",86,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",86,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",86,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",86,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",86,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",87,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",87,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",87,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",87,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",87,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",87,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",87,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",87,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",88,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",88,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",88,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",88,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",88,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",88,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",88,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",88,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",89,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",89,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",89,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",89,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",89,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",89,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",89,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",89,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",90,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",90,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",90,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",90,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",90,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",90,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",90,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",90,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",91,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",91,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",91,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",91,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",91,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",91,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",91,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",91,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",92,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",92,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",92,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",92,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",92,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",92,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",92,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",92,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",93,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",93,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",93,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",93,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",93,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",93,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",93,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",93,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",94,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",94,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",94,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",94,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",94,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",94,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",94,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",94,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",95,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",95,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",95,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",95,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",95,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",95,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",95,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",95,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",96,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",96,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",96,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",96,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",96,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",96,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",96,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",96,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",97,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",97,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",97,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",97,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",97,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",97,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",97,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",97,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",98,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",98,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",98,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",98,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",98,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",98,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",98,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",98,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",99,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",99,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",99,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",99,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",99,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",99,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",99,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",99,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",100,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",100,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",100,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",100,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",100,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",100,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",100,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",100,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",101,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",101,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",101,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",101,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",101,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",101,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",101,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",101,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",102,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",102,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",102,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",102,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",102,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",102,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",102,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",102,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",103,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",103,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",103,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",103,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",103,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",103,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",103,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",103,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",104,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",104,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",104,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",104,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",104,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",104,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",104,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",104,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",105,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",105,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",105,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",105,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",105,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",105,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",105,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",105,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",106,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",106,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",106,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",106,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",106,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",106,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",106,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",106,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",107,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",107,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",107,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",107,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",107,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",107,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",107,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",107,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",108,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",108,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",108,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",108,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",108,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",108,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",108,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",108,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",109,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",109,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",109,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",109,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",109,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",109,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",109,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",109,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",110,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",110,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",110,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",110,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",110,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",110,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",110,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",110,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",111,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",111,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",111,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",111,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",111,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",111,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",111,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",111,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",112,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",112,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",112,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",112,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",112,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",112,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",112,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",112,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",113,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",113,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",113,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",113,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",113,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",113,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",113,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",113,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",114,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",114,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",114,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",114,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",114,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",114,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",114,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",114,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",115,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",115,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",115,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",115,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",115,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",115,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",115,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",115,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",116,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",116,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",116,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",116,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",116,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",116,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",116,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",116,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",117,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",117,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",117,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",117,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",117,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",117,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",117,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",117,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",118,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",118,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",118,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",118,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",118,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",118,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",118,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",118,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",119,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",119,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",119,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",119,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",119,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",119,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",119,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",119,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",120,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",120,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",120,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",120,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",120,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",120,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",120,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",120,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",121,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",121,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",121,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",121,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",121,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",121,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",121,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",121,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",122,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",122,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",122,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",122,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",122,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",122,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",122,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",122,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",123,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",123,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",123,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",123,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",123,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",123,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",123,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",123,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",124,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",124,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",124,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",124,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",124,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",124,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",124,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",124,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",125,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",125,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",125,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",125,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",125,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",125,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",125,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",125,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",126,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",126,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",126,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",126,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",126,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",126,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",126,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",126,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",127,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",127,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",127,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",127,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",127,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",127,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",127,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",127,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",128,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",128,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",128,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",128,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",128,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",128,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",128,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",128,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",129,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",129,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",129,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",129,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",129,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",129,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",129,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",129,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",130,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",130,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",130,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",130,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",130,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",130,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",130,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",130,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",131,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",131,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",131,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",131,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",131,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",131,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",131,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",131,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",132,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",132,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",132,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",132,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",132,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",132,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",132,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",132,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",133,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",133,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",133,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",133,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",133,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",133,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",133,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",133,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",134,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",134,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",134,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",134,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",134,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",134,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",134,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",134,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",135,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",135,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",135,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",135,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",135,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",135,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",135,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",135,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",136,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",136,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",136,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",136,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",136,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",136,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",136,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",136,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",137,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",137,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",137,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",137,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",137,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",137,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",137,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",137,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",138,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",138,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",138,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",138,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",138,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",138,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",138,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",138,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",139,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",139,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",139,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",139,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",139,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",139,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",139,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",139,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",140,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",140,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",140,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",140,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",140,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",140,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",140,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",140,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",141,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",141,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",141,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",141,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",141,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",141,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",141,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",141,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",142,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",142,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",142,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",142,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",142,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",142,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",142,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",142,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",143,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",143,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",143,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",143,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",143,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",143,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",143,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",143,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",144,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",144,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",144,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",144,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",144,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",144,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",144,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",144,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",145,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",145,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",145,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",145,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",145,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",145,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",145,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",145,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",146,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",146,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",146,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",146,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",146,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",146,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",146,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",146,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",147,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",147,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",147,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",147,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",147,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",147,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",147,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",147,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",148,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",148,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",148,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",148,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",148,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",148,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",148,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",148,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",149,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",149,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",149,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",149,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",149,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",149,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",149,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",149,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",150,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",150,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",150,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",150,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",150,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",150,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",150,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",150,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",151,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",151,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",151,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",151,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",151,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",151,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",151,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",151,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",152,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",152,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",152,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",152,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",152,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",152,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",152,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",152,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",153,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",153,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",153,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",153,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",153,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",153,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",153,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",153,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",154,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",154,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",154,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",154,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",154,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",154,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",154,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",154,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",155,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",155,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",155,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",155,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",155,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",155,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",155,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",155,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",156,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",156,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",156,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",156,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",156,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",156,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",156,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",156,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",157,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",157,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",157,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",157,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",157,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",157,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",157,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",157,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",158,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",158,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",158,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",158,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",158,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",158,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",158,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",158,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS_122", ".inp", "MR122",159,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR102",159,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR795",159,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR74",159,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR67",159,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR59",159,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR515",159,0},
{"../../../../../pattern/enc/SYNC/SYNC_INP/", "SEQS", ".inp", "MR475",159,0},
#endif


};

#define SUB_DIR "encode/"
#if defined(OPT_AMR_FORMAT)
# define WFILE_EXT ".amr"
#else
# define WFILE_EXT ".cod"
#endif

#define FILE_SUFFIX ""

char * mode_data[]=
{
"MR122","MR122","MR102","MR102","MR795","MR795","MR74","MR74","MR67","MR67",
"MR59","MR59","MR515","MR515","MR475","MR475","MR515","MR515","MR59","MR59",
"MR67","MR67","MR74","MR74","MR795","MR795","MR102","MR102","MR122","MR122",
"MR102","MR102","MR795","MR795","MR74","MR74","MR67","MR67","MR59","MR59",
"MR515","MR515","MR475","MR475","MR515","MR515","MR59","MR59","MR67","MR67",
"MR74","MR74","MR795","MR795","MR102","MR102","MR122","MR122","MR102","MR102",
"MR795","MR795","MR74","MR74","MR67","MR67","MR59","MR59","MR515","MR515",
"MR475","MR475","MR515","MR515","MR59","MR59","MR67","MR67","MR74","MR74",
"MR795","MR795","MR102","MR102","MR122","MR122","MR102","MR102","MR795","MR795",
"MR74","MR74","MR67","MR67","MR59","MR59","MR515","MR515","MR475","MR475",
"MR515","MR515","MR59","MR59","MR67","MR67","MR74","MR74","MR795","MR795",
"MR102","MR102","MR122","MR122","MR102","MR102","MR795","MR795","MR74","MR74",
"MR67","MR67","MR59","MR59","MR515","MR515","MR475","MR475","MR515","MR515",
"MR59","MR59","MR67","MR67","MR74","MR74","MR795","MR795","MR102","MR102",
"MR122","MR122","MR102","MR102","MR795","MR795","MR74","MR74","MR67","MR67",
"MR59","MR59","MR515","MR515","MR475","MR475","MR515","MR515","MR59","MR59",
"MR67","MR67","MR74","MR74","MR795","MR795","MR102","MR102","MR122","MR122",
"MR102","MR102","MR795","MR795","MR74","MR74","MR67","MR67","MR59","MR59",
"MR515","MR515","MR475","MR475","MR515","MR515","MR59","MR59","MR67","MR67",
"MR74","MR74","MR795","MR795","MR102","MR102","MR122","MR122","MR102","MR102",
"MR795","MR795","MR74","MR74","MR67","MR67","MR59","MR59","MR515","MR515",
"MR475","MR475","MR515","MR515","MR59","MR59","MR67","MR67","MR74","MR74",
"MR795","MR795","MR102","MR102","MR122","MR122","MR102","MR102","MR795",
"MR795","MR74","MR74","MR67","MR67","MR59","MR59","MR515","MR515","MR475",
"MR475","MR515","MR515","MR59","MR59","MR67","MR67","MR74","MR74","MR795",
"MR795","MR102","MR102","MR122","MR122","MR102","MR102","MR795","MR795",
"MR74","MR74","MR67","MR67","MR59","MR59","MR515","MR515","MR475","MR475",
"MR515","MR515","MR59","MR59","MR67","MR67","MR74","MR74","MR795","MR795",
"MR102","MR102","MR122","MR122","MR102","MR102","MR795","MR795","MR74",
"MR74","MR67","MR67","MR59","MR59","MR515","MR515","MR475","MR475","MR515",
"MR515","MR59","MR59","MR67","MR67","MR74","MR74","MR795","MR795","MR102",
"MR102","MR122","MR122","MR102","MR102","MR795","MR795","MR74","MR74",
"MR67","MR67","MR59","MR59","MR515","MR515","MR475","MR475","MR515",
"MR515","MR59","MR59","MR67","MR67","MR74","MR74","MR795","MR795","MR102",
"MR102","MR122","MR122","MR102","MR102","MR795","MR795","MR74","MR74",
"MR67","MR67","MR59","MR59","MR515","MR515","MR475","MR475","MR515",
"MR515","MR59","MR59","MR67","MR67","MR74","MR74","MR795","MR795",
"MR102","MR102","MR122","MR122","MR102","MR102","MR795","MR795","MR74",
"MR74","MR67","MR67","MR59","MR59","MR515","MR515","MR475","MR475",
"MR515","MR515","MR59","MR59","MR67","MR67","MR74","MR74","MR795",
"MR795","MR102","MR102","MR122","MR122","MR102","MR102","MR795",
"MR795","MR74","MR74","MR67","MR67","MR59","MR59","MR515","MR515","MR475",
"MR475","MR515","MR515","MR59","MR59","MR67","MR67","MR74","MR74","MR795",
"MR795","MR102","MR102","MR122","MR122","MR102","MR102","MR795","MR795",
"MR74","MR74","MR67","MR67","MR59","MR59","MR515","MR515","MR475","MR475",
"MR515","MR515","MR59","MR59","MR67","MR67","MR74","MR74","MR795","MR795",
"MR102","MR102","MR122","MR122","MR102","MR102","MR795","MR795","MR74",
"MR74","MR67","MR67","MR59","MR59","MR515","MR515","MR475","MR475","MR515",
"MR515","MR59","MR59","MR67","MR67","MR74","MR74","MR795","MR795","MR102",
"MR102","MR122","MR122","MR102","MR102","MR795","MR795","MR74","MR74",
"MR67","MR67","MR59","MR59","MR515","MR515","MR475","MR475","MR515","MR515",
"MR59","MR59","MR67","MR67","MR74","MR74","MR795","MR795","MR102","MR102",
"MR122","MR122","MR102","MR102","MR795","MR795","MR74","MR74","MR67",
"MR67","MR59","MR59","MR515","MR515","MR475","MR475","MR515","MR515",
"MR59","MR59","MR67","MR67","MR74","MR74","MR795","MR795","MR102","MR102",
"MR122","MR122","MR102","MR102","MR795","MR795","MR74","MR74","MR67",
"MR67","MR59","MR59","MR515","MR515","MR475","MR475","MR515","MR515",
"MR59","MR59","MR67","MR67","MR74","MR74","MR795","MR795","MR102","MR102",
"MR122","MR122","MR102","MR102","MR795","MR795","MR74","MR74","MR67","MR67",
"MR59","MR59","MR515","MR515","MR475","MR475","MR515","MR515","MR59","MR59",
"MR67","MR67","MR74","MR74","MR795","MR795","MR102","MR102","MR122","MR122",
"MR102","MR102","MR795","MR795","MR74","MR74","MR67","MR67","MR59","MR59",
"MR515","MR515","MR475","MR475","MR515","MR515","MR59","MR59","MR67","MR67",
"MR74","MR74","MR795","MR795","MR102","MR102","MR122","MR122","MR102","MR102",
"MR795","MR795","MR74","MR74","MR67","MR67","MR59","MR59","MR515","MR515",
"MR475","MR475","MR515","MR515","MR59","MR59","MR67","MR67","MR74","MR74",
"MR795","MR795","MR102","MR102","MR122","MR122","MR102","MR102","MR795",
"MR795","MR74","MR74","MR67","MR67","MR59","MR59","MR515","MR515","MR475",
"MR475","MR515","MR515","MR59","MR59","MR67","MR67","MR74","MR74","MR795",
"MR795","MR102","MR102","MR122","MR122","MR102","MR102","MR795","MR795","MR74",
"MR74","MR67","MR67","MR59","MR59","MR515","MR515","MR475","MR475","MR515",
"MR515","MR59","MR59","MR67","MR67","MR74","MR74","MR795","MR795","MR102",
"MR102","MR122","MR122","MR102","MR102","MR795","MR795","MR74","MR74","MR67",
"MR67","MR59","MR59","MR515","MR515","MR475","MR475","MR515","MR515","MR59",
"MR59","MR67","MR67","MR74","MR74","MR795","MR795","MR102","MR102","MR122",
"MR122","MR102","MR102","MR795","MR795","MR74","MR74","MR67","MR67","MR59",
"MR59","MR515","MR515","MR475","MR475","MR515","MR515","MR59","MR59","MR67",
"MR67","MR74","MR74","MR795","MR795","MR102","MR102","MR122","MR122","MR102",
"MR102","MR795","MR795","MR74","MR74","MR67","MR67","MR59","MR59","MR515",
"MR515","MR475","MR475","MR515","MR515","MR59","MR59","MR67","MR67","MR74",
"MR74","MR795","MR795","MR102","MR102","MR122","MR122","MR102","MR102","MR795",
"MR795","MR74","MR74","MR67","MR67","MR59","MR59","MR515","MR515","MR475",
"MR475","MR515","MR515","MR59","MR59","MR67","MR67","MR74","MR74","MR795",
"MR795","MR102","MR102","MR122","MR122","MR102","MR102","MR795","MR795",
"MR74","MR74","MR67","MR67","MR59","MR59","MR515","MR515","MR475","MR475",
"MR515","MR515","MR59","MR59","MR67","MR67","MR74","MR74","MR795","MR795",
"MR102","MR102","MR122","MR122","MR102","MR102","MR795","MR795","MR74",
"MR74","MR67","MR67","MR59","MR59","MR515","MR515","MR475","MR475","MR515",
"MR515","MR59","MR59","MR67","MR67","MR74","MR74","MR795","MR795","MR102",
"MR102","MR122","MR122","MR102","MR102","MR795","MR795","MR74","MR74",
"MR67","MR67","MR59","MR59","MR515","MR515","MR475","MR475","MR515","MR515",
"MR59","MR59","MR67","MR67","MR74","MR74","MR795","MR795","MR102","MR102",
"MR122","MR122","MR102","MR102","MR795","MR795","MR74","MR74","MR67","MR67",
"MR59","MR59","MR515","MR515","MR475","MR475","MR515","MR515","MR59","MR59",
"MR67","MR67","MR74","MR74","MR795","MR795","MR102","MR102","MR122","MR122",
"MR102","MR102","MR795","MR795","MR74","MR74","MR67","MR67","MR59","MR59",
"MR515","MR515","MR475","MR475","MR515","MR515","MR59","MR59","MR67","MR67",
"MR74","MR74","MR795","MR795","MR102","MR102","MR122","MR122","MR102","MR102",
"MR795","MR795","MR74","MR74","MR67","MR67","MR59","MR59","MR515","MR515",
"MR475","MR475","MR515","MR515","MR59","MR59","MR67","MR67","MR74","MR74",
"MR795","MR795","MR102","MR102","MR122","MR122","MR102","MR102","MR795",
"MR795","MR74","MR74","MR67","MR67","MR59","MR59","MR515","MR515","MR475",
"MR475","MR515","MR515","MR59","MR59","MR67","MR67","MR74","MR74","MR795",
"MR795","MR102","MR102","MR122","MR122","MR102","MR102","MR795","MR795",
"MR74","MR74","MR67","MR67","MR59","MR59","MR515","MR515","MR475","MR475",
"MR515","MR515","MR59","MR59","MR67","MR67","MR74","MR74","MR795","MR795",
"MR102","MR102","MR122","MR122","MR102","MR102","MR795","MR795","MR74",
"MR74","MR67","MR67","MR59","MR59","MR515","MR515","MR475","MR475","MR515",
"MR515","MR59","MR59","MR67","MR67","MR74","MR74","MR795","MR795","MR102",
"MR102","MR122","MR122","MR102","MR102","MR795","MR795","MR74","MR74",
"MR67","MR67","MR59","MR59","MR515","MR515","MR475","MR475","MR515","MR515",
"MR59","MR59","MR67","MR67","MR74","MR74","MR795","MR795","MR102","MR102",
"MR122","MR122","MR102","MR102","MR795","MR795","MR74","MR74","MR67","MR67",
"MR59","MR59","MR515","MR515","MR475","MR475","MR515","MR515","MR59","MR59",
"MR67","MR67","MR74","MR74","MR795","MR795","MR102","MR102","MR122","MR122",
"MR102","MR102","MR795","MR795","MR74","MR74","MR67","MR67","MR59","MR59",
"MR515","MR515","MR475","MR475","MR515","MR515","MR59","MR59","MR67","MR67",
"MR74","MR74","MR795","MR795","MR102","MR102","MR122","MR122","MR102","MR102",
"MR795","MR795","MR74","MR74","MR67","MR67","MR59","MR59","MR515","MR515",
"MR475","MR475","MR515","MR515","MR59","MR59","MR67","MR67","MR74","MR74",
"MR795","MR795","MR102","MR102","MR122","MR122","MR102","MR102","MR795",
"MR795","MR74","MR74","MR67","MR67","MR59","MR59","MR515","MR515","MR475",
"MR475","MR515","MR515","MR59","MR59","MR67","MR67","MR74","MR74","MR795",
"MR795","MR102","MR102","MR122","MR122","MR102","MR102","MR795","MR795",
"MR74","MR74","MR67","MR67","MR59","MR59","MR515","MR515","MR475","MR475",
"MR515","MR515","MR59","MR59","MR67","MR67","MR74","MR74","MR795","MR795",
"MR102","MR102","MR122","MR122","MR102","MR102","MR795","MR795","MR74",
"MR74","MR67","MR67","MR59","MR59","MR515","MR515","MR475","MR475","MR515",
"MR515","MR59","MR59","MR67","MR67","MR74","MR74","MR795","MR795","MR102",
"MR102","MR122","MR122","MR102","MR102","MR795","MR795","MR74","MR74",
"MR67","MR67","MR59","MR59","MR515","MR515","MR475","MR475","MR515","MR515",
"MR59","MR59","MR67","MR67","MR74","MR74","MR795","MR795","MR102","MR102",
"MR122","MR122","MR102","MR102","MR795","MR795","MR74","MR74","MR67","MR67",
"MR59","MR59","MR515","MR515","MR475","MR475","MR515","MR515","MR59","MR59",
"MR67","MR67","MR74","MR74","MR795","MR795","MR102","MR102","MR122","MR122",
"MR102","MR102","MR795","MR795","MR74","MR74","MR67","MR67","MR59","MR59",
"MR515","MR515","MR475","MR475","MR515","MR515","MR59","MR59","MR67","MR67",
"MR74","MR74","MR795","MR795","MR102","MR102","MR122","MR122","MR102","MR102",
"MR795","MR795","MR74","MR74","MR67","MR67","MR59","MR59","MR515","MR515",
"MR475","MR475","MR515","MR515","MR59","MR59","MR67","MR67","MR74","MR74",
"MR795","MR795","MR102","MR102","MR122","MR122","MR102","MR102","MR795",
"MR795","MR74","MR74","MR67","MR67","MR59","MR59","MR515","MR515","MR475",
"MR475","MR515","MR515","MR59","MR59","MR67","MR67","MR74","MR74","MR795",
"MR795","MR102","MR102",
};

/*
*****************************************************************************
*                         LOCAL PROGRAM CODE
*****************************************************************************
*/
/*
 * read_mode  read next mode from mode file
 *
 * return 0 on success, EOF on end of file, 1 on other error
 */
int read_mode(FILE *file_modes, enum Mode *mode)
{
    char buf[10];
    
    if (fscanf(file_modes, "%9s\n", buf) != 1) {
        if (feof(file_modes))
            return EOF;

        printf("\nerror reading mode control file: %s\n",
                strerror(errno));
        return 1;
    }

    if (str2mode(buf, mode) != 0 || *mode == MRDTX) {
       printf("\ninvalid amr_mode found in mode control file: '%s'\n",
                buf);
        return 1;
    }

    return 0;
}


/*
*****************************************************************************
*                             MAIN PROGRAM 
*****************************************************************************
*/
int main(void)
{
  char *progname = "enc_arm";
  char *modeStr = NULL;
  char *usedModeStr = NULL;
  char *fileName = NULL;
  char *serialFileName = NULL;

  //char *modefileName = NULL;
  FILE *file_speech = NULL;           /* File of speech data               */
  FILE *file_serial = NULL;           /* File of coded bits                */
  //FILE *file_modes = NULL;            /* File with mode information        */
  
  Word16 *ptr_in,*ptr_out;
  Word16 new_speech[L_FRAME];         /* Pointer to new speech data        */
  Word16 serial[SERIAL_FRAMESIZE];    /* Output bitstream buffer           */
#if defined(OPT_AMR_FORMAT)
  Word16 serial_amr[SERIAL_FRAMESIZE];    /* Output bitstream buffer for amr format */
  int len_amr; /* packet length of amr format */
#endif

  Word32 frame;
  Word16 dtx = 0;                     /* enable encoder DTX                */
  
  /* changed eedodr */
  Word16 reset_flag;

  int i;
  enum Mode mode;
  enum Mode used_mode;
  enum TXFrameType tx_type;

  int useModeFile = 0;
  
  Speech_Encode_FrameState *speech_encoder_state = NULL;
  sid_syncState *sid_state = NULL;

	int nfile,k;
	char in_file[128]="";
	char out_file[128]="";
	int syncTest = 0;

  proc_head ("Encoder");
 
  printf("Code compiled with VAD option: %s\n\n", get_vadname());
  
  /*----------------------------------------------------------------------*
   * Process command line options                                         *
   *----------------------------------------------------------------------*/
  printf("pragram start\n");
  /*----------------------------------------------------------------------*
   * Check number of arguments                                            *
   *----------------------------------------------------------------------*/
/*
  if (   (argc != 4 && !useModeFile)
      || (argc != 3 &&  useModeFile))
  {
    printf (
      " Usage:\n\n"
      "   %s [-dtx] amr_mode            speech_file  bitstream_file\n\n"
      " or \n\n"
      "   %s [-dtx] -modefile=mode_file speech_file  bitstream_file\n\n"
      " -dtx                enables DTX mode\n"
      " -modefile=mode_file reads AMR modes from text file (one line per frame)\n\n",
             progname, progname);
      exit (1);
  }
*/
	nfile=sizeof(file_list)/sizeof(file_list[0]);
	printf("There are %d input files\n",nfile);

	for(k=0;k<nfile;k++)	
	{
		/* input file name and path */
		strcpy(in_file, file_list[k].path);
		strcat(in_file, file_list[k].name);
		strcat(in_file, file_list[k].ext);

		/* output file name and path */
		strcpy(out_file, file_list[k].path);
		strcat(out_file, SUB_DIR);
		if( !strncmp(file_list[k].name,"SEQS",4) )
		{
			char str[128];
			sprintf(str,"S%03d",file_list[k].position);
			strcat(out_file, str);
			syncTest = 1;
		}
		else
			strcat(out_file, file_list[k].name);
		strcat(out_file, "_");
		strcat(out_file, &file_list[k].mode[2]);
		strcat(out_file, WFILE_EXT);
		if( !strncmp(file_list[k].mode,"__MOD",5) )
			useModeFile = 1; /* perform mode switching test */
		else
			useModeFile = 0;

		/*----------------------------------------------------------------------*
		* Open mode file or convert mode string                                *
		*----------------------------------------------------------------------*/
		modeStr = file_list[k].mode;//ex  MR515
		fileName = in_file;//argv[2];//ex  inputfile.inp
		serialFileName = out_file;//argv[3];//ex  tmp.cod
		dtx = file_list[k].dtx;
		if((file_speech=fopen(fileName,"rb"))==NULL)
		{
			printf("Can't open speech file: %s\n",fileName);
			exit(1);
		}
		if((file_serial=fopen(serialFileName,"wb"))==NULL)
		{
			printf("Can't open serial output file: %s\n",serialFileName);
			exit(1);
		}
#if defined(OPT_AMR_FORMAT)
		/* AMR ID for amr file format */
		fwrite("#!AMR\n",6,1,file_serial);
#endif

		/* check and convert mode string */  // mode save enum mode op num
		//modeStr=m[frame];
		if( !useModeFile )
		{
			if (str2mode(modeStr, &mode) != 0 && mode != MRDTX)
			{
				printf("Invalid amr_mode specified: '%s'\n",modeStr);
				exit(1);
			}
		}

		printf("Input : %s\n",in_file);
		printf("Output: %s\n",out_file);
		if( dtx )
		{
#ifndef VAD2
			printf("DTX Enable, VAD1 algorithm used\n");
#else
			printf("DTX Enable, VAD2 algorithm used\n");
#endif
		}
		//printf("pragram mode at %s  %d\n",modeStr,mode);
		/*-----------------------------------------------------------------------*
		 * Initialisation of the coder.                                          *
		 *-----------------------------------------------------------------------*/
		if (   Speech_Encode_Frame_init(&speech_encoder_state, dtx, "encoder")
			|| sid_sync_init (&sid_state))
		exit(-1);

		/*-----------------------------------------------------------------------*
		 * Process speech frame by frame                                         *
		 *-----------------------------------------------------------------------*/
		frame = 0;
		while(1)
		{
			if( !frame && syncTest)
			{
				int position = file_list[k].position;
				if( !(fread(new_speech+position, 1, (L_FRAME-position)*sizeof(Word16), file_speech) == (L_FRAME-position)*sizeof(Word16)) )
					break;
				for(i=0;i<position;i++)
					new_speech[i] = 0;
			}
			else
			{
				if( !(fread(new_speech, 1, L_FRAME*sizeof(Word16), file_speech) == L_FRAME*sizeof(Word16)) )
					break;
			}
			//if(!frame) new_speech[0] = 0;
			/* read mode if performing mode switching test */
			if( useModeFile ) str2mode(mode_data[frame], &mode);

			frame++;
     
			for (i = 0; i < SERIAL_FRAMESIZE; i++)
				serial[i] = 0;

			/* check for homing frame */
			reset_flag = encoder_homing_frame_test(new_speech); // homing frame let sys reset
     
			/* encode speech */
			Speech_Encode_Frame(speech_encoder_state, mode,
								new_speech, &serial[1], &used_mode); 

     
			mode2str(mode, &modeStr);
			mode2str(used_mode, &usedModeStr);
     
			if( (frame&0xF) == 0)
				printf ("\n\rframe=%-8d mode=%-5s used_mode=%-5s", frame, modeStr, usedModeStr);
     
			/* include frame type and mode information in serial bitstream */
			sid_sync (sid_state, used_mode, &tx_type);
			serial[0] = tx_type;
			if (tx_type != TX_NO_DATA)
				serial[1+MAX_SERIAL_SIZE] = mode;
			else
				serial[1+MAX_SERIAL_SIZE] = -1;
#if defined(OPT_AMR_FORMAT)
			/* Convert cod file format into amr file */
			cod2amr(serial,serial_amr, mode, &len_amr);

			/* write bitstream to output file */
			fwrite(serial_amr, 1, len_amr, file_serial);
#else
			/* write bitstream to output file */
			if( !syncTest )
				fwrite(serial, 1, SERIAL_FRAMESIZE*sizeof(Word16), file_serial);
			else
			{
				if( frame == 5 ) fwrite(serial, 1, SERIAL_FRAMESIZE*sizeof(Word16), file_serial);
			}
#endif

			/* perform homing if homing frame was detected at encoder input */
			if (reset_flag != 0)
			{
				Speech_Encode_Frame_reset(speech_encoder_state);
				sid_sync_reset(sid_state);
			}
		} // while(1)
		printf ("\n%d frame(s) processed\n", frame);
  
		/*-----------------------------------------------------------------------*
		 * Close down speech coder                                               *
		 *-----------------------------------------------------------------------*/
		Speech_Encode_Frame_exit(&speech_encoder_state);
		sid_sync_exit (&sid_state);

		fclose(file_speech);
		fclose(file_serial);

	} // for each file

	return (0);
}

#if 0

//change the stack/heap location to read the stack size from ARMulator
#include "rt_misc.h"
__value_in_regs struct __initial_stackheap __user_initial_stackheap(
        unsigned R0, unsigned SP, unsigned R2, unsigned SL)
{
    struct __initial_stackheap config;
    config.heap_base = 0x20000;
	config.heap_limit = 0x100000;
    config.stack_base = 0x100000;   
    config.stack_limit= 0x20000;   
    return config;
}

#endif

