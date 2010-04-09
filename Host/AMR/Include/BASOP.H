#ifdef ECOS
#include "stdlib.h"
#else
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


#ifdef _ARM


__inline Word16 extract_h(Word32 L_var1)          /* Extract high, 1 */
{
    Word16 var_out;
	__asm
	{
		MOV		var_out, L_var1, asr #16
	}
    return(var_out);
}

__inline Word16 extract_l(Word32 L_var1)          /* Extract low, 1 */
{
    Word16 var_out;

    var_out = (Word16) L_var1;
    return(var_out);
}


__inline Word16 sature(Word32 L_var1)
{
    Word16 var_out;

    if (L_var1 > 0X00007fffL) {
        Overflow = 1;
        var_out = MAX_16;
    }
    else {
        if (L_var1 < (Word32)0xffff8000L) {
            Overflow = 1;
            var_out = MIN_16;
        }
        else {
            Overflow = 0;
            var_out = extract_l(L_var1);
        }
    }
    return(var_out);
}

__inline Word16 add(Word16 var1, Word16 var2) /* Short add, 1 */
{
    Word16 var_out;
    Word32 L_somme;

    L_somme = (Word32) var1 + (Word32) var2;
    var_out = sature(L_somme);
    return(var_out);
}

__inline Word16 sub(Word16 var1, Word16 var2)    /* Short sub, 1 */
{
    Word16 var_out;
    Word32 L_diff;

    L_diff = (Word32) var1 -  (Word32) var2;
    var_out = sature(L_diff);
    return(var_out);
}

__inline Word16 abs_s(Word16 var1)      /* Short abs, 1 */
{
    Word16 var_out;

    if (var1 == (Word16)0X8000 ) {
        var_out = MAX_16;
    }
    else {
        if (var1 < 0) {
            var_out = -var1;
        }
        else {
            var_out = var1;
        }
    }

    return(var_out);
}

__inline Word16 shr(Word16 var1, Word16 var2);    /* Short shift right,   1 */

__inline Word16 shl(Word16 var1, Word16 var2)   /* Short shift left,    1 */
{
    Word16 var_out;
    Word32 L_result;

    if (var2 < 0) {
        var_out = shr(var1, (Word16)-var2);
    }
    else {
        L_result = (Word32) var1 * ((Word32) 1 << var2);
        if ( ((var2 > 15) && (var1 != 0)) ||
             (L_result != (Word32)((Word16) L_result)) ) {
            Overflow = 1;
            var_out = (var1 > 0) ? MAX_16 : MIN_16;
        }
        else {
            var_out = extract_l(L_result);
        }
    }

    return(var_out);
}

__inline Word16 shr(Word16 var1, Word16 var2)     /* Short shift right,   1 */
{
    Word16 var_out;

    if (var2 < 0) {
        var_out = shl(var1, (Word16)-var2);
    }
    else {
        if (var2 >= 15) {
            var_out = (var1 < (Word16)0) ? (Word16)-1 : (Word16)0;
        }
        else {
            if (var1 < 0) {
                var_out = ~(( ~var1) >> var2 );
            }
            else {
                var_out = var1 >> var2;
            }
        }
    }

    return(var_out);
}

__inline Word16 mult(Word16 var1, Word16 var2)     /* Short mult, 1 */
{
    Word32 L_produit;
	__asm
	{
		SMULBB	L_produit, var1, var2
		QADD	L_produit, L_produit, L_produit
		MOV		var1, L_produit, asr #16
	}
    return(var1);
}


__inline Word32 L_mult(Word16 var1, Word16 var2)  /* Long mult, 1 */
{
    Word32 L_var_out;
	__asm
	{
		SMULBB	L_var_out, var1, var2
		QADD	L_var_out, L_var_out, L_var_out
	}
    return(L_var_out);
}


