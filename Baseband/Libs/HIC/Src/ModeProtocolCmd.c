/***************************************************************************
 *                                                                         *
 * Copyright (c) 2000 - 2005 Winbond Electronics Corp. All rights reserved.*
 *                                                                         *
 ***************************************************************************/

/****************************************************************************
 *
 * FILENAME
 *		$RCSfile: ModeProtocolCmd.c,v $
 *
 * VERSION
 *		$Revision: 1.2 $
 *
 * DESCRIPTION
 *		Functions for HIC protocol command mode.
 *
 * HISTORY
 *     $Log: ModeProtocolCmd.c,v $
 *     Revision 1.2  2006/03/18 15:12:10  xhchen
 *     Test MP4 loopback by BB, OK.
 *     Add checksum for FIFO in protocol command.
 *
 *     Revision 1.1  2006/01/17 09:42:12  xhchen
 *     Add B.B. testing applications.
 *
 *     Revision 1.1.2.16  2005/09/13 08:41:48  xhchen
 *     1. Mass storage demo added to FullDemo.
 *     2. wbhPlaybackJPEG/wbhPlaybackFile keeps the width/height proportion.
 *     3. Add wbSetLastError/wbGetLastError.
 *
 *     Revision 1.1.2.15  2005/09/07 03:32:06  xhchen
 *     1. Update command set API to WBCM_702_0.82pre14.doc.
 *     2. Add the function to playback audio by sending audio bitstream.
 *     3. Add "wbhicRescue()" to clear BUSY, DRQ an ERROR.
 *     4. Merget WYSun's testing code to "Samples/FlowTest".
 *
 *     Revision 1.1.2.14  2005/08/30 04:14:39  xhchen
 *     Makefile and create_project.js support searching dependence header file automatically.
 *     Add ywsun's filebrowser demo (for virtual com).
 *     Add audio test in "FlowTest".
 *
 *     Revision 1.1.2.13  2005/08/02 11:09:59  xhchen
 *     Remove "L" from all code
 *     Add functions:
 *     	ConvertString_CP950_to_UNICODE
 *     	ConvertString_CP936_to_UNICODE
 *     	ConvertString_ASCII_to_UNICODE
 *     Add a sample UnicodePath to show how to use a unicode string.
 *
 *     Revision 1.1.2.12  2005/07/21 07:52:42  xhchen
 *     ...
 *
 *     Revision 1.1.2.11  2005/07/15 08:20:08  xhchen
 *     Use PIPE to transfer FIFO instead of callback functions.
 *
 *     Revision 1.1.2.10  2005/06/29 02:21:26  xhchen
 *     Fix a bug in "Parse media file".
 *     Move sticker maker resource files to "reference."
 *
 *     Revision 1.1.2.9  2005/06/24 11:19:19  xhchen
 *     Fixed big endian problem.
 *     FIFO can use a function as it's R/W data now.
 *
 *     Revision 1.1.2.8  2005/06/01 07:43:24  xhchen
 *     Comic and burst added.
 *
 *     Revision 1.1.2.7  2005/05/25 06:27:52  xhchen
 *     Fix a bug in TYPE V command.
 *
 *     Revision 1.1.2.6  2005/05/16 08:33:27  xhchen
 *         1. Add HIC debug function EnableHICDebug(BOOL bEnable). When HIC debug is on, almost all the HIC I/O operation will be printed to console.
 *     	2. Command set API according to "WBCM_702_0.8.pdf".
 *         3. Add code to check card before playback/record.
 *     	4. MP3 playback window:
 *     	   a. Add a spectrum chart on the window
 *     	   b. Display ID3 tags (artist, album & genre) according to the "Language" settings.
 *     	   c. Add code to setting equalizer by users.
 *     	   d. Continue to play next audio file after current file is over.
 *
 *     Revision 1.1.2.5  2005/03/29 06:18:43  xhchen
 *     1. Document updated
 *     2. Add high level API "wbhConvertJPEG2BMP", "wbhReadConvertedBMP".
 *     3. Add high level API "wbhBurstImageFile".
 *     4. Add code for busrt capture in the sample "FullDemo".
 *     5. Add a new sample "JPEG2BMP". The demo can not work till now, because
 *         command "Get Decoded JPEG Data" returns a ERROR bit.
 *
 *     Revision 1.1.2.4  2005/01/11 09:42:01  xhchen
 *     Bug fixed: FIFO write in 16bit mode.
 *     Verify YHTu's new firmware.
 *
 *     Revision 1.1.2.3  2004/12/22 09:46:01  xhchen
 *     ...
 *
 *
 * REMARK
 *     None
 *
 **************************************************************************/



#include "../../Platform/Inc/Platform.h"
#include "../../SoftPipe/include/softpipe.h"
#include "../Inc/HIC.h"


