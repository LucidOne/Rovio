/***************************************************************************/
/*																												*/
/* Copyright (c) 2004 -  Winbond Electronics Corp. All rights reserved.*/
/*																												*/
/***************************************************************************/

/***************************************************************************
*
*	FILENAME
*
*		dsp_proc.c
*
*	VERSION
*
*		1.0
*
*	DESCRIPTION
*
*		Provide functions for AEC and AWB
*
*	DATA STRUCTURES
*
*		None
*
*	FUNCTIONS
*
*		
*		
*
*	HISTORY
*		09/14/04 Ver 1.0 Created by PC51 WCHung1
*       12/24/04
*
*	REMARK
*		Can be only used on little-endian system
* *
***************************************************************************/

#include "wblib.h"
#include "W99702_reg.h"
#include "dsplib.h"
#include "dsp_global.h"
#ifdef ECOS
#include "stdlib.h"
#include "stdio.h"
#else
#include <stdlib.h>
#include <stdio.h>
#endif

#include "i2clib.h"

int _dsp_MaxExpTime, _dsp_MinExpTime;
int _dsp_MaxExpStep;
int _dsp_MaxAGC, _dsp_MinAGC;
int _dsp_MaxAGC1,_dsp_MaxAGC2;	//k12274-1
int _dsp_ExpCtrl;
int _dsp_agcCtrl;

UINT8	_dsp_targetLum;		//k08245-1
UINT8 _dsp_exp_mode;
UINT8 _dsp_ExpStable;
int _dsp_avgLum=0;
int _dsp_old_avgLum=0;	//k01055-1
//int _dsp_old_ExpCtrl,_dsp_old_agc,_dsp_oldForeAvg;
int _dsp_old_ExpCtrl,_dsp_old_agc;
int	_dsp_FullAvgLum,_dsp_ForeAvgLum;
//int _dsp_gamma,_dsp_bGammaChg;
int _dsp_bGammaChg;
//int _dsp_too_dark, _dsp_updYScale,_dsp_bExpFlicker;
UINT8 _dsp_too_dark, _dsp_bExpFlicker=0;
int _dsp_FlickerCnt=0;	//k05265-2

unsigned char _dsp_ReIssueTopLim,_dsp_ReIssueLowLim,_dsp_LumRefH,_dsp_LumRefL;
unsigned char _dsp_LumLimH1, _dsp_LumLimH2,_dsp_LumLimL1,_dsp_LumLimL2;

extern UINT32 _dsp_FlashMode, _dsp_FlashStatus;
extern int _dsp_exp_EV;
extern int _dsp_AdjYmode;


//#define Slowest_FrameRatePerSec   3		//k12234-1 - the maximum fps = 0, that is, the default reg0x11[5:0]=0 (for example, OV sensors)
int _dsp_fpsCtrl=0;
int _dsp_Max_fpsCtrl;	//k05105-2
int _dsp_Min_fpsCtrl=3;	//k05115-3 - to replace "Slowest_FrameRatePerSec"
UINT8 _dsp_fps_change;	// 0 : no change, 1 : fps dec(fpsCtrl inc), 2 : fps inc (fpsCtrl dec)

//UINT8 _dsp_wtR[16],_dsp_wtG[16],_dsp_wtB[16];
//UINT8 _dsp_awbY[16],_dsp_awbR[16],_dsp_awbG[16],_dsp_awbB[16];
//UINT8 _dsp_cntR,_dsp_cntG,_dsp_cntB;

extern unsigned short int GamTblM_Normal[32];
extern unsigned char GamTblT_Normal[64];
//For darker environments
extern unsigned short int GamTblM_LowLight[32];
extern unsigned char GamTblT_LowLight[64];
//For contrast larger enviroments - 2
extern unsigned short int GamTblM_ContrastL[32];
extern unsigned char GamTblT_ContrastL[64];