__inline Word16 negate(Word16 var1) /* Short negate,  1 */
{
    Word16 var_out;

    var_out = (var1 == MIN_16) ? MAX_16 : -var1;
    return(var_out);
}

__inline Word16 round(Word32 L_var1)      /* Round, 1 */
{
    Word16 var_out;
    Word32 L_arrondi;
	__asm
	{
		QADD	L_arrondi, L_var1, #0x00008000
		MOV		var_out, L_arrondi, asr #16
	}
    return(var_out);
}


__inline Word32 L_mac(Word32 L_var3, Word16 var1, Word16 var2) /* Mac,    1 */
{
    Word32 L_produit;
	__asm
	{
		SMULBB	L_produit, var1, var2
		QDADD	L_var3, L_var3, L_produit
	}
    return(L_var3);
}


__inline Word32 L_msu(Word32 L_var3, Word16 var1, Word16 var2) /* Msu,    1 */
{
    Word32 L_produit;
	__asm
	{
		SMULBB	L_produit, var1, var2
		QDSUB	L_var3, L_var3, L_produit
	}
    return(L_var3);
}


__inline Word32 L_macNs(Word32 L_var3, Word16 var1, Word16 var2)/*[Mac without sat, 1*/
{
	Word32 L_var_out;
	__asm
	{
		SMULBB	L_var_out, var1, var2
		QADD	L_var_out, L_var_out, L_var_out
		ADD		L_var3, L_var3, L_var_out
	}
    return(L_var3);
}

__inline Word32 L_msuNs(Word32 L_var3, Word16 var1, Word16 var2) /* Msu without sat, 1*/
{
	Word32 L_var_out;
	__asm
	{
		SMULBB	L_var_out, var1, var2
		QADD	L_var_out, L_var_out, L_var_out
		SUB		L_var3, L_var3, L_var_out
	}
    return(L_var3);
}


__inline Word32 L_add(Word32 L_var1, Word32 L_var2)   /* Long add, 2 */
{
	__asm
	{
		QADD	L_var1, L_var1, L_var2
	}
    return(L_var1);
}

__inline Word32 L_sub(Word32 L_var1, Word32 L_var2)   /* Long sub,        2 */
{
	__asm
	{
		QSUB	L_var1, L_var1, L_var2
	}
    return(L_var1);
}

__inline Word32 L_add_c(Word32 L_var1, Word32 L_var2) /*Long add with c,  2 */
{
	__asm
	{
		ADD	L_var1, L_var1, L_var2
	}
    return(L_var1);
}


__inline Word32 L_sub_c(Word32 L_var1, Word32 L_var2) /*Long sub with c,  2 */
{
	__asm
	{
		SUB	L_var1, L_var1, L_var2
	}
    return(L_var1);
}

__inline Word32 L_negate(Word32 L_var1) /* Long negate,     2 */
{
    Word32 L_var_out;

    L_var_out = (L_var1 == MIN_32) ? MAX_32 : -L_var1;
    return(L_var_out);
}

__inline Word16 mult_r(Word16 var1, Word16 var2)  /* Mult with round,     2 */
{
    Word32 L_produit_arr;
	__asm
	{
		SMULBB	L_produit_arr, var1, var2
		QADD	L_produit_arr, L_produit_arr, #0x00004000
		QADD	L_produit_arr, L_produit_arr, L_produit_arr
		MOV		var1, L_produit_arr, asr #16
	}
    return(var1);
}

__inline Word32 L_shr(Word32 L_var1, Word16 var2); /* Long shift right,    2 */

__inline Word32 L_shl(Word32 L_var1, Word16 var2) /* Long shift left,     2 */
{
    Word32 L_var_out = 0L;

    if (var2 <= (Word16)0) {
        L_var_out = L_shr(L_var1, (Word16)-var2);
    }
    else {
        for(; var2 > (Word16)0; var2--) {
            if (L_var1 > (Word32) 0X3fffffffL) {
                Overflow = 1;
                L_var_out = MAX_32;
                break;
            }
            else {
                if (L_var1 < (Word32) 0xc0000000L) {
                    Overflow = 1;
                    L_var_out = MIN_32;
                    break;
                }
            }
            L_var1 *= 2L;
            L_var_out = L_var1;
        }
    }
    return(L_var_out);
}

