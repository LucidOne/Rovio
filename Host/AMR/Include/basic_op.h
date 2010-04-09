#ifndef _basic_op_h_

#define _basic_op_h_

#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif


/*___________________________________________________________________________
 |                                                                           |
 |   Prototypes for basic arithmetic operators                               |
 |___________________________________________________________________________|
*/


#ifdef ECOS
#include "stdio.h"
#include "stdlib.h"
#else
#include <stdio.h>
#include <stdlib.h>
#endif

/*________________________________________________________
___________________
 |
                  |
 |   Constants and Globals
                  |

|_________________________________________________________
__________________|
*/
extern Flag Overflow;
extern Flag Carry;

#define MAX_32 (Word32)0x7fffffffL
#define MIN_32 (Word32)0x80000000L

#define MAX_16 (Word16)0x7fff
#define MIN_16 (Word16)0x8000

/*________________________________________________________
___________________
 |
                  |
 |   Operators prototypes
                  |

|_________________________________________________________
__________________|
*/


//#define _ARM

#if defined(FPM_ARM) && defined(__TARGET_CPU_ARM946E_S) && !defined(__thumb) 


#if defined(OPT_BASIC_OP)
#	define add(x,y)	((x)+(y))
#	define Qadd(x,y)	(Word16)(((x)+(y) > 32767 )?32767:((x)+(y)<-32768)?-32768:((x)+(y)))
#else
	Word16 add (Word16 var1, Word16 var2);    /* Short add,           1   */
#	define Qadd	add
#endif
//------------------------------------------------------------------

#if defined(OPT_BASIC_OP)
#	define sub(x,y)	((x)-(y))
#	define Qsub(x,y)	((int)(x)-(int)(y) > 32767)?32767:((int)(x)-(int)(y) < -32768)?-32768:((x)-(y))
#else
	Word16 sub (Word16 var1, Word16 var2);    /* Short sub,           1   */
#	define Qsub		sub
#endif

//------------------------------------------------------------------

#if defined(OPT_BASIC_OP)
#	define abs_s(x)	_abs_s(x)
	__inline Word16 _abs_s(int x)
	{
		if(x<0)	return -x;
		return x;
	}
	__inline Word16 Qabs_s(int x)
	{
		if(x<0)	x = -x;
		if(x>=0x8000) 
			x=MAX_16;
		return x;
	}
#else
	Word16 abs_s (Word16 var1);               /* Short abs,           1   */
#	define Qabs_s abs_s	
#endif

//------------------------------------------------------------------

#if defined(OPT_BASIC_OP)
//Word16 shl (Word16 var1, Word16 var2);    /* Short shift left,    1   */
#	define shl(x,y)	(((y)<0)?((x)>>-(y)):((x)<<(y)))
	__inline int QPCshl(unsigned int x, int c)
	{
		int result;
		result = x << c;
		if( result > 32767 ) result = 32767;
		return result;
	}

	__inline int QCshl(int x, int c)
	{
		int result;
		result = x << c;
		if( result > 32767 ) result = 32767;
		if( result < -32768) result = -32768;
		return result;
	}

	__inline int Qshl(int x, int y)
	{
		int result;
		if( y < 0 )
			result = x >> -y;
		else
			result = x << y;
		if(result>32767)
			result = 32767;
		if(result<-32768)
			result = -32768;
		return result;
	}
#else
	Word16 shl (Word16 var1, Word16 var2);    /* Short shift left,    1   */
#	define Qshl		shl
#	define QCshl	shl
#	define QPCshl	shl
#endif


//------------------------------------------------------------------

#if defined(OPT_BASIC_OP)
//	Word16 shr (Word16 var1, Word16 var2);    /* Short shift right,   1   */
#	define shr(x,y) ((x)>>(y))
	__inline int Qshr(int x, int y)
	{
		if(y < 0)
		{
			int result;
			result = x << -y;
			if(result > 32767)
				result = 32767;
			return result;
		}
		return (x >> y);
	}
#else
	Word16 shr (Word16 var1, Word16 var2);    /* Short shift right,   1   */
#	define Qshr shr	
#endif

//------------------------------------------------------------------
	
#if defined(OPT_BASIC_OP)
#	define mult(x,y) ((x)*(y)>>15)
#else
	Word16 mult (Word16 var1, Word16 var2);   /* Short mult,          1   */
#endif

//------------------------------------------------------------------
	
#if defined(OPT_BASIC_OP)
#	define L_mult(x,y)	_QL_mult(x,y)
#	define QL_mult(x,y) _QL_mult(x,y)
	__inline int _QL_mult(int x, int y)
	{
		int result;
		__asm
		{
			SMULBB	result, x, y;
			QADD	result, result, result
		}
		return result;
	}
#else
	Word32 L_mult (Word16 var1, Word16 var2); /* Long mult,           1   */
#	define QL_mult	L_mult
#endif

//------------------------------------------------------------------
#if defined(OPT_BASIC_OP)
#	define negate(x)	(-(x))
	__inline Qnegate(int x)
	{
		if( x <-32767 )
			return 32767;
		return -x;
	}
#else
	Word16 negate (Word16 var1);              /* Short negate,        1   */
#	define Qnegate negate	
#endif

//------------------------------------------------------------------

#if defined(OPT_BASIC_OP)
#	define extract_h(x)		(x>>16)
#else
	Word16 extract_h (Word32 L_var1);         /* Extract high,        1   */
#endif

//------------------------------------------------------------------

#if defined(OPT_BASIC_OP)
#	define extract_l(x)	(Word16)(x)
#else
	Word16 extract_l (Word32 L_var1);         /* Extract low,         1   */
#endif

//------------------------------------------------------------------

