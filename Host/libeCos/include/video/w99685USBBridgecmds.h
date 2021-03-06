#ifndef __W99685USBBRIDGECMDS_H__
#define __W99685USBBRIDGECMDS_H__

#define USBDEBUG

#define USBBRIDGE_CMD ((UINT16)0x5b)

#define CMD_GETSTATUS		0x00
#define CMD_GETVENDORCMD1	0x01
#define CMD_GETVENDORCMD2	0x02
#define CMD_GETCTLOUTDATA	0x03
#define CMD_SENDCTLINDATA	0x04
#define CMD_GETBULKOUTDATA	0x05
#define CMD_SENDBULKINDATA	0x06
#define CMD_GETBULKSETLENGTH	0x07

#define STATUS_NOTHING			0x0
#define STATUS_USBPLUGIN		0x1
#define STATUS_USBPLUGOUT		0x2
#define STATUS_VENDORCMDIN		0x3
#define STATUS_CTLOUTDATAREADY		0x4
#define STATUS_CTLINDATAEMPTY		0x5
#define STATUS_BULKOUTDATAREADY		0x6
#define STATUS_BULKINDATAEMPTY		0x7
#define STATUS_BULKSETLENGTH		0x8
#define STATUS_USER_CMD_WRITE		0x9
#define STATUS_USER_CMD_READ		0xa
#endif
