#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "AECEEClib.h"
#include "h512.h"



#define _TEST_EEC




#define SQ(x)	((x)*(x))
#define ROUND_DIV(a,b)	( ((a)+(b/2)) /(b) )


short BUF[AEC_ADAPTIVEFILE_ORDER];

/* to get the echo signal, the d is echo buffer, the xin is far end voice buffer, and 
   W is prepare echo paht pointer 
   
*/
void GetTarget(short *d, short *xin,short *W)
{
	short i,j;
	
	int sum;
	
	for(i=0;i<AMR_BLOCK;i++)
	{
#ifdef _EEC		
		j = EEC_ADAPTIVEFILE_ORDER-1;
#else		
		j = AEC_ADAPTIVEFILE_ORDER-1;	
#endif
		while(j>0)
		{
			BUF[j] = BUF[j-1];
			j--;
		}
		BUF[0] = xin[i];

		j = 0;
		sum = 0;
#ifdef _EEC		
		while(j!=EEC_ADAPTIVEFILE_ORDER)
#else		
		while(j!=AEC_ADAPTIVEFILE_ORDER)
#endif
		{
			sum += ROUND_DIV(W[j] * BUF[j],32768);
			j++;
		
			
		}	
		
		if(sum > (signed int)MAX16)
		{
			d[i] = (short)MAX16;
			//printf("MAX\n");
		}
		else if(sum <= (signed int)MIN16)
		{
			d[i] = (short)MIN16;
			//printf("MIN\n");
		}
		else
			d[i] = (short)sum;	
		
		
	}

}











ECHOCANCELL_STRUCT sControl;

#ifdef _TEST_EEC

/* test the EEC module */
int main()
{

	FILE *fp;
	FILE *fp_x,*fp_y;
	
	short ReceiveDate_Xn[AMR_BLOCK];
	short TransmitData_Yn[AMR_BLOCK];
	short TAR_COEF[EEC_ADAPTIVEFILE_ORDER];
	short i;
	short sTrainFrameSize;
		
	
	extern ECHOCANCELL_STRUCT sControl;
		
	int len;
	
	/* result file */
	fp = fopen("..\\..\\signal_file\\EECout.pcm","wb");
	if(fp==NULL) 
	{
		printf("open out file fail\n");
		return 0;
	}
	
	/* voice + noise + echo signal in near end */
	fp_y = fopen("..\\..\\signal_file\\near.pcm","rb");
	if(fp_y==NULL) 
	{
		printf("open out file fail\n");
		return 0;
	}	
	
	/* voice + noise signal in far end */
	fp_x = fopen("..\\..\\signal_file\\echo_eec.pcm","rb");
	if(fp_x==NULL) 
	{
		printf("open out file fail\n");
		return 0;
	}	
	
	
	/* for preset the echo signal of target */
	for(len=0;len<EEC_ADAPTIVEFILE_ORDER;len++)
		BUF[len]=0;
	for(len=0;len<EEC_ADAPTIVEFILE_ORDER;len++)
		TAR_COEF[len] = (int)(h512[len]*4096);	//Q15 scal-down 4		

	/* reset the AECEEC control status */
	EchoCancellationReset(&sControl);

	/* training during */
	sTrainFrameSize = 10;
	
	/* enable EEC */
	sControl.bAECEEC_SW = 1;	
	
	for(len=0;len<1460;len++)
	{
		
			
		
		printf("block %d\n",len);
		
		
		if(sControl.bEECTrain==0)
		{
			/* training the EEC */
			for(i=0;i<AMR_BLOCK;i++)
				TransmitData_Yn[i] = GetWhiteNoise(); /* input the white noise to training */
		
			/* get echo signal of target */
			GetTarget(ReceiveDate_Xn, TransmitData_Yn,TAR_COEF);	
			
			/* training EEC processing */
			ElectricalEchoCancellationTraining(&sControl,ReceiveDate_Xn, AMR_BLOCK,sTrainFrameSize, TransmitData_Yn);
		}
		else
		{
			/* EEC cancellation in processing */
			
			/* get the transmit and receive signal in file */
			fread(ReceiveDate_Xn,AMR_BLOCK,2,fp_x);
			fread(TransmitData_Yn,AMR_BLOCK,2,fp_y);
			
			/* EEC processing */
			EchoCancellationProcessing(&sControl,ReceiveDate_Xn, AMR_BLOCK, TransmitData_Yn);
			
			/* get the result */
			fwrite(ReceiveDate_Xn,AMR_BLOCK,2,fp);
		}
	}

	fclose(fp);
	fclose(fp_x);
	fclose(fp_y);
	return 0;
}

#else

/* test the AEC module */
int main()
{

	FILE *fp;
	FILE *fp_x,*fp_y;
	
	short ReceiveDate_Xn[AMR_BLOCK];
	short TransmitData_Yn[AMR_BLOCK];

	
		
	
	extern ECHOCANCELL_STRUCT sControl;
		
	int len;

	/* result file */
	fp = fopen("..\\..\\signal_file\\AECout.pcm","wb");
	if(fp==NULL) 
	{
		printf("open out file fail\n");
		return 0;
	}
	/* transmit signal in near end */
	fp_y = fopen("..\\..\\signal_file\\near_plus_echo.pcm","rb");
	if(fp_y==NULL) 
	{
		printf("open out file fail\n");
		return 0;
	}	
	/* receive signal in far end */
	fp_x = fopen("..\\..\\signal_file\\far.pcm","rb");
	if(fp_x==NULL) 
	{
		printf("open out file fail\n");
		return 0;
	}	

	
	
	/* reset the AECEEC system */
	EchoCancellationReset(&sControl);


	/* enable AEC */
	sControl.bAECEEC_SW = 0;	

	printf("AEC start\n");	
	
	for(len=0;len<1460;len++)
	{
		
			
		
		printf("block %d\n",len);
		fread(ReceiveDate_Xn,AMR_BLOCK,2,fp_x);
		fread(TransmitData_Yn,AMR_BLOCK,2,fp_y);

		
		/* AEC processing */
		EchoCancellationProcessing(&sControl,ReceiveDate_Xn, AMR_BLOCK, TransmitData_Yn);
		
		
		fwrite(TransmitData_Yn,AMR_BLOCK,2,fp);
		
	}

	fclose(fp);
	fclose(fp_x);
	fclose(fp_y);
	return 0;
}

#endif








