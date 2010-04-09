#ifndef _CODAMR_H
#define _CODAMR_H
//-----------------------------------------------------------------------------------------------

/*  define myself data type  									*/
typedef unsigned char		byte;
typedef short				Word16;  
typedef int					Word32;
#define HeadSize			6
#define TX_SPEECH           0
#define TX_SID_FIRST		1
#define TX_SID_UPDATE		2
#define TX_NO_DATA          3
/****************************************************************/
//#define _DISPLAY

#define AMR_475_SizeOfCoreFrame			95
#define AMR_515_SizeOfCoreFrame			103
#define AMR_59_SizeOfCoreFrame			118
#define AMR_67_SizeOfCoreFrame			134
#define AMR_74_SizeOfCoreFrame			148
#define AMR_795_SizeOfCoreFrame			159
#define AMR_102_SizeOfCoreFrame 		204
#define AMR_122_SizeOfCoreFrame 		244
#define AMR_SID_SizeOfCoreFrame			35
#define GSM_EFR_SID_SizeOfCoreFrame		43
#define TDMA_EFR_SID_SizeOfCoreFrame	38
#define PDC_EFR_SID_SizeOfCoreFrame		37
#define future_size						-1
#define NO_DATA_SizeOfCoreFrame      	0
/****************************************************************/
#define	MR475_index						0x0000
#define	MR515_index						0x0001            
#define	MR59_index						0x0002
#define	MR67_index						0x0003
#define	MR74_index						0x0004
#define	MR795_index						0x0005
#define	MR102_index						0x0006
#define	MR122_index						0x0007

/***************************************************************/
#define	MR475_ByteSize						12
#define	MR515_ByteSize						13            
#define	MR59_ByteSize						15
#define	MR67_ByteSize						17
#define	MR74_ByteSize						19
#define	MR795_ByteSize						20
#define	MR102_ByteSize						26
#define	MR122_ByteSize						31
#define	AMR_SID_ByteSize					5
/**************************************************************/

int cod2amr(Word16 *serial_in,Word16 *out_buff,int mode, int *size);
int amr2cod(Word16 *in,Word16 *out);
int amr_packet_size(int mode);



//-----------------------------------------------------------------------------------------------
#endif