#if defined(OPT_BASIC_OP)
#	define round(x)	(((x)+0x8000) >> 16)
	__inline int Qround( int x)
	{
		int result;
		if( x > 0x7FFF0000 )
			return MAX_16;
		result = (x + 0x8000) >> 16;
		return result;
	}
#else
	Word16 round (Word32 L_var1);             /* Round,               1   */
#	define Qround	round
#endif

//------------------------------------------------------------------

#if defined(OPT_BASIC_OP)
#	define L_mac	QL_mac
	__inline int QL_mac(int acc, int x, int y)
	{
		int result;
		__asm
		{
			SMULBB	result, x, y;
			QDADD	result, acc, result;
		}
			return result;
	}

	__inline int OL_mac(int acc, int x, int y) // set Overflow flag
	{
		int product,result;
		
		product = x * y;
		if( product == 0x40000000L )
		{
			Overflow = 1;
			if( acc > 0 )
				return MAX_32;
			else
				return MAX_32 + acc;
		}
		else
		{
			result = acc + (product << 1);
			if( !((product^acc)>>31) && ((result^acc)>>31) )
			{	
				Overflow = 1;
				result=(acc<0)?MIN_32:MAX_32;
			}
			return result;
		}
	}

	__inline int QL_mda(int acc, int x, int y) /* for x == y */
	{
		int result;
		if( x == 0x8000 || acc == MAX_32 )
			return MAX_32;
		else
		{
			result = acc + (x * x << 1) ;
			if(result < 0)
				return MAX_32;
			else
				return result;
		}
	}
#else
Word32 L_mac (Word32 L_var3, Word16 var1, Word16 var2);   /* Mac,  1  */
#	define QL_mda	L_mac
#	define QL_mac	L_mac
#	define OL_mac	L_mac
#endif

//------------------------------------------------------------------

#if defined(OPT_BASIC_OP)
#	if defined(OPT_ULTRA_OPTIMIZE)
#		define L_msu	_L_msu	
#	endif
__inline Word32 _L_msu(Word32 L_var3, Word16 var1, Word16 var2) /* Msu,    1 */
{
    Word32 L_produit;
	__asm
	{
		SMULBB	L_produit, var1, var2
		QDSUB	L_var3, L_var3, L_produit
	}
    return(L_var3);
}

#else
	Word32 L_msu (Word32 L_var3, Word16 var1, Word16 var2);   /* Msu,  1  */
#endif

//------------------------------------------------------------------

Word32 L_macNs (Word32 L_var3, Word16 var1, Word16 var2); /* Mac without
                                                             sat, 1   */
//------------------------------------------------------------------

Word32 L_msuNs (Word32 L_var3, Word16 var1, Word16 var2); /* Msu without
                                                             sat, 1   */
//------------------------------------------------------------------

#if defined(OPT_BASIC_OP)
	//Word32 L_add (Word32 L_var1, Word32 L_var2);    /* Long add,        2 */
#	define L_add(x,y)	((x)+(y))
	__inline int QL_add(int x, int y)
	{
		int result;

		result = x + y;

		if( !((x^y)>>31) && ((result^x)>>31) )
		{
			result=(x<0)?MIN_32:MAX_32;
		}
		return result;
	}
			
#else
	Word32 L_add (Word32 L_var1, Word32 L_var2);    /* Long add,        2 */
#	define QL_add L_add
#endif

//------------------------------------------------------------------
#if defined(OPT_BASIC_OP)
#	define	L_sub(x,y)	((x)-(y))
	__inline int QL_sub(int x, int y)
	{
		int result;

		result = x - y;

		if( ((x^y)>>31) && ((result^x)>>31) )
		{
			result=(x<0)?MIN_32:MAX_32;
		}
		return result;
	}

#else
	Word32 L_sub (Word32 L_var1, Word32 L_var2);    /* Long sub,        2 */
#	define	QL_sub	L_sub
#endif

//------------------------------------------------------------------
#if defined(OPT_ULTRA_OPTIMIZE)
#	define L_add_c(x,y)	((x)+(y))
#else
	Word32 L_add_c (Word32 L_var1, Word32 L_var2);  /* Long add with c, 2 */
#endif

//------------------------------------------------------------------

#if defined(OPT_ULTRA_OPTIMIZE)
#	define L_sub_c(x,y)	((x)-(y))
#else
Word32 L_sub_c (Word32 L_var1, Word32 L_var2);  /* Long sub with c, 2 */
#endif

//------------------------------------------------------------------
#if defined(OPT_ULTRA_OPTIMIZE)
#	define L_negate(x)	(-(x))
#else
Word32 L_negate (Word32 L_var1);                /* Long negate,     2 */
#endif

//------------------------------------------------------------------

#if defined(OPT_ULTRA_OPTIMIZE)
#	define	mult_r(x,y)	((x)*(y)>>15)
#else
Word16 mult_r (Word16 var1, Word16 var2);       /* Mult with round, 2 */
#endif

//------------------------------------------------------------------

#if defined(OPT_ULTRA_OPTIMIZE)
#	define L_shl(x,shift)	((x)<<(shift))
#	define PL_shl	L_shl
#	define QL_shl1	L_shl
#	define QPL_shl2	L_shl
#	define QL_shl3	L_shl
#	define QL_shl4	L_shl
#	define QL_shl6	L_shl
#	define QL_shl7	L_shl
#	define QL_shl	L_shl
#else
#	if defined(OPT_BASIC_OP)
#		define L_shl(x,shift)	((x)<<(shift))
	
	__inline int QL_shl(int x, int shift)
	{
		if( shift < 0)
			return x >> -shift;
		else
		{
			for (; shift > 0; shift--)
			{
				if (x > (Word32) 0X3fffffffL)
				{
					//Overflow = 1;
	                x = MAX_32;
		            break;
			    }
				else
	            {
		            if (x < (Word32) 0xc0000000L)
			        {
	                    //Overflow = 1;
		                x = MIN_32;
			            break;
				    }
	            }
		        x <<= 1;
	        }
		}
		return x;
	}