void init_AE_CONST(unsigned char ExpRef)
{	//_dsp_LumLimH2, _dsp_LumLimH1, _dsp_ReIssueTopLim, _dsp_ReIssueLowLim, _dsp_LumLimL1, _dsp_LumLimL2
	//unsigned char avgRef;

	_dsp_LumRefH=(unsigned char)ExpRef+(unsigned char)5;
	_dsp_LumRefL=(unsigned char)ExpRef-(unsigned char)5;

	_dsp_ReIssueTopLim=(unsigned char)_dsp_LumRefH+3;
	_dsp_ReIssueLowLim=(unsigned char)_dsp_LumRefL-3;
	_dsp_LumLimH1=(unsigned char)ExpRef+12;
	_dsp_LumLimL1=(unsigned char)ExpRef-12;

	//tmp=25;	//ExpRef>>2;
	_dsp_LumLimH2=(unsigned char)ExpRef+25;
	_dsp_LumLimL2=(unsigned char)ExpRef-25;
}

void WriteGainEXP (int ExpTime, int agc_ctrl)
{	UINT32 ExpL,ExpM,ExpH;
	UINT32 regSerialBusCtrl;
	UINT32 AGC_L, AGC_H;
	UINT32 tmp;
	UINT32 pixClkDiv;

	regSerialBusCtrl=i2cGetSerialBusCtrl();


#ifdef OPT_OV9650
	i2cInitSerialBus (0,((regSerialBusCtrl&0x04)>>2),(regSerialBusCtrl>>8));
	ExpL=i2cReadI2C (0x04)&0xFC;
	ExpH=i2cReadI2C (0xA1)&0xC0;
	AGC_H=i2cReadI2C (0x03)&0x3F;

	if (_dsp_fps_change!=0)
	{	pixClkDiv=i2cReadI2C (0x11)&0xC0;
			pixClkDiv=pixClkDiv|_dsp_fpsCtrl;
		i2cWriteI2C (0x11,pixClkDiv);
	}

	ExpL=ExpL|(UINT32)(ExpTime&0x03);
	ExpM=((UINT32)(ExpTime>>2)&0xFF);
	ExpH=ExpH|(((UINT32)ExpTime>>10)&0x3F);
	AGC_L=(agc_ctrl&0xFF);
	AGC_H=AGC_H|((agc_ctrl>>8)&0x03);

	if (regSerialBusCtrl&0x04)	//fast serial bus
	{	i2cInitSerialBus (1,((regSerialBusCtrl&0x04)>>2),(regSerialBusCtrl>>8));

		i2cWriteFastI2C (1,0x00,(UINT8)AGC_L);
		i2cWriteFastI2C (1,0x00,(UINT8)AGC_H);

		i2cWriteFastI2C (1,0x04,(UINT8)ExpL);
		i2cWriteFastI2C (1,0x10,(UINT8)ExpM);
		i2cWriteFastI2C (1,0xA1,(UINT8)ExpH);
	}
	else
	{	i2cWriteI2C (0x00,(UINT8)AGC_L);
		i2cWriteI2C (0x00,(UINT8)AGC_H);

		i2cWriteI2C (0x04,ExpL);
		i2cWriteI2C (0x10,ExpM); 
		i2cWriteI2C (0xA1,ExpH); 
	}
#endif	//OPT_OV9650

#ifdef OPT_OV764x
	if (regSerialBusCtrl)	//fast serial bus
	{	i2cInitSerialBus (0,((regSerialBusCtrl&0x04)>>2),(regSerialBusCtrl>>8));
		ExpL=i2cReadI2C (0x76)&0xFC;

		i2cInitSerialBus (1,((regSerialBusCtrl&0x04)>>2),(regSerialBusCtrl>>8));
		i2cWriteFastI2C (1,0x00,(UINT32)agc_ctrl);
		ExpL=(UINT32)(ExpTime&0x03)|ExpL;
		i2cWriteFastI2C (1,0x76,ExpL);

		ExpH=(UINT32)(ExpTime>>2);
		i2cWriteFastI2C (1,0x10,ExpH);
	}
	else
	{	ExpL=i2cReadI2C (0x76)&0xFC;
		i2cWriteI2C (0x00,(UINT32)agc_ctrl);
		ExpL=(UINT32)(ExpTime&0x03)|ExpL;
		i2cWriteI2C (0x76,ExpL);

		ExpH=(UINT32)(ExpTime>>2);
		i2cWriteI2C (0x10,ExpH);
	}
#endif	//OPT_OV764x



#ifdef OPT_OV2630
	AGC_L=agc_ctrl&0xFF;
	AGC_H=(agc_ctrl&0x300)>>2;	//reg0x45[7:6]

	ExpL=ExpTime&0x03;
	ExpM=(ExpTime>>2)&0xFF;
	ExpH=(ExpTime>>10)&0x3F;

	i2cInitSerialBus (0,((regSerialBusCtrl&0x04)>>2),(regSerialBusCtrl>>8));
	if (_dsp_fps_change!=0)
	{	pixClkDiv=i2cReadI2C (0x11)&0xC0;
			pixClkDiv=pixClkDiv|_dsp_fpsCtrl;
		i2cWriteI2C (0x11,pixClkDiv);
	}

	if (regSerialBusCtrl&0x04)	//fast serial bus
	{	i2cInitSerialBus (0,((regSerialBusCtrl&0x04)>>2),(regSerialBusCtrl>>8));
		tmp=i2cReadI2C (0x04)&0xFC;	//[1:0]
		i2cInitSerialBus (1,((regSerialBusCtrl&0x04)>>2),(regSerialBusCtrl>>8));
			ExpL=ExpL|tmp;
		i2cWriteFastI2C (1,0x04, ExpL);
		i2cWriteFastI2C (1,0x10, ExpM);
			ExpH=ExpH|AGC_H;
		i2cWriteFastI2C (1,0x45,ExpH);
		i2cWriteFastI2C (1,0x00,AGC_L);
	}
	else
	{		tmp=i2cReadI2C (0x04)&0xFC;	//[1:0]
			ExpL=ExpL|tmp;
		i2cWriteI2C (0x04, ExpL);
		i2cWriteI2C (0x10, ExpM);
			ExpH=ExpH|AGC_H;
		i2cWriteI2C (0x45,ExpH);
		i2cWriteI2C (0x00,AGC_L);
	}
#endif	//OPT_OV2630

#ifdef OPT_MT9D011
		i2cWriteI2C_16b (0x09, ExpTime);
#endif	//OPT_MT9D011

	_dsp_fps_change=0;
}

