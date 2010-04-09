#ifndef STATUS_H
#define STATUS_H

int Config_GetStatus(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML);
void SetRunTimeState(HTTPCONNECTION hConnection);
int GetWebCamStateString(WEBCAMSTATE_P *pState, char *pcString);

#endif
