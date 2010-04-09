#ifndef VERSION_H
#define VERSION_H

/* get the product name. */
char *WRGetProductName(void);

/* get version number */
void WRGetProductVersionNum(int *pnMajor, int *pnMinor, const char **ppcDate, const char **ppcTime);

/* get the version number in string. */
char *WRGetProductVersion(void);

/* do with the http request "/GetVer.cgi". */
int Config_GetVer(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML);
#endif