void fill_gamma_table (void)
{	int gamIndex,i;
	UINT32 regdata;

	if (_dsp_too_dark==1)
	{	
		gamIndex=0;
		for (i=0;i<32;++i)
		{	regdata=(((UINT32)GamTblM_LowLight[i]&0xFF)<<24) | (((UINT32)GamTblM_LowLight[i]>>8)<<16);	//Cn
			++i;
			regdata=regdata|(((UINT32)GamTblM_LowLight[i]&0xFF)<<8) | ((UINT32)GamTblM_LowLight[i]>>8);	//Cn+1
			outpw ((REG_DSPGammaTbl1+gamIndex),regdata);
			gamIndex=gamIndex+4;
		}
	}	//if (_dsp_too_dark==1)
	else
	{
		gamIndex=0;
		for (i=0;i<32;++i)
		{	regdata=(((UINT32)GamTblM_Normal[i]&0xFF)<<24) | (((UINT32)GamTblM_Normal[i]>>8)<<16);	//Cn
			++i;
			regdata=regdata|(((UINT32)GamTblM_Normal[i]&0xFF)<<8) | ((UINT32)GamTblM_Normal[i]>>8);	//Cn+1
			outpw ((REG_DSPGammaTbl1+gamIndex),regdata);
			gamIndex=gamIndex+4;
		}
	}	//else
	_dsp_bGammaChg=0;

}