__inline Word32 L_shr(Word32 L_var1, Word16 var2) /* Long shift right,    2 */
{
    Word32 L_var_out;

    if (var2 < (Word16)0) {
        L_var_out = L_shl(L_var1, (Word16)-var2);
    }
    else {
        if (var2 >= (Word16)31) {
            L_var_out = (L_var1 < 0L) ? -1L : 0L;
        }
        else {
            if (L_var1< 0L)  {
                L_var_out = ~((~L_var1) >> var2);
            }
            else {
                L_var_out = L_var1 >> var2;
            }
        }
    }
    return(L_var_out);
}

__inline Word16 shr_r(Word16 var1, Word16 var2) /* Shift right with round, 2 */
{
    Word16 var_out;

    if (var2 > (Word16)15) {
        var_out = (Word16)0;
    }
    else {
        var_out = shr(var1,var2);

        if (var2 > (Word16)0) {
            if ((var1 & ((Word16)1 << (var2-(Word16)1))) != (Word16)0) {
                var_out++;
            }
        }
    }
    return(var_out);
}

__inline Word16 mac_r(Word32 L_var3, Word16 var1, Word16 var2)/* Mac with rounding, 2*/
{
    Word16 var_out;
    Word32 tmp;
	__asm
	{
		SMULBB	tmp, var1, var2
		QDADD	L_var3, L_var3, tmp
		QADD	L_var3, L_var3, #0x00008000
		MOV		var_out, L_var3, asr #16
	}
    return(var_out);
}


__inline Word16 msu_r(Word32 L_var3, Word16 var1, Word16 var2) /* Msu with rounding, 2*/
{
    Word16 var_out;
	Word32 tmp;
	__asm
	{
		SMULBB	tmp, var1, var2
		QDSUB	L_var3, L_var3, tmp
		QADD	L_var3, L_var3, #0x00008000
		MOV		var_out, L_var3, asr #16
	}
    return(var_out);
}

__inline Word32 L_deposit_h(Word16 var1)      /* 16 bit var1 -> MSB,     2 */
{
	__asm
	{
		MOV		var1, var1, lsl #16
	}
    return(var1);
}

__inline Word32 L_deposit_l(Word16 var1)     /* 16 bit var1 -> LSB,     2 */
{
	return ((Word32)var1);
}


__inline Word32 L_shr_r(Word32 L_var1, Word16 var2) /* Long shift right with round,  3*/
{
    Word32 L_var_out;

    if (var2 > (Word16)31) {
        L_var_out = 0L;
    }
    else {
        L_var_out = L_shr(L_var1,var2);
        if (var2 > (Word16)0) {
            if ( (L_var1 & ( 1L << (Word16)(var2-(Word16)1) )) != 0L)  {
                L_var_out++;
            }
        }
    }
    return(L_var_out);
}

__inline Word32 L_abs(Word32 L_var1) /* Long abs,   3 */
{
    Word32 L_var_out;

    if (L_var1 == MIN_32) {
        L_var_out = MAX_32;
    }
    else {
        if (L_var1 < 0L) {
            L_var_out = -L_var1;
        }
        else {
            L_var_out = L_var1;
        }
    }

    return(L_var_out);
}

__inline Word32 L_sat(Word32 L_var1)   /* Long saturation,       4 */
{
    Word32 L_var_out;


    L_var_out = L_var1;

    if (Overflow) {

        if (Carry) {
            L_var_out = MIN_32;
        }
        else {
            L_var_out = MAX_32;
        }

        Carry = 0;
        Overflow = 0;
    }

    return(L_var_out);
}

