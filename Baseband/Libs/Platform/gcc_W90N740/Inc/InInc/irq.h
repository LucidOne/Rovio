#ifndef __W90N740_IRQ_H__

INT32 sysSetInterruptType (UINT32 uIntNo, UINT32 uIntSourceType);
PVOID sysInstallISR (INT32 nIntTypeLevel, UINT32 uIntNo, PVOID fnNewISR);
INT32 sysEnableInterrupt (UINT32 uIntNo);
INT32 sysDisableInterrupt (UINT32 uIntNo);

#endif