int old_avgY[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};	//k03165-2
int avgY[16]={10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10};	//k03165-2
void UpdateExposure (void)
{
	UINT32 regdata;
	int tExp16,tAGC;
	int idx;
	UINT8 i;
	UINT8 old_exp_mode;
	int old_ExpCtrl2;
	int old2_avgLum;
	int tmpAvg;	//k02235-1
	int diff_avgYi_num;	//k03165-2

	regdata=inpw(REG_DSPAECAvg);
	_dsp_FullAvgLum=regdata>>8;
	_dsp_ForeAvgLum=regdata&0xff;


	old2_avgLum=_dsp_old_avgLum;		//k01055-1
	_dsp_old_avgLum=_dsp_avgLum;



	diff_avgYi_num=0;	//k03165-2
	for (i=0;i<16;i=i+4)
	{	
		old_avgY[i]=avgY[i];	old_avgY[i+1]=avgY[i+1];
		old_avgY[i+2]=avgY[i+2];	old_avgY[i+3]=avgY[i+3];
		regdata=inpw (REG_DSPSubWndAvgY1+i);
		avgY[i+3]=(int)(regdata&0xFF);		regdata=regdata>>8;
		avgY[i+2]=(int)(regdata&0xFF);		regdata=regdata>>8;
		avgY[i+1]=(int)(regdata&0xFF);		regdata=regdata>>8;
		avgY[i+0]=(int)(regdata&0xFF);

		//k03165-2-b
		if (abs(avgY[i]-old_avgY[i])<=3)	++diff_avgYi_num;
		if (abs(avgY[i+1]-old_avgY[i+1])<=3)	++diff_avgYi_num;
		if (abs(avgY[i+2]-old_avgY[i+2])<=3)	++diff_avgYi_num;
		if (abs(avgY[i+3]-old_avgY[i+3])<=3)	++diff_avgYi_num;
		//k03165-2-a
	}


	_dsp_avgLum=0;
	idx=0;
	for (i=0;i<16;++i)
	{	_dsp_avgLum=_dsp_avgLum+_dsp_AE_weight_block[i]*avgY[i];
		idx=idx+_dsp_AE_weight_block[i];
	}
	//_dsp_avgLum=_dsp_avgLum/idx;	//k12284-1-old AE_weight_block
	_dsp_avgLum=_dsp_avgLum>>6;		//k12284-1-new AE_weight_block

	if (_dsp_ExpStable==1)
	{	_dsp_exp_mode=0;

		if (_dsp_avgLum>=_dsp_ReIssueLowLim && _dsp_avgLum<=_dsp_ReIssueTopLim)
		{	
			if (_dsp_too_dark==1)
			{	_dsp_too_dark=0;
				_dsp_bGammaChg=1;
				fill_gamma_table ();
			}
			return;
		}
		else
			if (_dsp_bExpFlicker==1 && _dsp_avgLum>=_dsp_LumLimL2 && _dsp_avgLum<=_dsp_LumLimH2)
			{	return;
			}
			else
				if (_dsp_bExpFlicker==1 && diff_avgYi_num>=12 && abs(_dsp_avgLum-_dsp_old_avgLum)<=3)
				{	return;
				}
			else
				if (_dsp_too_dark==1 && _dsp_avgLum<(int)_dsp_LumLimL2)
				{	return;
				}
	}//if (_dsp_ExpStable==1)

	_dsp_old_agc=_dsp_agcCtrl;
	old_ExpCtrl2=_dsp_old_ExpCtrl;
	_dsp_old_ExpCtrl=_dsp_ExpCtrl;
	old_exp_mode=_dsp_exp_mode;
	_dsp_exp_mode=0;	//0 : no change, 1 : unsufficient luminosity (inc exp time), 2 : over exposure (dec exp time)

	_dsp_too_dark=0;	//normal
	_dsp_bGammaChg=0;
	tExp16=1;

	_dsp_fps_change=0;

	_dsp_ExpStable=0;
	if (_dsp_avgLum<(int)_dsp_LumRefL)
	{	_dsp_exp_mode=1;

		tmpAvg=(_dsp_avgLum*16)/(16+_dsp_agcCtrl);
		tmpAvg=(tmpAvg*(16+_dsp_MaxAGC))/16;
		
		if (_dsp_ExpCtrl<_dsp_MaxExpTime)
		{	if (_dsp_avgLum<_dsp_ReIssueLowLim)
			{	tExp16=(((int)_dsp_LumRefL-_dsp_avgLum)*_dsp_ExpCtrl)/(int)_dsp_LumRefL;
				idx=tExp16/10+(tExp16%10);

				if (tExp16>idx)	tExp16=tExp16/idx;

				if (_dsp_avgLum>(_dsp_LumLimH2<<1))	tExp16=tExp16*3;	//k03185-3
			}

					if (  ((_dsp_old_avgLum-_dsp_avgLum)<=1 && (_dsp_old_avgLum-_dsp_avgLum)>=-1)	//k01055-1-b
						|| ((_dsp_old_avgLum-_dsp_avgLum)<=10 && (_dsp_old_avgLum-_dsp_avgLum)>=-10 && _dsp_avgLum<=_dsp_LumLimL1) )	//k03145-1
					{	if (tExp16<2)	tExp16=2;
						else
							if (_dsp_avgLum<_dsp_LumLimL1)
							{	tExp16=tExp16*2;

								//k01055-1-b
								if ((old2_avgLum-_dsp_old_avgLum)<=5 && (old2_avgLum-_dsp_old_avgLum)>=-5)	//k03145-1
									tExp16=tExp16*3/2;
								//k01055-1-a
							}
					}		//k01055-1-a

			if (tExp16>_dsp_MaxExpStep || _dsp_avgLum<=20)	tExp16=_dsp_MaxExpStep;		//k03185-4
			else
				if (tExp16<=0 || old_exp_mode==2)	tExp16=1;		//k03185-1

			_dsp_ExpCtrl=_dsp_ExpCtrl+tExp16;
			if (_dsp_ExpCtrl>=_dsp_MaxExpTime)	_dsp_ExpCtrl=_dsp_MaxExpTime;
		}
		else
				//k02235-1-b-only for OV sensor
				if ((tmpAvg<_dsp_ReIssueLowLim || _dsp_agcCtrl==_dsp_MaxAGC) && _dsp_fpsCtrl<_dsp_Min_fpsCtrl)
				{	_dsp_fps_change=2;
					++_dsp_fpsCtrl;
				}
				else
				if ((tmpAvg>=_dsp_ReIssueLowLim || _dsp_fpsCtrl==_dsp_Min_fpsCtrl) && _dsp_agcCtrl<_dsp_MaxAGC)
				{	if (_dsp_avgLum<(int)_dsp_LumLimL2)	_dsp_agcCtrl=_dsp_agcCtrl+5;
					else
						if (_dsp_avgLum<(int)_dsp_ReIssueLowLim)	_dsp_agcCtrl=_dsp_agcCtrl+2;
						else	_dsp_agcCtrl=_dsp_agcCtrl+1;

					if (_dsp_agcCtrl>=_dsp_MaxAGC)	_dsp_agcCtrl=_dsp_MaxAGC;
					//AGC increase ==> exposure time should subtract an offset ???? at the same time
				}
				//k02235-1-b-only for OV sensor
				else
				{	_dsp_too_dark=1;
					_dsp_bGammaChg=1;
					_dsp_ExpStable = 1;
				}
	}//if (_dsp_avgLum<_dsp_LumRefL)
	else
		if (_dsp_avgLum>(int)_dsp_LumRefH)
		{	_dsp_exp_mode=2;
			
			if (_dsp_too_dark==1)
			{	_dsp_bGammaChg=1;
				_dsp_too_dark=0;
			}
			
			if (_dsp_avgLum>(int)_dsp_ReIssueTopLim && _dsp_agcCtrl>_dsp_MinAGC)
			{	if (_dsp_avgLum>=(int)_dsp_LumLimH2)	_dsp_agcCtrl=_dsp_agcCtrl-5;
				else
					if (_dsp_avgLum>_dsp_ReIssueTopLim)	_dsp_agcCtrl=_dsp_agcCtrl-2;
					else	_dsp_agcCtrl=_dsp_agcCtrl-1;

				if (_dsp_agcCtrl<=_dsp_MinAGC)	_dsp_agcCtrl=_dsp_MinAGC;
			}
			else
				if (_dsp_ExpCtrl>_dsp_MinExpTime)
				{	if (_dsp_avgLum>=_dsp_ReIssueTopLim)
					{	tExp16=((_dsp_avgLum-(int)_dsp_LumRefH)*_dsp_ExpCtrl)/_dsp_avgLum;
						idx=tExp16/10+(tExp16%10);

						if (tExp16>idx)	tExp16=tExp16/idx;

						if (_dsp_avgLum<(_dsp_LumLimL2-10))	tExp16=tExp16*3;	//k03185-3
					}
		
				if ( ((_dsp_old_avgLum-_dsp_avgLum)<=1 && (_dsp_old_avgLum-_dsp_avgLum)>=-1)	//k01055-1-b
					|| ((_dsp_old_avgLum-_dsp_avgLum)<=10 && (_dsp_old_avgLum-_dsp_avgLum)>=-10 && _dsp_avgLum>=_dsp_LumLimH2) )	//k03145-1
					{	if (tExp16<2)	tExp16=2;
						else
							if (_dsp_avgLum>_dsp_LumLimH1)
							{	tExp16=tExp16*2;

								//k01055-1-b
								if ((old2_avgLum-_dsp_old_avgLum)<=5 && (old2_avgLum-_dsp_old_avgLum)>=-5)	//k03145-1
									tExp16=tExp16*3/2;

								//k01055-1-a
							}
					}																											//k01055-1-a

					if (tExp16>_dsp_MaxExpStep || _dsp_avgLum>=200)	tExp16=_dsp_MaxExpStep;		//k03185-4
					else
						if (tExp16<=0 || old_exp_mode==1)	tExp16=1;	//k03185-1

					_dsp_ExpCtrl=_dsp_ExpCtrl-tExp16;
					if (_dsp_ExpCtrl<_dsp_MinExpTime)	_dsp_ExpCtrl=_dsp_MinExpTime;
				}
				else
					if (_dsp_agcCtrl>_dsp_MinAGC)
					{	_dsp_agcCtrl=_dsp_agcCtrl-1;
						//_dsp_ExpCtrl=_dsp_MaxExpTime;		//need ????
					}
					else
						if (_dsp_fpsCtrl>0)
						{	_dsp_fps_change=1;
							--_dsp_fpsCtrl;
						}
						else	_dsp_ExpStable=1;
		}//if (_dsp_avgLum>_dsp_LumRefH)
		else
		{	_dsp_ExpStable=1;
			_dsp_exp_mode=0;			//nop

			//k03185-1-b
			if (abs(_dsp_avgLum-_dsp_old_avgLum)<=3)
			{	if (_dsp_avgLum<=(_dsp_LumRefL+3) && _dsp_avgLum>=_dsp_LumRefL)
					_dsp_ExpCtrl=_dsp_ExpCtrl+1;
				else
					if (_dsp_avgLum>=(_dsp_LumRefH-3) && _dsp_avgLum<=_dsp_LumRefH)
					_dsp_ExpCtrl=_dsp_ExpCtrl-1;
			}
			//k03185-1-a
		}

		if (_dsp_fps_change!=0)
		{	

			if (_dsp_fpsCtrl==_dsp_Min_fpsCtrl)	_dsp_MaxAGC=_dsp_MaxAGC2;	//k12274-1 = 0x1F
			else	_dsp_MaxAGC=_dsp_MaxAGC1;		//k12274-1 = 0x0A

			if (_dsp_fpsCtrl==0)	_dsp_MinExpTime=1;
			else	//if (_dsp_fpsCtrl>0)
#ifdef OPT_OV2630
				_dsp_MinExpTime=_dsp_MaxExpTime*(((_dsp_fpsCtrl-1)+1)*2)/((_dsp_fpsCtrl+1)*2);
#else	//Non-OPT_OV2630
				_dsp_MinExpTime=_dsp_MaxExpTime*((_dsp_fpsCtrl-1)+1)/(_dsp_fpsCtrl+1);
#endif	//OPT_OV2630

			if (_dsp_fps_change==1)	//dec
			{	_dsp_ExpCtrl=_dsp_MaxExpTime;
				_dsp_agcCtrl = _dsp_MinAGC;			//not sure ??
			}
			else	//if (_dsp_fps_change==2)	//inc
			{	_dsp_ExpCtrl=_dsp_MinExpTime;
				_dsp_agcCtrl = _dsp_MinAGC;			//not sure ??
			}
		}	//if (_dsp_fps_change!=0)

	if (_dsp_bGammaChg==1)
		fill_gamma_table ();

	_dsp_bExpFlicker=0;
	if ((_dsp_old_avgLum<=(int)_dsp_LumRefL && _dsp_avgLum>=(int)_dsp_LumRefH)
			|| (_dsp_old_avgLum>=(int)_dsp_LumRefH && _dsp_avgLum<=(int)_dsp_LumRefL))
	{	
		if (old_ExpCtrl2>=_dsp_old_ExpCtrl)	tExp16=old_ExpCtrl2-_dsp_old_ExpCtrl;		//k03175-1
		else	tExp16=_dsp_old_ExpCtrl-old_ExpCtrl2;				//k03175-1

		if (tExp16<=1)
		{	_dsp_bExpFlicker = 1;

			//k03185-1-b
			if (_dsp_avgLum>=_dsp_old_avgLum && (_dsp_avgLum-_dsp_LumRefH)<=(_dsp_LumRefL-_dsp_old_avgLum))
				_dsp_ExpCtrl=_dsp_old_ExpCtrl;
			//k03185-1-a
			_dsp_ExpStable=1;			//k03165-2
		}
		else
			if ((_dsp_ExpCtrl<=_dsp_old_ExpCtrl && _dsp_ExpCtrl<=old_ExpCtrl2)
				|| (_dsp_ExpCtrl>=_dsp_old_ExpCtrl && _dsp_ExpCtrl>=old_ExpCtrl2))
				_dsp_ExpCtrl=(old_ExpCtrl2+_dsp_old_ExpCtrl)/2;
	}


	if (_dsp_FlashMode==0 && _dsp_too_dark==1)
		_dsp_FlashStatus = 1;
	else
		if (_dsp_FlashMode==1)
			_dsp_FlashStatus=1;
		else	//if (_dsp_FlashMode==2)
			_dsp_FlashStatus=0;
		WriteGainEXP (_dsp_ExpCtrl,_dsp_agcCtrl);
}