/* PL_shl constrain: x >= 0, no saturation */
#		define PL_shl(x, shift)	((shift) < 0)?((x) >> -(shift)):((x) << (shift))
		__inline int QPL_shl2(int x, int shift) /*constrain: x >= 0, shift == 2*/
		{
			if( x > 0x1FFFFFFF )
				return MAX_32;
			else
				return (x << 2);
		}
		__inline int QL_shl1(int x, int shift) /*constrain: shift == 1*/
		{
			if( x > (int)0x3FFFFFFF )
				return MAX_32;
			if( x < (int)0xC0000000 )
				return MIN_32;
			return (x << 1);
		}
		__inline int QL_shl3(int x, int shift) /*constrain: shift == 3*/
		{
			if( x > (int)0x0FFFFFFF )
				return MAX_32;
			if( x < (int)0xF0000000 )
				return MIN_32;
			return (x << 3);
		}
		__inline int QL_shl4(int x, int shift) /*constrain: shift == 4*/
		{
			if( x > (int)0x07FFFFFF )
				return MAX_32;
			if( x < (int)0xF8000000 )
				return MIN_32;
			return (x << 4);
		}
	__inline int QL_shl6(int x, int shift) /*constrain: shift == 4*/
	{
		if( x > (int)0x01FFFFFF )
			return MAX_32;
		if( x < (int)0xFE000000 )
			return MIN_32;
		return (x << 6);
	}
	__inline int QL_shl7(int x, int shift) /*constrain: shift == 4*/
	{
		if( x > (int)0x00FFFFFF )
			return MAX_32;
		if( x < (int)0xFF000000 )
			return MIN_32;
		return (x << 7);
	}
		
#	else
		Word32 L_shl (Word32 L_var1, Word16 var2);      /* Long shift left, 2 */
#		define PL_shl	L_shl
#		define QL_shl1	L_shl
#		define QPL_shl2	L_shl
#		define QL_shl3	L_shl
#		define QL_shl4	L_shl
#		define QL_shl6	L_shl
#		define QL_shl7	L_shl
#		define QL_shl	L_shl

#	endif // end of OPT BASIC OP
#endif // end of ULTRA OPTIMIZE
//------------------------------------------------------------------
#if defined(OPT_ULTRA_OPTIMIZE)
#	define L_shr(x,y)	((x)>>(y))
#	define L_shrs		L_shr
#	define L_shrsq		L_shr
#else
#if defined(OPT_BASIC_OP)
#	define	L_shr(x,shift)	((x)>>(shift))
/* L_shrs constrain: no saturation if shift < 0 */
#	define	L_shrs(x,shift)	((shift)<0)?((x)<<-(shift)):((x)>>(shift))
	__inline int L_shrsq(int x, int shift)
	{
		if(shift<0)
		{
			shift = -shift;
			for (; shift > 0; shift--)
			{
				if (x > (Word32) 0X3fffffffL)
				{
					Overflow = 1;
	                x = MAX_32;
		            break;
			    }
				else
	            {
		            if (x < (Word32) 0xc0000000L)
			        {
	                    Overflow = 1;
		                x = MIN_32;
			            break;
				    }
	            }
		        x <<= 1;
	        }
		}
		else
		{
			if( shift > 31 )
			{
				if(x < 0) x = -1;
				else x = 0;
			}
			else
				x = (x >> shift);
		}

		return x;
	
	}

#else
Word32 L_shr (Word32 L_var1, Word16 var2);      /* Long shift right, 2*/
#	define	L_shrs	L_shr
#	define	L_shrsq	L_shr
#endif
#endif // ULTRA OPTIMIZE
//------------------------------------------------------------------
#if defined(OPT_ULTRA_OPTIMIZE)
#	define shr_r(x,shift)	((x)>>(shift))
#	define Qshr_r	shr_r
#else
#if defined(OPT_BASIC_OP)
/* shr_r constrain: shift >= 0 */
#	define shr_r(x,shift)	(( (x) + (1<< ((shift)-1) )) >> (shift))
	__inline int Qshr_r(int x, int shift)
	{
		int result;
		if(shift < 0)
		{
			result = x << -shift;
			if( result > 32767 )
				result = 32767;
			if( result < -32768 )
				result = -32768;
		}
		else
		{
			result = (( (x) + (1<< ((shift)-1) )) >> (shift));
		}

		return result;
	}

#else
Word16 shr_r (Word16 var1, Word16 var2);        /* Shift right with
                                                   round, 2           */
#	define Qshr_r	shr_r
#endif
#endif // ULTRA OPTIMIZE
//------------------------------------------------------------------

Word16 mac_r (Word32 L_var3, Word16 var1, Word16 var2); /* Mac with
                                                           rounding,2 */
//------------------------------------------------------------------

Word16 msu_r (Word32 L_var3, Word16 var1, Word16 var2); /* Msu with
                                                           rounding,2 */
//------------------------------------------------------------------

#if defined(OPT_BASIC_OP)
#	define	L_deposit_h(x)	((int)(x)<<16)
#else
Word32 L_deposit_h (Word16 var1);        /* 16 bit var1 -> MSB,     2 */
#endif

//------------------------------------------------------------------

#if defined(OPT_BASIC_OP)
#	define	L_deposit_l(x)	((int)(x))
#else
Word32 L_deposit_l (Word16 var1);        /* 16 bit var1 -> LSB,     2 */
#endif

