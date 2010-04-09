#ifdef RECORDER
#ifndef RECORDER_H
#define RECORDER_H
#include "mail.h"
#include "../Ftp/ftp.h"


void recorder_releative_thread_create(Ftp_Info* ftpinfo, Mail_Info* mailinfo);
void recorder_releative_thread_release(void);
BOOL recorder_add_video(char* videobuf, int videolen);
void recorder_add_audio(char* audiobuf, int audiolen);
int recorder_run(int* bheader, int video_width, int video_height); 
int recoreder_init_buf(int videotype, int audiotype);
void recorder_free(void);
void recorder_dis(void);
void recorder_memset(void);
#endif
#endif