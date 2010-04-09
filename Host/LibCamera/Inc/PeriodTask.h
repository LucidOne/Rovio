#ifndef INC__PERIOD_TASK_H__
#define INC__PERIOD_TASK_H__



typedef struct tagPRD_TASK
{
	void (*fnTask)(void *pArg);
	void *pArg;
	cyg_tick_count_t tTimeout;
	cyg_tick_count_t tRemain;
	LIST_T list;
} PRD_TASK_T;



void prdInit(void);
void prdUninit(void);

void prdLock(void);
void prdUnlock(void);
void prdNothing(void *pArg);
void prdAddTask(PRD_TASK_T *pHandle, void (*fnTask)(void *pArg), cyg_tick_count_t tTimeout, void *pArg);
void prdDelTask(PRD_TASK_T *pHandle);

#endif

