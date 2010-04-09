#ifndef __COMMANDS_H__
#define __COMMANDS_H__



typedef struct
{
	UINT32 uValue;

	UCHAR aucBuffer[200];
	UINT32 uBufferLength;

	CHAR acFilePath[64];
	UINT32 uFileSize;
	
} HIC_PROCESS_DATA_T;

typedef struct
{
	HIC_PROCESS_DATA_T hData;

	MY_WAIT_OBJ_T cmd_wait_obj;
	UCHAR aucReg[6];
} HIC_COMMAND_T;


void hicOnCmd (HIC_COMMAND_T *pCmd);


#endif

