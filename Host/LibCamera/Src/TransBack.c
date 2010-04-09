#include "../Inc/CommonDef.h"
int Http_GetAudio(HTTPCONNECTION hConnection,
								int *piPostState,
								char **ppcPostBuf,
								int *piPostBufLen,
								int *piPostDataLen,
								char *pcFillData,
								int iFillDataLen,
								int iIsMoreData/*bool*/,
								void *pParam/*other parameter for extend use*/)
{
	IO_THREAD_READ_T pArg;
	char *senbuf = pcFillData;
	pArg.mediatype = MEDIA_TYPE_AUDIO;
	pArg.txbuf = (unsigned char*)pcFillData;
	pArg.txlen = iFillDataLen;
	//diag_printf("receive audio length: %d\n", iFillDataLen);
	iothread_Write(&pArg);
	/*
	while((senbuf + 320) <=(pcFillData+iFillDataLen)) 
	{
		pArg.txbuf = (unsigned char *)senbuf;
		pArg.txlen = 320;	
		iothread_Write(&pArg);
		senbuf += 320;
	}
	if(senbuf < (pcFillData+iFillDataLen))
	{
		pArg.txbuf = (unsigned char *)senbuf;
		pArg.txlen = pcFillData+iFillDataLen-senbuf;	
		iothread_Write(&pArg);
		
	}
	*/
	return 1;
}