#ifdef HIC_DEBUG
#	define PRINT_DEBUG_CMD_WRITE_REG(r_f, r_e, r_d, r_c, r_b, r_a) \
		do \
		{ \
			if (g_iHIC_DEBUG_CMD) \
			{ \
				sysPrintf ("(%08d) CMD WR REG[FEDCBA]: %02x %02x %02x %02x %02x %02x\n", __LINE__, \
					(int) (r_f), (int) (r_e), (int) (r_d), (int) (r_c), (int) (r_b), (int) (r_a)); \
			} \
		} while (0)
#	define PRINT_DEBUG_CMD_READ_REG(r_c, r_b, r_a, r_9) \
		do \
		{ \
			if (g_iHIC_DEBUG_CMD) \
			{ \
				sysPrintf ("(%08d) CMD RD REG[CBA9]: %02x %02x %02x %02x\n", __LINE__, \
					(int) (r_c), (int) (r_b), (int) (r_a), (int) (r_9)); \
			} \
		} while (0)
#else
#	define PRINT_DEBUG_CMD_WRITE_REG(r_f, r_e, r_d, r_c, r_b, r_a)
#	define PRINT_DEBUG_CMD_READ_REG(r_c, r_b, r_a, r_9)
#endif





/****************************************************************************
 *
 * FUNCTION
 *		__wbhicCmd_Send
 *
 * DESCRIPTION
 *		Send a command to W99702.
 *
 * INPUTS
 *		ucCommand: Command described by CF_REGF
 *		ucSubCommand: Sub command described by CF_REGE
 *		ucParam1: Parameter 1 described by CF_REGD
 *		ucParam2: Parameter 2 described by CF_REGC
 *		ucParam3: Parameter 3 described by CF_REGB
 *		ucParam4: Parameter 4 described by CF_REGA
 *
 * OUTPUTS
 *		None
 *
 * RETURN
 *		None
 *
 * REMARK
 *		None
 *
 ***************************************************************************/
VOID __wbhicCmd_Send (UCHAR ucCommand,     //CF_REGF
			                 UCHAR ucSubCommand,  //CF_REGE
            			     UCHAR ucParam1,      //CF_REGD
                        	 UCHAR ucParam2,      //CF_REGC
			                 UCHAR ucParam3,      //CF_REGB
            			     UCHAR ucParam4       //CF_REGA
    )
{
    CF_REGD = ucParam1;
    CF_REGC = ucParam2;
    CF_REGB = ucParam3;
    CF_REGA = ucParam4;
    //CF_REG9 = (bIsChecksumUsed ? 1 : 0);
    CF_REG9 = 0;

    CF_REGE = ucSubCommand;
    CF_REGF = ucCommand;
	PRINT_DEBUG_CMD_WRITE_REG (ucCommand, ucSubCommand, ucParam1, ucParam2, ucParam3, ucParam4);
}


/****************************************************************************
 *
 * FUNCTION
 *		__wbhicCmd_Recv
 *
 * DESCRIPTION
 *		Receive command's response on BUSY cleared.
 *
 * INPUTS
 *		None
 *
 * OUTPUTS
 *		pucRespon1: Response 1 described by CF_REGC
 *		pucRespon2: Response 1 described by CF_REGB
 *		pucRespon3: Response 1 described by CF_REGA
 *		pucRespon4: Response 1 described by CF_REG9
 *
 * RETURN
 *		None
 *
 * REMARK
 *		None
 *
 ***************************************************************************/
static VOID __wbhicCmd_Recv (UCHAR * pucRespon1, //CF_REGC
                             UCHAR * pucRespon2, //CF_REGB
                             UCHAR * pucRespon3, //CF_REGA
                             UCHAR * pucRespon4  //CF_REG9
    )
{
	UCHAR ucRespon1;
	UCHAR ucRespon2;
	UCHAR ucRespon3;
	UCHAR ucRespon4;


	ucRespon1 = CF_REGC;
	ucRespon2 = CF_REGB;
	ucRespon3 = CF_REGA;
	ucRespon4 = CF_REG9;
	PRINT_DEBUG_CMD_READ_REG (ucRespon1, ucRespon2, ucRespon3, ucRespon4);

    if (pucRespon1 != NULL)
        *pucRespon1 = ucRespon1;
    if (pucRespon2 != NULL)
        *pucRespon2 = ucRespon2;
    if (pucRespon3 != NULL)
        *pucRespon3 = ucRespon3;
    if (pucRespon4 != NULL)
        *pucRespon4 = ucRespon4;
}


