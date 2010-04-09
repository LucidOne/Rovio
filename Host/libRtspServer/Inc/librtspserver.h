/*
NS25 JFYan
**********/
#ifndef _RTSPSERVER_H
#define _RTSPSERVER_H
        
#include "stdio.h"
#include "unistd.h"
#include "stdlib.h"
//#include <errno.h>

struct rtsp_auth {
        struct rtsp_auth *next;
        struct rtsp_auth *prev;
        char username[128];
        char password[128];
};

typedef void (*Get_Frame_Func)(unsigned char** input_buf,int *input_len);
typedef void (*Init_Device_Func)(void);
typedef void (*RTSP_CALLBACK_PFUN)(void);
typedef void (*rtp_media_unref_buf_func)(void *d);

typedef void (*Swap_HttpTunnel_FD)(void *tunnel_info, int *swap_fd);
typedef void (*Swap_HttpTunnel_FD_End)(void *tunnel_info);
void Set_Swap_FD_Func(Swap_HttpTunnel_FD func, Swap_HttpTunnel_FD_End func_end);


int get_rtspmem_size(int count);
void rtsp_mem_init(char *buf, int buflen);
int get_server_size(void);
void rtsp_server_init(char *buf, int buflen);
void init_get_video(Get_Frame_Func func);
void init_get_audio(Get_Frame_Func func);
void init_wbdevice(Init_Device_Func func);
void set_encoderenable(Init_Device_Func func);
void set_encoderdisable(Init_Device_Func func);
void set_unref_buf_video(rtp_media_unref_buf_func func);
void set_unref_buf_audio(rtp_media_unref_buf_func func);
void set_auth_enable(void);
void set_auth_disable(void);
struct rtsp_auth *set_auth_id(const char *username, const char *password);
void del_auth_id(const char *username/*, char *password*/);
int RtspServerStart(int DbMode,RTSP_CALLBACK_PFUN pFun);
void set_config(const char*);
void exit_event_loop(void);
#endif