//------------------------------------------------------------------
#if defined(OPT_ULTRA_OPTIMIZE)
#	define L_shr_r(x,shift)	((x)>>(shift))
#	define L_shr_rl	L_shr_r
#	define QL_shr_r	L_shr_r
#else
#if defined(OPT_BASIC_OP)
/* L_shr_r constrain: shift >= 0 */
//#	define L_shr_r(x,shift)	(( (int)(x) + (1<< ((shift)-1) )) >> (shift))
#	define L_shr_r	_L_shr_r
	__inline int _L_shr_r(int x, int shift)	/*constrain: shift >= 0 */
	{
		x = (x >> (shift-1));
		x++;
		return (x >> 1);
	}
	__inline int L_shr_rl(int x, int shift)	/*constrain: shift >= 0 */
	{
		if( shift > 31 )
			return 0;
		x = (x >> (shift-1));
		x++;
		return (x >> 1);
	}
	__inline int QL_shr_r(int x, int shift)	
	{
		if(shift<0)
		{
			shift = -shift;
			for (; shift > 0; shift--)
			{
				if (x > (Word32) 0X3fffffffL)
				{
					Overflow = 1;
	                x = MAX_32;
		            break;
			    }
				else
	            {
		            if (x < (Word32) 0xc0000000L)
			        {
	                    Overflow = 1;
		                x = MIN_32;
			            break;
				    }
	            }
		        x <<= 1;
	        }
		}
		else
		{
			if( shift > 31 )
			{
				if(x < 0) x = -1;
				else x = 0;
			}
			else
			{
				if( shift > 0 )
				{
					x = (x >> (shift-1));
					x++;
					x = x >> 1;
				}
			}
		}

		return x;
	
	}
	

#else
	Word32 L_shr_r (Word32 L_var1, Word16 var2); /* Long shift right with
		                                            round,  3             */
#	define L_shr_rl	L_shr_r
#	define QL_shr_r	L_shr_r

#endif
#endif // ULTRA OPTIMIZE
//------------------------------------------------------------------

#if defined(OPT_BASIC_OP)
#	define L_abs(x)	(((x)<0)?-(x):(x))	/* constrain: no saturation */
#else
Word32 L_abs (Word32 L_var1);            /* Long abs,              3  */
#endif

//------------------------------------------------------------------

Word32 L_sat (Word32 L_var1);            /* Long saturation,       4  */

//------------------------------------------------------------------
#if defined(OPT_BASIC_OP)
#	define norm_s	_norm_s
	__inline int _norm_s(int x)	/* constrain: x >= 0 */
	{
		if( x >= 0x800 )
		{
			if( x >= 0x4000 )			return 0;
			if( x >= 0x2000 )			return 1;
			if( x >= 0x1000 )			return 2;
			if( x >= 0x800 )			return 3;
		}
		else if( x >= 0x80 )
		{
			if( x >= 0x400 )			return 4;
			if( x >= 0x200 )			return 5;
			if( x >= 0x100 )			return 6;
			if( x >= 0x80 )				return 7;
		}
		else if( x >= 0x8 )
		{
			if( x >= 0x40 )			return 8;
			if( x >= 0x20 )			return 9;
			if( x >= 0x10 )			return 10;
			if( x >= 0x8 )			return 11;
		}
		else
		{
			if( x >= 0x4 )			return 12;
			if( x >= 0x2 )			return 13;
			if( x >= 0x1 )			return 14;
			if( x == 0 )			return 0;
		}
	}
#else
Word16 norm_s (Word16 var1);             /* Short norm,           15  */
#endif


//------------------------------------------------------------------

#if defined(OPT_BASIC_OP)
#	define	div_s(x,y)	(((int)(x)<<15)/(y)) /* constrain: no saturation, x>0, y>0, y > x */
#else
Word16 div_s (Word16 var1, Word16 var2); /* Short division,       18  */
#endif

//------------------------------------------------------------------

Word16 norm_l (Word32 L_var1);           /* Long norm,            30  */   


//------------------------------------------------------------------
#define	Copy(in, out, size)	memcpy((char *)(out), (char *)(in), (size)*2)
#define Mpy_32_16(hi, lo, n) (((hi)*(n)<<1) + (((lo)*(n)>>15)<<1))


#else // FPM_ARM

#if defined(OPT_BASIC_OP)
#		if 1
#	define add(x,y)	((x)+(y))
#		else
#	define add	_add
	__inline int _add(int x, int y)
	{
		int result;
		result = x+y;
		if(result > 32767)
			return 32767;
		if(result < -32768 )
			return -32768;
		return result;
	}
#		endif
#	define Qadd(x,y)	(Word16)(((x)+(y) > 32767 )?32767:((x)+(y)<-32768)?-32768:((x)+(y)))
#else
	Word16 add (Word16 var1, Word16 var2);    /* Short add,           1   */
#	define Qadd	add
#endif
//------------------------------------------------------------------

#if defined(OPT_BASIC_OP)
#		if 1
#	define sub(x,y)	((x)-(y))
#		else
#	define sub	_sub
	__inline int _sub(int x, int y)
	{
		int result;
		result = x-y;
		if(result > 32767)
			return 32767;
		if(result < -32768 )
			return -32768;
		return result;
	}
#		endif
#	define Qsub(x,y)	((int)(x)-(int)(y) > 32767)?32767:((int)(x)-(int)(y) < -32768)?-32768:((x)-(y))
#else
	Word16 sub (Word16 var1, Word16 var2);    /* Short sub,           1   */
#	define Qsub		sub
#endif

//------------------------------------------------------------------

#if defined(OPT_BASIC_OP)
#	define abs_s(x)	_abs_s(x)
	__inline Word16 _abs_s(int x)
	{
		if(x<0)	return -x;
		return x;
	}
	__inline Word16 Qabs_s(int x)
	{
		if(x<0)	x = -x;
		if(x>=0x8000) 
			x=MAX_16;
		return x;
	}