BOOL __wbhicCmd_Wait_NOBUSY (UCHAR ucCommand,     //CF_REGF
			                 UCHAR ucSubCommand,  //CF_REGE
            			     BOOL bCheckError)
{
	BOOL bOK;
	UCHAR ucStatus = 0;

	//Wait NO BUSY.
	if (bCheckError)
	{
	    WB_COND_WAITFOR (
			(
				(ucStatus = CF_REGF),	//Save status to "ucStatus"
				((ucStatus & HICST_ERROR) != 0x00 || (ucStatus & HICST_BUSY) == 0x00)
			),
			CF_TIMEOUT_MSEC, bOK);
	}
	else
	{
		WB_COND_WAITFOR (
			((CF_REGF & HICST_BUSY) == 0x00),
			CF_TIMEOUT_MSEC, bOK);	
	}

    if (!bOK)
    {
    	char pcError[128];
    	sprintf (pcError, "Command: 0x%02x 0x%02x => Wait NO BUSY: Timeout!\n", (int) ucCommand, (int) ucSubCommand);
        D_ERROR (pcError);
        wbSetLastError (HIC_ERR_ST_BUSY);
        wbhicRescue ();
        return FALSE;   //Timeout if (!bOK)
    }

    if (bCheckError && (ucStatus & HICST_ERROR) != 0x00)
    {
       	char pcError[128];
    	sprintf (pcError, "Command: 0x%02x 0x%02x => Wait NO BUSY: Error!\n", (int) ucCommand, (int) ucSubCommand);
    	D_ERROR (pcError);
    	wbSetLastError (HIC_ERR_ST_ERROR);
    	wbhicRescue ();
    	return FALSE;	//Command returns an error.
    }

    return TRUE;
}


BOOL __wbhicCmd_Wait_DRQ (UCHAR ucCommand,     //CF_REGF
	                      UCHAR ucSubCommand  //CF_REGE
       			          )
{
	BOOL bOK;
	UCHAR ucStatus;

	//Wait DRQ.
    WB_COND_WAITFOR (
		(
			(ucStatus = CF_REGF),	//Save status to "ucStatus"
			((ucStatus & HICST_ERROR) != 0x00 || (ucStatus & HICST_DRQ) == HICST_DRQ)
		),
		CF_TIMEOUT_MSEC, bOK);

    if (!bOK)
    {
    	char pcError[128];
    	sprintf (pcError, "Command: 0x%02x 0x%02x => Wait DRQ: Timeout!\n", (int) ucCommand, (int) ucSubCommand);
        D_ERROR (pcError);
        wbSetLastError (HIC_ERR_ST_BUSY);
        wbhicRescue ();
        return FALSE;   //Timeout if (!bOK)
    }
    if ((ucStatus & HICST_ERROR) != 0x00)
    {
    	char pcError[128];
    	sprintf (pcError, "Command: 0x%02x 0x%02x => Wait DRQ: Error!\n", (int) ucCommand, (int) ucSubCommand);
    	D_ERROR (pcError);
    	wbSetLastError (HIC_ERR_ST_ERROR);
    	wbhicRescue ();
    	return FALSE;	//Command returns an error.
    }

    return TRUE;
}


/****************************************************************************
 *
 * FUNCTION
 *		wbhicCmd_I
 *
 * DESCRIPTION
 *		Type I command: Command without waiting response.
 *
 * INPUTS
 *		ucCommand: Command described by CF_REGF
 *		ucSubCommand: Sub command described by CF_REGE
 *		ucParam1: Parameter 1 described by CF_REGD
 *		ucParam2: Parameter 2 described by CF_REGC
 *		ucParam3: Parameter 3 described by CF_REGB
 *		ucParam4: Parameter 4 described by CF_REGA
 *
 * OUTPUTS
 *		None
 *
 * RETURN
 *		TRUE, success.
 *		FALSE, failed.
 *
 * REMARK
 *		None
 *
 ***************************************************************************/
BOOL wbhicCmd_I (UCHAR ucCommand,       //CF_REGF
                 UCHAR ucSubCommand,    //CF_REGE
                 UCHAR ucParam1,        //CF_REGD
                 UCHAR ucParam2,        //CF_REGC
                 UCHAR ucParam3,        //CF_REGB
                 UCHAR ucParam4 //CF_REGA
    )
{
	BOOL bReturn;
	
	sysLockWbc ();
	bReturn = __wbhicCmd_Wait_NOBUSY (ucCommand, ucSubCommand, FALSE);
	if (!bReturn)
		goto lOut;

	__wbhicCmd_Send (ucCommand,
                     ucSubCommand,
                     ucParam1, ucParam2, ucParam3, ucParam4);

	bReturn = __wbhicCmd_Wait_NOBUSY (ucCommand, ucSubCommand, TRUE);
	
lOut:
	sysUnlockWbc ();
	return bReturn;
}


