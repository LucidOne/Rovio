#ifndef _ASMARM_UACCESS_H
#define _ASMARM_UACCESS_H

/*
 * User space memory access functions
 */

#include "asm/errno.h"

#define VERIFY_READ 0
#define VERIFY_WRITE 1

/*
 * The exception table consists of pairs of addresses: the first is the
 * address of an instruction that is allowed to fault, and the second is
 * the address at which the program should continue.  No registers are
 * modified, so it is entirely up to the continuation code to figure out
 * what to do.
 *
 * All the routines below use bits of fixup code that are out of line
 * with the main instruction path.  This means when everything is well,
 * we don't even have to jump over them.  Further, they do not intrude
 * on our cache or tlb entries.
 */

struct exception_table_entry
{
	unsigned long insn, fixup;
};

/* Returns 0 if exception not found and fixup otherwise.  */
extern unsigned long search_exception_table(unsigned long);

static inline void __put_user_asm_byte(long x,int *addr,long *err);
static inline void __put_user_asm_half(long x,int *addr,long *err);
static inline void __put_user_asm_word(long x,int *addr,long *err);
static inline void __get_user_asm_byte(long *x,int *addr,long *err);
static inline void __get_user_asm_half(long *x,int *addr,long *err);
static inline void __get_user_asm_word(long *x,int *addr,long* err);

#define get_ds()	(KERNEL_DS)
#define get_fs()	(USER_DS) /* copied m68knommu --gmcnutt */
#define segment_eq(a,b)	((a) == (b))


//extern long __get_user_bad(void);

static inline long __get_user_bad()
{
	__asm {
		mov	r1, #0
		mov	r0, #-14
		//mov	pc, lr
	}
}

static inline void __get_user_size(long *x,int *ptr,int size,long *retval)
{
	do {
		switch (size) {
		case 1:	__get_user_asm_byte(x,ptr,retval);	break;
		case 2:	__get_user_asm_half(x,ptr,retval);	break;
		case 4:	__get_user_asm_word(x,ptr,retval);	break;
		/*
		case 8: {							\
			typeof(*(ptr)) __gu_val;				\
			memcpy(&__gu_val, ptr, sizeof(__gu_val));		\
			(x) = __gu_val;						\
		} break;
		*/
		default: *(x) = __get_user_bad(); 
		}
	} while (0);
}

//extern long __put_user_bad(void);

static inline long __put_user_bad()
{
	__asm {
		mov	r0, #-14
		//mov	pc, lr
	}
}

static inline void __put_user_size(long x,int *ptr,int size,long *retval)
{
	do {
		switch (size) {
		case 1: __put_user_asm_byte(x,ptr,retval);	break;
		case 2: __put_user_asm_half(x,ptr,retval);	break;
		case 4: __put_user_asm_word(x,ptr,retval);	break;
		/*
		case 8: {							\
			typeof(*(ptr)) __gu_val = (x);				\
			memcpy(ptr, &__gu_val, sizeof(__gu_val));		\
			} break;
		*/						\
		default: __put_user_bad();
		}
	} while (0);
}

/*
 * REVISIT_gjm -- access_ok
 *         Lots of drivers use this. __range_ok is going to try and used the
 *         current->addr_limit setting as a check. I don't know that we ever
 *         set that up the way it expects it.
 */

#define __range_ok(addr,size) 0
#define __addr_ok(addr) 1

#define access_ok(type,addr,size)	(__range_ok(addr,size) == 0)

static __inline int verify_area(int type, const void * addr, unsigned long size)
{
	return access_ok(type, addr, size) ? 0 : EFAULT;
}