__inline Word16 norm_s(Word16 var1) /* Short norm,  15 */
{
    Word16 var_out;

    if (var1 == (Word16)0) {
        var_out = (Word16)0;
    }
    else {
        if (var1 == (Word16) 0xffff) {
            var_out = (Word16)15;
        }
        else {
            if (var1 < (Word16)0) {
                var1 = ~var1;
            }

            for(var_out = (Word16)0; var1 < (Word16)0x4000; var_out++) {
                var1 <<= 1;
            }
        }
    }

    return(var_out);
}

__inline Word16 div_s(Word16 var1, Word16 var2)  /* Short division,       18 */
{
    Word16 var_out = (Word16)0;
    Word16 iteration;
    Word32 L_num;
    Word32 L_denom;

    if ((var1 > var2) || (var1 < (Word16)0) || (var2 < (Word16)0)) {
        printf("Division Error\n");
        exit(0);
    }

    if (var2 == (Word16)0) {
        printf("Division by 0, Fatal error \n");
        exit(0);
    }

    if (var1 == (Word16)0) {
        var_out = (Word16)0;
    }
    else {
        if (var1 == var2) {
            var_out = MAX_16;
        }
        else {
            L_num = L_deposit_l(var1);
            L_denom = L_deposit_l(var2);

            for(iteration=(Word16)0; iteration< (Word16)15;iteration++) {
                var_out <<=1;
                L_num <<= 1;

                if (L_num >= L_denom) {
                    L_num = L_sub(L_num,L_denom);
#ifdef _ARM
                    var_out = var_out + 1;
#else
                    var_out = add(var_out, (Word16)1);
#endif
                }
            }
        }
    }

    return(var_out);
}

__inline Word16 norm_l(Word32 L_var1)  /* Long norm,  30 */
{
    Word16 var_out;

    if (L_var1 == 0L) {
        var_out = (Word16)0;
    }
    else {
        if (L_var1 == (Word32)0xffffffffL) {
            var_out = (Word16)31;
        }
        else {
            if (L_var1 < 0L) {
                L_var1 = ~L_var1;
            }

            for(var_out = (Word16)0;L_var1 < (Word32)0x40000000L;var_out++) {
                L_var1 <<= 1L;
            }
        }
    }

    return(var_out);
}


/*
   Additional operators
*/
__inline Word32 L_mls( Word32 Lv, Word16 v )
{
   Word32   Temp, chp0;
   Word16	chp;

   Temp = Lv & (Word32) 0x0000ffff ;
   Temp = Temp * (Word32) v ;
	__asm
	{
		MOV		Temp, Temp, asr #15
		MOV		chp, Lv, asr #16

		SMULBB	chp0, v, chp
		QDADD	Temp, Temp, chp0
	}

   return Temp ;
}

__inline Word16 div_l( Word32  L_num, Word16 den )
{
    Word16   var_out = (Word16)0;
    Word32   L_den;
    Word16   iteration;

    if ( den == (Word16) 0 ) {
        printf("Division by 0 in div_l, Fatal error \n");
        exit(0);
    }

    if ( (L_num < (Word32) 0) || (den < (Word16) 0) ) {
        printf("Division Error in div_l, Fatal error \n");
        exit(0);
    }

    L_den = L_deposit_h( den ) ;

    if ( L_num >= L_den ){
        return MAX_16 ;
    }
    else {
#ifdef _ARM
		__asm
		{
			MOV		L_num, L_num, asr #1
			MOV		L_den, L_den, asr #1
		}
#else
        L_num = L_shr(L_num, (Word16)1) ;
        L_den = L_shr(L_den, (Word16)1);
#endif
        for(iteration=(Word16)0; iteration< (Word16)15;iteration++)
        {
#ifdef _ARM
			__asm
			{
				MOV		var_out, var_out, lsl 1
				MOV		L_num, L_num, lsl #1
			}
#else
            var_out = shl( var_out, (Word16)1);
            L_num   = L_shl( L_num, (Word16)1);
#endif
            if (L_num >= L_den) {
                L_num = L_sub(L_num,L_den);
#ifdef _ARM
                var_out = var_out + 1;
#else
                var_out = add(var_out, (Word16)1);
#endif
            }
        }

        return var_out;
    }
}