/****************************************************************************
 *
 * FUNCTION
 *		wbhicCmd_II
 *
 * DESCRIPTION
 *		Type II command: Command with response.
 *
 * INPUTS
 *		ucCommand: Command described by CF_REGF
 *		ucSubCommand: Sub command described by CF_REGE
 *		ucParam1: Parameter 1 described by CF_REGD
 *		ucParam2: Parameter 2 described by CF_REGC
 *		ucParam3: Parameter 3 described by CF_REGB
 *		ucParam4: Parameter 4 described by CF_REGA
 *
 * OUTPUTS
 *		pucRespon1: Response 1 described by CF_REGC
 *		pucRespon2: Response 1 described by CF_REGB
 *		pucRespon3: Response 1 described by CF_REGA
 *		pucRespon4: Response 1 described by CF_REG9
 *
 * RETURN
 *		TRUE, success.
 *		FALSE, failed.
 *
 * REMARK
 *		None
 *
 ***************************************************************************/
BOOL wbhicCmd_II (UCHAR ucCommand,      //CF_REGF
                  UCHAR ucSubCommand,   //CF_REGE
                  UCHAR ucParam1,       //CF_REGD
                  UCHAR ucParam2,       //CF_REGC
                  UCHAR ucParam3,       //CF_REGB
                  UCHAR ucParam4,       //CF_REGA
                  UCHAR * pucRespon1,   //CF_REGC
                  UCHAR * pucRespon2,   //CF_REGB
                  UCHAR * pucRespon3,   //CF_REGA
                  UCHAR * pucRespon4    //CF_REG9
    )
{
	BOOL bReturn;
	
	sysLockWbc ();

	bReturn = __wbhicCmd_Wait_NOBUSY (ucCommand, ucSubCommand, FALSE);
	if (!bReturn)
		goto lOut;

	__wbhicCmd_Send (ucCommand,
                     ucSubCommand,
                     ucParam1, ucParam2, ucParam3, ucParam4);

	bReturn = __wbhicCmd_Wait_NOBUSY (ucCommand, ucSubCommand, TRUE);
	if (!bReturn)
		goto lOut;

	__wbhicCmd_Recv (pucRespon1, pucRespon2, pucRespon3, pucRespon4);

	bReturn = __wbhicCmd_Wait_NOBUSY (ucCommand, ucSubCommand, TRUE);
	if (!bReturn)
		goto lOut;
	

lOut:
	sysUnlockWbc ();
	return bReturn;
}



/****************************************************************************
 *
 * FUNCTION
 *		__wbhicCmd_III_Write_Len
 *
 * DESCRIPTION
 *		Type III command: Command with mass data writing.
 *		(Write data length firstly.)
 *
 * INPUTS
 *		ucCommand: Command described by CF_REGF
 *		ucSubCommand: Sub command described by CF_REGE
 *		uBufferLength: The data length
 *
 * OUTPUTS
 *		None
 *
 * RETURN
 *		TRUE, success.
 *		FALSE, failed.
 *
 * REMARK
 *		None
 *
 ***************************************************************************/
BOOL __wbhicCmd_III_Write_Len (UCHAR ucCommand,   //CF_REGF
                               UCHAR ucSubCommand,        //CF_REGE
                               UINT32 uBufferLength       //Length of data buffer
    )
{
	BOOL bReturn;
    UCHAR pucParam[4];

	bReturn = __wbhicCmd_Wait_NOBUSY (ucCommand, ucSubCommand, FALSE);
	if (!bReturn)
		goto lOut;

    UINT32_2_UCHAR (uBufferLength, pucParam);

	//Send length as parameters.
	__wbhicCmd_Send (ucCommand,
                     ucSubCommand,
                     pucParam[3],
                     pucParam[2], pucParam[1], pucParam[0]);

	bReturn = __wbhicCmd_Wait_DRQ (ucCommand, ucSubCommand);
	if (!bReturn)
		goto lOut;

lOut:
	return bReturn;
}



/****************************************************************************
 *
 * FUNCTION
 *		wbhicCmd_III_Write
 *
 * DESCRIPTION
 *		Type III command: Command with mass data writing.
 *
 * INPUTS
 *		ucCommand: Command described by CF_REGF
 *		ucSubCommand: Sub command described by CF_REGE
 *		pSourceBuffer: If "uBufferType" is "WBHIC_FUNC_IO", it points to a
 *                     structure WBHIC_FIFO_FUNCSRC_PARAM_T, otherwise it's
 *                     a buffer.
 *		uBufferType: The input buffer's type. If its value is not
 *                   "WBHIC_FUNC_IO", it's used as the buffer length.
 *                   (for compatibility)
 *
 * OUTPUTS
 *		puBufferWritten: Length written.
 *
 * RETURN
 *		TRUE, success.
 *		FALSE, failed.
 *
 * REMARK
 *		None
 *
 ***************************************************************************/