static inline void __put_user_asm_byte(long x,int *addr,long *err)
{	
	__asm{
		strbt	x,[addr],#0
		//".section .fixup,\"ax\"\n"
		//align	2				
		//3:	mov	err, -EFAULT				
		//b	%B2					
		//"	.previous"
	}
}
#ifdef __ARMEB__
static inline void __put_user_asm_half(int x,int *addr, long *err)
{	
	unsigned long __temp = (unsigned long)(x);
	__put_user_asm_byte(__temp, (int)(addr) + 1, err);
	__put_user_asm_byte(__temp >> 8, addr, err);
}
#else
static inline void __put_user_asm_half(long x,int *addr,long *err)
{
	unsigned long __temp = (unsigned long)(x);
	__put_user_asm_byte(__temp, addr, err);
	__put_user_asm_byte(__temp >> 8, (int *)((int)(addr) + 1), err);
}
#endif

static inline void __put_user_asm_word(long x,int *addr,long *err)
{
	__asm{
		strt	x,[addr],#0
		//"2:\n"			
		//"	.section .fixup,\"ax\"\n"
		//"	.align	2\n"
		//"3:	mov	err, -EFAULT\n"
		//"	b	%B2\n"	
		//"	.previous"	
	}
}
static inline void __get_user_asm_byte(long *x,int *addr,long *err)
{
	int i,ret;
	__asm {
		ldrbt	i,[addr],#0
	//
	//AREA fixup, CODE, READONLY
	//	align	2
	//3	mov	ret, -EFAULT
	//	mov	i, #0
	//	b	%B2
	//	AREA |.text|, CODE, READONLYs
	}
	*x = i;
	
}
#ifdef __ARMEB__
static inline void __get_user_asm_half(long *x,int *addr,int *err)
{
	unsigned long __b1, __b2;
	__get_user_asm_byte((long*)&__b1, addr, err);
	__get_user_asm_byte((long*)&__b2, (int)(addr) + 1, err);
	*(x) = (__b1 << 8) | __b2;
}
#else
static inline void  __get_user_asm_half(long *x, int *addr, long *err)
{
	unsigned long __b1, __b2;
	__get_user_asm_byte((long*)&__b1, addr, err);
	__get_user_asm_byte((long*)&__b2, (int*)((int)(addr) + 1), err);
	*(x) = __b1 | (__b2 << 8);
}
#endif


static inline void __get_user_asm_word(long *x,int *addr,long* err)	
{
	int i,ret;
	__asm {
			ldrt	i,[addr],#0
		//2	AREA fixup, CODE, READONLY
		//	align	2
		//3	mov	ret, -EFAULT
		//	mov	i, #0
		//	b	%B2	
		//	AREA |.text|, CODE, READONLY
	}
	*x = i;
	//*err = ret;
}

/*
 * Single-value transfer routines.  They automatically use the right
 * size if we just have the right pointer type.  Note that the functions
 * which read from user space (*get_*) need to take care not to leak
 * kernel data even if the calling code is buggy and fails to check
 * the return value.  This means zeroing out the destination variable
 * or buffer on error.  Normally this is done out of line by the
 * fixup code, but there are a few places where it intrudes on the
 * main code path.  When we only write to user space, there is no
 * problem.
 *
 * The "__xxx" versions of the user access functions do not verify the
 * address space - it must have been done previously with a separate
 * "access_ok()" call.
 *
 * The "xxx_error" versions set the third argument to EFAULT if an
 * error occurs, and leave it unchanged on success.  Note that these
 * versions are void (ie, don't return a value as such).
 */
#define get_user(x,p)		__get_user_check((int *)(x),(int *)(p),sizeof(*(p)))
#define __get_user(x,p)		__get_user_nocheck((int *)(x),(int *)(p),sizeof(*(p)))
#define __get_user_error(x,p,e)	__get_user_nocheck_error((int *)(x),(int *)(p),sizeof(*(p)),(long *)(e))

#define put_user(x,p)		__put_user_check((int)(x),(int *)(p),sizeof(*(p)))
#define __put_user(x,p)		__put_user_nocheck((int)(x),(int *)(p),sizeof(*(p)))
#define __put_user_error(x,p,e)	__put_user_nocheck_error((int)(x),(int *)(p),sizeof(*(p)),(e))