void UpdateExposure_FL (void)
{
}

int avgRsw,avgGsw,avgBsw;	//for debug
UINT32 RAvg,GAvg,BAvg;	//for debug
volatile int tmpR1,tmpR2,tmpR3,tmpR4;
INT32	wbgR,wbgB,wbgGr,wbgGb,wbgG;	//k03155-1-for debug
//BOOL bAWB_SW_src=0;

void UpdateWhiteBalance (void)
{	INT32 initGain;
	UINT32	regdata;
	int swR[16],swG[16],swB[16];
	int old_wbgR,old_wbgB,old_wbgG;
	int WO_count;

	int nContrast, nMaxY, nMinY, i;
	int histo[12];

	regdata=inpw (REG_DSPColorBalCR1);
	old_wbgR=(regdata>>16)&0x1FF;
	old_wbgB=regdata&0x1FF;

	regdata=inpw (REG_DSPColorBalCR2);
	wbgGr=(regdata>>16)&0x1FF;
	wbgGb=regdata&0x1FF;
	old_wbgG=(wbgGr+wbgGb)/2;


		regdata=inpw (REG_DSPAWBWOAvg)&0x00FFFFFF;
		RAvg=(int)((regdata>>16)&0xFF);
		GAvg=(int)((regdata>>8)&0x00ff);
		BAvg=(int)((regdata&0x00ff));
		WO_count=(int)inpw(REG_DSPAWBWOCount);

	if (regdata==0x00FFFFFF && ((inpw (REG_DSPAECAvg)&0xFF00)>>8)<inpw (REG_DSPFrameDBL))	return;	//k04145-1
	if ((inpw(REG_CAPFuncEnable)&0x300)==0x00)	return;			//no planar & no packet

	for (i=0;i<12;)
	{	regdata=inpw (REG_DSPHistoReport1);
		histo[i]=regdata&0xFF;		++i;
		histo[i]=regdata>>16;		++i;
	}

	avgRsw=0;
	avgGsw=0;
	avgBsw=0;
	for (i=0;i<16;i=i+4)
	{	regdata=inpw (REG_DSPSubWndAvgR1+i);
			swR[i+3]=regdata&0xFF;	regdata=regdata>>8;
			swR[i+2]=regdata&0xFF;	regdata=regdata>>8;
			swR[i+1]=regdata&0xFF;	regdata=regdata>>8;
			swR[i+0]=regdata&0xFF;
		regdata=inpw (REG_DSPSubWndAvgG1+i);
			swG[i+3]=regdata&0xFF;	regdata=regdata>>8;
			swG[i+2]=regdata&0xFF;	regdata=regdata>>8;
			swG[i+1]=regdata&0xFF;	regdata=regdata>>8;
			swG[i+0]=regdata&0xFF;
		regdata=inpw (REG_DSPSubWndAvgB1+i);
			swB[i+3]=regdata&0xFF;	regdata=regdata>>8;
			swB[i+2]=regdata&0xFF;	regdata=regdata>>8;
			swB[i+1]=regdata&0xFF;	regdata=regdata>>8;
			swB[i+0]=regdata&0xFF;
		avgRsw=avgRsw+swR[i+3]+swR[i+2]+swR[i+1]+swR[i+0];
		avgGsw=avgGsw+swG[i+3]+swG[i+2]+swG[i+1]+swG[i+0];
		avgBsw=avgBsw+swB[i+3]+swB[i+2]+swB[i+1]+swB[i+0];
	}
	avgRsw=avgRsw>>4;
	avgGsw=avgGsw>>4;
	avgBsw=avgBsw>>4;

	if (RAvg==0 || GAvg==0 || BAvg==0)	return;

	if (RAvg==255 && GAvg==255 && BAvg==255 && WO_count==0)	return;	//k12294-1
	else
		if (avgRsw==0 && avgGsw==0 && avgBsw==0)	return;			//k12294-1

	if (GAvg>=(RAvg+80) && GAvg>=(BAvg+80))	return;
	else
		if (BAvg>=(RAvg+80) && BAvg>=(GAvg+80))	return;

	initGain=0x80;	

	if (RAvg>=GAvg && RAvg>=BAvg)
	{	wbgR=initGain;
		wbgG=((RAvg-GAvg)*initGain)/RAvg;
#ifndef OPT_OV2630
		wbgB=((RAvg-BAvg)*initGain*2)/(BAvg+RAvg);
#endif	//#ifndef OPT_OV2630

#ifdef OPT_MT9D011
		wbgB=((RAvg-BAvg)*initGain)/BAvg;
#endif	//#ifndef OPT_MT9D011

#ifdef OPT_OV2630
		if (GAvg>BAvg)	wbgB=((RAvg-BAvg)*initGain*2)/(BAvg+RAvg);	//k03235-1
		else	wbgB=((RAvg-BAvg)*initGain)/BAvg;	//k03235-1
#endif	//OPT_OV2630
			wbgG=wbgG+initGain;
			wbgB=wbgB+initGain;
	}
	else
		if (GAvg>=RAvg && GAvg>=BAvg)
		{	wbgR=((GAvg-RAvg)*initGain)/GAvg;
			wbgG=initGain;
			wbgB=((GAvg-BAvg)*initGain)/BAvg;
				//wbgR=(wbgR*2)/3+initGain;	//temporarily	//k11114-2
				//wbgB=(wbgB*2)+initGain;	//temporarily	//k11114-2
				wbgR=wbgR+initGain;
				wbgB=wbgB+initGain;
			}
			else
			{	wbgR=((BAvg-RAvg)*initGain)/BAvg;
				wbgG=((BAvg-GAvg)*initGain)/BAvg;
					wbgR=wbgR+initGain;
					wbgG=wbgG+initGain;
				wbgB=initGain;
			}



	if (wbgR>=0x1FF)	wbgR=0x1FF;
	else
		if (wbgR<=0)	wbgR=0;

	if (wbgG>=0x1FF)	wbgG=0x1FF;
	else
		if (wbgG<=0)	wbgG=0;

	if (wbgB>=0x1FF)	wbgB=0x1FF;
	else
		if (wbgB<=0)	wbgB=0;


	regdata=((UINT32)wbgR<<16)|wbgB;
	outpw (REG_DSPColorBalCR1,regdata);

	regdata=((UINT32)wbgG<<16)|wbgG;
	outpw (REG_DSPColorBalCR2,regdata);

}	//void UpdateWhiteBalance ()

void ExchangeOfExpTime (int old_pclk_rate[10], UINT8 old_subsampling, int old_MaxExpTime, 
												int old_fpsCtrl, int old_MaxFpsCtrl, int old_MinFpsCtrl)
{
}

