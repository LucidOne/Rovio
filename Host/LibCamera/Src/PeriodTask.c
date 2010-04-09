#include "../Inc/CommonDef.h"




static LIST_T		g_lPrdTaskHeader;
static LIST_T		g_lPrdTaskAdd;
static BOOL			g_bPrdExit;

/* Recursive mutex for cyg_cond_t */
static cyg_mutex_t	g_mPrdLock;
static int			g_nPrdLockCount;
static cyg_handle_t	g_hPrdLockThread;

static cyg_cond_t	g_cPrdWait;


static cyg_thread	g_thdPrd;	/* pthread_t for Period thread */
static cyg_handle_t	g_hPrd;
#pragma arm section zidata = "non_init"
static UINT32		g_auPrdStack[1024*16 / sizeof(UINT32)];
#pragma arm section zidata



static void prdRun(cyg_addrword_t pParam);
void prdInit()
{
	cyg_mutex_init(&g_mPrdLock);
	g_nPrdLockCount = 0;
	g_hPrdLockThread = NULL;
	
	cyg_cond_init(&g_cPrdWait, &g_mPrdLock);
	listInit(&g_lPrdTaskHeader);
	listInit(&g_lPrdTaskAdd);
	g_bPrdExit = FALSE;
	
	cyg_thread_create(9, &prdRun, NULL, "period", g_auPrdStack, sizeof(g_auPrdStack), &g_hPrd, &g_thdPrd);
	if (g_hPrd == NULL)
	{
		fprintf(stderr, "Period thread creation failed!\n");
		return;
	}
	cyg_thread_resume(g_hPrd);
}



void prdLock()
{
	if (g_hPrdLockThread != cyg_thread_self ())
	{
		cyg_mutex_lock (&g_mPrdLock);
		g_hPrdLockThread = cyg_thread_self ();
	}
	++g_nPrdLockCount;
}


void prdUnlock()
{
	if(g_nPrdLockCount > 0)
		--g_nPrdLockCount;
	else
	{
		diag_printf("*******lock != UNLONK**********\n");
		sysDisableIRQ(); while(1);
	}
	if (g_nPrdLockCount == 0)
	{
		g_hPrdLockThread = NULL;
		cyg_mutex_unlock (&g_mPrdLock);
		
		cyg_cond_signal(&g_cPrdWait);
	}
}


void prdBeforeCondWait()
{
	g_nPrdLockCount = 0;
	g_hPrdLockThread = NULL;
}

void prdAfterCondWait()
{
	g_nPrdLockCount = 1;
	g_hPrdLockThread = cyg_thread_self ();
}


void prdUninit()
{
	prdLock();

	g_bPrdExit = TRUE;

	prdUnlock();
	
	{
		cyg_thread_info thread_info;
		thread_join(&g_hPrd, &g_thdPrd, &thread_info);
	}
	cyg_cond_destroy(&g_cPrdWait);
	cyg_mutex_destroy(&g_mPrdLock);
}


void prdAddTask(PRD_TASK_T *pHandle, void (*fnTask)(void *pArg), cyg_tick_count_t tTimeout, void *pArg)
{
	if (pHandle == NULL	|| fnTask == NULL)
		return;
	
	prdLock();
		
	pHandle->fnTask		= fnTask;
	pHandle->pArg		= pArg;
	pHandle->tTimeout	= tTimeout;
	pHandle->tRemain	= (cyg_tick_count_t)-1;
	
	listInit(&pHandle->list);
	//listAttach(&pHandle->list, &g_lPrdTaskHeader);
	listAttach(&pHandle->list, &g_lPrdTaskAdd);
	
	prdUnlock();
}


void prdDelTask(PRD_TASK_T *pHandle)
{
	if (pHandle == NULL)
		return;
	
	prdLock();

	listDetach(&pHandle->list);
	
	prdUnlock();
}



void prdNothing(void *pArg)
{
}


static void prdRun(cyg_addrword_t pParam)
{
	cyg_tick_count_t tCurrent, tPeriod;
	
	cyg_tick_count_t tLastCheck = cyg_current_time();
	
	prdLock();
	while(!g_bPrdExit)
	{
		LIST_T		list;
		LIST_T		*pList;
		
		cyg_tick_count_t tMinRemain;

		/* Check current time. */
		tCurrent = cyg_current_time();
		tPeriod = tCurrent - tLastCheck;
		tLastCheck = tCurrent;

		/* Get the min remain time. */
		tMinRemain = (cyg_tick_count_t)-1;
		for (pList = g_lPrdTaskHeader.pNext; pList != &g_lPrdTaskHeader; pList = pList->pNext)
		{
			PRD_TASK_T	*pTask = GetParentAddr(pList, PRD_TASK_T, list);
			
			if (pTask->tRemain > pTask->tTimeout)
				pTask->tRemain = pTask->tTimeout;
				
			if (pTask->tRemain <= tPeriod)
				pTask->tRemain = 0;
			else
				pTask->tRemain -= tPeriod;


			if (tMinRemain > pTask->tRemain)
				tMinRemain = pTask->tRemain;
		}
		
		if (tMinRemain > 10000)
			tMinRemain = 10000;
			
		//diag_printf("Before wait: %d\n", (int) tMinRemain);
		
		prdBeforeCondWait();
		cyg_cond_timed_wait(&g_cPrdWait, tMinRemain + tLastCheck);
		prdAfterCondWait();

		//diag_printf("After wait: %d\n", (int) tMinRemain);
		
		/* Check current time. */
		tCurrent = cyg_current_time();
		tPeriod = tCurrent - tLastCheck;
		tLastCheck = tCurrent;
		
		listInit(&list);
		while ((pList = listGetNext(&g_lPrdTaskHeader)) != NULL)		
		{
			PRD_TASK_T	*pTask = GetParentAddr(pList, PRD_TASK_T, list);

			listDetach(pList);
			listAttach(pList, &list);
			
			if (pTask->tRemain > pTask->tTimeout)
				pTask->tRemain = pTask->tTimeout;

			if (pTask->tRemain <= tPeriod)
			{
				prdUnlock();
				(*pTask->fnTask)(pTask->pArg);
				prdLock();
				pTask->tRemain = pTask->tTimeout;
			}
			else
				pTask->tRemain -= tPeriod;
		}
		
		while ((pList = listGetNext(&list)) != NULL)
		{
			listDetach(pList);
			listAttach(pList, &g_lPrdTaskHeader);
		}
		
		while ((pList = listGetNext(&g_lPrdTaskAdd)) != NULL)
		{
			listDetach(pList);
			listAttach(pList, &g_lPrdTaskHeader);
		}
	}
	prdUnlock();
}




