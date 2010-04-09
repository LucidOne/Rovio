#ifndef __USB_H
#define __USB_H


//extern UINT32 volatile Bulk_Out_Transfer_Size;
//extern UINT8 volatile USBModeFlag;
typedef void (*usbException)(void);
typedef void usbExceptionForUser(void );
extern UINT32 USBRead(PCHAR pbuf, PUINT32 rt_len);
extern UINT32 USBWrite(PCHAR pbuf, PUINT32 rt_len);
extern void USBPrintf(PINT8 pcStr,...);
extern void USBInitForVCom(void );
extern void USBResetForVCom(void);
extern void USBExceptionHandle(void);
extern usbException USBRegiesterException(usbException);

#endif /* __USB_H */