BOOL wbhicCmd_III_Write (UCHAR ucCommand,
						 UCHAR ucSubCommand,
						 VOID *pSourceBuffer,
                         UINT32 uBufferType,
                         UINT32 * puBufferWritten       //The length written
    )
{
	BOOL bReturn;
	UINT32 uLen;

	sysLockWbc ();

	if (uBufferType == WBHIC_FIFO_PIPE_IO)
	{
		WBHIC_FIXLEN_PIPE_T *pPipeSrc = (WBHIC_FIXLEN_PIPE_T *) pSourceBuffer;
		uLen = pPipeSrc->uLength;
	}
	else
		uLen = uBufferType;


	bReturn = __wbhicCmd_III_Write_Len (ucCommand,
    									ucSubCommand, uLen);
	if (!bReturn)
		goto lOut;


	bReturn = wbhicWriteFIFO (pSourceBuffer, uBufferType, puBufferWritten);
	if (!bReturn)
    {
		CHAR pcError[64];
    	sprintf(pcError, "Command failed: 0x%02x 0x%02x.\n", (int) ucCommand, (int) ucSubCommand);
        D_ERROR (pcError);
		goto lOut;
	}

	bReturn = __wbhicCmd_Wait_NOBUSY (ucCommand, ucSubCommand, TRUE);


lOut:
	sysUnlockWbc ();
	return bReturn;
}



/****************************************************************************
 *
 * FUNCTION
 *		wbhicCmd_III_Read_Len
 *
 * DESCRIPTION
 *		Type III command: Command with mass data reading.
 *		(Step1, send command and read the length.)
 *
 * INPUTS
 *		ucCommand: Command described by CF_REGF
 *		ucSubCommand: Sub command described by CF_REGE
 *		ucParam1: Parameter 1 described by CF_REGD
 *		ucParam2: Parameter 2 described by CF_REGC
 *		ucParam3: Parameter 3 described by CF_REGB
 *		ucParam4: Parameter 4 described by CF_REGA
 *
 * OUTPUTS
 *		puBufferLength: The data length should be read from W99702 FIFO.
 *
 * RETURN
 *		TRUE, success.
 *		FALSE, failed.
 *
 * REMARK
 *		None
 *
 ***************************************************************************/
BOOL wbhicCmd_III_Read_Len (UCHAR ucCommand,    //CF_REGF
                            UCHAR ucSubCommand, //CF_REGE
                            UCHAR ucParam1,     //CF_REGD
                            UCHAR ucParam2,     //CF_REGC
                            UCHAR ucParam3,     //CF_REGB
                            UCHAR ucParam4,     //CF_REGA
                            UINT32 * puBufferLength     //Length of data buffer
    )
{

	BOOL bReturn;
    UCHAR pucRespon[4];

	bReturn = __wbhicCmd_Wait_NOBUSY (ucCommand, ucSubCommand, FALSE);
	if (!bReturn)
		goto lOut;

    //Send parameters.
    __wbhicCmd_Send (ucCommand,
                     ucSubCommand,
                     ucParam1, ucParam2, ucParam3, ucParam4);

	bReturn = __wbhicCmd_Wait_DRQ (ucCommand, ucSubCommand);
	if (!bReturn)
		goto lOut;

    //Read length;
    __wbhicCmd_Recv (&pucRespon[3],
    				 &pucRespon[2], &pucRespon[1], &pucRespon[0]);

    if (puBufferLength != NULL)
		UCHAR_2_UINT32 (pucRespon, *puBufferLength);

lOut:
	return bReturn;
}



/****************************************************************************
 *
 * FUNCTION
 *		__wbhicCmd_IV_or_V_Send
 *
 * DESCRIPTION
 *		Send Type IV or V command to W99702.
 *
 * INPUTS
 *		ucCommand: Command described by CF_REGF
 *		ucSubCommand: Sub command described by CF_REGE
 *		uParamNum: Number of parameters
 *		vaList:
 *				//Pointer of param1(VOID *)
 *				//Type or length of param1(UINT32)
 *				//Pointer of param2(VOID *)
 *				//Type or length of param2(UINT32)
 *				//...
 *
 * OUTPUTS
 *		None
 *
 * RETURN
 *		TRUE, success.
 *		FALSE, failed.
 *
 * REMARK
 *		None
 *
 ***************************************************************************/
