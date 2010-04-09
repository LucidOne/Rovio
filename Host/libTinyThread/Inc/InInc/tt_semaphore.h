#ifndef __TT_SEMAPHORE_H__
#define __TT_SEMAPHORE_H__

typedef struct
{
	cyg_mutex_t mutex;
	int lock_count;
	cyg_handle_t thread;
} TT_RMUTEX_T;	/* Recursive lock */


void tt_rmutex_init (TT_RMUTEX_T *rmutex);
void tt_rmutex_destroy (TT_RMUTEX_T *rmutex);
void tt_rmutex_lock (TT_RMUTEX_T *rmutex);
int tt_rmutex_can_lock__remove_it (TT_RMUTEX_T *rmutex);
int tt_rmutex_can_lock_in_irq (TT_RMUTEX_T *rmutex);
int tt_rmutex_try_lock_in_thd (TT_RMUTEX_T *rmutex);
int tt_rmutex_try_lock_in_dsr (TT_RMUTEX_T *rmutex);
void tt_rmutex_unlock (TT_RMUTEX_T *rmutex);



typedef struct
{
	cyg_cond_t cond;
	TT_RMUTEX_T *rmutex;
} TT_COND_T;

void tt_cond_init (TT_COND_T *cond, TT_RMUTEX_T *rmutex);
void tt_cond_destroy (TT_COND_T *cond);
int tt_cond_wait (TT_COND_T *cond);
int tt_cond_timed_wait (TT_COND_T *cond, cyg_tick_count_t abstime);
void tt_cond_signal (TT_COND_T *cond);
void tt_cond_broadcast (TT_COND_T *cond);



#endif
