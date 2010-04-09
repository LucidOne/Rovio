#ifndef __VP_THREAD_VCOM_H__
#define __VP_THREAD_VCOM_H__



typedef int (*func_t)(int argc, char**argv);
typedef struct CMD_FUNC
{
    char *pcmd;
    func_t func;
}CMD_FUNC;


void vcomInitThread(void);


#endif
