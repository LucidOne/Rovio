//#define _TEST_EEC

#define _EEC


#define AEC_ADAPTIVEFILE_ORDER	64

#ifdef _EEC
#define EEC_ADAPTIVEFILE_ORDER	64
#define _INCOMING_CALL_TRAIN_FRAME	10
#define _MAX_NOISE_EEC  			800	
#endif

#define _DOUBLE_TALK		4
#define _SINGLE_NEAR		3
#define _SINGLE_FAR			2
#define _MUTUAL_SILENCE		1


#define TRAIN_LEN1			500
#define TRAIN_LEN2			800



#define AMR_BLOCK	160
#define ESTIMATION_DURING	8
#define _INCREASE	32	// 1/1024 in Q15

#define _LAMDA			5

#define _LAMDA_ERL		5
#define _LAMDA_ERLE  	4

#define _LAMDA_ERL_THR	3
#define _LAMDA_ERLE_THR	2

#define _NLP_scal_factor		6
#define _NLP_threshold_floor	2

#define _SUPPRESSOR_COUNT_CONST	15360
#define _SUPPRESSOR_SCALE		4


#define _UPDATE_COUNT_THRESHOLD  30
#define _VAD_X_COUNT_THRESHOLD	300



/* test for experiment value */ 
#define NOISE_OFFSET			60	
#define _MAX_NOISE  			800	
#define _MAX_NOISE_OFFSET		200

#define _PRE_D_SCALE			128

#define _ERL_hi_constant		300
#define _ERL_lo_constant		100
#define _ERLE_hi_constant		300
#define _ERLE_lo_constant		100

#define _RESET_COUNT_THRESHOLD	3000

/*****************************/

#define		MAX16	 32767//0x7fff
#define 	MIN16	-32768//0x8000


#define 	Q16Shift		16
#define 	Q15Shift		15

//typedef BOOL	char

typedef struct
{
	short			sFilterCoef[AEC_ADAPTIVEFILE_ORDER];
	short 			sConvBuff[AEC_ADAPTIVEFILE_ORDER];
	short			sEx[ESTIMATION_DURING+1];
	short			sNx[2];
	short			sNy[2];
	char			bBx;
	char 			VAD_X_FLAG;
	int				VAD_X_COUNT;
	short			sY_buf[ESTIMATION_DURING+1];
	short			sE_buf[ESTIMATION_DURING+1];
	short			sEy[ESTIMATION_DURING+1];
	short			sEe[ESTIMATION_DURING+1];
	short			sERL[ESTIMATION_DURING+1];
	short			sERLE[ESTIMATION_DURING+1];
	char			bBy[2];			
	short			VAD_Y_COUNT;
	char			VAD_Y_FLAG;
	char			DTD_state;
	short  			usSuppressorCount;
	short			uSuppressor_down_factor;
	unsigned short	usUpdate_count;
	char			bCoefficient_update;
	unsigned short	usReset_count;
	char			bCoefficient_reset;
	short			sERL_threshold,sERLE_threshold;
	short			sEx2;
	char			cStepSize;
	char			bAECEEC_SW;	
#ifdef _EEC	
	char			bEECTrain;
	char			cFrame;
	short			sEECFilterCoef[EEC_ADAPTIVEFILE_ORDER];
	short 			sEECConvBuff[EEC_ADAPTIVEFILE_ORDER];
	short			sEx2_EEC;

	
	short			sEECEx[ESTIMATION_DURING+1];
	short			sEECNx[2];
	char			bEECBx;
	char 			EECVAD_X_FLAG;
	int				EECVAD_X_COUNT;	
	short  			usEECSuppressorCount;
	short			uEECSuppressor_down_factor;	
#endif	
} ECHOCANCELL_STRUCT;





short EchoCancellationProcessing(ECHOCANCELL_STRUCT *sControl, short *ReceiveDate_Xn, short sLenBySample, short *TransmitData_Yn);

short EchoCancellationReset(ECHOCANCELL_STRUCT *sControl);

#ifdef _EEC
short ElectricalEchoCancellationTraining(ECHOCANCELL_STRUCT *sControl, short *ReceiveDate_Xn, short sLenBySample,short sTrainFrameSize, short *TransmitData_Yn);
short GetWhiteNoise(void);
#endif
