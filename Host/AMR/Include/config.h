#ifndef AMR_CONFIG_H
#define AMR_CONFIG_H
//---------------------------------------------------------------------------------------
/* Use amr file formate */
//#define OPT_AMR_FORMAT 1

/* Default using VAD1 if VAD2 is undefined */
//#define VAD2

/* Optimize for basic op */
#define OPT_BASIC_OP 1

/*********************************/
/*	Optimized for encoder        */
/*                               */
/*********************************/
/* Optimize for cbsearch */
#define OPT_CBSEARCH 1

/* Optimize for pitch_fr */
#define OPT_PITCH_FR 1

#define OPT_CONVOLVE 1

#define OPT_COMP_CORR 1

#define OPT_SUB_FRAME_PRE_PROCESS 1

#define OPT_PITCH_OL 1

#define OPT_LPC 1

#define OPT_SYN_FILT 1

#define OPT_SUB_FRAME_POST_PROC 1

#define OPT_PRE_BIG 1

#define OPT_Q_PLSF_3 1

#define OPT_AZ_LSP 1

#define OPT_QUA_GAIN 1

#define OPT_POW2 1

#define OPT_INV_SQRT 1

#define OPT_VAD1 1

#define OPT_LOG2 1

#define OPT_VAD2 1



/* For code size purpose */


/* OPT_MR122_ONLY option is only for encoder */
//#define OPT_MR122_ONLY 1

//#define OPT_NO_DTX 1

#if defined(__AMR_DECODER) && defined(OPT_MR122_ONLY)
#	error "OPT_MR122_ONLY option is only for encoder"
#endif

#if defined(__AMR_DECODER) && defined(OPT_NO_DTX)
#	error "OPT_NO_DTX option is only for encoder"
#endif


#define OPT_CALC_EN 1



/*********************************/
/*	Optimized for decoder        */
/*                               */
/*********************************/
/* OPtimize for post-filter */
#define OPT_POST_FILTER 1

/* Optimize for dec_amr */
#define OPT_DEC_AMR 1

/* Optimize for pre_lt_3or6 */
#define OPT_PRE_LT_3OR6 1

/* Optimize for Post_Process */
#define OPT_POST_PROCESS 1

/* Optimzie for agc */
#define OPT_AGC 1

/* Optimize for Cb_gain_average */
#define OPT_CB_GAIN_AVERAGE 1

/* Optimize for ph_disp */
#define OPT_PH_DISP 1

/* Optimize for non-ISO compliant */
//#define OPT_ULTRA_OPTIMIZE 1

/* No 13 bits */
//#define NO13BIT 1



/* Platform specific option */
#ifdef FPM_ARM
	/* Optimize for add-multiply calculation */
#	define OPT_MAC 1
	/* Alignment control */
#	define ALIGN(x)	__align(x)
#else
#	define ALIGN(x)	
#endif

//#define ASSERT(x,str)	if( !(x) ){printf("ERROR: %s",str);exit(-1);}
#define ASSERT(x, str)

//#define malloc_sum 1

#define dbg_printf(...)
//#define dbg_printf diag_printf//clyu
//#define dbg_printf printf

//---------------------------------------------------------------------------------------
#endif