static BOOL __wbhicCmd_IV_or_V_Send (UCHAR ucCommand,
                                     UCHAR ucSubCommand,
                                     UINT32 uParamNum,
                                     va_list vaList1,
                                     va_list vaList2	//Duplication of vaList1, (some platform does not have va_copy).
                                     )
{
	UCHAR pucParamNum[4];
    UINT32 u;
    UINT32 uBufferLength;
    WBHIC_FIFO_PIPE_T wbhicFIFO_PIPE;

    /* Calculate length. */
    uBufferLength = sizeof (uParamNum);
    for (u = 0; u < uParamNum; u++)
    {
        VOID *pParamPtr;
        UINT32 uParamType;
        UINT32 uParamLen;

        pParamPtr	= va_arg (vaList1, VOID *);
        uParamType	= va_arg (vaList1, UINT32);

		if (uParamType == WBHIC_FIFO_PIPE_IO)
		{
			WBHIC_FIXLEN_PIPE_T *pPipeSrc = (WBHIC_FIXLEN_PIPE_T *) pParamPtr;
			uParamLen = pPipeSrc->uLength;
		}
		else
			uParamLen = uParamType;

        uBufferLength += sizeof (uParamLen);
        uBufferLength += uParamLen;
    }

    /* Send length of all parameters. */
    if (!__wbhicCmd_III_Write_Len (ucCommand,
    							   ucSubCommand, uBufferLength))
        return FALSE;

	wbhicPipeWriteFIFO_Init (&wbhicFIFO_PIPE, uBufferLength);
    /* Send number of parameters. */
    UINT32_2_UCHAR (uParamNum, pucParamNum);
    if (!wbhicPartWriteFIFO (&wbhicFIFO_PIPE.wbhicFIFO,
    						 pucParamNum,
    						 sizeof (pucParamNum), NULL))
        return FALSE;

    /* Send parameters. */
    uBufferLength = sizeof (uParamNum);
    for (u = 0; u < uParamNum; u++)
    {
    	UCHAR pucParamLen[4];
        VOID *pParamPtr;
        UINT32 uParamType;
        UINT32 uParamLen;

        pParamPtr	= va_arg (vaList2, VOID *);
        uParamType	= va_arg (vaList2, UINT32);

        if (uParamType == WBHIC_FIFO_PIPE_IO)
		{
			WBHIC_FIXLEN_PIPE_T *pPipeSrc = (WBHIC_FIXLEN_PIPE_T *) pParamPtr;
			uParamLen = pPipeSrc->uLength;
		}
        else
        	uParamLen = uParamType;

		UINT32_2_UCHAR(uParamLen, pucParamLen);
		if (!wbhicPartWriteFIFO (&wbhicFIFO_PIPE.wbhicFIFO,
								 pucParamLen,
								 sizeof (pucParamLen), NULL))
           	return FALSE;

		if (uParamType == WBHIC_FIFO_PIPE_IO)
		{
			WBHIC_FIXLEN_PIPE_T *pPipeSrc = (WBHIC_FIXLEN_PIPE_T *) pParamPtr;
			spConnect (&pPipeSrc->pipe, &wbhicFIFO_PIPE.pipe);
			spTransfer (&pPipeSrc->pipe);
			if (spGetLenRecv (&wbhicFIFO_PIPE.pipe) != pPipeSrc->uLength)
				return FALSE;
		}
		else
		{
        	if (!wbhicPartWriteFIFO (&wbhicFIFO_PIPE.wbhicFIFO,
        							 (CONST UCHAR *) pParamPtr,
        							 uParamLen, NULL))
            	return FALSE;
        }
    }

	wbhicPipeWriteFIFO_Uninit (&wbhicFIFO_PIPE);

    return TRUE;
}



/****************************************************************************
 *
 * FUNCTION
 *		__wbhicCmd_V_Recv
 *
 * DESCRIPTION
 *		Receive Type V command's response.
 *
 * INPUTS
 *		uResponNum: Number of responses
 *		vaList:
 *				//Pointer of response1(VOID *)
 *				//Type or max length of response1(UINT32)
 *				//Real length of response1(UINT32 *)
 *				//Pointer of response2(VOID *)
 *				//Type or max length of response2(UINT32)
 *				//Real length of response1(UINT32 *)
 *				//...
 *
 * OUTPUTS
 *		None
 *
 * RETURN
 *		TRUE, success.
 *		FALSE, failed.
 *
 * REMARK
 *		None
 *
 ***************************************************************************/