#else
	Word16 abs_s (Word16 var1);               /* Short abs,           1   */
#	define Qabs_s	abs_s
#endif

//------------------------------------------------------------------

#if defined(OPT_BASIC_OP)
//Word16 shl (Word16 var1, Word16 var2);    /* Short shift left,    1   */
#			if 1
#	define shl(x,y)	(((y)<0)?((x)>>-(y)):((x)<<(y)))
#			else
#	define shl	_shl
	__inline int _shl(int x, int y)
	{
		int result;
		if(y<0)
		{
			result = x >> (-y);
			return result;
		}
		else
		{
			result = x << y;
			if(result > 32767)
				result = 32767;
			if(result < -32768)
				result = -32768;

			return result;
		}
	}
#			endif
	__inline int QPCshl(unsigned int x, int c)
	{
		int result;
		result = x << c;
		if( result > 32767 ) result = 32767;
		return result;
	}

	__inline int QCshl(int x, int c)
	{
		int result;
		result = x << c;
		if( result > 32767 ) result = 32767;
		if( result < -32768) result = -32768;
		return result;
	}

	__inline int QNshl(int x, int y)
	{
		int result;
		if( y < 0 )
			result = x >> -y;
		else
			result = x << y;
		if(result<-32768)
			result = -32768;
		return result;
	}

	__inline int Qshl(int x, int y)
	{
		int result;
		if( y < 0 )
			result = x >> -y;
		else
			result = x << y;
		if(result>32767)
			result = 32767;
		if(result<-32768)
			result = -32768;
		return result;
	}
#else
	Word16 shl (Word16 var1, Word16 var2);    /* Short shift left,    1   */
#	define Qshl		shl
#	define QCshl	shl
#	define QPCshl	shl
#endif


//------------------------------------------------------------------

#if defined(OPT_BASIC_OP)
#		if 1
#	define shr(x,y) ((x)>>(y))
#		else
#	define shr _shr
	__inline int _shr(int x, int y)
	{
		if(y < 0)
		{
			int result;
			result = x << -y;
			if(result > 32767)
				result = 32767;
			return result;
		}
		return (x >> y);
	}
#		endif
	__inline int Qshr(int x, int y)
	{
		if(y < 0)
		{
			int result;
			result = x << -y;
			if(result > 32767)
				result = 32767;
			return result;
		}
		return (x >> y);
	}
#else
	Word16 shr (Word16 var1, Word16 var2);    /* Short shift right,   1   */
#	define Qshr	shr
#endif

//------------------------------------------------------------------
	
#if defined(OPT_BASIC_OP)
#	define mult(x,y) ((x)*(y)>>15)
#else
	Word16 mult (Word16 var1, Word16 var2);   /* Short mult,          1   */
#endif

//------------------------------------------------------------------
	
#if defined(OPT_BASIC_OP)
#	define L_mult(x,y)	((x)*(y)<<1)
#	define QL_mult(x,y) _QL_mult(x,y)
#		if 0
#	define L_mult(x,y)	__QL_mult(x,y)
	__inline int __QL_mult(int x, int y)
	{
		int result;
		result = x * y;
		if( result == 0x40000000L )
			result = MAX_32;
		else
			return (result << 1);
	}
#		endif
	__inline int _QL_mult(int x, int y)
	{
		int result;
		result = x * y;
		if( result == 0x40000000L )
			result = MAX_32;
		else
			return (result << 1);
	}
#else
	Word32 L_mult (Word16 var1, Word16 var2); /* Long mult,           1   */
#	define QL_mult	L_mult
#endif

//------------------------------------------------------------------
#if defined(OPT_BASIC_OP)
#		if 1
#	define negate(x)	(-(x))
#		else
#	define negate	_negate
	__inline _negate(int x)
	{
		if( x <-32767 )
			return 32767;
		return -x;
	}
#		endif

	__inline Qnegate(int x)
	{
		if( x <-32767 )
			return 32767;
		return -x;
	}


#else
	Word16 negate (Word16 var1);              /* Short negate,        1   */
#	define Qnegate negate
#endif

//------------------------------------------------------------------

#if defined(OPT_BASIC_OP)
#	define extract_h(x)		(x>>16)
#else
	Word16 extract_h (Word32 L_var1);         /* Extract high,        1   */
#endif

//------------------------------------------------------------------

#if defined(OPT_BASIC_OP)
#	define extract_l(x)	(Word16)(x)
#else
	Word16 extract_l (Word32 L_var1);         /* Extract low,         1   */
#endif

//------------------------------------------------------------------

#if defined(OPT_BASIC_OP)
#		if 1
#	define round(x)	(((x)+0x8000) >> 16)
#		else
#	define round	_Qround
	__inline int _Qround( int x)
	{
		int result;
		if( x > 0x7FFF0000 )
			return MAX_16;
		result = (x + 0x8000) >> 16;
		return result;
	}
#		endif
	__inline int Qround( int x)
	{
		int result;
		if( x > 0x7FFF0000 )
			return MAX_16;
		result = (x + 0x8000) >> 16;
		return result;
	}
#else
	Word16 round (Word32 L_var1);             /* Round,               1   */
#	define Qround	round
#endif

//------------------------------------------------------------------

