/* 
 * 	File :	sdio_spec.h
 * 	Addresses defined by SDIO Specifications 
 *
 *  Copyright © Marvell International Ltd. and/or its affiliates, 2003-2006
 */

#ifndef _SDIOSPEC_H_
#define _SDIOSPEC_H_

#define MAX_SDIO_TUPLE_BODY_LEN 255

/* CCCR (Card Common Control Registers) */
#define CCCR_SDIO_REV_REG		0x00
#define CCCR_FORMAT_VERSION(reg)    	(((reg) & 0xf0 >> 4))
#define SDIO_SPEC_REVISION(reg)		(((reg) & 0x0f))

#define SD_SPEC_REV_REG			0x01
#define SD_PHYS_SPEC_VERSION(reg)	(((reg) & 0x0f))

#define IO_ENABLE_REG			0x02
#define IOE(x)				(0x1U << (x))
#define IOE_1				IOE(1)
#define IOE_2				IOE(2)
#define IOE_3				IOE(3)
#define IOE_4				IOE(4)
#define IOE_5				IOE(5)
#define IOE_6				IOE(6)
#define IOE_7				IOE(7)

#define IO_READY_REG			0x03
#define IOR(x)				(0x1U << (x))
#define IOR_1				IOR(1)
#define IOR_2				IOR(2)
#define IOR_3				IOR(3)
#define IOR_4				IOR(4)
#define IOR_5				IOR(5)
#define IOR_6				IOR(6)
#define IOR_7				IOR(7)

#define INT_ENABLE_REG			0x04
#define IENM				0x1
#define IEN(x)				(0x1U << (x))
#define IEN_1				IEN(1)
#define IEN_2				IEN(2)
#define IEN_3				IEN(3)
#define IEN_4				IEN(4)
#define IEN_5				IEN(5)
#define IEN_6				IEN(6)
#define IEN_7				IEN(7)

#define INT_PENDING_REG			0x05
#define INT(x)				(0x1U << (x))
#define INT_1				INT(1)
#define INT_2				INT(2)
#define INT_3				INT(3)
#define INT_4				INT(4)
#define INT_5				INT(5)
#define INT_6				INT(6)
#define INT_7				INT(7)

#define IO_ABORT_REG			0x06
#define AS(x)				((x) & 0x7)
#define AS_1				AS(1)
#define AS_2				AS(2)
#define AS_3				AS(3)
#define AS_4				AS(4)
#define AS_5				AS(5)
#define AS_6				AS(6)
#define AS_7				AS(7)

#define RES				(0x1U << 3)

#define BUS_INTERFACE_CONTROL_REG 	0x07
#define BUS_WIDTH(reg)			((reg) & 0x3)
#define CD_DISABLE			(0x1U << 7)

#define CARD_CAPABILITY_REG		0x08
#define SDC				(0x1U << 0)
#define SMB				(0x1U << 1)
#define SRW				(0x1U << 2)
#define SBS				(0x1U << 3)
#define S4MI				(0x1U << 4)
#define E4MI				(0x1U << 5)
#define LSC				(0x1U << 6)
#define S4BLS				(0x1U << 7)

#define COMMON_CIS_POINTER_0_REG	0x09

#define COMMON_CIS_POINTER_1_REG	0x0a

#define COMMON_CIS_POINTER_2_REG	0x0b

#define BUS_SUSPEND_REG			0x0c
#define BUS_STATUS			(0x1U << 0)
#define BUS_REL_REQ_STATUS		(0x1U << 1)

#define FUNCTION_SELECT_REG		0x0d
#define FS(x)				((x) & 0xf)
#define FS_1				FS(1)
#define FS_2				FS(2)
#define FS_3				FS(3)
#define FS_4				FS(4)
#define FS_5				FS(5)
#define FS_6				FS(6)
#define FS_7				FS(7)
#define FS_MEM				FS(8)
#define DF				(0x1U << 7)

#define EXEC_FLAGS_REG			0x0e
#define EXM				0x0
#define EX(x)				(0x1U << (x))
#define EX_1				EX(1)
#define EX_2				EX(2)
#define EX_3				EX(3)
#define EX_4				EX(4)
#define EX_5				EX(5)
#define EX_6				EX(6)
#define EX_7				EX(7)

#define READY_FLAGS_REG			0x0f
#define RFM				0x0
#define RF(x)				(0x1U << (x))
#define RF_1				RF(1)
#define RF_2				RF(2)
#define RF_3				RF(3)
#define RF_4				RF(4)
#define RF_5				RF(5)
#define RF_6				RF(6)
#define RF_7				IEN(7)

/* FBR (Function Basic Registers) */
#define FN_CSA_REG(x)			(0x100 * (x) + 0x00)
#define FN_CIS_POINTER_0_REG(x)		(0x100 * (x) + 0x09)
#define FN_CIS_POINTER_1_REG(x)		(0x100 * (x) + 0x0a)
#define FN_CIS_POINTER_2_REG(x)		(0x100 * (x) + 0x0b)
#define FN_CSA_DAT_REG(x)		(0x100 * (x) + 0x0f)
#define FN_BLOCK_SIZE_0_REG(x)		(0x100 * (x) + 0x10)
#define FN_BLOCK_SIZE_1_REG(x)		(0x100 * (x) + 0x11)

