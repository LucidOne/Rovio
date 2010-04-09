#ifndef __WEB_FILES_H__
#define __WEB_FILES_H__

typedef struct
{
	const char *pcFile;
	const char *pcFileName;
	size_t	szLength;
} WEB_FILE_T;

int Http_WebFile (HTTPCONNECTION hConnection, void *pParam);
void RegisterWebFiles (void);

#endif

