/*
 * Copyright © Marvell International Ltd. and/or its affiliates, 2003-2006
 */

#ifndef _OS_HEADER1_
#define _OS_HEADER1_

typedef char CHAR;
typedef char *PCHAR;
typedef cyg_uint8 *PUCHAR;
typedef cyg_uint16 *PUSHORT;
typedef long LONG;
typedef LONG *PLONG;
typedef PLONG LONG_PTR;
typedef cyg_uint32 *ULONG_PTR;
typedef cyg_uint32 *Pu32;
typedef unsigned int UINT;
typedef UINT *PUINT;
typedef void VOID;
typedef VOID *PVOID;
typedef int WLAN_STATUS;
typedef cyg_uint8 BOOLEAN;
typedef BOOLEAN *PBOOLEAN;
typedef cyg_uint32 WLAN_OID;
typedef PVOID PDRIVER_OBJECT;
typedef PUCHAR PUNICODE_STRING;
typedef long long LONGLONG;
typedef unsigned long long ULONGLONG;
typedef LONGLONG *PLONGLONG;
typedef ULONGLONG *PULONGLONG;
typedef PUCHAR ANSI_STRING;
typedef ANSI_STRING *PANSI_STRING;
typedef unsigned short WCHAR;
typedef WCHAR *PWCHAR;
typedef WCHAR *LPWCH, *PWCH;
typedef WCHAR *NWPSTR;
typedef WCHAR *LPWSTR, *PWSTR;
typedef struct semaphore SEMAPHORE;

#ifdef __KERNEL__
typedef irqreturn_t IRQ_RET_TYPE;
#define IRQ_RET		return IRQ_HANDLED
#endif /* __KERNEL__ */

#endif /* _OS_HEADER1 */
