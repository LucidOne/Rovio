#ifndef _LINUX_LINKAGE_H
#define _LINUX_LINKAGE_H

//#include <linux/config.h>

#ifdef __cplusplus
#define CPP_ASMLINKAGE extern "C"
#else
#define CPP_ASMLINKAGE
#endif

#if defined __i386__
#define asmlinkage CPP_ASMLINKAGE __attribute__((regparm(0)))
#elif defined __ia64__
#define asmlinkage CPP_ASMLINKAGE __attribute__((syscall_linkage))
#else
#define asmlinkage CPP_ASMLINKAGE
#endif

#define SYMBOL_NAME_STR(X) #X
#define SYMBOL_NAME(X) X
#ifdef __STDC__
#define SYMBOL_NAME_LABEL(X) X##:
#else
#define SYMBOL_NAME_LABEL(X) X/**/:
#endif

#ifdef __arm__
#define __ALIGN .align 0
#define __ALIGN_STR ".align 0"
#endif /* __arm__ */

//clyu comment #define current_text_addr() ({ __label__ _l; _l: &&_l;})
static __inline void *current_text_addr() 
{
	int ret;
	__asm {
		mov ret, pc
	}
	return (void*)ret;
}

#ifdef __ASSEMBLY__

#define ALIGN __ALIGN
#define ALIGN_STR __ALIGN_STR

#define ENTRY(name) \
  .globl SYMBOL_NAME(name); \
  ALIGN; \
  SYMBOL_NAME_LABEL(name)

#endif

#endif
