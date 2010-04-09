#include "stdio.h"
#include "stdlib.h"
#include "wblib.h"
#include "wbspi.h"
#include "flash_file.h"


char *FILE_BUF;
const unsigned int MAGIC_APPENDING = 0x4150464C;

static int ffSearch (const void *p1st, const void *p2nd)
{
	FFILE_HEADER_T *p1stHeader = (FFILE_HEADER_T *)p1st;
	FFILE_HEADER_T *p2ndHeader = (FFILE_HEADER_T *)p2nd;
	return strcmp (p1stHeader->szName, p2ndHeader->szName);
}

int Read_FlashFile(void)
{
	unsigned int totallen = 0;  //FW(bin+file+8)+16 bytes header
	unsigned int binlen = 0;    //bin length
	unsigned int filelen = 0;   //file length
	char end[4];
	char endflags[] = {'W','B',0x5A,0xA5};
	usiMyRead(4,4,(UINT8*)&totallen);
	diag_printf("totallen is %d\n",totallen);
	usiMyRead(totallen - 12,4,(UINT8*)end);
	{
		int i =0 ;
		for(;i <4;i++)
			diag_printf("  %02x  ",end[i]);
		diag_printf("\n");
	}
	if (memcmp(end,endflags,sizeof(endflags)) != 0)
	{
		diag_printf("Have none flash files\n");
		return -1;
	}	
	usiMyRead(totallen - 8,4,(UINT8*)&binlen);
	diag_printf("binlen is %d\n",binlen);
	filelen = totallen -16 - binlen -12; //
	diag_printf("filelen is %d\n",filelen);
	FILE_BUF = (char*)malloc(filelen);
	if(FILE_BUF == NULL)
	{
		diag_printf("****malloc error");
		return -1;
	}
	usiMyRead(binlen+16,filelen,(UINT8*)FILE_BUF);	
	{
		int i = 0;
		for(;i <4;i++)
			diag_printf("  %02x  ",FILE_BUF[i]);
		diag_printf("\n");
	}
	return 0;
}

/* Search the files */
const FFILE_HEADER_T *ffGetFile (const char *pcFileName)
{
	unsigned int nCount;
	const FFILE_HEADER_T *pHeader;
	const FFILE_HEADER_T *pKey = (const FFILE_HEADER_T *)
		(pcFileName - (size_t) &(((FFILE_HEADER_T *)0)->szName));
		
	if (FILE_BUF == NULL)
		return NULL;
	if (memcmp (&MAGIC_APPENDING, FILE_BUF, sizeof(MAGIC_APPENDING)) != 0)
		return NULL;	/* Magic not found */

	memcpy (&nCount, FILE_BUF + sizeof(MAGIC_APPENDING), sizeof (nCount));
	pHeader = (const FFILE_HEADER_T *) bsearch (
		pKey,
		FILE_BUF + sizeof(MAGIC_APPENDING) + sizeof (nCount),
		(size_t) nCount,
		sizeof (FFILE_HEADER_T),
		&ffSearch
		);
	return pHeader;
}


/* Get the content pointer of a file */
const char *ffGetFilePtr (const FFILE_HEADER_T *pHeader)
{
	if (FILE_BUF == NULL)
		return NULL;

	if (pHeader == NULL)
		return NULL;
	else
		return (char *)pHeader + pHeader->nOffset;
}

/* Get the length of a file */
size_t ffGetFileSize (const FFILE_HEADER_T *pHeader)
{
	if (FILE_BUF == NULL)
		return NULL;

	if (pHeader == NULL)
		return 0;
	else
		return (size_t)pHeader->nSize;
}




