#ifndef RTSPSERVER_H
#define RTSPSERVER_H

//#include "librtspserver.h"
#include "../../LibCamera/Inc/CommonDef.h"

void set_auth_enable(void);
void set_auth_disable(void);
struct rtsp_auth *set_auth_id(const char *username, const char *password);
void del_auth_id(const char *username/*, char *password*/);
void RtspThreadRun(W99702_DATA_EXCHANGE_T* g_dataW99702, CMD_AUDIO_FORMAT_E audiotype, CMD_VIDEO_FORMAT_E videotype);
void RtspThreadRelease(void);

int Http_RtspTunnel(HTTPCONNECTION hConnection, void *pParam);

#endif
