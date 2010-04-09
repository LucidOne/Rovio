/*
 *  linux/include/asm-arm/checksum.h
 *
 * IP checksum routines
 *
 * Copyright (C) Original authors of ../asm-i386/checksum.h
 * Copyright (C) 1996-1999 Russell King
 */
#ifndef __ASM_ARM_CHECKSUM_H
#define __ASM_ARM_CHECKSUM_H

/*
 * computes the checksum of a memory block at buff, length len,
 * and adds in "sum" (32-bit)
 *
 * returns a 32-bit number suitable for feeding into itself
 * or csum_tcpudp_magic
 *
 * this function must be called with even lengths, except
 * for the last fragment, which may be odd
 *
 * it's best to have buff aligned on a 32-bit boundary
 */
unsigned int csum_partial(const unsigned char * buff, int len, unsigned int sum);

/*
 * the same as csum_partial, but copies from src while it
 * checksums, and handles user-space pointer exceptions correctly, when needed.
 *
 * here even more important to align src and dst on a 32-bit (or even
 * better 64-bit) boundary
 */

unsigned int
csum_partial_copy_nocheck(const char *src, char *dst, int len, int sum);

unsigned int
csum_partial_copy_from_user(const char *src, char *dst, int len, int sum, int *err_ptr);

/*
 * These are the old (and unsafe) way of doing checksums, a warning message will be
 * printed if they are used and an exception occurs.
 *
 * these functions should go away after some time.
 */
#define csum_partial_copy(src,dst,len,sum)	csum_partial_copy_nocheck(src,dst,len,sum)


/*
 * 	Fold a partial checksum without adding pseudo headers
 */

static inline unsigned int
csum_fold(unsigned int sum)
{
	__asm {
		adds	sum, sum, sum, lsl #16
		addcs	sum, sum, #0x10000
	}
	return (~sum) >> 16;
}

/*
 * this routine is used for miscellaneous IP-like checksums, mainly
 * in icmp.c
 */
static inline unsigned short
ip_compute_csum(unsigned char * buff, int len)
{
	return csum_fold(csum_partial(buff, len, 0));
}


#endif