static BOOL __wbhicCmd_V_Recv (UCHAR ucCommand,
							   UCHAR ucSubCommand,
							   UINT32 uResponNum,
							   va_list vaList)
{
	UCHAR pucBuf[4];
	UINT32 uResponLen;
	UINT32 u;
    WBHIC_FIFO_PIPE_T wbhicFIFO_PIPE;
    BOOL bOK;

    //Read length;
	if (!__wbhicCmd_Wait_DRQ (ucCommand, ucSubCommand))
        return FALSE;
    __wbhicCmd_Recv (&pucBuf[3],
    				 &pucBuf[2], &pucBuf[1], &pucBuf[0]);
	UCHAR_2_UINT32(pucBuf, uResponLen);

	//Begin to read FIFO.
	wbhicPipeReadFIFO_Init (&wbhicFIFO_PIPE, uResponLen);

	//Read and compare parameter number.
	bOK = wbhicPartReadFIFO (&wbhicFIFO_PIPE.wbhicFIFO,
							 pucBuf,
							 sizeof(pucBuf),
							 NULL);

	if (bOK)
	{
		UCHAR_2_UINT32(pucBuf, u);
		if (uResponNum == u)
		{
			for (u = 0; u < uResponNum; u++)
			{
				UINT32 uRealParamLen;
				UINT32 uSaveParamLen;
    	    	VOID *pParamPtr;
    		    UINT32 uParamType;
    		    UINT32 *puRealParamLen;
				UINT32 uParamLen;

				pParamPtr = va_arg (vaList, VOID *);
				uParamType = va_arg (vaList, UINT32);
				puRealParamLen = va_arg (vaList, UINT32 *);

				if (uParamType == WBHIC_FIFO_PIPE_IO)
				{
					WBHIC_FIXLEN_PIPE_T *pPipeTgt = (WBHIC_FIXLEN_PIPE_T *) pParamPtr;
					uParamLen = pPipeTgt->uLength;
				}
				else
					uParamLen = uParamType;

				/* Read parameter length. */
				bOK = wbhicPartReadFIFO (&wbhicFIFO_PIPE.wbhicFIFO,
										 pucBuf,
										 sizeof(pucBuf),
										 NULL);
				if (!bOK) break;

				//Save the real parameter length.
				UCHAR_2_UINT32(pucBuf, uRealParamLen);
				if (puRealParamLen != NULL)
					*puRealParamLen = uRealParamLen;

				//Calculate length of buffer to be saved.
				if (uParamLen < uRealParamLen)
					uSaveParamLen = uParamLen;
				else
					uSaveParamLen = uRealParamLen;

				if (uParamType == WBHIC_FIFO_PIPE_IO)
				{
					WBHIC_FIXLEN_PIPE_T *pPipeTgt = (WBHIC_FIXLEN_PIPE_T *) pParamPtr;
					wbhicPipeReadFIFO_SetLength (&wbhicFIFO_PIPE, uSaveParamLen);
					spConnect (&wbhicFIFO_PIPE.pipe, &pPipeTgt->pipe);
					spTransfer (&pPipeTgt->pipe);
					bOK = (spGetLenSent (&wbhicFIFO_PIPE.pipe) == uSaveParamLen ? TRUE : FALSE);
				}
				else
				{
					bOK = wbhicPartReadFIFO (&wbhicFIFO_PIPE.wbhicFIFO,
											 (UCHAR *)pParamPtr,
											 uSaveParamLen,
											 NULL);
				}
				if (!bOK) break;

				if (uRealParamLen > uSaveParamLen)
				{
					UINT32 uDropParamLen;
					uDropParamLen = uRealParamLen - uSaveParamLen;;
					while (uDropParamLen > 0 && bOK)
					{
						UCHAR uc;
						//Read but not save the data outof "pParamPtr".
						bOK = wbhicPartReadFIFO (&wbhicFIFO_PIPE.wbhicFIFO,
												 &uc,
												 sizeof (uc),
												 NULL);
						uDropParamLen--;
					}
				}
				if (!bOK) break;
			}
		}
	}

	if (bOK)
		wbhicPipeReadFIFO_Uninit (&wbhicFIFO_PIPE);

	return bOK;
}




/****************************************************************************
 *
 * FUNCTION
 *		wbhicCmd_IV
 *
 * DESCRIPTION
 *		Type IV command: Command Parameters in data port.
 *
 * INPUTS
 *		ucCommand: Command described by CF_REGF
 *		ucSubCommand: Sub command described by CF_REGE
 *		uParamNum: Number of parameters
 *		vaList:
 *				//Pointer of param1(VOID *)
 *				//Type or length of param1(UINT32)
 *				//Pointer of param2(VOID *)
 *				//Type or length of param2(UINT32)
 *				//...
 *
 * OUTPUTS
 *		None
 *
 * RETURN
 *		TRUE, success.
 *		FALSE, failed.
 *
 * REMARK
 *		None
 *
 ***************************************************************************/