#if defined(OPT_BASIC_OP)
#	define L_mac(acc, x, y)	_QL_mac(acc, x, y)
	__inline int _QL_mac(int acc, int x, int y)
	{
		int product,result;
		
		product = x * y;
		if( product == 0x40000000L )
		{
			//Overflow = 1;
			if( acc > 0 )
				return MAX_32;
			else
				return MAX_32 + acc;
		}
		else
		{
			result = acc + (product << 1);
			if( !((product^acc)>>31) && ((result^acc)>>31) )
			{	
				//Overflow = 1;
				result=(acc<0)?MIN_32:MAX_32;
			}
			return result;
		}
	}

	__inline int OL_mac(int acc, int x, int y) // set Overflow flag
	{
		int product,result;
		
		product = x * y;
		if( product == 0x40000000L )
		{
			Overflow = 1;
			if( acc > 0 )
				return MAX_32;
			else
				return MAX_32 + acc;
		}
		else
		{
			result = acc + (product << 1);
			if( !((product^acc)>>31) && ((result^acc)>>31) )
			{	
				Overflow = 1;
				result=(acc<0)?MIN_32:MAX_32;
			}
			return result;
		}
	}

	__inline int QL_mac(int acc, int x, int y)
	{
		int product,result;
		
		product = x * y;
		if( product == 0x40000000L )
		{
			//Overflow = 1;
			if( acc > 0 )
				return MAX_32;
			else
				return MAX_32 + acc;
		}
		else
		{
			result = acc + (product << 1);
			if( !((product^acc)>>31) && ((result^acc)>>31) )
			{	
				//Overflow = 1;
				result=(acc<0)?MIN_32:MAX_32;
			}
			return result;
		}
	}

	__inline int QL_mda(int acc, int x, int y) /* for x == y */
	{
		int result;
		if( x == 0x8000 || acc == MAX_32 )
			return MAX_32;
		else
		{
			result = acc + (x * x << 1) ;
			if(result < 0)
				return MAX_32;
			else
				return result;
		}
	}
#else
Word32 L_mac (Word32 L_var3, Word16 var1, Word16 var2);   /* Mac,  1  */
#	define QL_mda	L_mac
#	define QL_mac	L_mac
#	define OL_mac	L_mac
#endif

//------------------------------------------------------------------

#if defined(OPT_BASIC_OPT)
//#	define L_msu	QL_msu
	__inline int QL_msu(int acc, int x, int y)
	{
		int product,result;
		
		product = x * y;
		if( product == 0x40000000L )
		{
			if( aac < 0 )
				return MIN_32;
			else
				return (aac - MAX_32);
		}
		else
		{
			result = acc - product;
			if( ((product^acc)>>31) && ((result^product)>>31) )
				result=(acc<0)?MIN_32:MAX_32;
			return result;
		}
	}
#else
	Word32 L_msu (Word32 L_var3, Word16 var1, Word16 var2);   /* Msu,  1  */
#endif

//------------------------------------------------------------------

Word32 L_macNs (Word32 L_var3, Word16 var1, Word16 var2); /* Mac without
                                                             sat, 1   */
//------------------------------------------------------------------

Word32 L_msuNs (Word32 L_var3, Word16 var1, Word16 var2); /* Msu without
                                                             sat, 1   */
//------------------------------------------------------------------

#if defined(OPT_BASIC_OP)
	//Word32 L_add (Word32 L_var1, Word32 L_var2);    /* Long add,        2 */
#	define L_add(x,y)	((x)+(y))
	__inline int QL_add(int x, int y)
	{
		int result;

		result = x + y;

		if( !((x^y)>>31) && ((result^x)>>31) )
		{
			result=(x<0)?MIN_32:MAX_32;
		}
		return result;
	}
			
#else
	Word32 L_add (Word32 L_var1, Word32 L_var2);    /* Long add,        2 */
#	define QL_add L_add
#endif

//------------------------------------------------------------------
#if defined(OPT_BASIC_OP)
#	define	L_sub(x,y)	((x)-(y))
#		if 0
#	define L_sub	_QL_sub
	__inline int _QL_sub(int x, int y)
	{
		int result;

		result = x - y;

		if( ((x^y)>>31) && ((result^x)>>31) )
		{
			result=(x<0)?MIN_32:MAX_32;
		}
		return result;
	}
#		endif

	__inline int QL_sub(int x, int y)
	{
		int result;

		result = x - y;

		if( ((x^y)>>31) && ((result^x)>>31) )
		{
			result=(x<0)?MIN_32:MAX_32;
		}
		return result;
	}

#else
	Word32 L_sub (Word32 L_var1, Word32 L_var2);    /* Long sub,        2 */
#	define	QL_sub	L_sub
#endif

//------------------------------------------------------------------

Word32 L_add_c (Word32 L_var1, Word32 L_var2);  /* Long add with c, 2 */

//------------------------------------------------------------------

Word32 L_sub_c (Word32 L_var1, Word32 L_var2);  /* Long sub with c, 2 */

//------------------------------------------------------------------

Word32 L_negate (Word32 L_var1);                /* Long negate,     2 */

//------------------------------------------------------------------

Word16 mult_r (Word16 var1, Word16 var2);       /* Mult with round, 2 */

//------------------------------------------------------------------

#if defined(OPT_BASIC_OP)
#			if 1
#	define L_shl(x,shift)	((x)<<(shift))
#			else
#	define L_shl	_QL_shl
	__inline int _QL_shl(int x, int shift)
	{
		if( shift < 0)
			return x >> -shift;
		else
		{
			for (; shift > 0; shift--)
			{
				if (x > (Word32) 0X3fffffffL)
				{
					//Overflow = 1;
	                x = MAX_32;
		            break;
			    }
				else
	            {
		            if (x < (Word32) 0xc0000000L)
			        {
	                    //Overflow = 1;
		                x = MIN_32;
			            break;
				    }
	            }
		        x <<= 1;
	        }
		}
		return x;
	}
#			endif

__inline int QL_shl(int x, int shift)
	{
		if( shift < 0)
			return x >> -shift;
		else
		{
			for (; shift > 0; shift--)
			{
				if (x > (Word32) 0X3fffffffL)
				{
					//Overflow = 1;
	                x = MAX_32;
		            break;
			    }
				else
	            {
		            if (x < (Word32) 0xc0000000L)
			        {
	                    //Overflow = 1;
		                x = MIN_32;
			            break;
				    }
	            }
		        x <<= 1;
	        }
		}
		return x;
	}


