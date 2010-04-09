#ifndef __FLASH_FILE_H__
#define __FLASH_FILE_H__

#ifdef __cplusplus
extern "C" {
#endif


/*
Flash files structure
To search the files quickly, the filename must be sorted.
----------------------------------------------------------
4 bytes: total size including header
4 bytes: count of files

Header structure of file1
Header structure of file2
бн.
Header structure of file n

Content of file 1
Content of file 2
бн
Content of file n

4 bytes: Length of executables(Alignment 4 bytes)
4 bytes: Length of the whole firmware with flash files included
*/


typedef struct
{
	unsigned int	nOffset;	/* Offset to the header structure */
	unsigned int	nSize;		/* Size of file content */
	char			szName[128];
} FFILE_HEADER_T;

extern const unsigned int MAGIC_APPENDING;
const FFILE_HEADER_T *ffGetFile (const char *pcFileName);
const char *ffGetFilePtr (const FFILE_HEADER_T *pHeader);
size_t ffGetFileSize (const FFILE_HEADER_T *pHeader);
int Read_FlashFile(void);

#ifdef __cplusplus
}
#endif

#endif