//YC:  add here for current test!  CLYu will use asm to modify __arch_copy_to_user()
/*Prototype: int __arch_copy_to_user(void *to, const char *from, size_t n)
 * Purpose  : copy a block to user memory from kernel memory
 * Params   : to   - user memory
 *          : from - kernel memory
 *          : n    - number of bytes to copy
 * Returns  : Number of bytes NOT copied.*/

static inline long __arch_copy_to_user(void *to, const char *from, size_t n)
{
	memcpy(to, from, n);
	return 0;
}

static inline long __arch_copy_from_user(void *to, const char *from, size_t n)
{
	memcpy(to, from, n);
	return 0;
}

//clyu comment for __arch_copy_from_user and __arch_copy_to_user
//extern unsigned long __arch_copy_from_user(void *to, const void *from, unsigned long n);
#define __do_copy_from_user(to,from,n)				\
	(n) = __arch_copy_from_user(to,from,n)

//extern unsigned long __arch_copy_to_user(void *to, const void *from, unsigned long n);
#define __do_copy_to_user(to,from,n)				\
	(n) = __arch_copy_to_user(to,from,n)

static __inline unsigned long copy_from_user(void *to, const void *from, unsigned long n)
{
	if (access_ok(VERIFY_READ, from, n))
		__do_copy_from_user(to, from, n);
	else /* security hole */
	  memzero(to, n);
	return n;
}

static __inline unsigned long __copy_from_user(void *to, const void *from, unsigned long n)
{
	__do_copy_from_user(to, from, n);
	return n;
}

static __inline unsigned long copy_to_user(void *to, const void *from, unsigned long n)
{
	if (access_ok(VERIFY_WRITE, to, n))
		__do_copy_to_user(to, from, n);
	return n;
}

static __inline unsigned long __copy_to_user(void *to, const void *from, unsigned long n)
{
	__do_copy_to_user(to, from, n);
	return n;
}
/*
 * These are the work horses of the get/put_user functions
 */
static inline int __get_user_check(int *x,int *ptr, int size)
{
	long __gu_err = -EFAULT;
	long __gu_val = 0;				
	//const __typeof__(*(ptr)) *__gu_addr = (ptr);	
			
	if (access_ok(VERIFY_READ,/*__gu_addr*/ptr,size)) {		
		__gu_err = 0;
		__get_user_size(&__gu_val,ptr,(size),&__gu_err);
	}								
	*(x) = (int)__gu_val;
	return __gu_err;						
}

static inline int __get_user_nocheck(int *x,int *ptr,int size)
{
	long __gu_err = 0;
	long __gu_val;
	__get_user_size(&__gu_val,(ptr),(size),&__gu_err);
	*(x) = /*(__typeof__(*(ptr)))*/__gu_val;
	return __gu_err;
}

static inline long __get_user_nocheck_error(int *x,int *ptr,int size, long *err)
{
	long __gu_val;
	__get_user_size(&__gu_val,(ptr),(size),(err));
	*(x) = /*(__typeof__(*(ptr)))*/__gu_val;
	return 0;
}

static inline long __put_user_check(int x,int *ptr,int size)
{									
	long __pu_err = -EFAULT;		
	//int *__pu_addr = (ptr);			
	if (access_ok(VERIFY_WRITE,/*__pu_addr*/ptr,size)) {
		__pu_err = 0;						
		__put_user_size((x),/*__pu_addr*/ptr,(size),&__pu_err);
	}								
	return __pu_err;				
}

static inline long __put_user_nocheck(int x,int *ptr,int size)
{
	long __pu_err = 0;
	/*__typeof__(*(ptr)) *__pu_addr = (ptr);*/
	__put_user_size((x),ptr,(size),&__pu_err);
	return __pu_err;
}

static inline long __put_user_nocheck_error(int x,int *ptr,int size,long *err)
{
	__put_user_size((x),(ptr),(size),err);
	return 0;
}

#endif /* _ASMARM_UACCESS_H */