/* PL_shl constrain: x >= 0, no saturation */
#	define PL_shl(x, shift)	((shift) < 0)?((x) >> -(shift)):((x) << (shift))
	__inline int QPL_shl2(int x, int shift) /*constrain: x >= 0, shift == 2*/
	{
		if( x > 0x1FFFFFFF )
			return MAX_32;
		else
			return (x << 2);
	}
	__inline int QL_shl1(int x, int shift) /*constrain: shift == 1*/
	{
		if( x > (int)0x3FFFFFFF )
			return MAX_32;
		if( x < (int)0xC0000000 )
			return MIN_32;
		return (x << 1);
	}
	__inline int QL_shl3(int x, int shift) /*constrain: shift == 3*/
	{
		if( x > (int)0x0FFFFFFF )
			return MAX_32;
		if( x < (int)0xF0000000 )
			return MIN_32;
		return (x << 3);
	}
	__inline int QL_shl4(int x, int shift) /*constrain: shift == 4*/
	{
		if( x > (int)0x07FFFFFF )
			return MAX_32;
		if( x < (int)0xF8000000 )
			return MIN_32;
		return (x << 4);
	}

	__inline int QL_shl6(int x, int shift) /*constrain: shift == 4*/
	{
		if( x > (int)0x01FFFFFF )
			return MAX_32;
		if( x < (int)0xFE000000 )
			return MIN_32;
		return (x << 6);
	}
	__inline int QL_shl7(int x, int shift) /*constrain: shift == 4*/
	{
		if( x > (int)0x00FFFFFF )
			return MAX_32;
		if( x < (int)0xFF000000 )
			return MIN_32;
		return (x << 7);
	}

#else
	Word32 L_shl (Word32 L_var1, Word16 var2);      /* Long shift left, 2 */
#	define PL_shl	L_shl
#	define QL_shl	L_shl
#	define QL_shl1	L_shl
#	define QPL_shl2	L_shl
#	define QL_shl3	L_shl
#	define QL_shl4	L_shl
#	define QL_shl6	L_shl
#	define QL_shl7	L_shl

#endif

//------------------------------------------------------------------
#if defined(OPT_BASIC_OP)
#			if 1
#	define	L_shr(x,shift)	((x)>>(shift))
#			else
#	define	L_shr	_L_shr
	__inline int _L_shr(int x, int shift)
	{
		if(shift<0)
		{
			shift = -shift;
			for (; shift > 0; shift--)
			{
				if (x > (Word32) 0X3fffffffL)
				{
					Overflow = 1;
	                x = MAX_32;
		            break;
			    }
				else
	            {
		            if (x < (Word32) 0xc0000000L)
			        {
	                    Overflow = 1;
		                x = MIN_32;
			            break;
				    }
	            }
		        x <<= 1;
	        }
		}
		else
		{
			if( shift > 31 )
			{
				if(x < 0) x = -1;
				else x = 0;
			}
			else
				x = (x >> shift);
		}

//		printf("x:%d\n",x);
		return x;
	
	}
#			endif

/* L_shrs constrain: no saturation if shift < 0 */
#	define	L_shrs(x,shift)	((shift)<0)?((x)<<-(shift)):((x)>>(shift))
//#define L_shrs _L_shr
	__inline int L_shrsq(int x, int shift)
	{
		if(shift<0)
		{
			shift = -shift;
			for (; shift > 0; shift--)
			{
				if (x > (Word32) 0X3fffffffL)
				{
					Overflow = 1;
	                x = MAX_32;
		            break;
			    }
				else
	            {
		            if (x < (Word32) 0xc0000000L)
			        {
	                    Overflow = 1;
		                x = MIN_32;
			            break;
				    }
	            }
		        x <<= 1;
	        }
		}
		else
		{
			if( shift > 31 )
			{
				if(x < 0) x = -1;
				else x = 0;
			}
			else
				x = (x >> shift);
		}

		return x;
	
	}

#else
Word32 L_shr (Word32 L_var1, Word16 var2);      /* Long shift right, 2*/
#	define	L_shrs	L_shr
#	define	L_shrs	L_shr
#	define	L_shrsq L_shr

#endif

//------------------------------------------------------------------

#if defined(OPT_BASIC_OP)
#		if 1
/* shr_r constrain: shift >= 0 */
#	define shr_r(x,shift)	(( (x) + (1<< ((shift)-1) )) >> (shift))
#		else
#	define shr_r _shr_r
	__inline int _shr_r(int x, int shift)
	{
		int result;
		if(shift < 0)
		{
			result = x << -shift;
			if( result > 32767 )
				result = 32767;
			if( result < -32768 )
				result = -32768;
		}
		else
		{
			result = (( (x) + (1<< ((shift)-1) )) >> (shift));
		}

		return result;
	}
#		endif
	__inline int Qshr_r(int x, int shift)
	{
		int result;
		if(shift < 0)
		{
			result = x << -shift;
			if( result > 32767 )
				result = 32767;
			if( result < -32768 )
				result = -32768;
		}
		else
		{
			result = (( (x) + (1<< ((shift)-1) )) >> (shift));
		}

		return result;
	}

#else
Word16 shr_r (Word16 var1, Word16 var2);        /* Shift right with
                                                   round, 2           */
#	define Qshr_r	shr_r
#endif
//------------------------------------------------------------------

Word16 mac_r (Word32 L_var3, Word16 var1, Word16 var2); /* Mac with
                                                           rounding,2 */
//------------------------------------------------------------------