BOOL wbhicCmd_IV (UCHAR ucCommand,      //CF_REGF
                  UCHAR ucSubCommand,   //CF_REGE
                  UINT32 uParamNum,	    //Number of parameters
                  ...					//Pointer of param1(VOID *)
										//Type or length of param1(UINT32)
										//Pointer of param2(VOID *)
										//Type or length of param2(UINT32)
										//...
    )
{
	BOOL bReturn;
    va_list vaList1;
    va_list vaList2;

	sysLockWbc ();
	bReturn = __wbhicCmd_Wait_NOBUSY (ucCommand, ucSubCommand, FALSE);
	if (!bReturn)
		goto lOut;

	/* Why use va_start twice?
		Because "__wbhicCmd_IV_or_V_Send" has to rewind stdargs,
		but not all platforms support to use "va_copy" to save the "va_list" handle.
	   It is not recommended to use memcpy instead of va_copy!
	 */
    va_start (vaList1, uParamNum);
    va_start (vaList2, uParamNum);
    bReturn = __wbhicCmd_IV_or_V_Send (ucCommand,
    								   ucSubCommand,
    								   uParamNum,
    								   vaList1,
    								   vaList2);
    va_end (vaList2);
    va_end (vaList1);

	if (!bReturn)
		goto lOut;

    bReturn = __wbhicCmd_Wait_NOBUSY (ucCommand, ucSubCommand, TRUE);

lOut:
	sysUnlockWbc ();
	return bReturn;
}


/****************************************************************************
 *
 * FUNCTION
 *		wbhicCmd_V
 *
 * DESCRIPTION
 *		Type V command: Command response in data port.
 *
 * INPUTS
 *		ucCommand: Command described by CF_REGF
 *		ucSubCommand: Sub command described by CF_REGE
 *		uParamNum: Number of parameters
 *		vaList:
 *				//Pointer of param1(VOID *)
 *				//Type or length of param1(UINT32)
 *				//Pointer of param2(VOID *)
 *				//Type or length of param2(UINT32)
 *				//...
 *
 * OUTPUTS
 *		vaList: (After parameters)
 *				//Number of responses(UINT32)
 *				//Pointer of response1(VOID *)
 *				//Type or max length of response1(UINT32)
 *				//Real length of response1(UINT32 *)
 *				//Pointer of response2(VOID *)
 *				//Type or max length of response2(UINT32)
 *				//Real length of response1(UINT32 *)
 *				//...
 *
 * RETURN
 *		TRUE, success.
 *		FALSE, failed.
 *
 * REMARK
 *		None
 *
 ***************************************************************************/
BOOL wbhicCmd_V (UCHAR ucCommand,	//CF_REGF
                 UCHAR ucSubCommand,//CF_REGE
	             UINT32 uParamNum,	//Number of parameters
       		     ...				//Pointer of param1(VOID *)
			                		//Type or length of param1(UINT32)
    			            		//Pointer of param2(VOID *)
        	    		    		//Type or length of param2(UINT32)
        	    		    		//...
        	    		    		//Number of responses.(UINT32)
        	    		    		//Pointer of response1(VOID *)
        	    		    		//Type or max length of response1(UINT32)
        	    		    		//Real length of response1(UINT32 *)
        	    		    		//Pointer of response2(VOID *)
        	    		    		//Type or max length of response2(UINT32)
        	    		    		//Real length of response2(UINT32 *)
        	    		    		//...
	)
{
	BOOL bReturn;
    va_list vaList1;
    va_list vaList2;
    UINT32 u;
    UINT32 uResponNum;

	sysLockWbc ();

	bReturn = __wbhicCmd_Wait_NOBUSY (ucCommand, ucSubCommand, FALSE);
	if (!bReturn)
		goto lOut;

    va_start (vaList1, uParamNum);
    va_start (vaList2, uParamNum);
    bReturn = __wbhicCmd_IV_or_V_Send (ucCommand,
    								   ucSubCommand,
    								   uParamNum,
    								   vaList1,
    								   vaList2);
    va_end (vaList2);
    va_end (vaList1);

	if (!bReturn)
		goto lOut;

    //Read response, relocate the vaList1 firstly.
    va_start (vaList1, uParamNum);
	for (u = 0; u < uParamNum; u++)
	{
   	    UINT32 uParamLen;
       	VOID *pParamPtr;

       	uParamLen = va_arg (vaList1, UINT32);
       	pParamPtr = va_arg (vaList1, VOID *);
	}
    uResponNum = va_arg(vaList1, UINT32);

	bReturn = __wbhicCmd_V_Recv (ucCommand, ucSubCommand, uResponNum, vaList1);
	va_end (vaList1);

	if (!bReturn)
		goto lOut;

	bReturn = __wbhicCmd_Wait_NOBUSY (ucCommand, ucSubCommand, TRUE);


lOut:
	sysUnlockWbc ();
	return bReturn;
}