/* Function 0  -- duplicate, see the CCRC section */
#define FN0_CIS_POINTER_0_REG		FN_CIS_POINTER_0_REG(1)
#define FN0_CIS_POINTER_1_REG		FN_CIS_POINTER_1_REG(1)
#define FN0_CIS_POINTER_2_REG		FN_CIS_POINTER_1_REG(1)
#define FN0_BLOCK_SIZE_0_REG		FN_BLOCK_SIZE_0_REG(1)
#define FN0_BLOCK_SIZE_1_REG		FN_BLOCK_SIZE_1_REG(1)
/*      Function 1       */
#define FN1_CSA_REG			FN_CSA_REG(1)
#define FN1_CIS_POINTER_0_REG		FN_CIS_POINTER_0_REG(1)
#define FN1_CIS_POINTER_1_REG		FN_CIS_POINTER_1_REG(1)
#define FN1_CIS_POINTER_2_REG		FN_CIS_POINTER_1_REG(1)
#define FN1_CSA_DAT_REG			FN_CSA_DAT_REG(1)
#define FN1_BLOCK_SIZE_0_REG		FN_BLOCK_SIZE_0_REG(1)
#define FN1_BLOCK_SIZE_1_REG		FN_BLOCK_SIZE_1_REG(1)
/*      Function 2       */
#define FN2_CSA_REG			FN_CSA_REG(2)
#define FN2_CIS_POINTER_0_REG		FN_CIS_POINTER_0_REG(2)
#define FN2_CIS_POINTER_1_REG		FN_CIS_POINTER_1_REG(2)
#define FN2_CIS_POINTER_2_REG		FN_CIS_POINTER_2_REG(2)
#define FN2_CSA_DAT_REG			FN_CSA_DAT_REG(2)
#define FN2_BLOCK_SIZE_0_REG		FN_BLOCK_SIZE_0_REG(2)
#define FN2_BLOCK_SIZE_1_REG		FN_BLOCK_SIZE_1_REG(2)
/*      Function 3       */
#define FN3_CSA_REG			FN_CSA_REG(3)
#define FN3_CIS_POINTER_0_REG		FN_CIS_POINTER_0_REG(3)
#define FN3_CIS_POINTER_1_REG		FN_CIS_POINTER_1_REG(3)
#define FN3_CIS_POINTER_2_REG		FN_CIS_POINTER_2_REG(3)
#define FN3_CSA_DAT_REG			FN_CSA_DAT_REG(3)
#define FN3_BLOCK_SIZE_0_REG		FN_BLOCK_SIZE_0_REG(3)
#define FN3_BLOCK_SIZE_1_REG		FN_BLOCK_SIZE_1_REG(3)
/*      Function 4       */
#define FN4_CSA_REG			FN_CSA_REG(4)
#define FN4_CIS_POINTER_0_REG		FN_CIS_POINTER_0_REG(4)
#define FN4_CIS_POINTER_1_REG		FN_CIS_POINTER_1_REG(4)
#define FN4_CIS_POINTER_2_REG		FN_CIS_POINTER_2_REG(4)
#define FN4_CSA_DAT_REG			FN_CSA_DAT_REG(4)
#define FN4_BLOCK_SIZE_0_REG		FN_BLOCK_SIZE_0_REG(4)
#define FN4_BLOCK_SIZE_1_REG		FN_BLOCK_SIZE_1_REG(4)
/*      Function 5       */
#define FN5_CSA_REG			FN_CSA_REG(5)
#define FN5_CIS_POINTER_0_REG		FN_CIS_POINTER_0_REG(5)
#define FN5_CIS_POINTER_1_REG		FN_CIS_POINTER_1_REG(5)
#define FN5_CIS_POINTER_2_REG		FN_CIS_POINTER_2_REG(5)
#define FN5_CSA_DAT_REG			FN_CSA_DAT_REG(5)
#define FN5_BLOCK_SIZE_0_REG		FN_BLOCK_SIZE_0_REG(5)
#define FN5_BLOCK_SIZE_1_REG		FN_BLOCK_SIZE_1_REG(5)
/*      Function 6       */
#define FN6_CSA_REG			FN_CSA_REG(6)
#define FN6_CIS_POINTER_0_REG		FN_CIS_POINTER_0_REG(6)
#define FN6_CIS_POINTER_1_REG		FN_CIS_POINTER_1_REG(6)
#define FN6_CIS_POINTER_2_REG		FN_CIS_POINTER_2_REG(6)
#define FN6_CSA_DAT_REG			FN_CSA_DAT_REG(6)
#define FN6_BLOCK_SIZE_0_REG		FN_BLOCK_SIZE_0_REG(6)
#define FN6_BLOCK_SIZE_1_REG		FN_BLOCK_SIZE_1_REG(6)
/*      Function 7       */
#define FN7_CSA_REG			FN_CSA_REG(7)
#define FN7_CIS_POINTER_0_REG		FN_CIS_POINTER_0_REG(7)
#define FN7_CIS_POINTER_1_REG		FN_CIS_POINTER_1_REG(7)
#define FN7_CIS_POINTER_2_REG		FN_CIS_POINTER_2_REG(7)
#define FN7_CSA_DAT_REG			FN_CSA_DAT_REG(7)
#define FN7_BLOCK_SIZE_0_REG		FN_BLOCK_SIZE_0_REG(7)
#define FN7_BLOCK_SIZE_1_REG		FN_BLOCK_SIZE_1_REG(7)

/* FBR bit definitions */
#define FN_IODEV_INTERFACE_CODE(reg)	((reg) & 0xf)
#define FN_SUPPORTS_CSA			(0x1U << 6)
#define FN_CSA_ENABLE			(0x1U << 7)

/* Misc. helper definitions */
#define FN(x)				(x)
#define FN0				FN(0)
#define FN1				FN(1)
#define FN2				FN(2)
#define FN3				FN(3)
#define FN4				FN(4)
#define FN5				FN(5)
#define FN6				FN(6)
#define FN7				FN(7)

#endif /* __SDIO_SPEC__H */
