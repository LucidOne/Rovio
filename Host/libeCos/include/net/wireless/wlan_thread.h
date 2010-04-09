/*
 * Copyright © Marvell International Ltd. and/or its affiliates, 2003-2006
 */

#ifndef	__WLAN_THREAD_H_
#define	__WLAN_THREAD_H_

#include	"kapi.h"

typedef struct
{
    cyg_handle_t task;
    //wait_queue_head_t waitQ;

	cyg_flag_t waitQ_flag_q;
    //pid_t pid;
    void *priv;
} wlan_thread;

static inline void
wlan_activate_thread(wlan_thread * thr)
{
        /** Record the thread pid */
   // thr->pid = current->pid;

        /** Initialize the wait queue */
//    init_waitqueue_head(&thr->waitQ);
	cyg_flag_init(&thr->waitQ_flag_q);
	
}

static inline void
wlan_deactivate_thread(wlan_thread * thr)
{
    ENTER();

    LEAVE();
}

static inline void
wlan_create_thread(int (*wlanfunc) (void *), wlan_thread * thr, char *name)
{
   // thr->task = kthread_run(wlanfunc, thr, "%s", name);
}

extern int thread_stop(cyg_handle_t k);
static inline int
wlan_terminate_thread(wlan_thread * thr)
{
    ENTER();
#if 0
    /* Check if the thread is active or not */
    if (!thr->pid) {
        diag_printf("Thread does not exist\n");
        return -1;
    }
#endif
    //kthread_stop(thr->task);
    thread_stop(thr->task);
    
//    cyg_thread_delete(thr->task);

    LEAVE();
    return 0;
}

#endif
