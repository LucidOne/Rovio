/********************************************************************************************************/
/*																										*/	
/*	FUNCTION																							*/		
/* 		lcd.c																							*/
/*																										*/
/*	DESCRIPTION																							*/
/*		For down scale factor & poly phase filter 		   												*/
/*																										*/
/*	CALL BY																								*/
/*		capImageTransform();  																			*/	
/*																										*/	
/*  INPUT 																								*/	
/*		Image source and destingation dimesion															*/		
/*																										*/
/*	OUTPUT 																								*/	
/*		set the dda factor																				*/	
/*																										*/
/*	HISTORY																								*/
/*		Name				Date				Remark													*/	
/*		swchou				09-13-04					1th release										*/
/*																										*/
/*	REMARK																								*/
/*		None																							*/
/*																										*/		
/********************************************************************************************************/

#ifdef ECOS
#include "stdlib.h" 
#include "stdio.h"
#else
#include <stdlib.h> 
#include <stdio.h>
#endif
#include "w99702_reg.h"
#include "wbtypes.h"
#include "wbio.h"
#include "wblib.h"
#include "wberrcode.h"
#include "caplib.h"


//#define DBG_MSG
//#include "captest.h"
UCHAR maxlcd(UCHAR* plaan_arr, UCHAR* placn_arr, UCHAR* pacan_arr, UCHAR* paccn_arr);
UCHAR maxlcd_2(UCHAR* h_arr, UCHAR* w_arr, UCHAR ucLimit);
/*
	The planar and packet dda factor. if the dda factor for numator or denominator has same factor.
	The poly-phase filter will turn on.
*/
INT capSetDdaFactor(T_CAP_SET* ptCapSet)	 
{
	
	UINT simgwh, plaimgwh, pacimgwh;
	BOOL flag;
	USHORT lcdv1;
	UCHAR plaan, plabm, placn, pladm, pacan, pacbm, paccn, pacdm;
	UCHAR plaan_arr[12]={0},plabm_arr[12]={0},placn_arr[12]={0},pladm_arr[12]={0};
	UCHAR pacan_arr[12]={0},pacbm_arr[12]={0},paccn_arr[12]={0},pacdm_arr[12]={0};
	volatile UCHAR  ppfn=1,ppfm=1;
	
	simgwh = inpw(REG_CAPCropWinSize);
	if(ptCapSet->bIsInterlaceFields==TRUE)
	{
		UINT uHeight = (simgwh&0x0FFFF)*2;
		simgwh = (simgwh&0xFFFF0000)|uHeight;
	} 		
	plaimgwh=ptCapSet->uPlaImgWH;
	pacimgwh=ptCapSet->uPacImgWH;
	flag=ptCapSet->eFilterSelect;
	//UCHAR plaan_ptr=0, placn_ptr=0, pacan_ptr=0, paccn_ptr=0; 
	//UCHAR min;
	
	if( ((UINT16)(ptCapSet->uPlaImgWH) > (UINT16)(simgwh)) 		
		|| ((UINT16)(ptCapSet->uPacImgWH) > (UINT16)(simgwh)) )
		return ERR_CAP_WIDTH_HEIGHT;			//packet/planar H <= cropping H
	if( ((UINT16)(ptCapSet->uPlaImgWH >>16) > (UINT16)(simgwh>>16)) 
		||  ((UINT16)(ptCapSet->uPacImgWH>>16) > (UINT16)(simgwh>>16)) )
		return ERR_CAP_WIDTH_HEIGHT;			//packet/planar W <= cropping W
	
	
	
#ifdef DBG_MSG	
	printf("Cropping Window size = 0x%x\n", simgwh);
	printf("Planar Window size = 0x%x\n", plaimgwh);
	printf("Packet Window size = 0x%x\n", pacimgwh);
#endif	
	lcdv1=lcd(simgwh, plaimgwh);
	
	if( ((USHORT)(plaimgwh)%lcdv1 !=0) )
	{//
		return ERR_CAP_DDA;
	}
	plaan = (USHORT)(plaimgwh)/lcdv1;
	plabm = (USHORT)(simgwh)/lcdv1;
#ifdef DBG_MSG	
	printf("Planar Hieght DDA factor =%d/%d\n", plaan, plabm );
#endif	
	lcdv1 = lcd((simgwh>>16), (plaimgwh>>16));
	if( ((USHORT)(plaimgwh>>16)%lcdv1 !=0) )
	{
		return ERR_CAP_DDA;
	}
	placn = (USHORT)(plaimgwh>>16)/lcdv1;
	pladm = (USHORT)(simgwh>>16)/lcdv1;
#ifdef DBG_MSG		
	printf("Planar Width DDA factor =%d/%d\n", placn, pladm );
#endif	
	lcdv1=lcd(simgwh, pacimgwh);
	if( ((USHORT)(pacimgwh)%lcdv1 !=0) )
	{
		return ERR_CAP_DDA;
	}
	pacan = (USHORT)(pacimgwh)/lcdv1;
	pacbm = (USHORT)(simgwh)/lcdv1;
#ifdef DBG_MSG		
	printf("Packet Hieght DDA factor =%d/%d\n", pacan, pacbm );
#endif	
	lcdv1 = lcd((simgwh>>16), (pacimgwh>>16));
	if( ((USHORT)(pacimgwh>>16)%lcdv1 !=0) )
	{
		return ERR_CAP_DDA;
	}
	paccn = (USHORT)(pacimgwh>>16)/lcdv1;
	pacdm = (USHORT)(simgwh>>16)/lcdv1;
#ifdef DBG_MSG		
	printf("Packet Width DDA factor =%d/%d\n", paccn, pacdm );
#endif	
#if 0		
	tmp = ((UINT)(plaan)<<24) | ((UINT)(plabm)<<16) |  ((UINT)(placn)<<8) | pladm;
	outpw(REG_CAPPlaDownScale,tmp);	
	height = (UINT16)(dwCropImgWH)*plaan/plabm; 		//Calculate the height of Planar real image  
	width = (UINT16)(dwCropImgWH>>16)*placn/pladm; 		//Calculate the width of Planar real image
	outpw(REG_CAPlaRealSize, ((UINT)(width)<<16|(height)) ); 		

	tmp = ((UINT)(pacan)<<24) | ((UINT)(pacbm)<<16) |  ((UINT)(paccn)<<8) | pacdm;
	outpw(REG_CAPPacDownScale,tmp);		
	height = (UINT16)(dwCropImgWH)*pacan/pacbm; 		//Calculate the height of Planar real image  
	width = (UINT16)(dwCropImgWH>>16)*paccn/pacdm; 	//Calculate the width of Planar real imag		
	outpw(REG_CAPPacRealSize, ( (UINT)(width)<<16|(height) ) ); 
#else
	factor(plaan,plaan_arr);
	factor(plabm,plabm_arr);
	factor(placn,placn_arr);
	factor(pladm,pladm_arr);
	
	factor(pacan,pacan_arr);
	factor(pacbm,pacbm_arr);
	factor(paccn,paccn_arr);
	factor(pacdm,pacdm_arr);
	
	
	//The should reduce to a function 
	ppfn = maxlcd(plaan_arr,placn_arr,pacan_arr,paccn_arr); 	
	ppfm = maxlcd(plabm_arr,pladm_arr,pacbm_arr,pacdm_arr);		

	if( (ppfn>ppfm) || (plaan/ppfn>plabm/ppfm) || (placn/ppfn>pladm/ppfm) || 
					   (pacan/ppfn>pacbm/ppfm) || (paccn/ppfn>pacdm/ppfm) || (ppfm==1) )
	{//turn off poly phase due to conditional conflict
		ppfn=1;
		ppfm=1;
#ifdef DBG_MSG			
		printf("Poly-phase numerator=%d\n", ppfn);	
		printf("Poly-phase denominator=%d\n", ppfm);
#endif		
		if(flag==CAP_PLANAR_PPF)
		{
#ifdef DBG_MSG			
			printf("Enable planar poly phase filter\n");
#endif			
			capSinglePipePolyPhaseFilter(plaan,plabm,placn,pladm,CAP_PLANAR_PPF);	
		}
		else
		{//Turn off planar Polyphase filter bit  
			UINT tmp=0;
#ifdef DBG_MSG				
			printf("Planar Scale height factor numerator=%d\n", plaan);
			printf("Planar Scale height factor denominator=%d\n", plabm);	
			printf("Planar Scale width factor numerator=%d\n", placn);
			printf("Planar Scale width factor denominator=%d\n", pladm);
#endif	
			
			tmp=( ((UINT)(plaan)<<24) | ((UINT)(plabm)<<16) | ((UINT)(placn)<<8) | pladm );
			outpw(REG_CAPPlaDownScale,tmp);
			
			tmp=((inpw(REG_CAPDownScaleFilter))&0x2ff); //Turn off Planar PPF
			outpw(REG_CAPDownScaleFilter,tmp);
			//Calculate the packet real image
			{//04-09-16
			
			
			}
		}
		
		{
			UINT utmp;
			USHORT usheight, uswidth;
			UINT simgwh = inpw(REG_CAPCropWinSize);
			simgwh = inpw(REG_CAPCropWinSize);
			if(ptCapSet->bIsInterlaceFields==TRUE)
			{
				UINT uHeight = (simgwh&0x0FFFF)*2;
				simgwh = (simgwh&0xFFFF0000)|uHeight;
			} 
			utmp=inpw(REG_CAPDownScaleFilter);
			if(((utmp&0x100)!=0))
			{	
				ppfn = (utmp&0xf0)>>4; 
				ppfm = (utmp&0x0f);
				if(ppfm==0)
					ppfm=16;
			}
			else
			{
				ppfn = 1; 
				ppfm = 1;
			}	
			utmp=inpw(REG_CAPPlaDownScale);					
			usheight = (UINT16)(simgwh)*ppfn/ppfm*(utmp>>24)/((utmp&0x00ff0000)>>16); 		//Calculate the height of Planar real image  
			uswidth = (UINT16)(simgwh>>16)*ppfn/ppfm*((utmp&0x00ff00)>>8)/(utmp&0x00ff); 	//Calculate the width of Planar real imag		
			outpw(REG_CAPlaRealSize, ( (UINT)(uswidth)<<16|(usheight) ) ); 
		}
		
		if(flag==CAP_PACKET_PPF)
		{
#ifdef DBG_MSG				
			printf("Enable packet poly phase filter\n");
			printf("\n");
#endif			
			capSinglePipePolyPhaseFilter(pacan,pacbm,paccn,pacdm,CAP_PACKET_PPF);	
		}
		else
		{//Turn off packet Polyphase filter bit
			UINT tmp=0;
#ifdef DBG_MSG			
			printf("Packet Scale height factor numerator=%d\n", pacan);
			printf("Packet Scale height factor denominator=%d\n", pacbm);	
			printf("Packet Scale width factor numerator=%d\n", paccn);
			printf("Packet Scale width factor denominator=%d\n", pacdm);	
			printf("\n");
#endif			
			tmp=( ((UINT)(pacan)<<24) | ((UINT)(pacbm)<<16) | ((UINT)(paccn)<<8) | pacdm );
			outpw(REG_CAPPacDownScale,tmp);	
			tmp=((inpw(REG_CAPDownScaleFilter))&0x1ff);
			outpw(REG_CAPDownScaleFilter,tmp);
		}	
		
		{
			UINT utmp;
			USHORT usheight, uswidth;
			UINT simgwh = inpw(REG_CAPCropWinSize);
			simgwh = inpw(REG_CAPCropWinSize);
			if(ptCapSet->bIsInterlaceFields==TRUE)
			{
				UINT uHeight = (simgwh&0x0FFFF)*2;
				simgwh = (simgwh&0xFFFF0000)|uHeight;
			} 
			utmp=inpw(REG_CAPDownScaleFilter);
			if(((utmp&0x200)!=0))
			{	
				ppfn = (utmp&0xf0)>>4; 
				ppfm = utmp&0x0f;
				if(ppfm==0)
					ppfm=16;
			}
			else
			{
				ppfn = 1; 
				ppfm = 1;
			}	
			utmp=inpw(REG_CAPPacDownScale);					
			usheight = (UINT16)(simgwh)*ppfn/ppfm*(utmp>>24)/((utmp&0x00ff0000)>>16); 		//Calculate the height of Planar real image  
			uswidth = (UINT16)(simgwh>>16)*ppfn/ppfm*((utmp&0x00ff00)>>8)/(utmp&0x00ff); 	//Calculate the width of Planar real imag		
			outpw(REG_CAPPacRealSize, ( (UINT)(uswidth)<<16|(usheight) ) ); 
		}
	}	
	else
	{//turn on poly phase for packet & Planar
		UINT tmp=0;
		USHORT height, width;
		UINT dwCropImgWH = inpw(REG_CAPCropWinSize);
		if(ptCapSet->bIsInterlaceFields==TRUE)
		{
			UINT uHeight = (dwCropImgWH&0x0FFFF)*2;
			dwCropImgWH = (dwCropImgWH&0xFFFF0000)|uHeight;
		} 
		
		plaan=plaan/ppfn;
		placn=placn/ppfn;
		pacan=pacan/ppfn;
		paccn=paccn/ppfn;	
	
		plabm=plabm/ppfm;
		pladm=pladm/ppfm;
		pacbm=pacbm/ppfm;
		pacdm=pacdm/ppfm;
#ifdef DBG_MSG			
		printf("Poly-phase numerator=%d\n", ppfn);	
		printf("Poly-phase denominator=%d\n", ppfm);
#endif		
		if(ppfm==16)
			tmp=tmp|0x300|(ppfn<<4);			// DSF_M=0 mean the ppfm=16
		else
			tmp=tmp|0x300|(ppfn<<4)|(ppfm);	
		outpw(REG_CAPDownScaleFilter,tmp);
#ifdef DBG_MSG			
		printf("Planar Scale height factor numerator=%d\n", plaan);
		printf("Planar Scale height factor denominator=%d\n", plabm);	
		printf("Planar Scale width factor numerator=%d\n", placn);
		printf("Planar Scale width factor denominator=%d\n", pladm);
#endif		
		tmp = ((UINT)(plaan)<<24) | ((UINT)(plabm)<<16) |  ((UINT)(placn)<<8) | pladm;
		outpw(REG_CAPPlaDownScale,tmp);
#if 1		
		height = (UINT16)(dwCropImgWH)*ppfn/ppfm*plaan/plabm; 		//Calculate the height of Planar real image  
		width = (UINT16)(dwCropImgWH>>16)*ppfn/ppfm*placn/pladm; 	//Calculate the width of Planar real image
		outpw(REG_CAPlaRealSize, ((UINT)(width)<<16|(height)) ); 
#else	
		outpw(REG_CAPlaRealSize, ptCapSet->uPlaImgWH ); 
#endif	
#ifdef DBG_MSG	
		printf("Packet Scale height factor numerator=%d\n", pacan);
		printf("Packet Scale height factor denominator=%d\n", pacbm);	
		printf("Packet Scale width factor numerator=%d\n", paccn);
		printf("Packet Scale width factor denominator=%d\n", pacdm);
		printf("\n");
#endif		
		tmp = ((UINT)(pacan)<<24) | ((UINT)(pacbm)<<16) |  ((UINT)(paccn)<<8) | pacdm;
		outpw(REG_CAPPacDownScale,tmp);	

#if 1		
		height = (UINT16)(dwCropImgWH)*ppfn/ppfm*pacan/pacbm; 		//Calculate the height of Planar real image  
		width = (UINT16)(dwCropImgWH>>16)*ppfn/ppfm*paccn/pacdm; 	//Calculate the width of Planar real imag		
		outpw(REG_CAPPacRealSize, ( (UINT)(width)<<16|(height) ) ); 
#else
		//
		outpw(REG_CAPPacRealSize, ptCapSet->uPacImgWH ); 
#endif			
	}
#endif	
	return Successful;
}
UCHAR maxlcd(UCHAR* plaan_arr, UCHAR* placn_arr, UCHAR* pacan_arr, UCHAR* paccn_arr)
{
	UCHAR plaan_ptr=0, placn_ptr=0, pacan_ptr=0, paccn_ptr=0; 
	USHORT lcdv1;
	UCHAR min;
	UCHAR ucGcd=1;
	for(lcdv1=0;lcdv1<12;lcdv1=lcdv1+1)
	{//trace Polyphase
		min=fin_min(plaan_arr[plaan_ptr],placn_arr[placn_ptr],pacan_arr[pacan_ptr],paccn_arr[paccn_ptr]);
		if(min==0)
			break;//
		if( (plaan_arr[plaan_ptr]==min) && (paccn_arr[placn_ptr]==min) && 
			(pacan_arr[pacan_ptr]==min) && (paccn_arr[paccn_ptr]==min) )
		{//		
			ucGcd = ucGcd*min;		
			if(ucGcd>16)
			{
				ucGcd=ucGcd/min;
				break;								
			}				
			plaan_ptr=plaan_ptr+1;
			placn_ptr=placn_ptr+1;
			pacan_ptr=pacan_ptr+1;
			paccn_ptr=paccn_ptr+1;
		}
		else
		{//
			if(plaan_arr[plaan_ptr]==min)
				plaan_ptr=plaan_ptr+1;
			if(placn_arr[placn_ptr]==min)
				placn_ptr=placn_ptr+1;
			if(pacan_arr[pacan_ptr]==min)
				pacan_ptr=pacan_ptr+1;
			if(paccn_arr[paccn_ptr]==min)
				paccn_ptr=paccn_ptr+1;					
		}
	}
	return ucGcd;
}
UCHAR maxlcd_2(UCHAR* h_arr, UCHAR* w_arr, UCHAR ucLimit)
{
	UCHAR h_ptr=0, w_ptr=0; 
	USHORT lcdv1;
	UCHAR min;
	UCHAR ucGcd=1;
	for(lcdv1=0;lcdv1<12;lcdv1=lcdv1+1)
	{//trace Polyphase
		min=fin_min(h_arr[h_ptr],w_arr[w_ptr],255,255);
		if(min==0)
			break;//
		if( (h_arr[h_ptr]==min) && (w_arr[w_ptr]==min) )
		{//		
			ucGcd = ucGcd*min;	
			if(ucGcd>ucLimit)
			{
				ucGcd=ucGcd/min;
				break;								
			}
			h_ptr=h_ptr+1;
			w_ptr=w_ptr+1;
		}
		else
		{//
			if(h_arr[h_ptr]==min)
				h_ptr=h_ptr+1;
			if(w_arr[w_ptr]==min)
				w_ptr=w_ptr+1;			
		}
	}
	return ucGcd;
}