__inline Word16 i_mult(Word16 a, Word16 b)
{
   return  a*b;
}

#else
Word16 add(Word16 var1, Word16 var2);     /* Short add,           1 */
Word16 sub(Word16 var1, Word16 var2);     /* Short sub,           1 */
Word16 abs_s(Word16 var1);                /* Short abs,           1 */
Word16 shl(Word16 var1, Word16 var2);     /* Short shift left,    1 */
Word16 shr(Word16 var1, Word16 var2);     /* Short shift right,   1 */
Word16 mult(Word16 var1, Word16 var2);    /* Short mult,          1 */
Word32 L_mult(Word16 var1, Word16 var2);  /* Long mult,           1 */
Word16 negate(Word16 var1);               /* Short negate,        1 */
Word16 extract_h(Word32 L_var1);          /* Extract high,        1 */
Word16 extract_l(Word32 L_var1);          /* Extract low,         1 */
Word16 round(Word32 L_var1);              /* Round,               1 */
Word32 L_mac(Word32 L_var3, Word16 var1, Word16 var2); /* Mac,    1 */
Word32 L_msu(Word32 L_var3, Word16 var1, Word16 var2); /* Msu,    1 */
Word32 L_macNs(Word32 L_var3, Word16 var1, Word16 var2);/*[Mac without sat, 1*/
Word32 L_msuNs(Word32 L_var3, Word16 var1, Word16 var2);/* Msu without sat, 1*/

Word32 L_add(Word32 L_var1, Word32 L_var2);   /* Long add,        2 */
Word32 L_sub(Word32 L_var1, Word32 L_var2);   /* Long sub,        2 */
Word32 L_add_c(Word32 L_var1, Word32 L_var2); /*Long add with c,  2 */
Word32 L_sub_c(Word32 L_var1, Word32 L_var2); /*Long sub with c,  2 */
Word32 L_negate(Word32 L_var1);               /* Long negate,     2 */
Word16 mult_r(Word16 var1, Word16 var2);  /* Mult with round,     2 */
Word32 L_shl(Word32 L_var1, Word16 var2); /* Long shift left,     2 */
Word32 L_shr(Word32 L_var1, Word16 var2); /* Long shift right,    2 */
Word16 shr_r(Word16 var1, Word16 var2);/* Shift right with round, 2 */
Word16 mac_r(Word32 L_var3, Word16 var1, Word16 var2);/* Mac with rounding, 2*/
Word16 msu_r(Word32 L_var3, Word16 var1, Word16 var2);/* Msu with rounding, 2*/
Word32 L_deposit_h(Word16 var1);       /* 16 bit var1 -> MSB,     2 */
Word32 L_deposit_l(Word16 var1);       /* 16 bit var1 -> LSB,     2 */

Word32 L_shr_r(Word32 L_var1, Word16 var2);/* Long shift right with round,  3*/
Word32 L_abs(Word32 L_var1);            /* Long abs,              3 */

Word32 L_sat(Word32 L_var1);            /* Long saturation,       4 */

Word16 norm_s(Word16 var1);             /* Short norm,           15 */

Word16 div_s(Word16 var1, Word16 var2); /* Short division,       18 */

Word16 norm_l(Word32 L_var1);           /* Long norm,            30 */


/*
   Additional operators
*/
Word32 L_mls( Word32, Word16 ) ;        /* Wght ?? */
Word16 div_l( Word32, Word16 ) ;
Word16 i_mult(Word16 a, Word16 b);

#endif
