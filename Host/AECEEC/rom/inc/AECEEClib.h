

#define _EEC



#define AEC_ADAPTIVEFILE_ORDER	160

#ifdef _EEC
#define EEC_ADAPTIVEFILE_ORDER	80
#endif



#define AMR_BLOCK	160
#define LEN	AMR_BLOCK
#define ESTIMATION_DURING	8


/*****************************/

#define		MAX16	 32767//0x7fff
#define 	MIN16	-32768//0x8000




typedef struct
{
	short			sFilterCoef[AEC_ADAPTIVEFILE_ORDER];
	short 			sConvBuff[2*LEN];
	
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
	short 			sEECConvBuff[2*LEN];
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