UCHAR fin_min(UCHAR a, UCHAR b, UCHAR c, UCHAR d )
{
	//UCHAR min1, min2;
	//min1 = (a<b) ? a : b;
	//min2 = (c<d) ? c : d;
	//return ((min1<min2)? min1:min2);		
	return (  ( ((a<b) ? a : b) < ((c<d) ? c : d) ) ? ((a<b) ? a : b):((c<d) ? c : d)  );

}


void factor(UINT n, UCHAR *arr)
{
	UINT i,m;
	UCHAR t=0;
	if(n!=1 && n!=0)
	{
#ifdef DBG_MSG		
		printf(" %5u = ",n);
#endif	
		for(i=2;n!=1;i=i+1)
		{
			for(m=0;n%i==0;m=m+1)
			{
				arr[t]=i;
				t=t+1;	
				n/=i;
			}
#ifdef DBG_MSG			
			if(m!=0)		
				printf("%d^%d ",i,m);	
			if(n==1)	
				printf(".\n");		
			else if(m != 0)	
				printf("x ");
#endif			
		}		
	}
	else
	{
		arr[t]=1;
#ifdef DBG_MSG	
		printf(" %5u=%dx%d\n",n,n,n);
#endif	
	}	
}									
typedef struct tagFactor
{
	USHORT ucHeightNumer;
	USHORT ucHeightDenom;
	USHORT ucWidthNumer;
	USHORT ucWidthDenom;
}T_FACTOR;
void capSinglePipePolyPhaseFilter(USHORT an, USHORT bm, USHORT cn, USHORT dm, E_PPF_FLAG flag)
{	
	USHORT lcdv1, lcdv2;
	USHORT anp, bmp, cnp, dmp;
	UINT tmp;
	UCHAR ppfn,ppfm;
	lcdv1=lcd(bm,dm);					//Denominator of Height(bm) and Width(dm) 
	bmp=bm/lcdv1;	
	dmp=dm/lcdv1;	
	lcdv2=lcd(an,cn);					//Numerator of Height(an) and Width(cn) 
	anp=an/lcdv2;	
	cnp=cn/lcdv2;
	
	if((lcdv1>=16) || (lcdv2>=16))	
	{//over the poly-phase fiter
		//USHORT tmpgcd;
		UCHAR h_an_arr[12]={0},h_bm_arr[12]={0},w_cn_arr[12]={0},w_dm_arr[12]={0};
		factor(an,h_an_arr);	
		factor(bm,h_bm_arr);	
		factor(cn,w_cn_arr);	
		factor(dm,w_dm_arr);	
		ppfm = maxlcd_2(h_bm_arr,w_dm_arr,16);	
		ppfn = maxlcd_2(h_an_arr,w_cn_arr,ppfm); 				
		if( (ppfn>ppfm) || (an/ppfn>bm/ppfm) || (cn/ppfn>dm/ppfm) || (ppfm==1) )
		{//turn off poly phase due to conditional conflict
			lcdv1=1;
			lcdv2=1;
			anp=an;	
			bmp=bm;
			cnp=cn;
			dmp=dm;
		}
		else
		{
			lcdv2=ppfn;
			lcdv1=ppfm;
			anp=an/ppfn;	
			bmp=bm/ppfm;
			cnp=cn/ppfn;
			dmp=dm/ppfm;				
		}
	}
	else
	{	
		if((anp>bmp) || (cnp>dmp) )
		{		
			if( (anp*lcdv2)< lcdv1 ) 
			{
				if( cnp<(dmp*anp))
				{
					lcdv2=anp*lcdv2; 
					dmp=dmp*anp;	
					anp=1;			
				}
				else
				{//restore. It means poly f//Denominator of Height(bm) and Width(dm) 
					lcdv2 =1;
				}						
			}
			else if((cnp*lcdv2) < lcdv1)
			{
				if( anp <(bmp*cnp))
				{
					lcdv2=cnp*lcdv2;
					bmp=bmp*cnp;	
					cnp=1;		
				}			
				else
				{//restore It means poly filter will turn off
					anp = an;
					bmp = bm;
					cnp = cn;
					dmp = dm;
					lcdv1 =1;
					lcdv2 =1;									
				}												
			}	
		}
		else
		{
			if(lcdv1==1 && lcdv2==1)
			{
				if(bm<16 || dm<16)
				{	
					if(bm<16)
					{	
						if((dm*an<256) && ((dm*an)>=(cn*bm)) )
						{
							lcdv1=bm; lcdv2=an;
							anp=1; bmp=1;
							cnp=cn*bm; dmp=dm*an;
						}
					}	
					else if(dm<16) 
					{
						if((bm*cn<256) && ((bm*cn)>=(an*dm)))
						{
							lcdv1=dm; lcdv2=cn;
							anp=an*dm; bmp=bm*cn;
							cnp=1; dmp=1;
						}			
					}
				}
			}
		}
	}
	if(flag==CAP_PLANAR_PPF)
	{								
#ifdef DBG_MSG		
		printf("Planar Poly-Phase filter factor %d/%d\n", lcdv2,lcdv1);
		//printf("Hieght DDA factor =0x%x/0x%x\n", anp, bmp );
		//printf("Width DDA factor =0x%x/0x%x\n", cnp, dmp );
		printf("Planar Scale height factor numerator=%d\n", anp);
		printf("Planar Scale height factor denominator=%d\n", bmp);	
		printf("Planar Scale width factor numerator=%d\n", cnp);
		printf("Planar Scale width factor denominator=%d\n", dmp);
#endif		
		tmp=0;
		if(lcdv1>=lcdv2)
		{
			UINT uWidth;

			uWidth = 	 inpw(REG_CAPCropWinSize);	
			if( (lcdv1 != lcdv2) && (lcdv1 != 1) )
			{
				if(lcdv1==16)	
					tmp=tmp|0x100|(lcdv2<<4);
				else
					tmp=tmp|0x100|(lcdv2<<4)|(lcdv1);
			}
			else
			{	
				if(lcdv1==16)	
					tmp=tmp|(lcdv2<<4);		
				else
					tmp=tmp|(lcdv2<<4)|(lcdv1);		
			}
			
			if(uWidth*lcdv2/lcdv1>720)
			{//The polyphase line over 720 pixel.
				lcdv1=1;
				lcdv2=1;
				anp=an;	
				bmp=bm;
				cnp=cn;
				dmp=dm;
				tmp=0;
				goto TURN_OFF_PLANAR_PPF;
			}
			else
			{
				outpw(REG_CAPDownScaleFilter,tmp);
				tmp=( ((UINT)(anp)<<24) | ((UINT)(bmp)<<16) | ((UINT)(cnp)<<8) | dmp );
				outpw(REG_CAPPlaDownScale,tmp);	
			}	
		}
		else
		{
			//Turn off planar pipe polyphase filter, Due ppf m<n
TURN_OFF_PLANAR_PPF:			
			tmp = inpw(REG_CAPDownScaleFilter)&0x2FF;
			outpw(REG_CAPDownScaleFilter,tmp);
			tmp=( ((UINT)(an)<<24) | ((UINT)(bm)<<16) | ((UINT)(cn)<<8) | dm );
			outpw(REG_CAPPlaDownScale,tmp);
		}
	}	
		
	if(flag==CAP_PACKET_PPF)
	{
#ifdef DBG_MSG	
		printf("Packet Poly-Phase filter factor %d/%d\n", lcdv2,lcdv1);
		//printf("Hieght DDA factor =0x%x/0x%x\n", anp, bmp );
		//printf("Width DDA factor =0x%x/0x%x\n", cnp, dmp );
		printf("Packet Scale height factor numerator=%d\n", anp);
		printf("Packet Scale height factor denominator=%d\n", bmp);	
		printf("Packet Scale width factor numerator=%d\n", cnp);
		printf("Packet Scale width factor denominator=%d\n", dmp);
#endif		
		tmp=0;
		if(lcdv1>=lcdv2)
		{
			UINT uWidth;

			uWidth = 	 inpw(REG_CAPCropWinSize);		
			if( (lcdv1 != lcdv2) && (lcdv1 != 1) )
			{
				if(lcdv1==16)	
					tmp=tmp|0x200|(lcdv2<<4);
				else
					tmp=tmp|0x200|(lcdv2<<4)|(lcdv1);
			}
			else
			{
				if(lcdv1==16)	
					tmp=tmp|(lcdv2<<4);	
				else
					tmp=tmp|(lcdv2<<4)|(lcdv1);	
			}		
			if(uWidth*lcdv2/lcdv1>720)
			{//The polyphase line over 720 pixel.
				lcdv1=1;
				lcdv2=1;
				anp=an;	
				bmp=bm;
				cnp=cn;
				dmp=dm;
				goto TURN_OFF_PACKET_PPF;
			}
			else
			{
			
				outpw(REG_CAPDownScaleFilter,tmp);
				tmp=( ((UINT)(anp)<<24) | ((UINT)(bmp)<<16) | ((UINT)(cnp)<<8) | dmp );
				outpw(REG_CAPPacDownScale,tmp);
			}	
		}
		else
		{//Turn off packet pipe polyphase filter, Due ppf m<n
TURN_OFF_PACKET_PPF:		
			tmp = inpw(REG_CAPDownScaleFilter)&0x1FF;
			outpw(REG_CAPDownScaleFilter,tmp);
			tmp=( ((UINT)(an)<<24) | ((UINT)(bm)<<16) | ((UINT)(cn)<<8) | dm );
			outpw(REG_CAPPacDownScale,tmp);
		}	
	}	
}
USHORT lcd(USHORT m1, USHORT m2)
{
	USHORT m;
	if(m1<m2)
	{
		m=m1; m1=m2; m2=m;
	}
	if(m1%m2==0)
		return m2;
	else
		return (lcd(m2,m1%m2));		
}