Word16 msu_r (Word32 L_var3, Word16 var1, Word16 var2); /* Msu with
                                                           rounding,2 */
//------------------------------------------------------------------

#if defined(OPT_BASIC_OP)
#	define	L_deposit_h(x)	((int)(x)<<16)
#else
Word32 L_deposit_h (Word16 var1);        /* 16 bit var1 -> MSB,     2 */
#endif

//------------------------------------------------------------------

#if defined(OPT_BASIC_OP)
#	define	L_deposit_l(x)	((int)(x))
#else
Word32 L_deposit_l (Word16 var1);        /* 16 bit var1 -> LSB,     2 */
#endif

//------------------------------------------------------------------

#if defined(OPT_BASIC_OP)
/* L_shr_r constrain: shift >= 0 */
//#	define L_shr_r(x,shift)	(( (int)(x) + (1<< ((shift)-1) )) >> (shift))
#		if 1
#	define L_shr_r	_L_shr_r
	__inline int _L_shr_r(int x, int shift)	/*constrain: shift >= 0 */
	{
		x = (x >> (shift-1));
		x++;
		return (x >> 1);
	}
#		else
#	define L_shr_r	_L_shr_r
	__inline int _L_shr_r(int x, int shift)	
	{
		if(shift<0)
		{
			shift = -shift;
			for (; shift > 0; shift--)
			{
				if (x > (Word32) 0X3fffffffL)
				{
					Overflow = 1;
	                x = MAX_32;
		            break;
			    }
				else
	            {
		            if (x < (Word32) 0xc0000000L)
			        {
	                    Overflow = 1;
		                x = MIN_32;
			            break;
				    }
	            }
		        x <<= 1;
	        }
		}
		else
		{
			if( shift > 31 )
			{
				if(x < 0) x = -1;
				else x = 0;
			}
			else
			{
				if( shift > 0 )
				{
					x = (x >> (shift-1));
					x++;
					x = x >> 1;
				}
			}
		}

		return x;
	
	}
#		endif


	__inline int L_shr_rl(int x, int shift)	/*constrain: shift >= 0 */
	{
		if( shift > 31 )
			return 0;
		x = (x >> (shift-1));
		x++;
		return (x >> 1);
	}

	__inline int QL_shr_r(int x, int shift)	
	{
		if(shift<0)
		{
			shift = -shift;
			for (; shift > 0; shift--)
			{
				if (x > (Word32) 0X3fffffffL)
				{
					Overflow = 1;
	                x = MAX_32;
		            break;
			    }
				else
	            {
		            if (x < (Word32) 0xc0000000L)
			        {
	                    Overflow = 1;
		                x = MIN_32;
			            break;
				    }
	            }
		        x <<= 1;
	        }
		}
		else
		{
			if( shift > 31 )
			{
				if(x < 0) x = -1;
				else x = 0;
			}
			else
			{
				if( shift > 0 )
				{
					x = (x >> (shift-1));
					x++;
					x = x >> 1;
				}
			}
		}

		return x;
	
	}


#else
	Word32 L_shr_r (Word32 L_var1, Word16 var2); /* Long shift right with
		                                            round,  3             */
#	define L_shr_rl	L_shr_r
#	define QL_shr_r	L_shr_r


#endif
//------------------------------------------------------------------

#if defined(OPT_BASIC_OP)
#	define L_abs(x)	(((x)<0)?-(x):(x))	/* constrain: no saturation */
#			if 0
#	define L_abs	QL_abs
	__inline QL_abs(int x)
	{
		if( x < 0 )
		{
			if( x == (int)0x80000000 )
				return MAX_32;
			return -x;
		}
		else
			return x;
	}
#			endif
#else
Word32 L_abs (Word32 L_var1);            /* Long abs,              3  */
#endif

//------------------------------------------------------------------

Word32 L_sat (Word32 L_var1);            /* Long saturation,       4  */

//------------------------------------------------------------------

#if defined(OPT_BASIC_OP)
#	define norm_s	_norm_s
	__inline int _norm_s(int x)	/* constrain: x >= 0 */
	{
		if( x >= 0x800 )
		{
			if( x >= 0x4000 )			return 0;
			if( x >= 0x2000 )			return 1;
			if( x >= 0x1000 )			return 2;
			if( x >= 0x800 )			return 3;
		}
		else if( x >= 0x80 )
		{
			if( x >= 0x400 )			return 4;
			if( x >= 0x200 )			return 5;
			if( x >= 0x100 )			return 6;
			if( x >= 0x80 )				return 7;
		}
		else if( x >= 0x8 )
		{
			if( x >= 0x40 )			return 8;
			if( x >= 0x20 )			return 9;
			if( x >= 0x10 )			return 10;
			if( x >= 0x8 )			return 11;
		}
		else
		{
			if( x >= 0x4 )			return 12;
			if( x >= 0x2 )			return 13;
			if( x >= 0x1 )			return 14;
			if( x == 0 )			return 0;
		}
	}
#else
Word16 norm_s (Word16 var1);             /* Short norm,           15  */
#endif


//------------------------------------------------------------------

#if defined(OPT_BASIC_OP)
#	define	div_s(x,y)	(((int)(x)<<15)/(y)) /* constrain: no saturation, x>0, y>0, y > x */
#else
Word16 div_s (Word16 var1, Word16 var2); /* Short division,       18  */
#endif

//------------------------------------------------------------------

Word16 norm_l (Word32 L_var1);           /* Long norm,            30  */   


//------------------------------------------------------------------
#define	Copy(in, out, size)	memcpy((char *)(out), (char *)(in), (size)*2)
#define Mpy_32_16(hi, lo, n) (((hi)*(n)<<1) + (((lo)*(n)>>15)<<1))

#endif // FPM_ARM


#endif /* _basic_op_h_ */